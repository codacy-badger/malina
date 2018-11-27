#include "../command_event.h"
#include "../../../iDomTools/test/iDomTools_fixture.h"

class command_event_Class_fixture : public iDomTOOLS_ClassTest
{
public:
    command_event_Class_fixture()
    {

    }

protected:
    std::unique_ptr<command_event> test_command_event;

    std::vector<std::string> test_v;
    void SetUp() final
    {
        iDomTOOLS_ClassTest::SetUp();
        test_command_event = std::make_unique <command_event> ("event");
    }

    void TearDown() final
    {
        iDomTOOLS_ClassTest::TearDown();
    }
};

TEST_F(command_event_Class_fixture, eventList)
{
    test_v.push_back("event");
    auto ret = test_command_event->execute(test_v,&test_my_data);
    EXPECT_THAT(ret, ::testing::HasSubstr("pilot"));
}

TEST_F(command_event_Class_fixture, eventPilot)
{
    test_v.push_back("event");
    test_v.push_back("pilot");
    auto ret = test_command_event->execute(test_v,&test_my_data);
    EXPECT_THAT(ret, ::testing::HasSubstr("pilot"));
}

TEST_F(command_event_Class_fixture, clearEventPilot)
{
    test_my_data.myEventHandler.run("pilot")->addEvent("pilot test");
    EXPECT_EQ(test_my_data.myEventHandler.run("pilot")->howManyEvent(),1);
    test_v.push_back("event");
    test_v.push_back("pilot");
    test_v.push_back("clear");
    test_command_event->execute(test_v,&test_my_data);
    EXPECT_EQ(test_my_data.myEventHandler.run("pilot")->howManyEvent(),0);
}

TEST_F(command_event_Class_fixture, clearsSomeEventPilot)
{
    for(int i = 0; i<10; ++i){
    test_my_data.myEventHandler.run("pilot")->addEvent("pilot test");
    }
    EXPECT_EQ(test_my_data.myEventHandler.run("pilot")->howManyEvent(),10);
    test_v.push_back("event");
    test_v.push_back("pilot");
    test_v.push_back("clear");
    test_v.push_back("3");
    test_v.push_back("6");
    test_command_event->execute(test_v,&test_my_data);
    EXPECT_EQ(test_my_data.myEventHandler.run("pilot")->howManyEvent(),7);
}

TEST_F(command_event_Class_fixture, intensityEventPilot)
{
    test_my_data.myEventHandler.run("pilot")->addEvent("pilot test");
    EXPECT_EQ(test_my_data.myEventHandler.run("pilot")->howManyEvent(),1);
    test_v.push_back("event");
    test_v.push_back("pilot");
    test_v.push_back("intensity");
    auto ret = test_command_event->execute(test_v,&test_my_data);
    EXPECT_STREQ(ret.c_str(),"event pilot 1 intensity per last minute!");
    EXPECT_EQ(test_my_data.myEventHandler.run("pilot")->howManyEvent(),1);
}

TEST_F(command_event_Class_fixture, wrongParamiter)
{
    test_v.push_back("event");
    test_v.push_back("pilot");
    test_v.push_back("clear");
    test_v.push_back("3");
    test_v.push_back("6");
    test_v.push_back("clear");
    test_v.push_back("3");
    test_v.push_back("6");
    test_v.push_back("clear");
    test_v.push_back("3");
    test_v.push_back("6");
    auto ret = test_command_event->execute(test_v,&test_my_data);
    EXPECT_THAT(ret,::testing::HasSubstr("event"));
}
