#include <iostream>
#include "menu_buderus.h"
#include "menu_root.h"

MENU_BUDERUS::MENU_BUDERUS(thread_data *my_data, LCD_c* lcdPTR, MENU_STATE_MACHINE* msm, STATE lcdLED): MENU_STATE_BASE (my_data, lcdPTR, msm, lcdLED)
{
}

MENU_BUDERUS::MENU_BUDERUS(const MENU_BUDERUS &base):MENU_STATE_BASE(base)
{
}

MENU_BUDERUS::MENU_BUDERUS(MENU_BUDERUS &&base):MENU_STATE_BASE(std::move(base))
{
}

MENU_BUDERUS &MENU_BUDERUS::operator=(const MENU_BUDERUS &base)
{
    if(&base != this){
        my_dataPTR = base.my_dataPTR;
        lcdPTR = base.lcdPTR;
        stateMachinePTR = base.stateMachinePTR;
    }
    return * this;
}

MENU_BUDERUS &MENU_BUDERUS::operator=(MENU_BUDERUS &&base)
{
    if(&base != this){
        my_dataPTR = base.my_dataPTR;
        lcdPTR = base.lcdPTR;
        stateMachinePTR = base.stateMachinePTR;
    }
    return * this;
}

MENU_BUDERUS::~MENU_BUDERUS()
{
}

void MENU_BUDERUS::entry()
{
    menuDatabase.pushBack({"RUN CIRCLE PUMP", [=](){my_dataPTR->ptr_buderus->runCirclePompForWhile(); return "done";}});
    menuDatabase.pushBack({"TEMP INSIDE", [=](){return std::to_string(my_dataPTR->ptr_buderus->getInsideTemp());}});
    menuDatabase.pushBack({"TEMP OUTDOOR", [=](){return std::to_string(my_dataPTR->ptr_buderus->getOutdoorTemp());}});
    menuDatabase.pushBack({"   EXIT",   [=]() { changeStateTo<MENU_ROOT>(); return "done";}});
    print(menuDatabase.getCurrent().name, arrow);
}

void MENU_BUDERUS::exit()
{

}

void MENU_BUDERUS::keyPadOk()
{
   auto str = menuDatabase.getCurrent().function();
   lcdPTR->printString("", str);
}

void MENU_BUDERUS::keyPadUp()
{
    menuDatabase.up();
    print(menuDatabase.getCurrent().name, arrow);
}

void MENU_BUDERUS::keyPadDown()
{
    menuDatabase.down();
    print(menuDatabase.getCurrent().name, arrow);
}

std::string MENU_BUDERUS::getStateName()
{
    return typeid (this).name();
}
