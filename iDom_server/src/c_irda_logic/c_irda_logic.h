#ifndef C_IRDA_LOGIC_H
#define C_IRDA_LOGIC_H





#include <queue>
#include <iostream>
#include <string.h>
#include "../blockQueue/blockqueue.h"   // brak
#include "../files_tree/files_tree.h"   //brak

//#include "../iDom_server/src/iDom_server.h"
#include <wiringPi.h>
//extern bool go_while;
class c_irda_logic
{
private:
    std::queue <char> irda_queue;
    blockQueue char_queue;
    char who;


public:
    c_irda_logic();
    void _add(char X);
    char _get();
    int  _size();
    void _distribute();

};

#endif // C_IRDA_LOGIC_H
