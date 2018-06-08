#include "lightning.h"

LIGHTNING::LIGHTNING()
{
    puts("LIGHTNING::LIGHTNING()");
}

LIGHTNING::~LIGHTNING()
{
    puts("LIGHTNING::~LIGHTNING()");
}

CARDINAL_DIRECTIONS::ALARM_INFO LIGHTNING::lightningAlert(nlohmann::json jj)
{
    CARDINAL_DIRECTIONS::ALARM_INFO data;
    nlohmann::json i;
    try{
        i = jj.at("response").get<nlohmann::json>();
    }
    catch (...)
    {
        std::cout << " zly JSON " <<std::endl;
        return data;
    }

#ifdef BT_TEST
    //  std::cout <<"\n\n data all " << i.dump(4) <<" size:"<< i.size() <<std::endl;
#endif
    auto _size = i.size();
    if (_size == 0)
    {
        data.riseAlarm = false;
        return data;
    }
    STATISTIC<int> ageAver(_size);
    STATISTIC<double> distanceKmAver(_size);
    STATISTIC<int> bearingAver(_size);
    for (auto j : i){
#ifdef BT_TEST
        std::cout <<"\n distance " << j.at("relativeTo").at("bearingENG").get<std::string>() << std::endl;
        std::cout <<"distance " << j.at("relativeTo").at("distanceKM").get<double>() << std::endl;
        std::cout <<"timestamp " << j.at("age").get<int>() << std::endl;
#endif
        ageAver.push_back(j.at("age").get<int>());
        distanceKmAver.push_back(j.at("relativeTo").at("distanceKM").get<double>());
        bearingAver.push_back(static_cast<int>(CARDINAL_DIRECTIONS::stringToCardinalDirectionsEnum(
                                                   j.at("relativeTo").at("bearingENG").get<std::string>()))
                              );
    }
    data.bearingENG = static_cast<CARDINAL_DIRECTIONS::CARDINAL_DIRECTIONS_ENUM>(bearingAver.mode());
    data.distance = distanceKmAver.average();
    data.timestamp = ageAver.median();

    data.data << "średni czas upłynięty od ostatniego uderzenia pioruna: "<< data.timestamp << " sek \\n";
    data.data << "średnia odległość ostatniego uderzenia pieruna: "<< data.distance <<" km \\n ";
    data.data << "kierunek uderzeń piorunów: " << CARDINAL_DIRECTIONS::cardinalDirectionsEnumToHuman(data.bearingENG) ;

    if(i.size() > 0){
        // std::cout << "jest size: " << i.size()<<std::endl;
        data.riseAlarm = true;
    }

    return data;
}

bool LIGHTNING::checkLightningAlert(CARDINAL_DIRECTIONS::ALARM_INFO *info)
{
#ifdef BT_TEST
    std::cout << "LIGHTNING::checkLightningAlert() bool "<< info->riseAlarm <<" local " << alarmState << std::endl
              << " distance " << info->distance << std::endl;
#endif
    if(info->riseAlarm == false && alarmState == false){
#ifdef BT_TEST
        std::cout << "(info->riseAlarm == false || alarmState == false)"<<std::endl;
#endif
        return false;
    }
    if(info->riseAlarm == false && alarmState == true){
#ifdef BT_TEST
        std::cout << "(info->riseAlarm == false || alarmState == true)"<<std::endl;
#endif
        alarmState = false;
        return false;
    }
    if(info->riseAlarm == true && alarmState == false){

#ifdef BT_TEST
        std::cout << "(info->riseAlarm == true || alarmState == false)"<<std::endl;
#endif
        alarmState = true;
        lightningTime = Clock::getTime();
        return true;
    }
    return false;
}

