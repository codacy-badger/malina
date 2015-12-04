#include "serialib/serialib.h"              //brak
//#include "wiadomosc/wiadomosc..h"
//#include "c_connection/c_connection.h"
#include "functions/functions.h"            // brak
//#include "logger/logger.hpp"
//#include "c_master_irda/master_irda.h"
//#include "functions/mpd_cli...h"
//#include "blockQueue/blockqueue..h"
#include "parser/parser.hpp"

#include <wiringPi.h>
const int jeden =1;
char *  _logfile  = "/tmp/iDom_log.log";
std::string buffer ;
Logger log_file_mutex(_logfile);
int max_msg = MAX_MSG_LEN*sizeof(int32_t);
bool go_while = true;

//////////// watek wysylajacy/obdbierajacy dane z portu RS232 ////////
void *Send_Recieve_rs232_thread (void *przekaz){
    thread_data_rs232 *data_rs232;

    data_rs232 = (thread_data_rs232*)przekaz;
   // serialib port_arduino;   // obiekt port rs232
    //Serial.begin(9600);
    SerialPi serial_ardu(strdup( data_rs232->portRS232.c_str()));

    serial_ardu.begin( atoi( data_rs232->BaudRate.c_str()));
    std::string znak;

    log_file_mutex.mutex_lock();
    log_file_cout << INFO <<"otwarcie portu RS232 " <<  data_rs232->portRS232 << data_rs232->BaudRate<<std::endl;
    //log_file_cout << INFO <<"w buforze jest bajtow " << port_arduino.Peek() << std::endl;
    log_file_mutex.mutex_unlock();
    std::cout << "";

    while (go_while)
    {
        usleep(500);
        pthread_mutex_lock(&C_connection::mutex_who);
        if (data_rs232->pointer.ptr_who[0] == RS232)
        {
            pthread_mutex_lock(&C_connection::mutex_buf);

            data_rs232->pointer.ptr_who[0] = data_rs232->pointer.ptr_who[1];
            data_rs232->pointer.ptr_who[1]= RS232;
            serial_ardu.write(buffer.c_str());
            delay (1000);
            if (serial_ardu.available() > 0)
            {
               buffer.erase();
                while (serial_ardu.available() > 0){

                    buffer+=serial_ardu.read();

                }
            }
            std::cout << " odebralem: " << buffer << std::endl;

            pthread_mutex_unlock(&C_connection::mutex_buf);
        }
        pthread_mutex_unlock(&C_connection::mutex_who);


    }

    pthread_exit(NULL);
}
//////////// watek do obslugi polaczeni miiedzy nodami  //////////////

void *f_serv_con_node (void *data){
    thread_data  *my_data;
    my_data = (thread_data*)data;
    std::cout<<"start watek master \n";
    pthread_detach( pthread_self () );





    std::cout<<"koniec  watek master \n";

    pthread_exit(NULL);
} //  koniec f_serv_con_node
/////////////////////  watek do obslugi irda //////////////////////////////

void *f_master_irda (void *data){
    thread_data  *my_data;
    my_data = (thread_data*)data;
    std::cout<<"start watek irda master 22222222 \n";
    pthread_detach( pthread_self () );

     master_irda irda(my_data);
    irda.run();

    std::cout<<"koniec  watek master \n";

    pthread_exit(NULL);
} //  koniec master_irda
///////////  watek wymiany polaczenia /////////////////////

void *Server_connectivity_thread(void *przekaz){
    thread_data  *my_data;
    my_data = (thread_data*)przekaz;

    pthread_detach( pthread_self () );

    // std::cout << " przed while  soket " <<my_data->s_v_socket_klienta << std::endl;

    C_connection *client = new C_connection( my_data);
      digitalWrite(LED7,1);
      my_data->mainLCD->set_print_song_state(3200);
      my_data->mainLCD->printString(true,0,0,"TRYB SERWISOWY!");
      my_data->mainLCD->printString(false,0,1,  inet_ntoa( my_data->from.sin_addr)   );
    while (1)
    {
        if( client->c_recv(0) == -1 )
        {

            break;
        }

       // ###########################  analia wiadomoscu ####################################//
        if ( client->c_analyse() == false )   // stop runing idom_server
        {
            client->c_send("exit");
            my_data->mainLCD->set_print_song_state(0);
            my_data->mainLCD->set_lcd_STATE(2);
            my_data->mainLCD->clear();
            my_data->mainLCD->noBacklight();
            sleep (3);
            go_while = false;
            break;
        }



        // ###############################  koniec analizy   wysylanie wyniku do RS232 lub  TCP ########################

        if( client->c_send(0 ) == -1 )
        {
            perror( "send() ERROR" );
            break;
        }
    }
    digitalWrite(LED7,0);
     my_data->mainLCD->set_print_song_state(0);
     my_data->mainLCD->set_lcd_STATE(2);
     sleep (3);
    delete client;
    pthread_exit(NULL);

} ////////////  server_connectivity_thread

void *main_thread( void * unused)
{
    time_t czas;
    struct tm * ptr;
    time( & czas );
    ptr = localtime( & czas );
    std::string data = asctime( ptr );
    config server_settings   =  read_config ( "/etc/config/iDom_SERVER/iDom_server"    );     // strukruta z informacjami z pliku konfigu
    struct sockaddr_in server;
    int v_socket;
    int max_msg = MAX_MSG_LEN*sizeof(int32_t);
    //thread_data m_data;
    thread_data node_data; // przekazywanie do watku
    pthread_t thread_array[MAX_CONNECTION]={0,0,0,0,0,0,0,0,0,0};
    pthread_t rs232_thread_id;
    unsigned int who[2]={FREE, FREE};
    int32_t bufor[ MAX_MSG_LEN ];

     
	///////////////////////////////////////////  zaczynam wpisy do logu ////////////////////////////////////////////////////////////
    log_file_mutex.mutex_lock();
    log_file_cout << "\n*****************************************************************\n*****************************************************************\n  "<<  " \t\t\t\t\t start programu " << std::endl;
    log_file_cout << INFO  << "ID serwera\t"<< server_settings.ID_server << std::endl;
    log_file_cout << INFO  << "PortRS232\t"<< server_settings.portRS232 << std::endl;
    log_file_cout << INFO  << "BaudRate RS232\t"<< server_settings.BaudRate << std::endl;
    log_file_cout << INFO  << "port TCP \t"<< server_settings.PORT << std::endl;
    log_file_cout << INFO  << "serwer ip \t"<< server_settings.SERVER_IP  <<std::endl;
    log_file_cout << INFO  << "dodatkowe NODY w sieci:\n"  <<std::endl;
 
    for (u_int i=0;i<server_settings.AAS.size();++i){
        log_file_cout << INFO << server_settings.AAS[i].id<<" "<< server_settings.AAS[i].SERVER_IP <<std::endl;
    }
    log_file_cout << INFO  << "baza z filami \t"<< server_settings.MOVIES_DB_PATH << std::endl;
    log_file_cout << INFO << " \n" << std::endl;
	log_file_cout << INFO << "------------------------ START PROGRAMU -----------------------"<< std::endl;
    log_file_mutex.mutex_unlock();

    ///////////////////////////////////////////////  koniec logowania do poliku  ///////////////////////////////////////////////////
    
	///////////////////////////////////////////////  start wiringPi  //////////////////////////////////////////////
	if (wiringPiSetup () == -1)
        exit (1) ;

      pinMode(LED7, OUTPUT); 		// LED  na wyjscie  GPIO
      pinMode(GPIO_SPIK, OUTPUT);    // gpio pin do zasilania glosnikow
      pinMode(BUTTON_PIN, INPUT);   //  gpio pin przycisku
      /////////////////////////////// LCD ///////////////////////////////
      LCD_c mainLCD(0x27,16,2);
      //////////////     przegladanie plikow ////////////////////

      files_tree main_tree( server_settings.MOVIES_DB_PATH, &mainLCD);

      ///////////////////////////////// MENU /////////////////////////////////

      menu_tree main_MENU("/etc/config/iDom_SERVER/MENU", &mainLCD);
     /////////////////////////////////////////////////   wypelniam  struktury przesylane do watkow  ////////////////////////
    thread_data_rs232 data_rs232;
    data_rs232.BaudRate=server_settings.BaudRate;
    data_rs232.portRS232=server_settings.portRS232;
    data_rs232.pointer.ptr_buf=bufor;
    data_rs232.pointer.ptr_who=who;
    pthread_create(&rs232_thread_id,NULL,&Send_Recieve_rs232_thread,&data_rs232 );    ///  start watku do komunikacji rs232
   
    /////////////////////////////////  tworzenie pliku mkfifo  dla sterowania omx playerem
  
    int temp;
    temp = mkfifo("/tmp/cmd",0666);
     
   if (temp == 0 )
    {
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "mkfifo - plik stworzony" << std::endl;
        log_file_mutex.mutex_unlock();
    }
    else if (temp == -1)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR << "mkfifo - "<<strerror(  errno ) << std::endl;
        log_file_mutex.mutex_unlock();
    }

    //////////////////////////////////////////////////////////////////////



    int SERVER_PORT = atoi(server_settings.PORT.c_str());
    server_settings.SERVER_IP = conv_dns(server_settings.SERVER_IP);
    const char *SERVER_IP = server_settings.SERVER_IP.c_str();

    node_data.server_settings=&server_settings;
    node_data.pointer.ptr_buf=bufor;
    node_data.pointer.ptr_who=who;
    node_data.mainLCD=&mainLCD;
    node_data.main_tree=&main_tree;
    node_data.main_MENU=&main_MENU;
    // start watku irda
    pthread_create (&thread_array[4], NULL,&f_master_irda ,&node_data);
    log_file_mutex.mutex_lock();
    log_file_cout << INFO << " watek wystartowal polaczenie irda "<< thread_array[4] << std::endl;
    log_file_mutex.mutex_unlock();
    pthread_detach( thread_array[4] );
    // start watku  mpd_cli
    pthread_create (&thread_array[5], NULL,&main_mpd_cli ,&node_data);
    log_file_mutex.mutex_lock();
    log_file_cout << INFO << " watek wystartowal klient mpd "<< thread_array[5] << std::endl;
    log_file_mutex.mutex_unlock();
    pthread_detach( thread_array[5] );

    if (server_settings.ID_server == 1001){    ///  jesli  id 1001  to wystartuj watek do polaczeni z innym nodem masterem 
      
        pthread_create (&thread_array[3], NULL,&f_serv_con_node ,&node_data);
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << " watek wystartowal dla NODA MASTERA "<< thread_array[3] << std::endl;
        log_file_mutex.mutex_unlock();
        pthread_detach( thread_array[3] );
    }

    else
    {
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "NIE startuje NODA MASTERA do polaczen z innymi " << std::endl;
        log_file_mutex.mutex_unlock();

    }

    bzero( & server, sizeof( server ) );

    server.sin_family = AF_INET;
    server.sin_port = htons( SERVER_PORT );
    if( inet_pton( AF_INET, SERVER_IP, & server.sin_addr ) <= 0 )
    {
        perror( "inet_pton() ERROR" );
        exit( - 1 );
    }

    if(( v_socket = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror( "socket() ERROR" );
        exit( - 1 );
    }

    if( fcntl( v_socket, F_SETFL, O_NONBLOCK ) < 0 ) // fcntl()
    {
        perror( "fcntl() ERROR" );
        exit( - 1 );
    }
    // zgub wkurzający komunikat błędu "address already in use"
    int yes =1;
    if( setsockopt( v_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) == - 1 ) {
        perror( "setsockopt" );
        exit( 1 );
    }
    socklen_t len = sizeof( server );
    if( bind( v_socket,( struct sockaddr * ) & server, len ) < 0 )
    {

        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << "BIND problem: " <<  strerror(  errno )<< std::endl;
        log_file_mutex.mutex_unlock();
        perror( "bind() ERROR" );
        exit( - 1 );
    }

    if( listen( v_socket, MAX_CONNECTION ) < 0 )
    {
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << "Listen problem: " <<  strerror(  errno )<< std::endl;
        log_file_mutex.mutex_unlock();
        perror( "listen() ERROR" );
        exit( - 1 );
    }
    //std::cout <<" przed while \n";

    struct sockaddr_in from;
    int v_sock_ind = 0;

    ///////////////////////////////////////////////////// WHILE ////////////////////////////////////////////////////

    while (1)
    {

        bzero( & from, sizeof( from ) );
        if (!go_while) {break;}

        usleep(100000);

        if(( v_sock_ind = accept( v_socket,( struct sockaddr * ) & from, & len ) ) < 0 )
        {
            continue;
        }

        ////////////////////////   jest połacznie   wiec wstawiamy je  do nowego watku  i  umieszczamy id watku w tablicy  w pierwszym wolnym miejscy ////////////////////
        for (int con_counter=0; con_counter< MAX_CONNECTION; ++con_counter)
        {
            if ( thread_array[con_counter]==0 || pthread_kill(thread_array[con_counter], 0) == ESRCH )   // jesli pozycja jest wolna (0)  to wstaw tam  jesli jest zjęta wyslij sygnal i sprawdz czy waŧek żyje ///

            {
                if ( con_counter!=MAX_CONNECTION -1)
                {

                    node_data.s_client_sock =v_sock_ind;
                    node_data.from=from;
                    
                    pthread_create (&thread_array[con_counter], NULL,&Server_connectivity_thread,&node_data);

                    log_file_mutex.mutex_lock();
                    log_file_cout << INFO << " watek wystartowal  "<< thread_array[con_counter] << std::endl;
                    log_file_mutex.mutex_unlock();

                    pthread_detach( thread_array[con_counter] );
                    break;

                }
                else
                {

                    log_file_mutex.mutex_lock();
                    log_file_cout << INFO << " za duzo klientow "<< thread_array[con_counter] << std::endl;
                    log_file_mutex.mutex_unlock();

                    int32_t bufor_tmp[ MAX_MSG_LEN ];


                    if(( recv( v_sock_ind, bufor_tmp, max_msg, 0 ) ) <= 0 )
                    {
                        perror( "recv() ERROR" );
                        break;
                    }
                    for (int i =0 ; i < MAX_MSG_LEN ; ++i )
                    {
                        bufor_tmp[i]= -123 ;

                    }

                    if(( send( v_sock_ind, bufor_tmp, max_msg, MSG_DONTWAIT ) ) <= 0 )
                    {
                        perror( "send() ERROR" );
                        break;
                    }
                    shutdown( v_sock_ind, SHUT_RDWR );
                    continue;

                }

            }
 
        }
    } // while

    log_file_mutex.mutex_lock();
    log_file_cout << INFO << " koniec programu  "<<   std::endl;
    log_file_mutex.mutex_unlock();

    sleep(3);
   
    pthread_exit(NULL);
} // threade

int main()
{
    pthread_mutex_init(&C_connection::mutex_buf, NULL);
    pthread_mutex_init(&C_connection::mutex_who, NULL);
    pthread_mutex_init(&blockQueue::mutex_queue_char, NULL);
    pthread_mutex_init(&Logger::mutex_log, NULL);
    pthread_t  main_th;
    pthread_create (&main_th, NULL,&main_thread,NULL);

    pthread_join(main_th ,NULL);
    pthread_mutex_destroy(&Logger::mutex_log);
    pthread_mutex_destroy(&C_connection::mutex_buf);
    pthread_mutex_destroy(&C_connection::mutex_who);
    pthread_mutex_destroy(&blockQueue::mutex_queue_char);

    return 0;
}
 
