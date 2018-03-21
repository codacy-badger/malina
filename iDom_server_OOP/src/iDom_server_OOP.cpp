#include "iDom_server_OOP.h"
#include "functions/functions.h"            // brak
#include <wiringPi.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "c_connection/c_connection.h"
#include "TASKER/tasker.h"
#include "c_master_irda/master_irda.h"
#include "RADIO_433_eq//radio_433_eq.h"
#include "thread_functions/rs232_thread.h"
#include "433MHz/RFLink/rflinkhandler.h"
#include "command/commandClass/command_ardu.h"

std::string  _logfile = "/mnt/ramdisk/iDom_log.log";
std::string buffer ;
Logger log_file_mutex(_logfile);

////////////// watek wysylajacy/obdbierajacy dane z portu RS232 ////////
//void Send_Recieve_rs232_thread (thread_data_rs232 *data_rs232){

//    SerialPi serial_ardu(strdup( data_rs232->portRS232.c_str()));
//    serial_ardu.begin( std::stoi( data_rs232->BaudRate));
//#ifndef BT_TEST
//    log_file_mutex.mutex_lock();
//    log_file_cout << INFO <<"otwarcie portu RS232 " <<  data_rs232->portRS232 << "  " <<data_rs232->BaudRate<<std::endl;
//    log_file_mutex.mutex_unlock();
//#endif
//    SerialPi serial_ardu_clock(strdup( data_rs232->portRS232_clock.c_str()));
//    serial_ardu_clock.begin( std::stoi( data_rs232->BaudRate));
//#ifndef BT_TEST
//    log_file_mutex.mutex_lock();
//    log_file_cout << INFO <<"otwarcie portu RS232_clock " <<  data_rs232->portRS232_clock <<" "<< data_rs232->BaudRate <<std::endl;
//    log_file_mutex.mutex_unlock();
//#endif
//    /////////////////////////////////////////////////// RESET ARDUINO AFTER RESTART ////////////////////////////////
//    puts("restart arduino\n");
//    //C_connection::mutex_who.lock();
//    {
//        std::lock_guard<std::mutex> lockWho(useful_F::mutex_who);
//        buffer = "reset:00;";
//        serial_ardu.print(buffer.c_str());
//    }
//    //C_connection::mutex_who.unlock();
//    puts("test testo po lock");
//    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    while(useful_F::go_while)
//    {
//        //puts("test");
//        std::this_thread::sleep_for( std::chrono::milliseconds(50));
//        //puts("test testo po lock");
//        { //mutex who
//            std::lock_guard<std::mutex> lockWho(useful_F::mutex_who);

//            if(data_rs232->pointer.ptr_who[0] == iDomConst::RS232)
//            {
//                std::lock_guard<std::mutex> lockBuf(useful_F::mutex_buf);
//                data_rs232->pointer.ptr_who[0] = data_rs232->pointer.ptr_who[1];
//                data_rs232->pointer.ptr_who[1] = iDomConst::RS232;
//                serial_ardu.print(buffer.c_str());

//                buffer.erase();

//                while(useful_F::go_while){
//                    if(serial_ardu.available()>0){
//                        buffer += serial_ardu.read();
//                    }
//                    if(buffer[buffer.size()-1] == ';')
//                    {
//                        buffer.erase(buffer.end()-1);
//                        break;
//                    }
//                }
//            }
//            else if(data_rs232->pointer.ptr_who[0] == iDomConst::CLOCK)
//            {
//                std::lock_guard<std::mutex> lockBuf(useful_F::mutex_buf);
//                data_rs232->pointer.ptr_who[0] = data_rs232->pointer.ptr_who[1];
//                data_rs232->pointer.ptr_who[1] = iDomConst::RS232;
//                serial_ardu_clock.print(buffer.c_str());

//                buffer.erase();

//                while(useful_F::go_while){
//                    if(serial_ardu_clock.available()>0){
//                        buffer += serial_ardu_clock.read();
//                        buffer += serial_ardu_clock.read();
//                        break;
//                    }
//                }
//            }
//            else if(data_rs232->pointer.ptr_who[0] == iDomConst::FREE)
//            {
//                std::string bufor = "";
//                if(serial_ardu.available()>0) {

//                    while (useful_F::go_while){
//                        // std::cout << "serial_ardu.available(): "<<serial_ardu.available()<<std::endl;
//                        if(serial_ardu.available()>0){
//                            char t = serial_ardu.read();
//                            // std::cout << "t: "<<t<<std::endl;
//                            if(t == ';'){
//                                serial_ardu.flush();
//#ifdef BT_TEST
//                                useful_F::go_while = false;
//                                return;
//#endif
//                                break;
//                            }
//                            else{
//                                // printf("%c",t);
//                                bufor.push_back(t);
//                            }
//                        }
//                        else{
//#ifdef BT_TEST
//                            useful_F::go_while = false;
//                            return;
//#endif
//                        }
//                    }
//                    lockWho.~lock_guard();
//                    TASKER mainTasker(useful_F::myStaticData);
//                    mainTasker.dataFromRS232(bufor);
//                    continue;
//                }
//            }

//        }
//    }
//}

//////////////// watek RFLink //////////////////////////////////
void RFLinkHandlerRUN(thread_data *my_data){
    std::string msgFromRFLink;
    RC_433MHz rc433(my_data);
    my_data->main_RFLink->flush();
    std::vector <std::string>  v;
    v.push_back("ardu");
    v.push_back("433MHz");
    v.push_back("");

    std::this_thread::sleep_for( std::chrono::seconds(5));
    command_ardu workerRFLink("ardu",my_data);

    while(useful_F::go_while){
        std::this_thread::sleep_for( std::chrono::milliseconds(50));

        msgFromRFLink = rc433.receiveCode();
//        if (msgFromRFLink.size() > 0){

//            try{
//                my_data->main_RFLink->
//                        rflinkMAP[my_data->main_RFLink->getArgumentValueFromRFLinkMSG(msgFromRFLink,
//                                                                                      "ID")].counter();
//                my_data->main_RFLink->
//                        rflinkMAP[my_data->main_RFLink->getArgumentValueFromRFLinkMSG(msgFromRFLink,
//                                                                                      "ID")].msg = msgFromRFLink;
//            }
//            catch(std::string e){
//                std::cout << "wyjatek w szukaniu: " << e<<std::endl;
//            }

//            my_data->myEventHandler.run("433MHz")->addEvent("RFLink "+msgFromRFLink);
//        }

        if (msgFromRFLink.size() > 0){
            v[2] = msgFromRFLink;
            workerRFLink.execute(v,my_data);
        }

    }
}

//////////// watek do obslugi polaczeni miedzy nodami  //////////////
void f_serv_con_node (thread_data  *my_data){
    my_data->myEventHandler.run("node")->addEvent("start and stop node");
    useful_F::clearThreadArray(my_data);
} //  koniec f_serv_con_node

/////////////////////  watek do obslugi irda //////////////////////////////
void f_master_irda (thread_data  *my_data){
    master_irda irda(my_data);
    irda.run();
    useful_F::clearThreadArray(my_data);
} //  koniec master_irda

///////////  watek wymiany polaczenia /////////////////////

/////////////////////  watek CRON //////////////////////////////
void f_master_CRON (thread_data  *my_data){
    CRON my_CRON(my_data);
    my_CRON.run();
    useful_F::clearThreadArray(my_data);
} //  koniec CRON

//////////////////////////////////////////////////////////
void Server_connectivity_thread(thread_data  *my_data){
    C_connection *client = new C_connection( my_data);
    static unsigned int connectionCounter = 0;
    bool key_ok = false;
    std::string tm = inet_ntoa( my_data->from.sin_addr);
    if("192.168.1.1" != tm && my_data->ptr_MPD_info->isPlay == false) {
        my_data->mainLCD->set_print_song_state(32);
        my_data->mainLCD->printString(true,0,0,"USER CONNECTED!");
        my_data->mainLCD->printString(false,0,1,tm);
    }
    if("192.168.1.1" == tm || "192.168.1.144" == tm)
    {
        if( ++connectionCounter > 9){
            connectionCounter = 0;
            my_data->main_iDomTools->sendViberMsg("ktoś kombinuje z polaczeniem do serwera!",
                                                  my_data->server_settings->viberReceiver.at(0),
                                                  my_data->server_settings->viberSender+"_ALERT!");
        }
        client->setEncrypted(false);
    }
    else {
        connectionCounter = 0;
    }

    log_file_mutex.mutex_lock();
    log_file_cout << INFO <<"polaczenie z adresu  " <<  tm   <<std::endl;
    log_file_mutex.mutex_unlock();
    my_data->myEventHandler.run("connections")->addEvent(tm);

    int recvSize = client->c_recv(0);
    if(recvSize == -1)  {
        key_ok = false;
    }
    //std::cout <<"WYNIK:"<< client->c_read_buf().size()<<"a to wlasny" << RSHash().size()<<"!"<<std::endl;
    std::string KEY_OWN = useful_F::RSHash() ;
    client->setEncriptionKey(KEY_OWN);
    std::string KEY_rec = client->c_read_buf(recvSize);

    if(KEY_rec == KEY_OWN)   // stop runing idom_server
    {
        key_ok = true;
        if(client->c_send("OK") == -1)
        {
            key_ok = false;
        }
    }
    else{
        key_ok = false;
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL <<"AUTHENTICATION FAILED! " <<  inet_ntoa( my_data->from.sin_addr)   <<std::endl;
        log_file_cout << CRITICAL <<"KEY RECIVED: " << KEY_rec << " KEY SERVER: "<< KEY_OWN   <<std::endl;
        client->cryptoLog(KEY_rec);//  setEncriptionKey(KEY_rec);
        log_file_cout << CRITICAL <<"KEY RECIVED\n\n " << KEY_rec <<"\n\n"<<  std::endl;
        log_file_mutex.mutex_unlock();

        std::string msg ="podano zły klucz autentykacji - sprawdz logi " ;
        msg.append(inet_ntoa( my_data->from.sin_addr));
        my_data->main_iDomTools->sendViberMsg(msg,
                                              my_data->server_settings->viberReceiver.at(0),
                                              my_data->server_settings->viberSender+"_ALERT!");
        KEY_rec.clear();

        if(client->c_send("\nFAIL\n") == -1)
        {
            key_ok = false;
        }
    }
    /// ///////////////////////user level
    {
        int recvSize = client->c_recv(0);
        if(recvSize == -1)  {

        }
        else {

        }
        std::string userLevel = client->c_read_buf(recvSize);
        client->c_send("OK you are "+ userLevel);
        puts("user level to:");
        puts(userLevel.c_str());

        if(userLevel == "ROOT"){
            client->mainCommandHandler = new commandHandlerRoot(my_data);
        }
        else{
            client->mainCommandHandler = new commandHandler(my_data);
        }
    }
    while (useful_F::go_while && key_ok)
    {
        int recvSize = client->c_recv(0) ;
        if(recvSize == -1)
        {
            puts("klient sie rozlaczyl");
            break;
        }
        // ###########################  analia wiadomoscu ####################################//
        try
        {
            client->c_analyse(recvSize);
        }
        catch (std::string s)
        {
            puts("close server - throw");
            useful_F::go_while = false;
            client->c_send("CLOSE");
            //delete client;
            break;
        }

        // ###############################  koniec analizy   wysylanie wyniku do RS232 lub  TCP ########################
        if( client->c_send(0) == -1)
        {
            perror("send() ERROR");
            break;
        }
    }
    my_data->mainLCD->set_print_song_state(0);
    my_data->mainLCD->set_lcd_STATE(2);
    client->onStopConnection();
    sleep (3);
    delete client;
}

int main()
{
    std::ofstream pidFile;
    pidFile.open("/mnt/ramdisk/pid-iDom.txt");
    pidFile << getpid();
    pidFile.close();

    pthread_mutex_init(&Logger::mutex_log, NULL);
    config server_settings = useful_F::configFileToStruct();
    struct sockaddr_in server;
    int v_socket;

    thread_data node_data; // przekazywanie do watku
    node_data.server_settings = &server_settings;
    time(&node_data.start);

    Thread_array_struc thread_array[iDomConst::MAX_CONNECTION];
    for (int i = 0 ; i< iDomConst::MAX_CONNECTION;++i){
        thread_array[i].thread_name = "  -empty-  ";
        thread_array[i].thread_socket = 0;
    }

    unsigned int who[2] = {iDomConst::FREE, iDomConst::FREE};
    ///////////////////////////////////////////  zaczynam wpisy do logu ////////////////////////////////////////////////////////////
    log_file_mutex.mutex_lock();
    log_file_cout << "\n*****************************************************************\n*****************************************************************\n  "<<  " \t\t\t\t\t start programu " << std::endl;
    log_file_cout << INFO << "ID serwera\t"<< server_settings.ID_server << std::endl;
    log_file_cout << INFO << "PortRS232\t"<< server_settings.portRS232 << std::endl;
    log_file_cout << INFO << "PortRS232_clock\t"<< server_settings.portRS232_clock << std::endl;
    log_file_cout << INFO << "BaudRate RS232\t"<< server_settings.BaudRate << std::endl;
    log_file_cout << INFO << "RFLinkPort\t"<< server_settings.RFLinkPort << std::endl;
    log_file_cout << INFO << "RFLinkBaudRate\t"<< server_settings.RFLinkBaudRate << std::endl;
    log_file_cout << INFO << "port TCP \t"<< server_settings.PORT << std::endl;
    log_file_cout << INFO << "serwer ip \t"<< server_settings.SERVER_IP  <<std::endl;
    log_file_cout << INFO << "baza z filami \t"<< server_settings.MOVIES_DB_PATH << std::endl;
    log_file_cout << INFO << "klucz ThingSpeak \t"<<server_settings.TS_KEY << std::endl;
    log_file_cout << INFO << "thread MPD \t" << server_settings.THREAD_MPD << std::endl;
    log_file_cout << INFO << "thread CRON \t" << server_settings.THREAD_CRON << std::endl;
    log_file_cout << INFO << "thread IRDA \t" << server_settings.THREAD_IRDA << std::endl;
    log_file_cout << INFO << "thread RS232 \t" << server_settings.THREAD_RS232 << std::endl;
    log_file_cout << INFO << "thread DUMMY \t" << server_settings.THREAD_DUMMY << std::endl;
    log_file_cout << INFO << " \n" << std::endl;
    log_file_cout << INFO << "------------------------ START PROGRAMU -----------------------"<< std::endl;
    log_file_mutex.mutex_unlock();

    ///////////////////////////////////////////////  koniec logowania do poliku  ///////////////////////////////////////////////////

    /////////////////////////////// RC 433MHz ////////////////////
    RADIO_EQ_CONTAINER rc433MHz(&node_data);
    rc433MHz.loadConfig(server_settings.radio433MHzConfigFile);
    node_data.main_REC = &rc433MHz;

    RFLinkHandler rflinkHandler(&node_data);
    bool rflink_work = rflinkHandler.init();
    node_data.main_RFLink = &rflinkHandler;

    if (rflink_work == true){
        //start watku czytania RFLinka
        int freeSlotID = useful_F::findFreeThreadSlot(thread_array);
        thread_array[freeSlotID].thread = std::thread(RFLinkHandlerRUN, &node_data);
        thread_array[freeSlotID].thread_name = "RFLink_thread";
        thread_array[freeSlotID].thread_socket = 1;
        thread_array[freeSlotID].thread_ID = thread_array[freeSlotID].thread.get_id();
        thread_array[freeSlotID].thread.detach();
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "watek wystartowal  RFLink"<< thread_array[freeSlotID].thread_ID << std::endl;
        log_file_mutex.mutex_unlock();
    }
    ///////////////////////////////////////////////  start wiringPi  //////////////////////////////////////////////
    if(wiringPiSetup() == -1){
        exit(1) ;
    }

    /////////////////////////////// MPD info /////////////////////////
    MPD_info my_MPD_info;
    /////////////////////////////// iDom Status //////////////////////
    iDomSTATUS my_iDomStatus;
    node_data.main_iDomStatus = &my_iDomStatus;
    /////////////////////////////// LCD //////////////////////////////
    LCD_c mainLCD(0x27,16,2);
    //////////////     przegladanie plikow ////////////////////
    files_tree main_tree( server_settings.MOVIES_DB_PATH, &mainLCD);
    /////////////////////////////// iDom Tools ///////////////////////
    iDomTOOLS my_iDomTools(&node_data);
    ///////////////////////////////// MENU /////////////////////////////////
    menu_tree main_MENU(server_settings.MENU_PATH, &mainLCD);
    //////////////////////////////// SETTINGS //////////////////////////////
    node_data.main_iDomStatus->addObject("house",node_data.idom_all_state.houseState);
    /////////////////////////////////////////////////   wypelniam  struktury przesylane do watkow  ////////////////////////
    thread_data_rs232 data_rs232;
    data_rs232.BaudRate = server_settings.BaudRate;
    data_rs232.portRS232 = server_settings.portRS232;
    data_rs232.portRS232_clock = server_settings.portRS232_clock;
    data_rs232.pointer.ptr_who = who;

    ///  start watku do komunikacji rs232

    int freeSlotID = useful_F::findFreeThreadSlot(thread_array);

    if(server_settings.THREAD_RS232 == "YES")
    {
        thread_array[freeSlotID].thread = std::thread(Send_Recieve_rs232_thread,&data_rs232);
        thread_array[freeSlotID].thread_name = "RS232_thread";
        thread_array[freeSlotID].thread_socket = 1;
        thread_array[freeSlotID].thread_ID = thread_array[freeSlotID].thread.get_id();
        thread_array[freeSlotID].thread.detach();
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "watek wystartowal  RS232 "<< thread_array[freeSlotID].thread_ID << std::endl;
        log_file_mutex.mutex_unlock();
    }

    /////////////////////////////////  tworzenie pliku mkfifo  dla sterowania omx playerem

    int temp = mkfifo("/mnt/ramdisk/cmd",0666);

    if(temp == 0)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "mkfifo - plik stworzony "<<strerror(errno) << std::endl;
        log_file_mutex.mutex_unlock();
    }
    else if(temp == -1)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR << "mkfifo - "<<strerror(errno) << std::endl;
        log_file_mutex.mutex_unlock();
    }

    //////////////////////////////////////////////////////////////////////

    useful_F::setStaticData(&node_data);
    /////////////////////////////////////////////////////////
    int SERVER_PORT = server_settings.PORT;
    server_settings.SERVER_IP = useful_F::conv_dns(server_settings.SERVER_IP);
    const char *SERVER_IP = server_settings.SERVER_IP.c_str();
    node_data.pointer.ptr_who = who;
    node_data.mainLCD = &mainLCD;
    node_data.main_tree = &main_tree;
    node_data.main_MENU = &main_MENU;
    node_data.main_THREAD_arr = &thread_array[0];
    node_data.sleeper = 0;
    node_data.ptr_MPD_info = &my_MPD_info;

    pilot_led mainPilotLED;
    node_data.ptr_pilot_led = &mainPilotLED;
    //dodanie pilota
    node_data.main_iDomTools = &my_iDomTools;

    std::unique_ptr <pilot> pilotPTR( new pilot(&node_data.key_map));
    pilotPTR->setup();

    // start watku irda
    if(server_settings.THREAD_IRDA == "YES")
    {
        freeSlotID = useful_F::findFreeThreadSlot(thread_array);
        thread_array[freeSlotID].thread = std::thread (f_master_irda, &node_data);
        thread_array[freeSlotID].thread_name = "IRDA_master";
        thread_array[freeSlotID].thread_socket = 1;
        thread_array[freeSlotID].thread_ID = thread_array[freeSlotID].thread.get_id();
        thread_array[freeSlotID].thread.detach();
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "watek wystartowal polaczenie irda "<< thread_array[freeSlotID].thread_ID << std::endl;
        log_file_mutex.mutex_unlock();
    }

    // start watku  mpd_cli
    if(server_settings.THREAD_MPD == "YES")
    {
        freeSlotID = useful_F::findFreeThreadSlot(thread_array);
        thread_array[freeSlotID].thread = std::thread (main_mpd_cli, &node_data);
        thread_array[freeSlotID].thread_name = "MPD_client";
        thread_array[freeSlotID].thread_socket = 1;
        thread_array[freeSlotID].thread_ID = thread_array[freeSlotID].thread.get_id();
        thread_array[freeSlotID].thread.detach();
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "watek wystartowal klient mpd "<< thread_array[freeSlotID].thread_ID << std::endl;
        log_file_mutex.mutex_unlock();
    }

    // start watku CRONa
    if(server_settings.THREAD_CRON == "YES")
    {
        freeSlotID = useful_F::findFreeThreadSlot(thread_array);
        thread_array[freeSlotID].thread = std::thread(f_master_CRON, &node_data);
        thread_array[freeSlotID].thread_name = "CRON_master";
        thread_array[freeSlotID].thread_socket = 1;
        thread_array[freeSlotID].thread_ID = thread_array[freeSlotID].thread.get_id();
        thread_array[freeSlotID].thread.detach();
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "watek CRON wystartowal "<< thread_array[freeSlotID].thread_ID << std::endl;
        log_file_mutex.mutex_unlock();
    }

    if(server_settings.THREAD_DUMMY == "YES"){

        freeSlotID = useful_F::findFreeThreadSlot(thread_array);
        thread_array[freeSlotID].thread = std::thread(f_serv_con_node, &node_data);
        thread_array[freeSlotID].thread_name = "node master";
        thread_array[freeSlotID].thread_socket = 1;
        thread_array[freeSlotID].thread_ID = thread_array[freeSlotID].thread.get_id();
        thread_array[freeSlotID].thread .detach();
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "watek wystartowal dla NODA MASTERA "<< thread_array[freeSlotID].thread_ID << std::endl;
        log_file_mutex.mutex_unlock();
    }
    else
    {
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "NIE startuje NODA MASTERA do polaczen z innymi " << std::endl;
        log_file_mutex.mutex_unlock();
    }
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    if(inet_pton(AF_INET, SERVER_IP, & server.sin_addr) <= 0)
    {
        perror("inet_pton() ERROR");
        exit(-1);
    }

    if((v_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() ERROR");
        exit(-1);
    }

    if(fcntl(v_socket, F_SETFL, O_NONBLOCK) < 0) // fcntl()
    {
        perror("fcntl() ERROR");
        exit(-1);
    }
    // zgub wkurzający komunikat błędu "address already in use"
    int yes = 1;
    if(setsockopt(v_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == - 1) {
        perror("setsockopt");
        exit(1);
    }
    socklen_t len = sizeof(server);
    if(bind(v_socket,( struct sockaddr *) & server, len) < 0)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << "BIND problem: " <<  strerror(errno)<< std::endl;
        log_file_cout << CRITICAL << "awaryjne ! zamykanie gniazda  " << shutdown(v_socket, SHUT_RDWR) << std::endl;
        log_file_mutex.mutex_unlock();
        perror("bind() ERROR");
        exit(-1);
    }

    if(listen(v_socket, iDomConst::MAX_CONNECTION) < 0)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << "Listen problem: " <<  strerror(errno)<< std::endl;
        log_file_mutex.mutex_unlock();
        perror("listen() ERROR");
        exit(-1);
    }
    struct sockaddr_in from;
    /////////////////////////////////////////////////// INFO PART //////////////////////////////////////////////////
    node_data.main_iDomTools->sendViberMsg("iDom server wystartował", server_settings.viberReceiver.at(0),server_settings.viberSender);

    ///////////////////////////////////////////////////// WHILE ////////////////////////////////////////////////////

    while (1)
    {
        int v_sock_ind = 0;
        memset( &from,0, sizeof(from));
        if(!useful_F::go_while) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if(( v_sock_ind = accept( v_socket,(struct sockaddr *) & from, & len)) < 0)
        {
            continue;
        }

        ////////////////////////   jest połacznie   wiec wstawiamy je  do nowego watku  i  umieszczamy id watku w tablicy  w pierwszym wolnym miejscy ////////////////////

        int freeSlotID = useful_F::findFreeThreadSlot(thread_array);

        if( freeSlotID != -1)

        {
            node_data.s_client_sock = v_sock_ind;
            node_data.from = from;
            thread_array[freeSlotID].thread = std::thread(Server_connectivity_thread, &node_data);
            thread_array[freeSlotID].thread_name = inet_ntoa(node_data.from.sin_addr);
            thread_array[freeSlotID].thread_socket = v_sock_ind;
            thread_array[freeSlotID].thread_ID = thread_array[freeSlotID].thread.get_id();
            thread_array[freeSlotID].thread.detach();
            log_file_mutex.mutex_lock();
            log_file_cout << INFO << "watek wystartowal  "<< thread_array[freeSlotID].thread_ID << std::endl;
            log_file_mutex.mutex_unlock();

        }
        else
        {
            log_file_mutex.mutex_lock();
            log_file_cout << INFO << "za duzo klientow "<< thread_array[freeSlotID].thread_ID << std::endl;
            log_file_mutex.mutex_unlock();

            if( (send(v_sock_ind, "za duzo kientow \nEND.\n",22 , MSG_DONTWAIT)) <= 0)
            {
                perror("send() ERROR");
                break;
            }
            continue;
        }
    } // while
    // zamykam gniazdo

    iDomTOOLS::turnOffSpeakers();
    node_data.mainLCD->set_print_song_state(0);
    node_data.mainLCD->set_lcd_STATE(2);
    node_data.mainLCD->clear();
    node_data.mainLCD->noBacklight();
    node_data.main_iDomTools->MPD_stop();
    log_file_mutex.mutex_lock();
    log_file_cout << INFO << "zamykanie gniazda wartosc "  << shutdown(v_socket, SHUT_RDWR)<< std::endl;
    log_file_cout << ERROR << "gniazdo ind  "<<strerror(errno) << std::endl;
    log_file_cout << INFO << "koniec programu  "<<   std::endl;
    log_file_mutex.mutex_unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    pthread_mutex_destroy(&Logger::mutex_log);
    return 0;
}
