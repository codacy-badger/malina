#include <gtest/gtest.h>
#include "/home/pi/programowanie/iDom_server_OOP/test/iDom_TESTs-CMAKE/test_data.h"
#include "../idomtools.h"
#include "/home/pi/programowanie/iDom_server_OOP/src/functions/functions.h"
#include "../../RADIO_433_eq/radio_433_eq.h"
#include "testJSON.h"

class iDomTOOLS_ClassTest : public ::testing::Test
{
public:
    TEST_JSON test_Json;
    LIGHTNING test_lightning;
    CARDINAL_DIRECTIONS::ALARM_INFO test_struct;
    thread_data test_my_data;
    iDomSTATUS test_status;
    config test_server_set;
    RADIO_EQ_CONTAINER test_rec;
    iDOM_STATE main_iDomStatus;
    ALERT test_alarmTime;
    pilot_led test_pilot_led;

    /// pointer
    iDomTOOLS* test_idomTOOLS;
    /////// method
    iDomTOOLS_ClassTest():test_rec(&test_my_data)
    {
        std::cout << "konstruktor testu " <<std::endl;
    }
    void SetUp()
    {
        std::cout << "SetUP testu " <<std::endl;
        test_server_set.TS_KEY = "key test";
        test_server_set.viberSender = "test sender";
        test_server_set.viberReceiver = {"R1","R2"};
        test_server_set.saveFilePath = "/mnt/ramdisk/iDomStateTest2.save";
        test_server_set.radio433MHzConfigFile = "/mnt/ramdisk/433_eq_conf.json";
        test_rec.loadConfig(test_server_set.radio433MHzConfigFile);

        test_my_data.main_REC = (&test_rec);
        test_my_data.server_settings = &test_server_set;
        test_my_data.main_iDomStatus = &test_status;
        test_my_data.alarmTime = test_alarmTime;
        test_my_data.idom_all_state = main_iDomStatus;
        test_my_data.ptr_pilot_led = &test_pilot_led;

        test_status.addObject("house");

        /////////// create
        test_idomTOOLS = new iDomTOOLS(&test_my_data);

        test_my_data.main_iDomTools = test_idomTOOLS;
        useful_F::myStaticData = &test_my_data;
    }

    void TearDown()
    {
        delete test_idomTOOLS;
    }
};

