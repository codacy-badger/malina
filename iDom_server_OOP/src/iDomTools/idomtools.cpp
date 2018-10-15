#include <algorithm>
#include <fstream>
#include <string>
#include <typeinfo>

#include "idomtools.h"
#include "../functions/functions.h"
#include "../../libs/emoji/emoji.h"
#include "../../libs/Statistic/statistic.h"
#include "../CRON/cron.hpp"
#include "../RADIO_433_eq/radio_433_eq.h"
#include "json.hpp"

iDomTOOLS::iDomTOOLS(thread_data *myData): key(myData->server_settings->TS_KEY)
{
    puts("iDomTOOLS::iDomTOOLS()");
    my_data = myData;

    //////////////////////////////////// temeprature /////////////////

    allThermometer.add("inside");
    allThermometer.add("outside");
    allThermometerUpdate.add("inside");
    allThermometerUpdate.add("outside");
    /////////////////////////////////////////////////////////////////
#ifndef BT_TEST
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
#endif
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

    //////// button 433MHz
    buttonPointerVector = my_data->main_REC->getButtonPointerVector();

    lastButton433MHzLockUnlockTime = Clock::getTime() + Clock(23,58);

}

TEMPERATURE_STATE iDomTOOLS::hasTemperatureChange(const std::string& thermometerName, double reference, double histereza )
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
                                                      +to_string_with_precision(oldTemp)+" ref: "
                                                      +to_string_with_precision(reference));
        allThermometer.setState(thermometerName, TEMPERATURE_STATE::Over);
        return TEMPERATURE_STATE::Over;
    }
    else if (newTemp <= reference - histereza &&
             oldTemp >  reference - histereza &&
             lastState != TEMPERATURE_STATE::Under)
    {
        my_data->myEventHandler.run("test")->addEvent("under: new "+to_string_with_precision(newTemp)+" old: "
                                                      +to_string_with_precision(oldTemp)+" ref: "
                                                      +to_string_with_precision(reference));
        allThermometer.setState(thermometerName, TEMPERATURE_STATE::Under);
        return TEMPERATURE_STATE::Under;
    }

    my_data->myEventHandler.run("test")->addEvent("noChanges: new "+to_string_with_precision(newTemp)+" old: "
                                                  +to_string_with_precision(oldTemp)+" ref: "+to_string_with_precision(reference));

    allThermometer.setState(thermometerName, TEMPERATURE_STATE::NoChanges);
    return TEMPERATURE_STATE::NoChanges;
}

void iDomTOOLS::sendSMSifTempChanged(const std::string& thermomethernName, int reference)
{
    TEMPERATURE_STATE status = hasTemperatureChange(thermomethernName,reference,0.5);
    std::string m = "temperature "+thermomethernName+" over "+ EMOJI::emoji(E_emoji::NORTH_EAST_ARROW)
            + to_string_with_precision(reference);

    if (status == TEMPERATURE_STATE::Over){
        my_data->myEventHandler.run("temperature")->addEvent(m);
        if (reference < 2){
            sendViberMsg(m,my_data->server_settings->viberReceiver.at(0),my_data->server_settings->viberSender);
            sendViberMsg(m,my_data->server_settings->viberReceiver.at(1),my_data->server_settings->viberSender);
        }
        else{
            sendViberMsg(m,my_data->server_settings->viberReceiver.at(0),my_data->server_settings->viberSender);
        }
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

std::string iDomTOOLS::getThermoStats(const std::string& name)
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
#ifndef BT_TEST
        log_file_mutex.mutex_lock();
        log_file_cout << WARNING << msg << std::endl;
        log_file_mutex.mutex_unlock();
#endif
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
#ifndef BT_TEST
        log_file_mutex.mutex_lock();
        log_file_cout << WARNING << msg << std::endl;
        log_file_mutex.mutex_unlock();
#endif
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
    useful_F::myStaticData->main_iDomTools->saveState_iDom();
}

void iDomTOOLS::turnOffSpeakers()
{
    digitalWrite(iDomConst::GPIO_SPIK, LOW);
    useful_F::myStaticData->main_iDomStatus->setObjectState("speakers",STATE::OFF);
    // useful_F::myStaticData->main_iDomTools->saveState_iDom();
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
        puts("def");
#ifndef BT_TEST
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << " blad odczytu stanu pinu zasilania drukarki "<<   std::endl;
        log_file_mutex.mutex_unlock();
#endif
    }
}

void iDomTOOLS::turnOnOff433MHzSwitch(const std::string& name)
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
    saveState_iDom();
}

void iDomTOOLS::turnOn433MHzSwitch(std::string name)
{
    RADIO_SWITCH *m_switch = dynamic_cast<RADIO_SWITCH*>(my_data->main_REC->getEqPointer(std::move(name)));
    m_switch->on();
    //saveState_iDom();
}

void iDomTOOLS::turnOff433MHzSwitch(std::string name)
{
    RADIO_SWITCH *m_switch = dynamic_cast<RADIO_SWITCH*>(my_data->main_REC->getEqPointer(std::move(name)));
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
        my_data->myEventHandler.run("iDom")->addEvent("433MHz can not start due to home state: "+
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
        my_data->myEventHandler.run("iDom")->addEvent("433MHz can not start due to home state: "+
                                                      stateToString(my_data->idom_all_state.houseState));
    }
    my_data->main_iDomTools->ledOFF();
}

void iDomTOOLS::lockHome()
{
    my_data->idom_all_state.houseState = STATE::LOCK;
    my_data->main_iDomStatus->setObjectState("house", STATE::LOCK);
    my_data->main_iDomTools->sendViberPicture("dom zablokownay!",
                                              "http://cyniu88.no-ip.pl/images/iDom/iDom/lock.jpg",
                                              my_data->server_settings->viberReceiver.at(0),
                                              my_data->server_settings->viberSender);
#ifndef BT_TEST
    log_file_mutex.mutex_lock();
    log_file_cout << INFO << "status domu - "+stateToString(my_data->idom_all_state.houseState)<<   std::endl;
    log_file_mutex.mutex_unlock();
#endif
    saveState_iDom();
}

void iDomTOOLS::unlockHome()
{
    my_data->idom_all_state.houseState = STATE::UNLOCK;
    my_data->main_iDomStatus->setObjectState("house", STATE::UNLOCK);
    my_data->main_iDomTools->sendViberPicture("dom odblokownay!",
                                              "http://cyniu88.no-ip.pl/images/iDom/iDom/unlock.jpg",
                                              my_data->server_settings->viberReceiver.at(0),
                                              my_data->server_settings->viberSender);
#ifndef BT_TEST
    log_file_mutex.mutex_lock();
    log_file_cout << INFO << "status domu - "+stateToString(my_data->idom_all_state.houseState)<<   std::endl;
    log_file_mutex.mutex_unlock();
#endif
    saveState_iDom();
}

void iDomTOOLS::switchActionOnLockHome()
{
    ////switch 433mhz
    for (auto m_switch : my_data->main_REC->getSwitchPointerVector()){
        m_switch->onLockHome();
    }
}

void iDomTOOLS::switchActionOnUnlockHome()
{
    ////switch 433mhz
    for (auto m_switch : my_data->main_REC->getSwitchPointerVector()){
        m_switch->onUnlockHome();
    }
}

std::string iDomTOOLS::buttonPressed(const std::string& id)
{
    for (auto n : buttonPointerVector){
        if (id == n->getID()){
            return n->getName();
        }
    }
    throw "UNKNOWN BUTTON ID: " + std::to_string(id);
}

void iDomTOOLS::button433MHzPressedAction(const std::string& name)
{
    if (name == "locker"){
        RADIO_BUTTON* buttonLocker = static_cast<RADIO_BUTTON*>(my_data->main_REC->getEqPointer(name) );
        button433mhzLockerPressed(buttonLocker);
    }
}

void iDomTOOLS::button433mhzLockerPressed(RADIO_BUTTON *radioButton)
{
    static unsigned int counter = 0;

    Clock t  = Clock::getTime();
    if (lastButton433MHzLockUnlockTime != t /*|| (lastButton433MHzLockUnlockTime + Clock(0,1)) == t*/)
    {
        //#ifdef BT_TEST
        std::cout << "LOCKER TEST iDomTOOLS::button433mhzLockerPressed()" <<std::endl;
        //#endif
        lastButton433MHzLockUnlockTime = t;
        counter = 0;
        if(my_data->idom_all_state.houseState != STATE::UNLOCK)
        {
            buttonUnlockHome();
            puts("\nodblokuje dom\n");
            radioButton->setState(STATE::UNLOCK);
        }
        else if (my_data->main_iDomStatus->getObjectState("music") == STATE::PLAY)
        {
            ledOFF();
            MPD_stop();
            turnOffPrinter();
            radioButton->setState(STATE::STOP);
            switchActionOnLockHome();
            //TODO  dodać wylaczanie wiatraka
        }
        else if (my_data->main_iDomStatus->getObjectState("music") == STATE::STOP)
        {
            MPD_play(my_data);
            switchActionOnUnlockHome();
            if(isItDay() == false)
            {
                ledOn(my_data->ptr_pilot_led->colorLED[2]);
            }
            radioButton->setState(STATE::PLAY);
        }
    }
    else
    {
        ++counter;
        if (counter == 2)
        {
            buttonLockHome();
            puts("\nzablokuje dom\n");
            radioButton->setState(STATE::LOCK);
        }
        //#ifdef BT_TEST
        std::cout << "LOCKER TEST iDomTOOLS::button433mhzLockerPressed()- counter: "<<counter <<std::endl;
        //#endif
    }

}

void iDomTOOLS::buttonLockHome()
{
    ledOFF();
    MPD_stop();
    turnOffPrinter();
    lockHome();
}

void iDomTOOLS::buttonUnlockHome()
{
    unlockHome();
    MPD_play(my_data);
    if(isItDay() == false){
        ledOn(my_data->ptr_pilot_led->colorLED[2]);
    }
}

bool iDomTOOLS::isItDay()
{
    Clock now = Clock::getTime();
    if(now < iDomTOOLS::getSunriseClock() || now > iDomTOOLS::getSunsetClock()){
        return false;
    }
    return true;
}

CARDINAL_DIRECTIONS::ALARM_INFO iDomTOOLS::getLightningStruct()
{
    std::lock_guard<std::mutex>  lock(m_lightningMutex);
    return m_lightningStruct;
}

void iDomTOOLS::setLightningStruct(CARDINAL_DIRECTIONS::ALARM_INFO &s)
{
    std::lock_guard<std::mutex>  lock(m_lightningMutex);
    //std::cout <<"struktura setowana " << s.data.str() <<std::endl;
    m_lightningStruct = s;

    //std::cout <<"struktura już po setowaniu " << m_lightningStruct.data.str() <<std::endl;
}

void iDomTOOLS::checkLightning()
{
    nlohmann::json jj = useful_F_libs::getJson(my_data->server_settings->lightningApiURL);

    CARDINAL_DIRECTIONS::ALARM_INFO lightningData = lightning.lightningAlert(jj);
    setLightningStruct(lightningData);
    bool result = lightning.checkLightningAlert(&lightningData);

    if(result == true)
    {

        m_viber.setAvatar("http://cyniu88.no-ip.pl/avatar/lightning.jpg");
        STATE stateMSG = sendViberMsgBool("UWAGA BURZA KOŁO KRAKOWA! "+EMOJI::emoji(E_emoji::THUNDER_CLOUD_AND_RAIN)
                                          +"\\n\\n "+lightningData.data.str() ,
                                          my_data->server_settings->viberReceiver.at(0),
                                          my_data->server_settings->viberSender);

        stateMSG = sendViberMsgBool("UWAGA BURZA KOŁO KRAKOWA! "+EMOJI::emoji(E_emoji::THUNDER_CLOUD_AND_RAIN)
                                    +"\\n\\n "+lightningData.data.str() ,
                                    my_data->server_settings->viberReceiver.at(1),
                                    my_data->server_settings->viberSender);

        m_viber.setAvatar(my_data->server_settings->viberAvatar);
        if(stateMSG == STATE::SEND_OK)
        {
#ifndef BT_TEST
            log_file_mutex.mutex_lock();
            log_file_cout << INFO << "wysłano informacje o burzy"<< std::endl;
            log_file_mutex.mutex_unlock();
#endif
        }
        else
        {
#ifndef BT_TEST
            log_file_mutex.mutex_lock();
            log_file_cout << ERROR << "nie wysłano informacje o burzy"<< std::endl;
            log_file_mutex.mutex_unlock();
#endif
        }
    }
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

std::string iDomTOOLS::getWeatherEvent(const std::string& city, unsigned int radius)
{
    std::string url = "http://burze.dzis.net/ramka.php?miejscowosc=";
    url.append(city);
    url.append("&promien=");
    url.append(std::to_string(radius));
    return useful_F_libs::httpPost(url, 10);
}

std::vector<WEATHER_ALER> iDomTOOLS::getAlert(std::string data)
{
    std::vector<WEATHER_ALER> wAlert;
    WEATHER_ALER tempWA;
    std::string d = useful_F_libs::removeHtmlTag(data);
    std::vector<std::string> vect;

    vect =  useful_F::split(d,'\n');
    vect.pop_back();
    for (auto n : vect)
    {
        if (n.find("brak") == std::string::npos)
        {
            tempWA.alert = n;
            tempWA.name = n;
            tempWA.sended = false;
            wAlert.push_back(tempWA);
        }
    }
    return {wAlert};
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
    std::stringstream  text;
    std::string smogText = getSmog();
    int smogInt = std::stoi(smogText);
    text <<  "Godzina: " << Clock::getTime().getString();
    text << ". \nWschód słońca: " << getSunrise();
    text << ". \nZachód słońca: " << getSunset();
    text << ". \nDługość dnia: " << dayL[0] << " godzin " << dayL[1] << " minut";
    text <<". \n";
    dayL = getTemperature();
    text << "Temperatura na zewnątrz: " << dayL[1] << " stopnia. \n";
    text << "Temperatura w pokoju: " << dayL[0] << " stopnia. \n";
    text << "Smog: " << smogText << " mg/m^3. \n";
    if (smogInt > 50){
        int result = smogInt *2 ;
        text << "UWAGA! Maksymalna wartość przekroczona " << result << "%.";
    }
    return text.str();
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
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, useful_F_libs::WriteCallback);
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
#ifndef BT_TEST
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << "wyjatek substr() e getSmog() !!!!!!"<< std::endl;
        log_file_mutex.mutex_unlock();
#endif
    }

    readBuffer = useful_F_libs::find_tag(readBuffer);

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
    allThermometer.updateAll(&_temperature);
    sendSMSifTempChanged("outside",0);
    sendSMSifTempChanged("inside",24);
    std::string s = useful_F_libs::httpPost(addres,10);

    if(s == "0"){
#ifndef BT_TEST
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << " błąd wysyłania temoeratury na thingspeak: "<< s <<   std::endl;
        log_file_mutex.mutex_unlock();
#endif
    }
}

std::string iDomTOOLS::sendSMStoPlusGSM(const std::string &login, const std::string &pass, const std::string &number,
                                        std::string msg, int silentFrom , int silentTo  )
{
    if (silentFrom !=0 && silentTo !=0){
        // TODO
    }
    std::replace(msg.begin(),msg.end(),' ','+');
    std::string address = "http://darsonserver.5v.pl/bramkaPlus?login=";
    address +=login+"&password="+pass+"&sender=iDom&number="+number+"&message="+msg;

    std::string readBuffer = useful_F_libs::httpPost(address,10);

#ifndef BT_TEST
    log_file_mutex.mutex_lock();
    log_file_cout << INFO <<"wysłano SMSa otreśći: " <<  msg<<std::endl;
    log_file_mutex.mutex_unlock();
#endif
    return readBuffer +"\n"+address;
}

void iDomTOOLS::cameraLedON(const std::string& link)
{
    Clock t = Clock::getTime();
    SunRiseSet sun;
    Clock sunRise, sunSet;
    sunRise = sun.getSunRise();
    sunSet = sun.getSunSet();
    sunSet += Clock(23,30); // +23:30 == -00:30
    if (t <= sunRise || t >= sunSet){
        //printf("zapalam leda!\n");
        std::string s = useful_F_libs::httpPost(link,10);
        if (s == "ok.\n"){
            my_data->main_iDomStatus->setObjectState("cameraLED",STATE::ON);
            //  printf("w ifie\n");
        }
    }
    // printf("nie odpalam leda!\n");
}

void iDomTOOLS::cameraLedOFF(const std::string& link)
{
    std::string s = useful_F_libs::httpPost(link,10);
    //printf (" camera response '%s' \n", s.c_str());
    if (s == "ok.\n"){
        my_data->main_iDomStatus->setObjectState("cameraLED",STATE::OFF);
        //printf("w ifie\n");
    }
}

nlohmann::json iDomTOOLS::sendViberMsg(const std::string &msg,
                                       const std::string &receiver,
                                       const std::string &senderName,
                                       const std::string& accessToken,
                                       const std::string& url)
{
    nlohmann::json jj;
    std::lock_guard<std::mutex>  lock(m_msgMutex);
    jj = nlohmann::json::parse( m_viber.sendViberMSG(msg,receiver,senderName,accessToken,url));
    return jj;
}

nlohmann::json iDomTOOLS::sendViberPicture(const std::string &msg,
                                           const std::string &image,
                                           const std::string &receiver,
                                           const std::string &senderName,
                                           const std::string& accessToken,
                                           const std::string& url)
{
    nlohmann::json jj;
    std::lock_guard<std::mutex>  lock(m_msgMutex);
    jj = nlohmann::json::parse(m_viber.sendViberPicture(msg,image,receiver,senderName,accessToken,url));
    return jj;
}

STATE iDomTOOLS::sendViberMsgBool(const std::string &msg,
                                  const std::string &receiver,
                                  const std::string &senderName,
                                  const std::string& accessToken,
                                  const std::string& url)
{
    nlohmann::json jj = sendViberMsg(msg,receiver,senderName,accessToken,url);
    STATE ret = STATE::SEND_NOK;
    if(jj.find("status_message")!= jj.end())
    {
        if(jj.at("status_message").get<std::string>() == "ok")
        {
            ret = STATE::SEND_OK;
        }
    }
    else
    {
#ifndef BT_TEST
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR << "nie wyslanno wiadomosci viber"<< jj.dump()<< std::endl;
        log_file_mutex.mutex_unlock();
#endif
    }
    return ret;
}

STATE iDomTOOLS::sendViberPictureBool(const std::string& msg,
                                      const std::string& image,
                                      const std::string& receiver,
                                      const std::string& senderName,
                                      const std::string& accessToken,
                                      const std::string& url)
{
    nlohmann::json jj = sendViberPicture(msg,image,receiver,senderName,accessToken,url);
    STATE ret = STATE::SEND_NOK;
    if(jj.at("status_message").get<std::string>() == "ok")
    {
        ret = STATE::SEND_OK;
    }
    else
    {
#ifndef BT_TEST
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR << "nie wyslanno wiadomosci viber"<< jj.dump()<< std::endl;
        log_file_mutex.mutex_unlock();
#endif
    }
    return ret;
}

std::string iDomTOOLS::postOnFacebook(const std::string& msg, const std::string& image)
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

std::string iDomTOOLS::ledClear(unsigned int from, unsigned int to)
{
    LED_Strip ledColor(0,60,0,0,0,"BLACK");
    useful_F::send_to_arduino(my_data,ledColor.get(from, to));
    return "Led cleared";
}

std::string iDomTOOLS::ledOn(const LED_Strip& ledColor, unsigned int from, unsigned int to)
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
    unsigned int fromVol = my_data->alarmTime.fromVolume;
    unsigned int   toVol = my_data->alarmTime.toVolume;
    unsigned int radioId = my_data->alarmTime.radioID;

    Clock now = Clock::getTime();

    if (now == my_data->alarmTime.time && my_data->alarmTime.state == STATE::ACTIVE){
        my_data->alarmTime.state = STATE::WORKING;
        MPD_volumeSet(my_data, fromVol);
        MPD_play(my_data,radioId);
        my_data->main_iDomStatus->setObjectState("alarm",STATE::DEACTIVE);
    }

    if (my_data->alarmTime.state == STATE::WORKING){
        unsigned int vol = MPD_getVolume(my_data) + 1;
        if (vol < toVol){
            MPD_volumeSet(my_data, vol);

            if(iDomTOOLS::isItDay() == false){
                my_data->main_iDomTools->ledOn(my_data->ptr_pilot_led->colorLED[2],fromVol,vol);
            }
        }
        else{
            my_data->alarmTime.state = STATE::DEACTIVE;
            if(iDomTOOLS::isItDay() == false){
                my_data->main_iDomTools->turnOn433MHzSwitch("ALARM");
            }
        }
    }
}

void iDomTOOLS::saveState_iDom()
{
    iDom_SAVE_STATE info(my_data->server_settings->saveFilePath);
    nlohmann::json jsonAlarm;
    nlohmann::json jsonMPD;
    nlohmann::json json_iDomLOCK;
    nlohmann::json json433Mhz;
    //////////////////// iDom
    json_iDomLOCK["iDomLock"] = stateToString(my_data->idom_all_state.houseState);
    //////////////////// alarm
    jsonAlarm["alarm"] = my_data->main_iDomStatus->getObjectStateString("alarm");
    jsonAlarm["time"]  = my_data->alarmTime.time.getString();
    jsonAlarm["fromVolume"] = my_data->alarmTime.fromVolume;
    jsonAlarm["toVolume"] = my_data->alarmTime.toVolume;
    jsonAlarm["radioID"] = my_data->alarmTime.radioID;
    //////////////////// mpd
    jsonMPD["music"] = my_data->main_iDomStatus->getObjectStateString("music");
    jsonMPD["speakers"] = my_data->main_iDomStatus->getObjectStateString("speakers");
    ////////////////// 433Mhz
    auto switch433vector = my_data->main_REC->getSwitchPointerVector();
    for (auto v : switch433vector)
    {
        v->getName();
        json433Mhz[v->getName()] = stateToString(v->getState());
    }
    ///
    nlohmann::json json;
    json["iDom"] = json_iDomLOCK;
    json["ALARM"] = jsonAlarm;
    json["MPD"] = jsonMPD;
    json["433Mhz"] = json433Mhz;

    info.write(json);
#ifdef BT_TEST
    std::cout << json <<std::endl;
    std::cout << " saved to " << my_data->server_settings->saveFilePath <<std::endl;
#endif
}

void iDomTOOLS::readState_iDom()
{
    try
    {
        iDom_SAVE_STATE info(my_data->server_settings->saveFilePath);
        nlohmann::json jj = info.read();
#ifdef BT_TEST
        std::cout << "JSON: " << jj.dump(4) << std::endl;
#endif
        nlohmann::json json433MHz = jj.at("433Mhz");

        for (nlohmann::json::iterator it = json433MHz.begin(); it != json433MHz.end(); ++it)
        {
            if( it.value() == "ON"){
                my_data->main_iDomTools->turnOn433MHzSwitch(it.key());
            }
            else if ( it.value() == "OFF"){
                my_data->main_iDomTools->turnOff433MHzSwitch(it.key());
            }
        }
        auto iDomLock = jj.at("iDom").at("iDomLock").get<std::string>();

        if(iDomLock == "UNLOCK")
            unlockHome();
        else if (iDomLock == "LOCK")
            lockHome();

        auto mpdMusic = jj.at("MPD").at("music").get<std::string>();
        auto mpdSpeakers = jj.at("MPD").at("speakers").get<std::string>();

        if(mpdMusic == "PLAY")
            MPD_play(my_data);
        else if(mpdMusic == "STOP")
            MPD_stop();
        if(mpdSpeakers == "ON")
            turnOnSpeakers();
        else if(mpdSpeakers == "OFF")
            turnOffSpeakers();

        auto alarmState = jj.at("ALARM").at("alarm").get<std::string>();
        auto alarmTime  = jj.at("ALARM").at("time").get<std::string>();
        my_data->alarmTime.fromVolume = jj.at("ALARM").at("fromVolume").get<int>();
        my_data->alarmTime.toVolume = jj.at("ALARM").at("toVolume").get<int>();
        my_data->alarmTime.radioID = jj.at("ALARM").at("radioID").get<int>();

        if (alarmState == "ACTIVE"){
            my_data->alarmTime.time = Clock(alarmTime);
            my_data->alarmTime.state = STATE::ACTIVE;
            my_data->main_iDomStatus->setObjectState("alarm", my_data->alarmTime.state);
            saveState_iDom();
        }
    }
    catch(...)
    {
#ifndef BT_TEST
        log_file_mutex.mutex_lock();
        log_file_cout << ERROR << "nie ma pliku json z stanem iDom"<< std::endl;
        log_file_mutex.mutex_unlock();
#endif
#ifdef BT_TEST
        std::cout << "nie ma pliku json z stanem iDom" << std::endl;
#endif
    }
}

