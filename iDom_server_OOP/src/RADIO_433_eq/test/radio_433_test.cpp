#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../radio_433_eq.h"

RC_433MHz::RC_433MHz(thread_data *test_my_data)
{
    this->m_my_data = test_my_data;
}
void RC_433MHz::sendCode(const std::string& code)
{
    std::cout << "sendCode(): " << code << std::endl;
}

class Switch_Class_fixture : public testing::Test
{
protected:
    Switch_Class_fixture(){
        test_rec = std::make_shared<RADIO_EQ_CONTAINER>(&test_my_data);
        test_server_set._server.radio433MHzConfigFile = "/mnt/ramdisk/433_eq_conf.json";
        test_my_data.server_settings = &test_server_set;
        test_rec->loadConfig(test_server_set._server.radio433MHzConfigFile);
        test_my_data.main_REC = test_rec;
        test_my_data.main_iDomStatus = std::make_unique<iDomSTATUS>();
    }
    std::shared_ptr<RADIO_EQ_CONTAINER> test_rec;
    thread_data test_my_data;
    CONFIG_JSON test_server_set;
};
TEST_F(Switch_Class_fixture, getSwitchPointerVector)
{
    auto v = test_rec->getSwitchPointerVector();
    EXPECT_EQ(v.size(),5);
}

TEST_F(Switch_Class_fixture, getButtonPointerVector)
{
    auto v = test_rec->getButtonPointerVector();
    EXPECT_EQ(v.size(),3);
}

TEST_F(Switch_Class_fixture, switch_alarm_on)
{
    RADIO_SWITCH* ptr = dynamic_cast<RADIO_SWITCH*>(test_rec->getEqPointer("ALARM"));

    EXPECT_EQ(ptr->getType(),RADIO_EQ_TYPE::SWITCH);
    puts("radio switch type");
    EXPECT_EQ(ptr->getState(),STATE::UNDEFINE);
    puts("radio switch state");
    ptr->on();
    EXPECT_EQ(ptr->getState(),STATE::ON);
    ptr->off();
    EXPECT_EQ(ptr->getState(),STATE::OFF);
    ptr->onSunset();
    EXPECT_EQ(ptr->getState(),STATE::OFF);
}

TEST_F(Switch_Class_fixture, weatherStruct)
{
    WEATHER_STRUCT test_WS;
    EXPECT_DOUBLE_EQ(0.0, test_WS.getTemperature()) << "Tempertura zla";

    test_WS.putData("20;03;LaCrosse;ID=0506;TEMP=0137;");
    EXPECT_DOUBLE_EQ(31.1, test_WS.getTemperature()) << "Tempertura zla";

    test_WS.putData("20;03;LaCrosse;ID=0506;TEMP=8130;BARO=999;");
    EXPECT_DOUBLE_EQ(-30.4, test_WS.getTemperature()) << "Tempertura zla";

    EXPECT_EQ(999, test_WS.getBarometricPressure()) << "zle cisneinie";

    std::string retString = test_WS.getDataString();
    EXPECT_THAT(retString, testing::HasSubstr("Pressure= 999kPa"));
}

TEST_F(Switch_Class_fixture, read_write_config_json)
{
    auto v = test_rec->getSwitchPointerVector();
    EXPECT_EQ(v.size(),5);
    test_rec->saveConfig(test_server_set._server.radio433MHzConfigFile);
    v = test_rec->getSwitchPointerVector();
    EXPECT_EQ(v.size(),5);
}

TEST_F(Switch_Class_fixture, addUnexistsRadioEq)
{
    RADIO_EQ_CONFIG tCfg;
    tCfg.name = "cyniu";
    tCfg.ID = "8899";
    std::string _name = tCfg.name;
    EXPECT_FALSE(test_rec->nameExist(_name));
    test_rec->addRadioEq(tCfg, "PIR");
    EXPECT_FALSE(test_rec->nameExist(_name));
}

TEST_F(Switch_Class_fixture, add_and_erase_switch)
{
    RADIO_EQ_CONFIG tCfg;
    tCfg.name = "test";
    test_rec->addRadioEq(tCfg, RADIO_EQ_TYPE::SWITCH);
    auto v = test_rec->getSwitchPointerVector();
    EXPECT_EQ(v.size(),6);
    test_rec->saveConfig(test_server_set._server.radio433MHzConfigFile);
    v = test_rec->getSwitchPointerVector();
    EXPECT_EQ(v.size(),6);

    ///////delete
    test_rec->deleteRadioEq(tCfg.name);
    v = test_rec->getSwitchPointerVector();
    EXPECT_EQ(v.size(),5);
    test_rec->saveConfig(test_server_set._server.radio433MHzConfigFile);
    v = test_rec->getSwitchPointerVector();
    EXPECT_EQ(v.size(),5);
}

TEST_F(Switch_Class_fixture, loadConfig)
{
    thread_data test_my_data2;
    auto  test_rec = std::make_shared<RADIO_EQ_CONTAINER>(&test_my_data2);
    test_rec->loadConfig("/mnt/ramdisk/433_eq_conf_fake.json");
    test_my_data2.main_REC = test_rec;
    EXPECT_FALSE(test_my_data2.main_REC->nameExist("firstt"));
    EXPECT_TRUE(test_my_data2.main_REC->nameExist("listwa"));
}

TEST_F(Switch_Class_fixture, getUnexistPtr)
{
    EXPECT_THROW(test_my_data.main_REC->getEqPointer("kokos"),std::string);
}

TEST_F(Switch_Class_fixture, onLock_onUnlock_HOME)
{
    RADIO_EQ_CONFIG tCfg;
    tCfg.name = "cyniu";
    tCfg.ID = "8899";
    tCfg.lock = "ON";

    auto testRadioS = static_cast<RADIO_SWITCH*>(test_rec->getEqPointer("C"));
    testRadioS->setCode(tCfg);
    testRadioS->onLockHome();
    std::string eventStr = test_my_data.myEventHandler.run("iDom")->getEvent();
    EXPECT_THAT(eventStr, testing::HasSubstr("cyniu ON due to 433MHz button pressed"));

    tCfg.lock = "OFF";
    testRadioS->setCode(tCfg);
    testRadioS->onLockHome();
    eventStr = test_my_data.myEventHandler.run("iDom")->getEvent();
    EXPECT_THAT(eventStr, testing::HasSubstr("cyniu OFF due to 433MHz button pressed"));

    ////////////////////// unlock
    tCfg.unlock = "ON";

    testRadioS->setCode(tCfg);
    testRadioS->onUnlockHome();
     eventStr = test_my_data.myEventHandler.run("iDom")->getEvent();
    EXPECT_THAT(eventStr, testing::HasSubstr("cyniu ON due to 433MHz button pressed"));

    tCfg.unlock = "OFF";
    testRadioS->setCode(tCfg);
    testRadioS->onUnlockHome();
    eventStr = test_my_data.myEventHandler.run("iDom")->getEvent();
    EXPECT_THAT(eventStr, testing::HasSubstr("cyniu OFF due to 433MHz button pressed"));
}

TEST_F(Switch_Class_fixture, us_wrong_configurated_switch)
{
    auto testRadioS = static_cast<RADIO_SWITCH*>(test_rec->getEqPointer("C"));
    EXPECT_EQ(testRadioS->getState(), STATE::UNDEFINE);
    testRadioS->on();
    EXPECT_EQ(testRadioS->getState(), STATE::UNDEFINE);
    testRadioS->off();
    EXPECT_EQ(testRadioS->getState(), STATE::UNDEFINE);
    testRadioS->onFor15sec();
    EXPECT_EQ(testRadioS->getState(), STATE::UNDEFINE);
}
