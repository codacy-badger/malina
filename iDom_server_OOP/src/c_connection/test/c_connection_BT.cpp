#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../c_connection.h"
#include "test_data.h"
#include "../../iDomTools/mock/iDomToolsMock.h"
#include "../../RADIO_433_eq/radio_433_eq.h"

class c_connection_fixture : public testing::Test
{
public:
    c_connection_fixture():main_iDomTools(std::make_shared<iDomToolsMock>()),
                           test_connection(std::nullptr_t()),
                           test_rec(std::make_shared<RADIO_EQ_CONTAINER>(&test_my_data))
    {
        test_server_set._server.encrypted = true;
        test_my_data.server_settings = &test_server_set;
        test_my_data.main_iDomStatus = std::make_unique<iDomSTATUS>();
        test_my_data.main_iDomTools = main_iDomTools;
        test_my_data.main_REC = test_rec;
        test_my_data.s_client_sock = 0;
        test_connection = std::make_unique<C_connection>(&test_my_data);
        test_connection->m_encriptionKey = "key";
        test_connection->m_encrypted = false;
        test_connection->c_socket = 0;
    }

protected:
    thread_data test_my_data;
    CONFIG_JSON test_server_set;
    std::shared_ptr<iDomToolsMock> main_iDomTools;
    std::unique_ptr<C_connection> test_connection;
    std::shared_ptr<RADIO_EQ_CONTAINER> test_rec;
    void crypto_fixture(std::string &toEncrypt, std::string key)
    {
        test_connection->crypto(toEncrypt, std::move(key), true);
    }
};

TEST_F(c_connection_fixture, crypto)
{
    std::string key = "210116556";
    std::string test_msg = "kokosowa ksiezniczka";
    std::string toEncrypt = test_msg;
    crypto_fixture(toEncrypt, key);

    for(int i = 0; i < toEncrypt.size(); ++i)
    {
        EXPECT_NE(test_msg[i],toEncrypt[i]) << " niestety równe: " << toEncrypt[i]
                                               << " na indeksie: " << i;
    }
    std::cout << "wiadomość: " << test_msg << " zakodowane: "<< toEncrypt << std::endl;
    crypto_fixture(toEncrypt, key);
    std::cout << "wiadomość: " << test_msg << " odkodowane: "<< toEncrypt << std::endl;
    EXPECT_STREQ(toEncrypt.c_str(), test_msg.c_str()) << "wiadomosci nie są równe";
}

TEST_F(c_connection_fixture, c_analyse)
{
    test_connection->m_mainCommandHandler = std::make_unique<commandHandlerRoot>(&test_my_data);
    int i = 0;
    std::string strMsg = "fake command";
    for(char n : strMsg)
        test_connection->c_buffer[i++] = n;
    test_connection->setEncrypted(false);
    test_connection->c_analyse(static_cast<int>(strMsg.size()));
    EXPECT_THAT(test_my_data.myEventHandler.run("command")->getEvent()
                ,testing::HasSubstr(strMsg));
}

TEST_F(c_connection_fixture, c_recv)
{
    EXPECT_EQ(-1, test_connection->c_recv(1));
}

TEST_F(c_connection_fixture, c_send)
{
    EXPECT_EQ(-1, test_connection->c_send("test"));
}

TEST_F(c_connection_fixture, cryptoLog)
{
    std::string msg("tajna wiadomosc");
    std::string msgBackup(msg);
    test_connection->setEncriptionKey("key_test");
    test_connection->setEncrypted(true);
    ///////szyfrowanie
    test_connection->cryptoLog(msg);
    EXPECT_STRNE(msgBackup.c_str(), msg.c_str());
    std::cout << "ZASZUFROWANY: " << msg << std::endl;
    ////// deszyfracja
    test_connection->cryptoLog(msg);
    EXPECT_STREQ(msgBackup.c_str(), msg.c_str());
}

TEST_F(c_connection_fixture, onStopConnection)
{
    EXPECT_CALL(*main_iDomTools.get(), cameraLedOFF(testing::_));
    TEST_DATA::return_httpPost = "ok.\n";
    test_connection->onStopConnection();
}

TEST_F(c_connection_fixture, exitFlow)
{
    test_connection->m_mainCommandHandler = std::make_unique<commandHandlerRoot>(&test_my_data);
    EXPECT_CALL(*main_iDomTools.get(), close_iDomServer());
    int i = 0;
    std::string strMsg = "program stop server";
    for(char n : strMsg)
        test_connection->c_buffer[i++] = n;
    test_connection->setEncrypted(false);
    test_connection->c_analyse(strMsg.size());
}

TEST_F(c_connection_fixture, emptyCommand)
{

    std::array<Thread_array_struc,iDomConst::MAX_CONNECTION >test_ThreadArrayStruc;
    test_my_data.main_THREAD_arr = &test_ThreadArrayStruc;
    test_my_data.main_THREAD_arr->at(3).thread_socket = 1;
    test_my_data.main_THREAD_arr->at(3).thread_ID = std::this_thread::get_id();

    test_connection->m_mainCommandHandler = std::make_unique<commandHandlerRoot>(&test_my_data);

    std::string strMsg = "";
    test_connection->setEncrypted(false);
    test_connection->c_analyse(static_cast<int>(strMsg.size()));
    EXPECT_STREQ(test_connection->getStr_buf().c_str(), "empty command");
}

