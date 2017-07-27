#ifndef IDOMTOOLS_H
#define IDOMTOOLS_H

#include <map>
#include <string>
#include <chrono>
#include "../iDom_server_OOP.h"
#include "../../libs/sunrise-sunset/sunriseset.h"
#include "sys/sysinfo.h"

struct thread_data;
struct LED_Strip;

enum class TEMPERATURE_STATE{
    Under,
    Over,
    NoChanges,
    Unknown
};
enum class USER_LEVEL{
    ROOT,
    USER
};

struct temperature {
    //temperature(std::string name) : thermometrName(name){};
    double newTemp = 1.0101;
    double oldTemp = 1.0101;
    double maxValue = 0;
    std::string maxDate;
    double minValue = 0;
    std::string minDate;
    std::string thermometrName;

    TEMPERATURE_STATE lastState = TEMPERATURE_STATE::Unknown;
};

class iDomTOOLS
{
    std::map <std::string ,temperature> thermometer;
    thread_data *my_data;
    SunRiseSet sun;
    std::string key;

public:
    std::vector <std::string> textToSpeachVector;
    iDomTOOLS(thread_data *myData);

    void setTemperature (std::string name, float value);
    TEMPERATURE_STATE hasTemperatureChange(std::string thermometerName, double reference, double histereza);
    void sendSMSifTempChanged(std::string thermomethernName, int reference);

    static void turnOnSpeakers();
    static void turnOffSpeakers();

    std::string getSunrise(bool extend = false);
    std::string getSunset(bool extend = false);
    std::string getDayLenght(bool extend = false);
    std::string getSystemInfo();

    void textToSpeach(std::vector <std::string> *textVector);
    std::string getTextToSpeach();

    std::vector <std::string> getTemperature();
    std::string getTemperatureString();
    Clock getTime();
    std::string getSmog();
    void send_temperature_thingSpeak();
    static size_t  WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static std::string find_tag (const std::string &temp);

    std::string sendSMStoPlusGSM(std::string login, std::string pass, std::string number, std::string msg, int silentFrom = 0, int silentTo =0);

    //////////////////// LED part //////////////////////////
    std::string ledOFF();
    std::string ledClear();
    std::string ledOn(LED_Strip ledColor);

    //////////////////// MPD part //////////////////////////
    static void MPD_play();
    static void MPD_stop();
    static void MPD_next();
    static void MPD_prev();
    static void MPD_pause();
    static void MPD_volumeUp();
    static void MPD_volumeDown();
    static void MPD_volumeSet(thread_data* my_data, int vol);
    static void MPD_playId(thread_data* my_data,int id);
};

#endif // IDOMTOOLS_H
