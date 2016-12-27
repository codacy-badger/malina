#include "command_idom.h"

command_iDom::command_iDom(std::string name):command(name)
{

}

std::string command_iDom::execute(std::vector<std::string> &v, thread_data *my_data)
{
    if (v[1]=="speakers")
    {
        if (v[2] =="ON"){
            my_data->main_iDomTools->turnOnSpeakers();
            return "speakers ON";
        }
        else if (v[2]=="OFF"){
            my_data->main_iDomTools->turnOffSpeakers();
            return "speakers OFF";
        }
        else{
            return "unknow speakers action: "+v[2];
        }
    }
    else if (v[1]=="sunset"){
        return my_data->main_iDomTools->getSunset();
    }
    else if (v[1]=="sunrise"){
        return my_data->main_iDomTools->getSunrise();
    }
    else if (v[1]=="day" && v[2]=="lenght"){
        return my_data->main_iDomTools->getDayLenght();
    }
    else if (v[1]=="sun"){
        std::string ret;
        ret = my_data->main_iDomTools->getSunrise();
        ret.append("\n");
        ret.append(my_data->main_iDomTools->getSunset());
        ret.append("\n");
        ret.append(my_data->main_iDomTools->getDayLenght());
        ret.append("\n");
        return ret;
    }
    else if (v[1]=="sysinfo"){
        return my_data->main_iDomTools->getSystemInfo();
    }

    return " unknown command "+ v[1];
}

std::string command_iDom::help()
{
    std::string ret = "iDom - for control smart home\n";
    ret.append("iDom speakers ON/OFF - to on or off speakers\n");
    ret.append("iDom sunset/sunrise/day lenght  - to show those parameters\n");
    ret.append("iDom sun  - get sunrise, sunset and day lenght\n");
    ret.append("iDom sysinfo  - geg system info \n");
    return ret;
}
