#ifndef MENU_ROOT_H
#define MENU_ROOT_H

#include "menu_base.h"

class MENU_ROOT : public MENU_STATE_BASE
{
public:
    MENU_ROOT(thread_data *my_data, LCD_c* lcdPTR, MENU_STATE_MACHINE* msm);
    MENU_ROOT(const MENU_ROOT &base);
    MENU_ROOT(MENU_ROOT &&base);
    MENU_ROOT &operator= (const MENU_ROOT &base);
    MENU_ROOT &operator= (MENU_ROOT &&base);
    ~MENU_ROOT() = default;
    void entry();
    void exit();
    std::string getStateName();
    void keyPadMenu();
};

#endif // MENU_ROOT_H
