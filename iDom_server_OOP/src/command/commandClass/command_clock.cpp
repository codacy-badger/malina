#include "command_clock.h"
#include "../../../src/functions/functions.h"

command_clock::command_clock(const std::string &name):command(name)
{

}

std::string command_clock::execute(std::vector<std::string> &v, thread_data *my_data)
{
    if(v.size()<2)
        return "can not set clock";
    std::stringstream ss;
    ss << "TEXT:" << v.at(1) << ";" ;
    my_data->main_Rs232->print(ss.str());
    return "done";
}

std::string command_clock::help() const
{
    return "clock <number/string> - put number/string (max 4 digits) to 7segment LCD\n";
}
