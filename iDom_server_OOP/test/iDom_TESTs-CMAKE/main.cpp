#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "/home/pi/programowanie/iDom_server_OOP/libs/useful/useful.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest( &argc, argv );

    //::testing::GTEST_FLAG(filter) = "iDomTOOLS_Class.hasTemperatureChange";
    //::testing::GTEST_FLAG(filter) = "rs232_thread.send_Recieve_rs232_thread";
    return RUN_ALL_TESTS();
}

