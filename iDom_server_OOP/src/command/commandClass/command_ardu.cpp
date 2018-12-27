#include "command_ardu.h"

command_ardu::command_ardu(const std::string &name):command(name)
{
    this->m_mainRadioButton = std::nullptr_t(); //TODO zamienic na wektor przyciskow
}

command_ardu::command_ardu(const std::string& name, thread_data *my_data):command(name)
  ,m_button433MHzVector(my_data->main_REC->getButtonPointerVector())
{
    //m_button433MHzVector = my_data->main_REC->getButtonPointerVector();
    m_mainRadioButton = static_cast<RADIO_BUTTON*>(my_data->main_REC->getEqPointer("locker"));

    m_weatherStVe = my_data->main_REC->getWeather_StationPtrVector();
    m_mainWeatherStation = static_cast<RADIO_WEATHER_STATION*>(my_data->main_REC->getEqPointer("first"));
}

std::string command_ardu::execute(std::vector<std::string> &v, thread_data *my_data)
{
    std::string str_buf = " only for internal usage!";
    if (v.size() > 1){
        if(v[1] == "show"){
            RADIO_WEATHER_STATION* st = static_cast<RADIO_WEATHER_STATION*>(my_data->main_REC->getEqPointer("first"));
            str_buf = st->data.getDataString();
        }
        if(v[1] == "433MHz"){
            my_data->myEventHandler.run("433MHz")->addEvent("RFLink: "+v[2]);
            try {
                my_data->main_RFLink->
                        rflinkMAP[my_data->main_RFLink->getArgumentValueFromRFLinkMSG(v[2],
                        "ID")].counter();
                my_data->main_RFLink->
                        rflinkMAP[my_data->main_RFLink->getArgumentValueFromRFLinkMSG(v[2],
                        "ID")].msg = v[2];
            }
            catch(const std::string& e){
                std::cout << "wyjatek w szukaniu: " << e <<std::endl;
                pingAndOkRecv( my_data, v[2]);
            }
            //TODO add command
            try {
                if (m_mainRadioButton->getID() == my_data->main_RFLink->getArgumentValueFromRFLinkMSG(v[2],"ID") )
                {
                    my_data->main_iDomTools->button433mhzLockerPressed(m_mainRadioButton);
                }
            }
            catch (const std::string& e){ }
            try {
                if (m_mainWeatherStation->getID() == my_data->main_RFLink->getArgumentValueFromRFLinkMSG(v[2],"ID") )
                {
                    m_mainWeatherStation->data.putData(v[2]);
                }
            }
            catch (std::string& e){ }
        }
    }
    return str_buf;
}

std::string command_ardu::help()
{
    return " only for internal usege\n";
}

void command_ardu::pingAndOkRecv(thread_data *my_data, const std::string& s)
{
    if (s.find("OK;") != std::string::npos)
        my_data->main_RFLink->okTime = Clock::getUnixTime();
    else if (s.find("PONG;") != std::string::npos)
        my_data->main_RFLink->pingTime = Clock::getUnixTime();
    std::cout << "poing && ok recv " << s <<std::endl
              << " ok time: "<<my_data->main_RFLink->okTime<< std::endl
              << " ping time: "<<my_data->main_RFLink->pingTime<< std::endl;
}
