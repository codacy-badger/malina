#include "command_wifi.h"

command_wifi::command_wifi(const std::string &name):command(name)
{
}

std::string command_wifi::execute(std::vector<std::string> &v, thread_data *my_data)
{
    return "done";
}

std::string command_wifi::help()
{
    return "internal wifi command";
}
