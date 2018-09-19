#ifndef IDOM_SAVE_STATE_H
#define IDOM_SAVE_STATE_H

#include <mutex>
//#include "../iDom_server_OOP.h"
#include "json.hpp"

class iDom_SAVE_STATE
{
public:
    std::mutex m_mutex;
    iDom_SAVE_STATE(const std::string& path);
    ~iDom_SAVE_STATE();

    nlohmann::json read();
    void write(const nlohmann::json &jj);

private:
    std::string m_path = "NULL";
};

#endif // IDOM_SAVE_STATE_H
