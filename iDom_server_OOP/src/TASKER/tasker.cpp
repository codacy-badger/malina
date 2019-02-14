#include "tasker.h"
#include "../functions/functions.h"


TASKER::TASKER(thread_data *my_data)
{
    this->my_data = my_data;
    topic = "iDom-client/command";
}

void TASKER::runTasker()
{
    if(counter == 0){
        my_data->mainLCD->checkState();
        ++counter;
    }
    else{
        counter = 0;
    }

    if(my_data->mqttHandler->getReceiveQueueSize() > 0)
    {

        auto kk = my_data->mqttHandler->getMessage();

        if(kk.first == topic)
        {
            auto v = useful_F::split(kk.second, ' ');
            auto ret = commandMQTT.run(v,my_data);

            my_data->mqttHandler->publish(my_data->server_settings->_mqtt_broker.topicPublish + "/command",
                                          ret);
        }
        else
            puts(":zly topic");

    }
}
