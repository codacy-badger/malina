#include <iomanip>
#include <fstream>
#include "idomkey_access.h"
#include "../iDom_server_OOP.h"

void iDomKEY_ACCESS::writeJSON()
{
    std::ofstream o(m_pathDatabase);
    o << std::setw(4) << m_data << std::endl;
}

void iDomKEY_ACCESS::readJSON()
{
    // read a JSON file
    std::ifstream i(m_pathDatabase);
    log_file_mutex.mutex_lock();
    log_file_cout << INFO << "czytam zapisany stan kluczy iDom" << std::endl;
    log_file_mutex.mutex_unlock();
    try {
        i >> m_data;
    } catch (...) {
        log_file_mutex.mutex_lock();
        log_file_cout << CRITICAL << "problem z czytaniem zapisanych stanów kluczy iDom" << std::endl;
        log_file_mutex.mutex_unlock();
    }
}

iDomKEY_ACCESS::iDomKEY_ACCESS(const std::string &path_database):
    m_pathDatabase(path_database)
{
    m_className.append(typeid (this).name());
    iDom_API::addToMap(m_className,this);
    readJSON();
}

iDomKEY_ACCESS::iDomKEY_ACCESS(const iDomKEY_ACCESS &k): m_data(k.m_data), m_pathDatabase(k.m_pathDatabase)
{
    puts("konstruktor kopiujacy iDomKEY_ACCESS");
}

iDomKEY_ACCESS::~iDomKEY_ACCESS()
{
    iDom_API::removeFromMap(m_className);
}

void iDomKEY_ACCESS::addKEY(const std::string &name, size_t size, bool temp )
{
    std::string _key = m_generator.random_string(size);
    nlohmann::json temp_J;
    temp_J["name"] = name;
    temp_J["key"] = _key;
    temp_J["temporary"] = temp;
    m_data[name] = temp_J;
    writeJSON();
}

void iDomKEY_ACCESS::addTempKEY(const std::string &name, size_t size)
{
    addKEY(name,size,true);
}

std::string iDomKEY_ACCESS::getKEY(const std::string &name)
{
    return m_data[name].at("key").get<std::string>();
}

std::string iDomKEY_ACCESS::listKEY()
{
    std::stringstream ret;

    ret << m_data.dump(4);

    return ret.str();
}

bool iDomKEY_ACCESS::useKEY(const std::string &name, const std::string &key)
{
        std::string k = m_data[name].at("key").get<std::string>();

        bool toDel = m_data[name].at("temporary").get<bool>();

        if(toDel){
            m_data.erase(name);
            writeJSON();
        }
        return (key == k);
}

std::string iDomKEY_ACCESS::dump() const
{
    std::stringstream ret;

    ret << "m_data.size(): " << m_data.size() << std::endl;
    ret << "m_pathDatabase: " << m_pathDatabase << std::endl;
    return ret.str();
}
