#ifndef GLOBAL_H
#define GLOBAL_H
#include <iostream>
#include <fstream>
#include <strings.h>
#include <string>
#include <cstdlib>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <boost/lexical_cast.hpp>
#include <wiringPi.h>

 //#include <lirc/lirc_client.h>


// MOJE BIBLIOTEKI
#include "logger/logger.hpp"
#include "LCD_c/lcd_c.h"
#include "files_tree/files_tree.h"
#include "menu_tree/menu_tree.h"
//#include "CRON/cron.hpp"

constexpr int MAX_MSG_LEN= 18;
constexpr int MAX_CONNECTION =10;
constexpr int FREE =1;
constexpr int RS232 =11;
constexpr int ok   =  0;

#define log_file_cout  f_log //std::cout   zmien f_log na std::cout  i bedzie wypisywac na ekran
#define log_file_mutex f_log

constexpr int ON =1;
constexpr int OFF =0;

#define BUZZER 7
#define GPIO_SPIK 21
#define BUTTON_PIN 25

extern std::string  _logfile  ;

extern Logger log_file_mutex;

 extern std::string buffer;

//extern int max_msg  ;

extern bool go_while  ;


struct s_pointer{
    unsigned int *ptr_who;
    int32_t *ptr_buf;
};

struct Thread_array_struc {
    pthread_t  thread_ID;
    std::string thread_name;
    int thread_socket;
};
//struct s_arg{
//    serialib port_arduino;
//    struct struktura_wiadomosci s_msg;
//    union unia_wiadomosci s_unia_msg;
//    struct struktura_wiadomosci r_msg;
//    union unia_wiadomosci r_unia_msg;
//};

struct addresses_mac{
    std::string name_MAC;
    std::string MAC;
    bool state;
    int option1;
    int option2;
    int option3;
    int option4;
    int option5;
    int option6;
};

struct address_another_servers {
    int id;
    std::string SERVER_IP;
};

struct config{
    std::string portRS232;
    std::string BaudRate;
    std::string PORT;
    std::string SERVER_IP;
    std::string MPD_IP;
    std::string MOVIES_DB_PATH;
    unsigned int ID_server;

    std::vector <addresses_mac> A_MAC;
    int v_delay  ;
    std::vector <address_another_servers> AAS;

};
struct thread_data{
    int s_client_sock;
    struct sockaddr_in from;
    struct config *server_settings;
    struct s_pointer pointer;
    LCD_c *mainLCD;
    files_tree *main_tree;
    menu_tree *main_MENU;
    Thread_array_struc *main_THREAD_arr;
    time_t start;
    time_t now_time;
    int sleeper;

    //CRON * main_cron;

};


struct thread_data_rs232{

    std::string portRS232;
    std::string BaudRate;
    struct s_pointer pointer;

};

//int parser_bufor ( int32_t bufor_tmp[]);
/*

*/
#endif // GLOBAL_H
