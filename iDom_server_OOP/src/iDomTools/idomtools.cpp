#include <algorithm>
#include <fstream>
#include <string>

#include "idomtools.h"
#include "../functions/functions.h"
#include "../../libs/emoji/emoji.h"
#include "../CRON/cron.hpp"
#include "../RADIO_433_eq/radio_433_eq.h"

iDomTOOLS::iDomTOOLS(thread_data *myData) : key(myData->server_settings->TS_KEY)
{
    my_data = myData;
    //////////////////////////////////// temeprature /////////////////

    allThermometer.add("inside");
    allThermometer.add("outside");
    allThermometerUpdate.add("inside");
    allThermometerUpdate.add("outside");
    /////////////////////////////////////////////////////////////////
    pinMode(iDomConst::GPIO_SPIK, OUTPUT);    // gpio pin do zasilania glosnikow
    digitalWrite(iDomConst::GPIO_SPIK,LOW);
    pinMode(iDomConst::GPIO_PRINTER,OUTPUT);  /// gpio pin do zsilania drukarki
    digitalWrite(iDomConst::GPIO_PRINTER,LOW);
    pinMode(iDomConst::BUTTON_PIN, INPUT);   //  gpio pin przycisku

    if (wiringPiISR (iDomConst::BUTTON_PIN, INT_EDGE_BOTH, &useful_F::button_interrupt) < 0 ) {
        log_file_cout.mutex_lock();
        log_file_cout << CRITICAL <<"Unable to setup ISR RISING "<<std::endl;
        log_file_cout.mutex_unlock();
    }
    my_data->main_iDomStatus->addObject("cameraLED",STATE::UNKNOWN);
    my_data->main_iDomStatus->addObject("printer",STATE::OFF);
    my_data->main_iDomStatus->addObject("speakers",STATE::OFF);
    my_data->main_iDomStatus->addObject("alarm",STATE::DEACTIVE);

    ///////// setup viber api
    m_viber.setAvatar(my_data->server_settings->viberAvatar);
    m_viber.setAccessToken(my_data->server_settings->viberToken);
    m_viber.setURL("https://chatapi.viber.com/pa/send_message");
    ///////// setup faceboook api
    m_facebook.setAccessToken(my_data->server_settings->facebookAccessToken);
}

TEMPERATURE_STATE iDomTOOLS::hasTemperatureChange(std::string thermometerName, double reference, double histereza )
{
    reference += 0.0055;
    const auto newTemp = allThermometer.getTemp(thermometerName);
    const auto oldTemp = allThermometer.getOldTemp(thermometerName);
    const auto lastState = allThermometer.getLastState(thermometerName);
    if (newTemp >= reference + histereza &&
            oldTemp  < reference + histereza &&
            lastState != TEMPERATURE_STATE::Over)
    {
        my_data->myEventHandler.run("test")->addEvent("over: new "+  to_string_with_precision(newTemp)+" old: "
                                                      +to_string_with_precision(oldTemp)+" ref: "+to_string_with_precision(reference));
        allThermometer.setState(thermometerName, TEMPERATURE_STATE::Over);
        return TEMPERATURE_STATE::Over;
    }
    else if (newTemp <= reference - histereza &&
             oldTemp >  reference - histereza &&
             lastState != TEMPERATURE_STATE::Under)
    {
        my_data->myEventHandler.run("test")->addEvent("under: new "+to_string_with_precision(newTemp)+" old: "
                                                      +to_string_with_precision(oldTemp)+" ref: "+to_string_with_precision(reference));
        allThermometer.setState(thermometerName, TEMPERATURE_STATE::Under);
        return TEMPERATURE_STATE::Under;
    }
    else
    {
        my_data->myEventHandler.run("test")->addEvent("noChanges: new "+to_string_with_precision(newTemp)+" old: "
                                                      +to_string_with_precision(oldTemp)+" ref: "+to_string_with_precision(reference));

        return TEMPERATURE_STATE::NoChanges;
    }
    my_data->myEventHandler.run("test")->addEvent("unknown: new "+to_string_with_precision(newTemp)+" old: "
                                                  +to_string_with_precision(oldTemp)+" ref: "+to_string_with_precision(reference));

    return TEMPERATURE_STATE::Unknown;
}

void iDomTOOLS::sendSMSifTempChanged(std::string thermomethernName, int reference)
{
    TEMPERATURE_STATE status = hasTemperatureChange(thermomethernName,reference,0.5);
    std::string m = "temperature "+thermomethernName+" over "+ EMOJI::emoji(E_emoji::NORTH_EAST_ARROW)
            + to_string_with_precision(reference);

    if (status == TEMPERATURE_STATE::Over){
        my_data->myEventHandler.run("temperature")->addEvent(m);
        sendViberMsg(m,my_data->server_settings->viberReceiver.at(0),my_data->server_settings->viberSender);
    }
    else if (status == TEMPERATURE_STATE::Under){
        m ="temperature " + thermomethernName+" under "+EMOJI::emoji(E_emoji::SOUTH_EAST_ARROW)
                +to_string_with_precision(reference);
        my_data->myEventHandler.run("temperature")->addEvent(m);
        if (reference < 2){
            sendViberPicture(m,"http://canacopegdl.com/images/cold/cold-14.jpg",
                             my_data->server_settings->viberReceiver.at(0),
                             my_data->server_settings->viberSender);
            sendViberPicture(m,"http://canacopegdl.com/images/cold/cold-14.jpg",
                             my_data->server_settings->viberReceiver.at(1),
                             my_data->server_settings->viberSender);
            postOnFacebook(m,"http://canacopegdl.com/images/cold/cold-14.jpg");
        }
        else {
            sendViberMsg(m,my_data->server_settings->viberReceiver.at(0),
                         my_data->server_settings->viberSender);
        }
    }
    else{
        //my_data->myEventHandler.run("unknown")->addEvent("temperatura nie przeszla przez "+to_string_with_precision(reference));
    }
}

std::string iDomTOOLS::getThermoStats(std::string name)
{
    return  allThermometerUpdate.getStatsByName(name);
}

void iDomTOOLS::updateTemperatureStats()
{
    auto v = getTemperature();
    allThermometerUpdate.updateAll(&v);
    allThermometerUpdate.updateStats("outside");
    allThermometerUpdate.updateStats("inside");

    if( true == allThermometerUpdate.isMoreDiff("outside",2.1)){
        auto  data = allThermometerUpdate.getLast2("outside");
        std::string msg = "alarm roznicy temeratur na polu! " + to_string_with_precision(data.first) +" na "+
                to_string_with_precision(data.second);

        if (data.first > data.second){
            msg += " temperatura maleje " + EMOJI::emoji(E_emoji::CHART_WITH_DOWNWARDS_TREND);
        }
        else{
            msg += " temperatura rośnie " + EMOJI::emoji(E_emoji::CHART_WITH_UPWARDS_TREND);
        }

        sendViberMsg(msg  ,
                     my_data->server_settings->viberReceiver.at(0),
                     my_data->server_settings->viberSender);

        log_file_mutex.mutex_lock();
        log_file_cout << WARNING << msg << std::endl;
        log_file_mutex.mutex_unlock();
    }
    if( true == allThermometerUpdate.isMoreDiff("inside",2.1)){
        auto  data = allThermometerUpdate.getLast2("inside");
        std::string msg = "alarm roznicy temeratur na mieszkaniu! " + to_string_with_precision(data.first) +" na "+
                to_string_with_precision(data.second);

        if (data.first > data.second){
            msg += " temperatura maleje " + EMOJI::emoji(E_emoji::CHART_WITH_DOWNWARDS_TREND);
        }
        else{
            msg += " temperatura rośnie " + EMOJI::emoji(E_emoji::CHART_WITH_UPWARDS_TREND);
        }

        sendViberMsg(msg  ,
                     my_data->server_settings->viberReceiver.at(0),
                     my_data->server_settings->viberSender);

        log_file_mutex.mutex_lock();
        log_file_cout << WARNING << msg << std::endl;
        log_file_mutex.mutex_unlock();
    }
}

void iDomTOOLS::turnOnSpeakers()
{
    if (useful_F::myStaticData->idom_all_state.houseState == STATE::UNLOCK)
    {
        digitalWrite(iDomConst::GPIO_SPIK, HIGH);
        useful_F::myStaticData->main_iDomStatus->setObjectState("speakers",STATE::ON);
    }
    else{
        useful_F::myStaticData->myEventHandler.run("speakers")->addEvent("speakers can not start due to home state: "+
                                                                         stateToString(useful_F::myStaticData->idom_all_state.houseState));
    }

}

void iDomTOOLS::turnOffSpeakers()
{
    digitalWrite(iDomConst::GPIO_SPIK, LOW);
    useful_F::myStaticData->main_iDomStatus->setObjectState("speakers",STATE::OFF);
}

void iDomTOOLS::turnOnPrinter()
{
    if (my_data->idom_all_state.houseState == STATE::UNLOCK)
    {
        digitalWrite(iDomConst::GPIO_PRINTER,HIGH);
        my_data->myEventHandler.run("230V")->addEvent("230v drukarki ON");
        my_data->main_iDomStatus->setObjectState("printer",STATE::ON);
    }
    else{
        my_data->myEventHandler.run("230V")->addEvent("Printer can not start due to home state: "+
                                                      stateToString(my_data->idom_all_state.houseState));
    }
}

void iDomTOOLS::turnOffPrinter()
{
    digitalWrite(iDomConst::GPIO_PRINTER,LOW);
    my_data->myEventHandler.run("230V")->addEvent("230v drukarki OFF");
    my_data->main_iDomStatus->setObjectState("printer",STATE::OFF);
}

PIN_STATE iDomTOOLS::getPinState(int pin_number)
{
    int pin_state = digitalRead(pin_number);

    switch (pin_state){
    case 0:
        return PIN_STATE::LOW_STATE;
    case 1:
        return PIN_STATE::HIGH_STATE;
    default:
        return PIN_STATE::UNKNOWN_STATE;
    }
}

void iDomTOOLS::turnOnOffPrinter()
{
    PIN_STATE pinState = getPinState(iDomConst::GPIO_PRINTER);
    switch (pinState){
    case PIN_STATE::HIGH_STATE:
        turnOffPrinter();
        my_data->mainLCD->set_lcd_STATE(10);
        my_data->mainLCD->printString(true,0,0,"230V OFF");
        break;
    case PIN_STATE::LOW_STATE:
        turnOnPrinter();
        my_data->mainLCD->set_lcd_STATE(10);
        my_data->mainLCD->printString(true,0,0,"230V ON");
        break;
    default:
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << " blad odczytu stanu pinu zasilania drukarki "<<   std::endl;
        log_file_mutex.mutex_unlock();
    }
}

void iDomTOOLS::turnOnOff433MHzSwitch(std::string name)
{
    STATE listwaState = my_data->main_iDomStatus->getObjectState(name);
    RADIO_SWITCH *m_switch = dynamic_cast<RADIO_SWITCH*>(my_data->main_REC->getEqPointer(name));
    if (listwaState == STATE::ON){
        my_data->mainLCD->set_lcd_STATE(10);
        my_data->mainLCD->printString(true,0,0,"230V OFF "+name);
        m_switch->off();
    }
    else if (listwaState == STATE::OFF){
        my_data->mainLCD->set_lcd_STATE(10);
        my_data->mainLCD->printString(true,0,0,"230V ON "+name);
        m_switch->on();
    }
}

void iDomTOOLS::turnOn433MHzSwitch(std::string name)
{
    RADIO_SWITCH *m_switch = dynamic_cast<RADIO_SWITCH*>(my_data->main_REC->getEqPointer(name));
    m_switch->on();
}

void iDomTOOLS::turnOff433MHzSwitch(std::string name)
{
    RADIO_SWITCH *m_switch = dynamic_cast<RADIO_SWITCH*>(my_data->main_REC->getEqPointer(name));
    m_switch->off();
}

void iDomTOOLS::runOnSunset()
{
    if (my_data->idom_all_state.houseState == STATE::UNLOCK)
    {
        ////switch 433mhz
        for (auto m_switch : my_data->main_REC->getSwitchPointerVector()){
            m_switch->onSunset();
        }
    }
    else{
        my_data->myEventHandler.run("433MHz")->addEvent("433MHz can not start due to home state: "+
                                                        stateToString(my_data->idom_all_state.houseState));
    }
}

void iDomTOOLS::runOnSunrise()
{
    if (my_data->idom_all_state.houseState == STATE::UNLOCK)
    {
        ////switch 433mhz
        for (auto m_switch : my_data->main_REC->getSwitchPointerVector()){
            m_switch->onSunrise();
        }
    }
    else{
        my_data->myEventHandler.run("433MHz")->addEvent("433MHz can not start due to home state: "+
                                                        stateToString(my_data->idom_all_state.houseState));
    }
}

void iDomTOOLS::lockHome()
{
    my_data->idom_all_state.houseState = STATE::LOCK;
    my_data->main_iDomStatus->setObjectState("house", STATE::LOCK);
    my_data->main_iDomTools->sendViberPicture("dom zablokownay!",
                                              "http://cyniu88.no-ip.pl/images/iDom/iDom/lock.jpg",
                                              my_data->server_settings->viberReceiver.at(0),
                                              my_data->server_settings->viberSender);
    log_file_mutex.mutex_lock();
    log_file_cout << INFO << "status domu - "+stateToString(my_data->idom_all_state.houseState)<<   std::endl;
    log_file_mutex.mutex_unlock();
}

void iDomTOOLS::unlockHome()
{
    my_data->idom_all_state.houseState = STATE::UNLOCK;
    my_data->main_iDomStatus->setObjectState("house", STATE::UNLOCK);
    my_data->main_iDomTools->sendViberPicture("dom odblokownay!",
                                              "http://cyniu88.no-ip.pl/images/iDom/iDom/unlock.jpg",
                                              my_data->server_settings->viberReceiver.at(0),
                                              my_data->server_settings->viberSender);
    log_file_mutex.mutex_lock();
    log_file_cout << INFO << "status domu - "+stateToString(my_data->idom_all_state.houseState)<<   std::endl;
    log_file_mutex.mutex_unlock();
}

std::string iDomTOOLS::getSunrise(bool extend )
{
    Clock tt = sun.getSunRise();
    if (extend == true){
        return "Sunrise time: "+tt.getString();
    }
    return tt.getString();
}

std::string iDomTOOLS::getSunset(bool extend )
{
    Clock tt = sun.getSunSet();
    if (extend == true){
        return  "Sunset time: "+tt.getString();
    }
    return tt.getString();
}

Clock iDomTOOLS::getSunsetClock()
{
    return sun.getSunSet();
}

Clock iDomTOOLS::getSunriseClock()
{
    return sun.getSunRise();
}

std::string iDomTOOLS::getDayLenght(bool extend )
{
    Clock tt = sun.getDayLength();
    if (extend == true){
        return "Day Lenght : "+tt.getString();
    }
    return tt.getString();
}

std::string iDomTOOLS::getSystemInfo()
{
    struct sysinfo info;
    sysinfo(&info);
    long input_seconds = info.uptime;
    auto days = input_seconds / 60 / 60 / 24;
    auto hours = (input_seconds / 60 / 60) % 24;
    auto minutes = (input_seconds / 60) % 60;
    auto seconds = input_seconds % 60;

    std::string  ret;
    ret=  "System uptime: " + std::to_string(days)+" day ";
    ret+=  std::to_string(hours) +" hours " ;
    ret+=  std::to_string(minutes)+ " minutes ";
    ret+=  std::to_string(seconds)+ " seconds ";
    ret.append("\n");
    ret+= "Load: ";
    ret+= std::to_string(info.loads[0]/65536)+"% - 1 min, "+ std::to_string(info.loads[1]/65536)+"% - 5 min, "+
            std::to_string(info.loads[2]/65536)+"% - 15 min.\n ";

    std::stringstream k;
    k <<"load average : "<<(info.loads[0] )
            <<" 5min: "  <<(info.loads[1] )
            <<" 15min: " <<(info.loads[2] )<<std::endl;
    // puts(k.str().c_str());
    return ret;
}

std::string iDomTOOLS::getWeatherEvent(std::string city, unsigned int radius)
{
    std::string url = "http://burze.dzis.net/ramka.php?miejscowosc=";
    url.append(city);
    url.append("&promien=");
    url.append(std::to_string(radius));
    return useful_F::httpPost(url, 10);
}

void iDomTOOLS::textToSpeach(std::vector<std::string> *textVector)
{
    if (textVector->empty() ){
        return;
    }
    std::string txt;

    for (auto a : *textVector){
        txt += a;
    }
    /////////// start thread  TTS - python use ////////////////////////
    std::string command = " python /home/pi/programowanie/iDom_server_OOP/script/PYTHON/gadacz.py ";
    command += "\""+ txt +"\"";
    if(my_data->ptr_MPD_info->isPlay){

    }
    else {
        turnOnSpeakers();
    }
    system(command.c_str());
    if(my_data->ptr_MPD_info->isPlay){

    }
    else {
        turnOffSpeakers();
    }
}

std::string iDomTOOLS::getTextToSpeach()
{
    std::vector<std::string> dayL = useful_F::split(getDayLenght(),':');
    std::string  text;
    std::string smogText = getSmog();
    int smogInt = std::stoi(smogText);
    text =  "Godzina: "+ Clock::getTime().getString();
    text += ". \nWschód słońca: "+getSunrise();
    text += ". \nZachód słońca: "+getSunset();
    text += ". \nDługość dnia: "+ dayL[0]+" godzin "+dayL[1]+" minut";
    text +=". \n";
    dayL = getTemperature();
    text += "Temperatura na zewnątrz: "+ dayL[1]+" stopnia. \n";
    text += "Temperatura w pokoju: "+ dayL[0]+" stopnia. \n";
    text += "Smog: "+ smogText +" mg/m^3. \n";
    if (smogInt > 50){
        int result = smogInt *2 ;
        text += "UWAGA! Maksymalna wartość przekroczona "+ std::to_string(result)+"%.";
    }
    return text;
}

std::vector<std::string> iDomTOOLS::getTemperature()
{
    std::vector<std::string>  vect = useful_F::split(useful_F::send_to_arduino(my_data,"temperature:22;"),':');
    return vect;
}

std::string iDomTOOLS::getTemperatureString()
{
    return useful_F::send_to_arduino(my_data,"temperature:22;");
}

std::string iDomTOOLS::getSmog()
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "www.smog.krakow.pl");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
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
    int start = readBuffer.find("<h2 class=\"polution\">");
    try {
        readBuffer = readBuffer.substr(start, 40);
    }
    catch (...){
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << "wyjatek substr() e getSmog() !!!!!!"<< std::endl;
        log_file_mutex.mutex_unlock();
    }

    readBuffer = find_tag(readBuffer);

    return readBuffer;
}

void iDomTOOLS::send_temperature_thingSpeak()
{
    std::vector<std::string> _temperature = getTemperature();
    std::string addres = "api.thingspeak.com/update?key=";
    addres+=key;
    addres+="&field1=";
    addres+= _temperature.at(0);
    // addres.erase(addres.size()-2,addres.size());
    addres+= "&field3="+_temperature.at(1);
    addres+="&field2="+getSmog();
    //////////////////////////////// pozyskanie temperatury
    ///
    ///
    //_temperature.erase(_temperature.size()-2,_temperature.size());

    allThermometer.updateAll(&_temperature);
    sendSMSifTempChanged("outside",0);
    sendSMSifTempChanged("inside",24);

    useful_F::httpPost(addres,10);
}

size_t iDomTOOLS::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string iDomTOOLS::find_tag(const std::string& temp)
{
    std::string value="";
    for (unsigned int i = 0; i<temp.size();++i){

        if (temp.at(i) =='>')
        {  int z = i+1;
            while (temp.at(z)!='<')
            {
                value+= temp.at(z);
                ++z;
            }
            break;
        }
    }
    return value;
}

std::string iDomTOOLS::sendSMStoPlusGSM(std::string login, std::string pass, std::string number,
                                        std::string msg,int silentFrom , int silentTo  )
{
    if (silentFrom !=0 && silentTo !=0){
        // TODO
    }
    std::replace(msg.begin(),msg.end(),' ','+');
    std::string address = "http://darsonserver.5v.pl/bramkaPlus?login=";
    address +=login+"&password="+pass+"&sender=iDom&number="+number+"&message="+msg;

    std::string readBuffer = useful_F::httpPost(address,10);


    log_file_mutex.mutex_lock();
    log_file_cout << INFO <<"wysłano SMSa otreśći: " <<  msg<<std::endl;
    log_file_mutex.mutex_unlock();
    return readBuffer +"\n"+address;
}

void iDomTOOLS::cameraLedON(std::string link)
{
    Clock t = Clock::getTime();
    SunRiseSet sun;
    Clock sunRise, sunSet;
    sunRise = sun.getSunRise();
    sunSet = sun.getSunSet();
    sunSet += Clock(23,30); // +23:30 == -00:30
    if (t <= sunRise || t >= sunSet){
        //printf("zapalam leda!\n");
        std::string s = useful_F::httpPost(link,10);
        if (s == "ok.\n"){
            my_data->main_iDomStatus->setObjectState("cameraLED",STATE::ON);
            //  printf("w ifie\n");
        }
    }
    // printf("nie odpalam leda!\n");
}

void iDomTOOLS::cameraLedOFF(std::string link)
{
    std::string s = useful_F::httpPost(link,10);
    //printf (" camera response '%s' \n", s.c_str());
    if (s == "ok.\n"){
        my_data->main_iDomStatus->setObjectState("cameraLED",STATE::OFF);
        //printf("w ifie\n");
    }
}

std::string iDomTOOLS::sendViberMsg(std::string msg,
                                    std::string receiver,
                                    std::string senderName,
                                    std::string accessToken,
                                    std::string url)
{
    std::lock_guard<std::mutex>  lock(m_msgMutex);
    return  m_viber.sendViberMSG(msg,receiver,senderName,accessToken,url);
}

std::string iDomTOOLS::sendViberPicture(std::string msg,
                                        std::string image,
                                        std::string receiver,
                                        std::string senderName,
                                        std::string accessToken,
                                        std::string url)
{
    std::lock_guard<std::mutex>  lock(m_msgMutex);
    return  m_viber.sendViberPicture(msg,image,receiver,senderName,accessToken,url);
}

std::string iDomTOOLS::postOnFacebook(std::string msg,std::string image)
{
    std::lock_guard<std::mutex>  lock(m_msgMutex);
    if (image != "NULL"){
        return m_facebook.postPhotoOnWall(image,msg);
    }

    return  m_facebook.postTxtOnWall(msg);
}

std::string iDomTOOLS::ledOFF()
{
    return useful_F::send_to_arduino(my_data,"LED_STOP:2;");
}

std::string iDomTOOLS::ledClear()
{
    return useful_F::send_to_arduino(my_data,"LED_CLEAR:2;");
}

std::string iDomTOOLS::ledOn(LED_Strip ledColor, unsigned int from, unsigned int to)
{
    if (my_data->idom_all_state.houseState == STATE::UNLOCK)
    {
        return useful_F::send_to_arduino(my_data,ledColor.get(from, to));
    }
    else{
        my_data->myEventHandler.run("LED")->addEvent("LED can not start due to home state: "+
                                                     stateToString(my_data->idom_all_state.houseState));
    }
    return "iDom LOCKED!";
}

void iDomTOOLS::checkAlarm()
{
    Clock now = Clock::getTime();
    if (now == my_data->alarmTime.time && my_data->alarmTime.state == STATE::ACTIVE){
        my_data->alarmTime.state = STATE::WORKING;
        MPD_volumeSet(my_data, 50);
        MPD_play(my_data);
        my_data->main_iDomStatus->setObjectState("alarm",STATE::DEACTIVE);
    }

    if (my_data->alarmTime.state == STATE::WORKING){
        int vol = MPD_getVolume(my_data) + 1;
        if (vol < 59){
            MPD_volumeSet(my_data, vol);
        }
        else{
            my_data->alarmTime.state = STATE::DEACTIVE;
            if(now < iDomTOOLS::getSunriseClock() || now > iDomTOOLS::getSunsetClock()){
                my_data->main_iDomTools->turnOn433MHzSwitch("ALARM");
                my_data->main_iDomTools->ledOn(my_data->ptr_pilot_led->colorLED[0],59,vol);
            }
        }
    }
}
