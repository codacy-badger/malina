#include <curl/curl.h>
#include "command_idom.h"

command_iDom::command_iDom(std::string name):command(name)
{

}

std::string command_iDom::execute(std::vector<std::string> &v, thread_data *my_data)
{
    if (v.size()<2){
        return "need parameter!";
    }
    if (v[1]=="speakers")
    {
        if (v[2] =="ON"){
            iDomTOOLS::turnOnSpeakers();
            return "speakers ON";
        }
        else if (v[2]=="OFF"){
            iDomTOOLS::turnOffSpeakers();
            return "speakers OFF";
        }
        else{
            return "unknow speakers action: "+v[2];
        }
    }
    else if (v[1]=="sunset"){
        return my_data->main_iDomTools->getSunset(true);
    }
    else if (v[1]=="sunrise"){
        return my_data->main_iDomTools->getSunrise(true);
    }
    else if (v[1]=="day" && v[2]=="lenght"){
        return my_data->main_iDomTools->getDayLenght(true);
    }
    else if (v[1]=="sun"){
        std::string ret;
        ret = my_data->main_iDomTools->getSunrise(true);
        ret.append("\n");
        ret.append(my_data->main_iDomTools->getSunset(true));
        ret.append("\n");
        ret.append(my_data->main_iDomTools->getDayLenght(true));
        ret.append("\n");
        return ret;
    }
    else if (v[1]=="sysinfo"){
        return my_data->main_iDomTools->getSystemInfo();
    }
    else if (v[1]=="temperature"){
        return my_data->main_iDomTools->getTemperatureString();
    }
    else if (v[1]=="sms"){
        if (v.size() <3){
            return "function not ready yet";
        }
        else{
            std::string m;
            for(size_t i = 2; i<v.size();++i){
                m+= v[i]+" ";
            }
            return my_data->main_iDomTools->sendSMStoPlusGSM("yanosik-info","yanosik24","782490815",m);
        }
    }
    else if (v[1]=="text"){
        return my_data->main_iDomTools->getTextToSpeach();
    }
    else if (v[1]=="LED"){
        if (v.size() != 7){
            if (v[2]=="OFF"){
                return my_data->main_iDomTools->ledOFF();
            }
            else{
                return "need more parameter from-to-R-G-B";
            }
        }
        else {
            LED_Strip strip(v[2],v[3],v[4],v[5],v[6]);
            return my_data->main_iDomTools->ledOn(strip);
        }
    }
    else if (v[1]=="say"){
        if (v.size()<3){
            std::vector<std::string> vTTS ={ my_data->main_iDomTools->getTextToSpeach()};
            my_data->main_iDomTools->textToSpeach(&vTTS);
        }
    }
    else if (v[1]=="smog"){
        return my_data->main_iDomTools->getSmog()+" mg/m^3";
    }
    else if (v[1]=="230V"){
        if (v.size() > 2 && v[2]=="ON"){
            my_data->main_iDomTools->turnOnPrinter();
            return "230V ON";
        }
        else if(v.size() > 2 && v[2]=="OFF"){
            my_data->main_iDomTools->turnOffPrinter();
            return "230V OFF";
        }
        else {
            return "wrong paramiter";
        }
    }
    else if (v[1]=="wifi"){
        std::string readBuffer = my_data->main_iDomTools->httpPost("http://cyniu88.no-ip.pl/cgi-bin/kto_wifi.sh",10);
        return readBuffer;
    }
    else if (v[1]=="kill"){

        if (v[2]=="thread"){
            my_data->main_THREAD_arr[std::stoi(v[3]) ].thread.~thread();
            return "done!";
        }
    }
    else if (v[1]=="facebook"){
        std::string msg;
        for (unsigned int i = 2 ; i < v.size(); ++i){
            msg+=" ";
            msg+=v[i];
        }
        return my_data->main_iDomTools->postOnFacebook(msg);
    }
    else if (v[1]=="viber"){
        std::string msg;
        for (unsigned int i = 2 ; i < v.size(); ++i){
            msg+=" ";
            msg+=v[i];
        }
        return my_data->main_iDomTools->sendViberMsg(msg, my_data->server_settings->viberReceiver,my_data->server_settings->viberSender);
    }
    else if (v[1]=="camera"){

        if (v.size() < 4){
            return "not enough parameters";
        }
        if (v[2]=="LED" && v[3] == "ON"){
            my_data->main_iDomTools->cameraLedON(my_data->server_settings->cameraLedON);
        }
        else if (v[2]=="LED" && v[3] == "OFF"){
            my_data->main_iDomTools->cameraLedOFF(my_data->server_settings->cameraLedOFF);
        }
        return "led DONE";
    }
    return "iDom - unknown parameter: "+ v[1];
}

std::string command_iDom::help()
{
    std::string ret = "iDom - for control smart home\n";
    ret.append("iDom speakers ON/OFF - to on or off speakers\n");
    ret.append("iDom 230v ON/OFF     - to on or off printers");
    ret.append("iDom sunset/sunrise/day lenght  - to show those parameters\n");
    ret.append("iDom sun        - get sunrise, sunset and day lenght\n");
    ret.append("iDom sysinfo    - get system info \n");
    ret.append("iDom text       - get text to speach\n");
    ret.append("iDom say <text> - say standatrd info or <text> \n");
    ret.append("iDom sms <text> - send sms<text> \n");
    ret.append("iDom LED <FROM> <TO> <R> <G> <B> - set RGB LED strip\n");
    ret.append("iDom LED OFF    - led off\n");
    ret.append("iDom temperature - get temperature from all termomether\n");
    ret.append("iDom smog       - get current SMOG level (KRAKOW)\n");
    ret.append("iDom kill thread <ID>  - kill thread\n");
    ret.append("iDom camera LED ON/OFF - LED camera work\n");
    ret.append("iDom facebook ... - post on facebook wall\n");
    ret.append("iDom viber ...   - send viber msg\n");
    return ret;
}
