#include <iostream>
#include <fstream>
#include <regex>
#include <iterator>
#include <vector>

#include "functions.h"
#include "../thread_functions/iDom_thread.h"

std::vector<std::string> useful_F::split(const std::string& s, char separator ){
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(separator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = ++pos;
    }
    try
    {
        output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    }
    catch (...)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << "wyjatek substr() w useful_F::split() !!!!!!"<< std::endl;
        log_file_mutex.mutex_unlock();
    }
    return output;
}

thread_data* useful_F::myStaticData = std::nullptr_t();

void useful_F::setStaticData(thread_data *my_dataPtr)
{
    myStaticData = my_dataPtr;
}

void useful_F::tokenizer ( std::vector <std::string> &command, std::string separator, std::string &text){
    std::string temp;

    for(char n: text)
    { // the initializer may be an array
        bool is_sep = false;
        for(char m: separator)
        {
            if (n == m)
                is_sep = true;
        }
        if (is_sep == false)
            temp += n;
        else
        {
            if (!temp.empty())
            {
                command.push_back( temp);
                temp = "";
            }
        }
    }
    if (!temp.empty())
        command.push_back(temp);

    if (command.size() == 0)
        throw std::string("command empty");
}

////// watek sleeper
void useful_F::sleeper_mpd (thread_data *my_data, const std::string& threadName)
{
    unsigned int t = 60/my_data->sleeper;
    unsigned int k = 0;

    for (; my_data->sleeper > 0; my_data->sleeper--)
    {
        useful_F::sleep(60);
        k += t;
        my_data->main_iDomTools->ledClear(0,k);
    }
    my_data->main_iDomTools->ledOFF();
    my_data->main_iDomTools->MPD_stop();
    my_data->main_iDomTools->turnOff433MHzSwitch("listwa");

    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "zaczynam procedure konca watku " << threadName << std::endl;
    log_file_mutex.mutex_unlock();

    iDOM_THREAD::stop_thread(threadName,my_data);

    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "koniec watku SLEEP_MPD" << std::endl;
    log_file_mutex.mutex_unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////// watek kodi
void useful_F::kodi (thread_data *my_data, const std::string& threadName)
{
    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "start wątku "<<threadName << std::endl;
    log_file_mutex.mutex_unlock();

    my_data->mainLCD->set_print_song_state(100);
    my_data->mainLCD->printString(false,2,1,"  KODI");

    my_data->main_iDomStatus->setObjectState("KODI",STATE::ACTIVE);
    //status mpd
    STATE musicState = my_data->main_iDomStatus->getObjectState("music");
    // status glosnikow
    STATE speakersState = my_data->main_iDomStatus->getObjectState("speakers");

    if (musicState != STATE::STOP)
        my_data->main_iDomTools->MPD_pause();
    if (speakersState != STATE::ON)
        my_data->main_iDomTools->turnOnSpeakers();
    //system

    int ret = useful_F::runLinuxCommand("runuser -u pi kodi");

    if(ret != 0)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL<< "kodi zamkniete z błędem "<<ret << std::endl;
        log_file_mutex.mutex_unlock();
    }
    //przywracanie danych

    if(musicState == STATE::PLAY)
        my_data->main_iDomTools->MPD_play(my_data);
    else
        my_data->main_iDomTools->turnOffSpeakers();
    //koniec

    my_data->main_iDomStatus->setObjectState("KODI",STATE::DEACTIVE);
    my_data->mainLCD->set_print_song_state(0);
    iDOM_THREAD::stop_thread("kodi smartTV",my_data);
}
std::string useful_F::RSHash(const std::string& data, unsigned int b, unsigned int a)
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
        a = a * b;
    }
    return std::to_string((hash & 0x7FFFFFFF));
}

std::string useful_F::sek_to_uptime(long long secy )
{
    const int min = 60; //s
    const int houry = 3600; //s
    const int day = 86400; //s
    int number_day, number_hour, number_min, number_sec;
    int temp1, temp2, temp3;
    number_day = secy / day;
    std::stringstream text;
    text << "\n" << number_day << " day ";
    temp1 = secy % day;
    number_hour = temp1 / houry;
    text << number_hour << " hours ";
    temp2 = temp1 % houry;
    number_min = temp2 / min;
    text << number_min << " minutes ";
    temp3 = temp2 % min;
    number_sec = temp3;
    text << number_sec << " seconds";
    return text.str();
}

//wysylanie pliku
std::string useful_F::l_send_file(std::string path, std::string find, bool reverse )
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
                if (reverse)
                {
                    if(std::string::npos!=str_temp.find(find))
                    {
                        str_buf+=str_temp +"\n";
                    }
                    else
                    {
                        if(str_buf.size()<3)
                        {
                            str_buf+="    ";
                        }
                    }
                }
                else
                {
                    if(std::string::npos == str_temp.find(find))
                    {
                        str_buf+=str_temp+"\n";
                    }
                }
                if(str_buf.size()<3)
                {
                    str_buf+="    ";
                }
            }
        }
        log_file.close();
    }
    return str_buf;
}

CONFIG_JSON useful_F::configJsonFileToStruct(nlohmann::json jj)
{
    CONFIG_JSON  cj;
#ifdef BT_TEST
    std::cout << "json: " << jj.dump(4);
#endif
    cj._nightLight.to = jj["Night-light"].at("main").at("to").get<int>();
    cj._nightLight.from = jj["Night-light"].at("main").at("from").get<int>();
    cj._nightLight.colorLED = stringToColor.at( jj["Night-light"].at("main").at("color").get<std::string>());
    /////////////////////// server settings
    cj._server.PORT = jj["Server_settings"].at("PORT").get<int>();
    cj._server.encrypted =  jj["Server_settings"].at("ENCRYPTED").get<bool>();
    cj._server.MENU_PATH =  jj["Server_settings"].at("MENU_PATH").get<std::string>();
    cj._server.MOVIES_DB_PATH = jj["Server_settings"].at("MOVIES_DB_PATH").get<std::string>();
    cj._server.ID_server = jj["Server_settings"].at("ID").get<int>();
    cj._server.SERVER_IP = jj["Server_settings"].at("SERVER_IP").get<std::string>();
    cj._server.saveFilePath = jj["Server_settings"].at("saveFilePath").get<std::string>();
    cj._server.radio433MHzConfigFile = jj["433MHz_settings"].at("433MHz_config").get<std::string>();
    cj._server.omxplayerFile = jj["omxplayer_settings"].at("omxplayerFile").get<std::string>();
    cj._server.ftpServer.URL = jj["FTP_settings"].at("FTP_URL").get<std::string>();
    cj._server.ftpServer.user = jj["FTP_settings"].at("FTP_LOGIN").get<std::string>();
    cj._server.ftpServer.pass = jj["FTP_settings"].at("FTP_PASS").get<std::string>();
    cj._server.TS_KEY = jj["THINGSPEAK_settings"].at("TS_KEY").get<std::string>();
    cj._server.lightningApiURL = jj["lightning_settings"].at("LIGHTNING_API_URL").get<std::string>();
    cj._server.MPD_IP = jj["MPD_settings"].at("MPD_IP").get<std::string>();
    /////////////////////// viber
    cj._fb_viber.viberToken = jj["viber_settings"].at("VIBER_TOKEN").get<std::string>();
    cj._fb_viber.viberAvatar = jj["viber_settings"].at("VIBER_AVATAR").get<std::string>();
    cj._fb_viber.viberSender = jj["viber_settings"].at("VIBER_SENDER").get<std::string>();;
    for(auto k : jj["viber_settings"].at("VIBER_RECEIVER"))
    {
        cj._fb_viber.viberReceiver.push_back(k.get<std::string>());
    }
    ////////////////////// facebook
    cj._fb_viber.facebookAccessToken = jj["facebook_settings"].at("FB_ACCESS_TOKEN").get<std::string>();
    /////////////////////// thread settings
    cj._runThread.MPD    = jj["THREAD"].at("MPD").at("run").get<bool>();
    cj._runThread.RFLink = jj["THREAD"].at("RFLink").at("run").get<bool>();
    cj._runThread.IRDA   = jj["THREAD"].at("IRDA").at("run").get<bool>();
    cj._runThread.CRON   = jj["THREAD"].at("CRON").at("run").get<bool>();
    cj._runThread.RS232  = jj["THREAD"].at("RS232").at("run").get<bool>();
    cj._runThread.DUMMY  = jj["THREAD"].at("DUMMY").at("run").get<bool>();
    cj._runThread.MQTT   = jj["THREAD"].at("MQTT").at("run").get<bool>();
    /////////////////////// camera
    cj._camera.cameraLedOFF = jj["camera_settings"].at("CAMERA_LED_OFF").get<std::string>();
    cj._camera.cameraLedON  = jj["camera_settings"].at("CAMERA_LED_ON").get<std::string>();
    cj._camera.cameraURL    = jj["camera_settings"].at("CAMERA_SNAPSHOT").get<std::string>();
    /////////////////////// mqtt broker
    cj._mqtt_broker.qos  = jj["mqtt_broker_settings"].at("qos").get<int>();
    cj._mqtt_broker.port = jj["mqtt_broker_settings"].at("port").get<int>();
    cj._mqtt_broker.host = jj["mqtt_broker_settings"].at("host").get<std::string>();
    cj._mqtt_broker.topicPublish   = jj["mqtt_broker_settings"].at("publish topic").get<std::string>();
    cj._mqtt_broker.topicSubscribe = jj["mqtt_broker_settings"].at("subscribe topic").get<std::string>();
    /////////////////////// rs232
    cj._rs232.BaudRate  = jj["RS232_settings"].at("BaudRate").get<int>();
    cj._rs232.portRS232 = jj["RS232_settings"].at("portRS232").get<std::string>();
    cj._rs232.portRS232_clock = jj["RS232_settings"].at("portRS232_clock").get<std::string>();
    /////////////////////// RFLink
    cj._rflink.RFLinkPort = jj["RFLink_settings"].at("RFLinkPort").get<std::string>();
    cj._rflink.RFLinkBaudRate = jj["RFLink_settings"].at("RFLinkBaudRate").get<int>();

    return cj;
}

std::map<std::string, iDom_API*> iDom_API::m_map_iDom_API;

std::string iDom_API::getDump(){
        std::stringstream ret;
        for(auto it:m_map_iDom_API){
            ret <<"map element: " << it.first << std::endl << it.second->dump();
        }

        return ret.str();
    }
