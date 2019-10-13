#ifndef LIGHT_BULB_H
#define LIGHT_BULB_H

#include <mutex>
#include <functional>

#include "../idom_api.h"
#include "../../libs/useful/useful.h"

class light_bulb: public iDom_API
{
    STATE m_status = STATE::UNKNOWN;
    std::string m_name;
    int m_ID;
    std::mutex m_operationMutex;

public:
    light_bulb(std::string& name, int id);
    light_bulb(const light_bulb& a);
    light_bulb(const light_bulb&& a);
    light_bulb& operator = (const light_bulb& a);
    light_bulb& operator = (const light_bulb&& a);
    void on(std::function<void(std::string s)>onOn);
    void off(std::function<void(std::string s)>onOff);
    STATE getStatus();
    void setStatus(STATE s);
    std::string getName() const;
    std::string dump() const;
};

#endif // LIGHT_BULB_H
