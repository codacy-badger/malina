#include <gtest/gtest.h>
#include "../functions.h"
#include "../../RADIO_433_eq/radio_433_eq.h"
#include "../../iDomTools/test/iDomTools_fixture.h"

class functions_fixture : public iDomTOOLS_ClassTest{

};

void useful_F::sleep_1min()
{
    std::cout << "sleep_1min()"<<std::endl;
}
TEST(functions_, tokenizer)
{
     std::string test_msg = "one=two three";
     std::vector<std::string> test_v;

     EXPECT_EQ(test_v.size(), 0);
     useful_F::tokenizer(test_v,"= ",test_msg);

     EXPECT_EQ(test_v.size(),3);
     EXPECT_STREQ(test_v.at(2).c_str(),"three");
}

TEST(functions_, removeHtmlTag)
{
     std::string test_msg = "<html>test</html>";
     std::string test_pure_str = useful_F_libs::removeHtmlTag(test_msg);

     EXPECT_STREQ(test_pure_str.c_str(),"test");
}

TEST(functions_, repalceAll)
{
     std::string test_msg = "one two three";
     std::string test_pure_str = useful_F_libs::replaceAll(test_msg,"two","zero");

     EXPECT_STREQ(test_pure_str.c_str(),"one zero three");
}

TEST(functions_, split)
{
    std::string test_msg = "one two three";
    std::vector<std::string> test_v;

    EXPECT_EQ(test_v.size(), 0);
    test_v = useful_F::split(test_msg,' ');

    EXPECT_EQ(test_v.size(),3);
    EXPECT_STREQ(test_v.at(2).c_str(),"three");
}

TEST(functions_, RSHash)
{
    std::string msg = "test msg";
    std::string s1  = useful_F::RSHash(msg, 33, 44);
    std::string s2  = useful_F::RSHash(msg, 33 ,44);
    EXPECT_STREQ(s1.c_str(), s2.c_str());

    s1 = useful_F::RSHash(msg, 35, 44);
    s2 = useful_F::RSHash(msg, 33 ,44);
    EXPECT_STRNE(s1.c_str(), s2.c_str());
}

TEST_F(functions_fixture, setStaticData)
{
    test_my_data.sleeper = 99;
    EXPECT_EQ( useful_F::myStaticData->sleeper, 99 );
    thread_data test_my_data2;
    test_my_data2.sleeper = 88;
    EXPECT_EQ( test_my_data2.sleeper, 88 );
    EXPECT_EQ( useful_F::myStaticData->sleeper, 99 );
    useful_F::setStaticData(&test_my_data2);
    EXPECT_EQ( useful_F::myStaticData->sleeper, 88 );
}

TEST_F(functions_fixture, sleepThread)
{
    Thread_array_struc test_THRARRSTR;
    test_my_data.main_THREAD_arr = &test_THRARRSTR;

    MPD_info test_ptr_MPD;
    test_ptr_MPD.volume = 3;
    test_my_data.ptr_MPD_info = &test_ptr_MPD;

   // RADIO_EQ_CONTAINER_STUB test_rec(&test_my_data);
    RADIO_EQ_CONTAINER test_rec(&test_my_data);
    test_rec.loadConfig(test_server_set.radio433MHzConfigFile);
    test_my_data.main_REC = (&test_rec);
    test_my_data.alarmTime.time = Clock::getTime();
    test_my_data.alarmTime.state = STATE::ACTIVE;

    RADIO_EQ_CONFIG cfg;
    cfg.name = "listwa";
    cfg.ID = "209888";
    cfg.onCode = "send ON";
    cfg.offCode= "send OFF";
    RADIO_EQ* test_RS = new RADIO_SWITCH(&test_my_data,cfg,RADIO_EQ_TYPE::SWITCH);
    //EXPECT_CALL(test_rec, getEqPointer("listwa")).WillRepeatedly(testing::Return(test_RS));

    test_my_data.sleeper = 10;

    blockQueue test_q;
    test_q._clearAll();
    EXPECT_EQ(test_q._size(),0);
    useful_F::sleeper_mpd(&test_my_data);
    EXPECT_EQ(test_q._size(),1);
    EXPECT_EQ(test_q._get(), MPD_COMMAND::STOP)<<"NIE ZATRZYMANO MUZYKI :(";
    delete test_RS;
}
