#include <iostream>
#include <fstream>
#include <regex>
#include <iterator>
#include <vector>

#include "functions.h"

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
}

////// watek sleeper
void useful_F::sleeper_mpd (thread_data  *my_data)
{
    unsigned int t = 60/my_data->sleeper;
    unsigned int k = 0;

    for (; my_data->sleeper >0 ; my_data->sleeper-- )
    {
        useful_F::sleep_1min();
        k += t;
        my_data->main_iDomTools->ledClear(0,k);
    }
    my_data->main_iDomTools->ledOFF();
    my_data->main_iDomTools->MPD_stop();
    my_data->main_iDomTools->turnOff433MHzSwitch("listwa");

    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "zaczynam procedure konca watku SLEEP_MPD" <<  std::endl;
    log_file_mutex.mutex_unlock();

    try
    {
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
    catch (std::system_error &e)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR<< "zlapano wyjatek w  watku SLEEP_MPD: " << e.what()<< std::endl;
        log_file_mutex.mutex_unlock();
    }

    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "koniec  watku SLEEP_MPD" <<  std::endl;
    log_file_mutex.mutex_unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////// watek kodi
void useful_F::kodi (thread_data  *my_data)
{
    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "start wątku KODI" <<  std::endl;
    log_file_mutex.mutex_unlock();

    my_data->mainLCD->set_print_song_state(100);
    my_data->mainLCD->printString(false,2,1,"  KODI   ");

    my_data->main_iDomStatus->setObjectState("KODI",STATE::ACTIVE);
    //  status mpd
    STATE musicState = my_data->main_iDomStatus->getObjectState("music");
    // status glosnikow
    STATE speakersState = my_data->main_iDomStatus->getObjectState("speakers");

    if (musicState != STATE::STOP)
        my_data->main_iDomTools->MPD_pause();
    if (speakersState != STATE::ON)
        my_data->main_iDomTools->turnOnSpeakers();
    //system
#ifdef BT_TEST
    int ret = system("ls");
#else
    int ret = system("runuser -u pi kodi");
#endif
    std::cout << "system() zwraca ret " << ret <<std::endl;
    //przywracanie danych

    if(musicState == STATE::PLAY)
        my_data->main_iDomTools->MPD_play(my_data);
    else
        my_data->main_iDomTools->turnOffSpeakers();
    //koniec

    try
    {
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
    catch (std::system_error &e)
    {
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR<< "zlapano wyjatek w  watku KODI: " << e.what()<< std::endl;
        log_file_mutex.mutex_unlock();
    }

    my_data->main_iDomStatus->setObjectState("KODI",STATE::DEACTIVE);
    my_data->mainLCD->set_print_song_state(0);
    log_file_mutex.mutex_lock();
    log_file_cout << INFO<< "koniec  watku KODI" <<  std::endl;
    log_file_mutex.mutex_unlock();
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
        a    = a * b;
    }
    return std::to_string((hash & 0x7FFFFFFF));
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
    puts("return -1");
    return -1;
}
