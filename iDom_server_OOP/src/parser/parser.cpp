#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "parser.hpp"


config read_config  ( const char* file_path    )
{
    std::string content ;
    std::string variable ;
    std::string v_value ;
    std::fstream config_file;

    config  v_set;

    config_file.open(file_path, std::ios::in  );
    if( config_file.good() == false )
    {
        log_file_mutex.mutex_lock();
        log_file_cout << "Brak pliku konfiguracyjnego" << std::endl;
        log_file_mutex.mutex_unlock();

        return v_set;
    }

    while( !config_file.eof() )
    {
        getline (config_file, content);

        for (unsigned i=0;  i<content.length(); ++i)
        {
            if ( content.at(i)==':' && content.at(i+1)=='=')
            {   for (unsigned int j=i+2;j<content.length(); ++j )
                {
                    if ( content.at(j)==' '|| content.at(j)==0x9 ){   // przerwij odczyt  jesli znajdzesz spacje lub tab
                        break;
                    }
                    // std::cout << (int)content.at(j);
                    v_value.push_back(content.at(j));  // v_value zmiennej
                }
                break;
            }
            if (content.at(i)!=' ' )
            {
                variable.push_back(content.at(i));  // variable
            }
        }
        if (variable == "portRS232")
        {
            v_set.portRS232= v_value;
        }
        if (variable == "portRS232_clock")
        {
            v_set.portRS232_clock= v_value;
        }
        if (variable == "ID")
        {
            v_set.ID_server = std::stoi(  v_value);
        }
        if (variable == "SERVER_IP")
        {
            v_set.SERVER_IP= v_value;
        }
        if (variable == "MOVIES_DB_PATH")
        {
            v_set.MOVIES_DB_PATH= v_value;
        }
        if (variable == "MENU_PATH")
        {
            v_set.MENU_PATH=v_value;
        }
        if (variable == "TS_KEY"){
            v_set.TS_KEY = v_value;
        }
        if (variable == "MPD_IP")
        {
            v_set.MPD_IP= v_value;
        }       
        if (variable == "BaudRate")
        {
            v_set.BaudRate= v_value;
        }
        if (variable == "DELAY")
        {
            v_set.v_delay  = std::stoi(v_value );
        }
        if (variable == "PORT")
        {
            v_set.PORT= v_value;
        }

        variable = v_value = "";
    }

   config_file.close();

    return v_set;
}
