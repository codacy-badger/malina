#include "satel_integra.h"

#include <bitset>

void wypiszOdpowiedzSerwera( unsigned char *komunikat, unsigned int dlugosc)
{
    int i;
    printf("Rozmiar odpowiedzi: %i\nOdpowiedz serwera:\n", dlugosc);
    for (i=0; i<dlugosc; i++)
    {
        if (i==3 || i==dlugosc-4)
            printf("----------\n");
        printf("%#02i : %#04x\n", i, komunikat[i]);
        std::cout << i << ": " << std::bitset<16>(komunikat[i]) << std::endl;
    }
}

SATEL_INTEGRA::SATEL_INTEGRA()
{
    m_className.append(typeid (this).name());
    iDom_API::addToMap(m_className,this);
}

SATEL_INTEGRA::~SATEL_INTEGRA()
{
    iDom_API::removeFromMap(m_className);
}

void SATEL_INTEGRA::setIntegraPin(const std::string &pin)
{
    m_pin = pin;
}

void SATEL_INTEGRA::connectIntegra(const std::string &host, const int port)
{
    m_sock = socket(AF_INET , SOCK_STREAM , 0);
    if (m_sock == -1)
        puts("Nie można stworzyć połączenia socket");
    m_server.sin_addr.s_addr = inet_addr(host.c_str());
    m_server.sin_family = AF_INET;

    m_server.sin_port = htons(port);

    if (connect(m_sock , (struct sockaddr *)&m_server , sizeof(m_server)) < 0)
        puts("Nie udało się nawiązać połączenia");
    printf("Połączony\n");
}

void SATEL_INTEGRA::disconnectIntegra()
{
    shutdown(m_sock, SHUT_RDWR );
}

std::string SATEL_INTEGRA::getIntegraInfo()
{
    std::string msg;

    msg.push_back(INTEGRA_ENUM::VERSION); // get integra info

    sendIntegra(msg);

    (void) recvIntegra();

    std::stringstream rec;
    rec << "Typ centrali: INTEGRA " << satelType(m_message[3])<< std::endl;
    rec << "Wersja: " << m_message[4] << "." << m_message[5] << m_message[6] << " z dnia: " << m_message[7] <<
           m_message[8] << m_message[9] << m_message[10]  << "-" << m_message[11] << m_message[12] << '-' << m_message[13] <<
           m_message[14] << std::endl;
    rec << "Wersja językowa: " << (int)m_message[15] << " (" << (m_message[15]==0 ? "polska" : "inny język") << ")" << std::endl;
    rec << "Dane zapisane w pamięci FLASH: " << (m_message[16]==255 ? "tak" : "nie") << std::endl;
    return rec.str();
}

std::string SATEL_INTEGRA::checkIntegraOut()
{
    std::string msg;
    msg.push_back(INTEGRA_ENUM::OUTPUTS_STATE); // get integra info

    sendIntegra(msg);

    int recSize = recvIntegra();
    wypiszOdpowiedzSerwera(m_message, recSize);
    return "done";
}

bool SATEL_INTEGRA::isAlarmArmed()
{
    std::string msg;
    msg.push_back(INTEGRA_ENUM::ARMED_PARTITIONS);

    sendIntegra(msg);

    int recSize = recvIntegra();

    if(m_message[2] != INTEGRA_ENUM::ARMED_PARTITIONS){
        std::cout << "zła odpowedz servera" << std::endl;
        return false;
    }
    switch (m_message[3]) {
    case 0: return false; //not armed
    case 3: return true; // all armed
    default: std::cout << "blad sprawdzania uzborjenia" << std::endl;
    }

    return false;
}

std::string SATEL_INTEGRA::dump() const
{
    std::stringstream ss;
    ss << "m_message " << m_message << std::endl;
    return ss.str();
}

unsigned short SATEL_INTEGRA::calculateCRC(const std::string& msg)
{
    unsigned short crc = 0x147A;

    for (int i = 0; i < msg.size(); ++i)
    {
        crc = (crc << 1) | (crc >> 15);
        crc = crc ^ 0xFFFF;
        crc = crc + (crc >> 8) + msg.at(i);
    }

    return crc;
}

int SATEL_INTEGRA::sendIntegra(const std::string &msg)
{
    std::string message;
    //header
    message.push_back(INTEGRA_ENUM::HEADER);
    message.push_back(INTEGRA_ENUM::HEADER);

    // command
    auto crc = calculateCRC(msg);
    message.append(msg);
    message.push_back((crc >> 8) & 0xff); //crc1
    message.push_back(crc & 0xff);   //crc2
    // end message
    message.push_back(INTEGRA_ENUM::HEADER);
    message.push_back(INTEGRA_ENUM::END);
    return send(m_sock, message.c_str(), message.length(), 0 );
}

int SATEL_INTEGRA::recvIntegra()
{
    int size = recv(m_sock, m_message, 2000, 0);
    std::cout << " cyniu size: " << size << std::endl;
    if (size < 0) //rozmiar odp
        puts("Nie udało się pobrać odpowiedzi z serwera");
    if (m_message[0] != INTEGRA_ENUM::HEADER
            || m_message[1] != INTEGRA_ENUM::HEADER
            || m_message[size-1] != INTEGRA_ENUM::END
            || m_message[size-2] != INTEGRA_ENUM::HEADER)
        puts("Urządzenie zajęte (busy) lub niewłaściwy format odpowiedzi");
    return size;
}

const char* SATEL_INTEGRA::satelType(unsigned char t)
{
    switch(t)
    {
    case 0: return "24";
    case 1: return "32";
    case 2: return "64";
    case 3: return "128";
    case 4: return "128-WRL SIM300";
    case 132: return "128-WRL LEON";
    case 66: return "64 PLUS";
    case 67: return "128 PLUS";
    case 72: return "256 PLUS";
    default: return "Nieznany typ";
    }
}
