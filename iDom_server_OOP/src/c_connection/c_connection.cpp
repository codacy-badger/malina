#include "c_connection.h"
#include <iostream>
//us



pthread_mutex_t C_connection::mutex_buf = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t C_connection::mutex_who = PTHREAD_MUTEX_INITIALIZER;

// konstruktor
C_connection::C_connection (thread_data  *my_data):c_socket(my_data->s_client_sock),
    c_from(my_data->from),mainCommandHandler(my_data)
{
    this -> pointer = &my_data->pointer;
    this -> my_data = my_data;
    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "konstruuje nowy obiekt do komunikacj na gniezdzie " << c_socket <<  std::endl;
    log_file_mutex.mutex_unlock();
}


// destruktor
C_connection::~C_connection()
{
    shutdown( c_socket, SHUT_RDWR );
    for (int i =0 ; i< MAX_CONNECTION;++i)
    {
        if (my_data->main_THREAD_arr[i].thread_ID == pthread_self())
        {
            my_data->main_THREAD_arr[i].thread_ID = 0;
            my_data->main_THREAD_arr[i].thread_name ="  -empty-  ";
            my_data->main_THREAD_arr[i].thread_socket=0;
            break;
        }
    }
    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "koniec komunikacji - kasuje obiekt" <<  std::endl;
    log_file_mutex.mutex_unlock();
}


int C_connection::c_send(int para)
{
    std::string  len = std::to_string( str_buf.length());

    if(( send( c_socket, len.c_str() ,len.length(), para ) ) <= 0 )
    {
        return -1;
    }
    recv_size = recv( c_socket, c_buffer , MAX_buf, para );

    if(recv_size < 0 )
    {
        //perror( "recv() ERROR" );
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR << "recv() error - " << strerror(  errno ) <<   std::endl;
        log_file_mutex.mutex_unlock();
        return -1;
    }
    else if (recv_size == 0)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "gniazdo zamkniete" <<   std::endl;
        log_file_mutex.mutex_unlock();
        return -1;
    }

    unsigned int len_send = str_buf.length();
    unsigned int len_temp =0;

    while (len_send > 0)
    {
        len_temp = send( c_socket, str_buf.c_str() ,str_buf.length(), para ) ;
        std::cout <<"wyslalem bajtow: " << len_temp <<std::endl;
        if(len_temp  <= 0 )
        {
            return -1;
        }
        len_send -= len_temp;

        std::cout << "zostalo wyslac: " << len_send <<std::endl;

        str_buf.erase(0,len_temp);

    }




return 0;
}

int C_connection::c_send(std::string command )
{
    str_buf = command;
    c_send(0);
    return 0;
}

int C_connection::c_recv(int para)
{
    //std::cout <<"RECV" <<std::endl;
    for (unsigned int i =0 ; i< sizeof(c_buffer);++i)
    {
        c_buffer[i]=',';
    }

    struct timeval tv;
    tv.tv_sec = 90;
    tv.tv_usec = 0;
    setsockopt(c_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&tv , sizeof(struct timeval));

    recv_size = recv( c_socket, c_buffer , MAX_buf, para );

    if(recv_size < 0 )
    {
        //perror( "recv() ERROR" );
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR << "recv() error - " << strerror(  errno ) <<   std::endl;
        log_file_mutex.mutex_unlock();
        return -1;
    }
    else if (recv_size == 0)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "gniazdo zamkniete" <<   std::endl;
        log_file_mutex.mutex_unlock();
        return -1;
    }

    //std::cout << "giazdo odebralo bajtow: " <<recv_size    <<std::endl;




    return 1;
}






int C_connection::c_analyse()
{
    std::string buf;
    for(char n : c_buffer){
        if (n!=','){

            buf+=n;
        }
    }

    std::vector <std::string> command;

    tokenizer(command," \n,", buf);  // podzia  zdania na wyrazy
    str_buf=command[command.size()-1];

    //std::cout << "D: "<<RSHash( )<<std::endl;
    str_buf= "unknown command\n";
    for(std::string  t : command)
    {
        str_buf+=t+" ";
    }
    std::cout << "bufcio: " << str_buf<< std::endl;
    switch (command.size())
    {
    case 1 :

        {
            str_buf  = mainCommandHandler.run(command,my_data);
            break;
        }
    case 2 :
        if(command[0]=="stop")
        {
            //std::cout << " jest stop";
            if (command[1]=="server")
            {
                //std::cout <<"wychodze!!!!";
                c_send("\nCLOSE.\n");

                return false;
            }
            else
            {   str_buf="stop what? \n";
                break;
            }
        }



        else if (command[0]=="show")
        {
            if (command[1]=="log")
            {
                //l_send_file(_logfile,"");

                break;
            }
            else if (command[1]=="thread")
            {
                str_buf = " No ID";
                break;
            }

        }


    case 3:

        if (command[0]=="show")
        {
            if (command[1]=="log")
            {
                l_send_file(_logfile,command[2],true);

                break;
            }
            else if (command[1]=="thread")
            {
                if (command [2] !="all"){
                    str_buf  = my_data->main_THREAD_arr[std::stoi(command[2])].thread_name;
                    str_buf  += " ID: ";
                    str_buf += std::to_string(my_data->main_THREAD_arr[std::stoi(command[2])].thread_ID);
                    str_buf  += " socket: ";
                    str_buf  += std::to_string(my_data->main_THREAD_arr[std::stoi(command[2])].thread_socket);

                    break;
                }

                else {
                    str_buf.erase();
                    for (int i =0 ; i< MAX_CONNECTION;++i)
                    {
                        str_buf  += std::to_string(i)+"\t";
                        str_buf  += my_data->main_THREAD_arr[i].thread_name;
                        str_buf  += "\t ID: ";
                        str_buf  += std::to_string(my_data->main_THREAD_arr[i].thread_ID);

                        if (my_data->main_THREAD_arr[i].thread_socket !=0){
                            str_buf  += " socket: ";
                            str_buf  += std::to_string(my_data->main_THREAD_arr[i].thread_socket);
                        }

                        str_buf  += "\n";


                    }


                    break;
                }
            }

        }

        else
        {
            str_buf  = mainCommandHandler.run(command,my_data);
            break;
        }
    case 4:
        if (command[0]=="show")
        {
            if (command[1]=="log" &&  command [2] =="no")
            {
                l_send_file(_logfile,command[3],false);

                break;
            }
        }
    case 5:

        str_buf = mainCommandHandler.run(command, my_data);
        break;
    default :
        //std::cout << " nic nie przyszlo komenda z dupy " << c_buffer<<std::endl;
        str_buf +="\n";

    }


    return true;
}




