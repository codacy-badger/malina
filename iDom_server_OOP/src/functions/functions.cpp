#include <netdb.h>
#include "functions.h"
#include "../../libs/config_parser/parser.hpp"

bool useful_F::go_while = true;
std::mutex useful_F::mutex_buf;
std::mutex useful_F::mutex_who;

// przerobka  adresu na ip . //////////////////////////////////
std::string useful_F::conv_dns (std::string temp){

    int i;
    struct hostent * he;
    struct in_addr ** addr_list;
    std::string s_ip;

    if(( he = gethostbyname( temp.c_str() ) ) == NULL )
    {
        herror( "gethostbyname" );
        return "- 1";
    }
    // print information about this host:
    printf( "Official name is: %s\n", he->h_name );
    printf( "IP addresses: " );
    addr_list =( struct in_addr ** ) he->h_addr_list;

    for( i = 0; addr_list[ i ] != NULL; i++ )
    {
        printf( "%s ", inet_ntoa( * addr_list[ i ] ) );
        s_ip += inet_ntoa( * addr_list[ i ] );
    }
    printf( "\ndone " );
    return s_ip;
}

config useful_F::configFileToStruct()
{
    std::map<std::string, std::string>  confMap = read_config("/etc/config/iDom_SERVER/iDom_server.cfg");     // strukruta z informacjami z pliku konfig
    config confStruct;

    confStruct.BaudRate  = confMap.at("BaudRate");
    confStruct.portRS232 = confMap.at("portRS232");
    confStruct.portRS232_clock = confMap.at("portRS232_clock");

    ///// camera
    confStruct.cameraLedOFF = confMap.at("CAMERA_LED_OFF");
    confStruct.cameraLedON  = confMap.at("CAMERA_LED_ON");
    confStruct.cameraURL    = confMap.at("CAMERA_SNAPSHOT");

    confStruct.ID_server = std::stoi(confMap.at("ID"));
    confStruct.SERVER_IP = confMap.at("SERVER_IP");

    confStruct.MOVIES_DB_PATH = confMap.at("MOVIES_DB_PATH");
    confStruct.MENU_PATH    = confMap.at("MENU_PATH");

    confStruct.TS_KEY = confMap.at("TS_KEY");
    confStruct.MPD_IP = confMap.at("MPD_IP");

    confStruct.v_delay = std::stoi(confMap.at("DELAY"));
    confStruct.PORT = std::stoi(confMap.at("PORT"));

    //// thread
    confStruct.THREAD_MPD   = confMap.at("THREAD_MPD");
    confStruct.THREAD_RS232 = confMap.at("THREAD_RS232");
    confStruct.THREAD_CRON  = confMap.at("THREAD_CRON");
    confStruct.THREAD_IRDA  = confMap.at("THREAD_IRDA");
    confStruct.THREAD_DUMMY = confMap.at("THREAD_DUMMY");

    ////// facebook
    confStruct.facebookAccessToken = confMap.at("FB_ACCESS_TOKEN");

    //////////// viber
    confStruct.viberSender = confMap.at("VIBER_SENDER");
    confStruct.viberToken  = confMap.at("VIBER_TOKEN");
    confStruct.viberAvatar = confMap.at("VIBER_AVATAR");
    //confStruct.viberReceiver.insert(confStruct.viberReceiver.begin(), confMap.at("VIBER_RECEIVER_ROOT"));
    confStruct.viberReceiver = split(confMap.at("VIBER_RECEIVER"), '|');

    ///////// FTP server
    confStruct.ftpServer.URL  = confMap.at("FTP_URL");
    confStruct.ftpServer.user = confMap.at("FTP_LOGIN");
    confStruct.ftpServer.pass = confMap.at("FTP_PASS");
    //    confStruct = confMap.at();
    //    confStruct = confMap.at();
    //    confStruct = confMap.at();
    ///////////// RADIO 433
    confStruct.radio433MHzConfigFile = confMap.at("433MHz_config");


    return confStruct;
}

////////////////////////////////////////// not used now
/*
void ChangeEndianness(int32_t * value,int MAX_MSG_LEN_INT)
{
    if (__BYTE_ORDER == __BIG_ENDIAN)
    {

    }
    else if( __BYTE_ORDER == __LITTLE_ENDIAN)
    { //cout << "little-endian" << endl
        for (int i =0 ; i < MAX_MSG_LEN_INT ; ++i )
        {
            int32_t result = 0;
            result |= (value[i] & 0x000000FF) << 24;
            result |= (value[i] & 0x0000FF00) << 8;
            result |= (value[i] & 0x00FF0000) >> 8;
            result |= (value[i] & 0xFF000000) >> 24;

            value[i] = result;
        }
    }
}// ChangeEndianness

void binary(int val)
{    for (int i = 31; i >= 0; i--)
    {
        if ((i + 1) % 8 == 0)
            std::cout << ' ';
        std::cout << ((val >> i) % 2);
    }
     std::cout << std::endl;
}
*/
std::string useful_F::send_to_arduino_clock (thread_data *my_data_logic, std::string msg){

    while (go_while)
    {
        std::this_thread::sleep_for( std::chrono::milliseconds(50) );
        {
            std::lock_guard<std::mutex> lockWho(useful_F::mutex_who);

            if (my_data_logic->pointer.ptr_who[0] == iDomConst::FREE)
            {
                {
                    std::lock_guard<std::mutex> lockBuf(useful_F::mutex_buf);
                    my_data_logic->pointer.ptr_who[0]=iDomConst::CLOCK;
                    my_data_logic->pointer.ptr_who[1]= pthread_self();
                    buffer=msg;
                }
                break;
            }
        }
    }

    while (go_while)
    {
        std::this_thread::sleep_for( std::chrono::milliseconds(50) );
        {
            std::lock_guard<std::mutex> lockWho(useful_F::mutex_who);
            if (my_data_logic->pointer.ptr_who[0] == pthread_self())
            {
                {
                    std::lock_guard<std::mutex> lockBuf(useful_F::mutex_buf);
                    //C_connection::mutex_buf.lock();
                    my_data_logic->pointer.ptr_who[0]=iDomConst::FREE;
                    my_data_logic->pointer.ptr_who[1]= 0;
                    msg=buffer;
                }
                break;
            }
        }
    }
    return msg;
} //end send_to_arduino_clock

std::string useful_F::send_to_arduino (thread_data *my_data_logic, std::string msg){

    while (go_while)
    {
        std::this_thread::sleep_for( std::chrono::milliseconds(50) );
        {
            std::lock_guard<std::mutex> lockWho(useful_F::mutex_who);
            if (my_data_logic->pointer.ptr_who[0] == iDomConst::FREE)
            {
                {
                    std::lock_guard<std::mutex> lockBuf(useful_F::mutex_buf);
                    my_data_logic->pointer.ptr_who[0]=iDomConst::RS232;
                    my_data_logic->pointer.ptr_who[1]= pthread_self();
                    buffer=msg;
                }
                break;
            }
        }
    }

    while (go_while)
    {
        std::this_thread::sleep_for( std::chrono::milliseconds(50) );
        {
            std::lock_guard<std::mutex> lockWho(useful_F::mutex_who);
            if (my_data_logic->pointer.ptr_who[0] == pthread_self())
            {
                {
                    std::lock_guard<std::mutex> lockBuf(useful_F::mutex_buf);
                    my_data_logic->pointer.ptr_who[0]=iDomConst::FREE;
                    my_data_logic->pointer.ptr_who[1]= 0;
                    msg=buffer;
                }
                break;
            }
        }
    }
    return msg;
} //end send_to_arduino

std::string  useful_F::sek_to_uptime(long long secy )
{
    const int min = 60; //s
    const int houry = 3600; //s
    const int day = 86400; //s
    int number_day, number_hour, number_min, number_sec;
    int temp1, temp2, temp3;
    number_day = secy / day;
    std::string text = "\n" +std::to_string( number_day )+ " day " ;
    temp1 = secy % day;
    number_hour = temp1 / houry;
    text += std::to_string( number_hour ) +" hours "  ;
    temp2 = temp1 % houry;
    number_min = temp2 / min;
    text += std::to_string(number_min)+" minutes "  ;
    temp3 = temp2 % min;
    number_sec = temp3;
    text += std::to_string( number_sec )+ " seconds " ;
    return text;
}

////// watek sleeper
void useful_F::sleeper_mpd (thread_data  *my_data)
{

    for (; my_data->sleeper >0 ; my_data->sleeper-- ){
        std::this_thread::sleep_for( std::chrono::seconds(60) );
    }
    my_data->main_iDomTools->ledOFF();
    my_data->main_iDomTools->MPD_stop();
    my_data->main_iDomTools->turnOff433MHzSwitch("listwa");
    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "zaczynam procedure konca watku SLEEP_MPD" <<  std::endl;
    log_file_mutex.mutex_unlock();
    try {
        for (int i =0 ; i< iDomConst::MAX_CONNECTION;++i)
        {
            if (my_data->main_THREAD_arr[i].thread_ID == std::this_thread::get_id())
            {
                //my_data->main_THREAD_arr[i].thread.detach();
                my_data->main_THREAD_arr[i].thread_name ="  -empty-  ";
                my_data->main_THREAD_arr[i].thread_ID =  std::thread::id();
                my_data->main_THREAD_arr[i].thread_socket = 0;
                break;
            }
        }
    }
    catch (std::system_error &e){
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR<< "zlapano wyjatek w  watku SLEEP_MPD: " << e.what()<< std::endl;
        log_file_mutex.mutex_unlock();
    }

    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "koniec  watku SLEEP_MPD" <<  std::endl;
    log_file_mutex.mutex_unlock();

}
std::string useful_F::RSHash(std::string data, unsigned int b, unsigned int a)
{
    time_t act_time;
    struct tm * act_date;
    time(&act_time);
    act_date = localtime(&act_time);
    char buffer[10];
    strftime(buffer,10,"%M%H%w",act_date);
    std::string str(buffer);
    str+=data;
    unsigned int hash = 0;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = hash * a + str[i];
        a    = a * b;
    }
    return std::to_string((hash & 0x7FFFFFFF));
}

void useful_F::write_to_mkfifo(  std::string msg)
{
    int fd = open("/mnt/ramdisk/cmd", O_WRONLY| O_NONBLOCK);
    write(fd, msg.c_str(), msg.size());
    close(fd);
}

std::string useful_F::read_from_mkfifo()
{
    char buf[10];
    /* open, read, and display the message from the FIFO */
    int fd = open("/mnt/ramdisk/cmd", O_RDONLY | O_NONBLOCK);
    read(fd, buf, 10);
    close(fd);
    return (std::string(buf));
}

//wysylanie pliku
std::string useful_F::l_send_file(std::string path, std::string find  , bool reverse )
{
    std::string str_buf;
    if(find.empty()==true)
    {
        std::fstream log_file;
        log_file.open(path.c_str(),std::ios::in);
        if( log_file.good() == false )
        {
            return " can not open file !";
        }
        else
        {
            str_buf.erase();
            while( !log_file.eof() )
            {
                str_buf += log_file.get();
            }
            str_buf.erase(str_buf.length()-1, str_buf.length());

            log_file.close();
        }
    }
    else
    {
        std::fstream log_file;
        log_file.open(path.c_str(),std::ios::in);
        if( log_file.good() == false )
        {
            return " can not open file !";
        }
        else
        {
            str_buf.erase();
            std::string str_temp;
            while( std::getline(log_file,str_temp) )
            {
                if (reverse){
                    if(std::string::npos!=str_temp.find(find)){
                        str_buf+=str_temp +"\n";
                    }
                    else{
                        if(str_buf.size()<3){
                            str_buf+="    ";
                        }
                    }
                }
                else{
                    if(std::string::npos == str_temp.find(find)){
                        str_buf+=str_temp+"\n";
                    }
                }
                if(str_buf.size()<3){
                    str_buf+="    ";
                }
            }
        }
        log_file.close();
    }
    return str_buf;
}

void useful_F::clearThreadArray(thread_data* my_data)
{
    for (int i = 0 ; i< iDomConst::MAX_CONNECTION;++i)
    {
        if (my_data->main_THREAD_arr[i].thread_ID == std::this_thread::get_id())
        {
            //my_data->main_THREAD_arr[i].thread.detach();
            my_data->main_THREAD_arr[i].thread_name   = "  -empty-  ";
            my_data->main_THREAD_arr[i].thread_socket = 0;
            my_data->main_THREAD_arr[i].thread_ID     = std::thread::id();
            break;
        }
    }
}

int useful_F::findFreeThreadSlot(Thread_array_struc *array)
{
    for (int i = 0 ; i< iDomConst::MAX_CONNECTION;  ++i)
    {
        if (array[i].thread_socket == 0)
        {
            return i;
        }
    }
    return -1;
}

std::string useful_F::httpPost(std::string url, int timeoutSeconds)
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, iDomTOOLS::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return readBuffer;
}

void useful_F::downloadFile(std::string url, std::string path, int timeoutSeconds)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(path.c_str(),"wb");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
}

volatile unsigned int  useful_F::lastInterruptTime = 0;
std::mutex useful_F::mut;

void useful_F::button_interrupt( )
{
    std::lock_guard <std::mutex > lock(useful_F::mut);
    static int counter = 0;
    counter++;
    //TODO delete counter it is not needed

    volatile unsigned int m = millis();
    volatile auto a = m - useful_F::lastInterruptTime;
    if (a > 50){
        log_file_mutex.mutex_lock();
        log_file_cout << INFO << "przerwanie przycisku " <<counter<< std::endl;
        log_file_mutex.mutex_unlock();
        useful_F::myStaticData->myEventHandler.run("interrupt")->addEvent("przerwanie z przycisku");
        printf ("przerwanie %d - %d = %d\n", m, useful_F::lastInterruptTime, a);
        printf("counter %d \n status is %d\n",counter,digitalRead(iDomConst::BUTTON_PIN));

        if (digitalRead(iDomConst::BUTTON_PIN)==HIGH){
            //iDomTOOLS::playMPD(useful_F::myStaticData);
            unsigned int menuCounter = 0;
            while (digitalRead(iDomConst::BUTTON_PIN)==HIGH){
                menuCounter++;
                if(menuCounter==6){
                    useful_F::myStaticData->mainLCD->set_lcd_STATE(100);
                    useful_F::myStaticData->mainLCD->printString(true,0,0,"MUSIC");
                    puts("MUSIC");
                }
                if (menuCounter==60000000){
                    useful_F::myStaticData->mainLCD->set_lcd_STATE(100);
                    useful_F::myStaticData->mainLCD->printString(true,0,0,"LED");
                    puts("LED");
                }
                if (menuCounter==120000000){
                    useful_F::myStaticData->mainLCD->set_lcd_STATE(100);
                    useful_F::myStaticData->mainLCD->printString(true,0,0,"SERVER");
                    puts("SERVER");
                }
            }
        }
        //        else{
        //            useful_F::myStaticData->mainLCD->set_lcd_STATE(100);
        //            useful_F::myStaticData->mainLCD->printString(true,0,0,"DONE");
        //            puts("DONE");
        //        }
        useful_F::lastInterruptTime = millis();
    }
}
