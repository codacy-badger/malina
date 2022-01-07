#include <Arduino.h>
#include <assert.h>
#include <EEPROM.h>
#include <Relay.h>
#include <Button.h>
#define MY_GATEWAY_SERIAL
#include <MySensors.h>
#include <avr/wdt.h>

#include <PubSubClient.h>
#include <UIPEthernet.h>

static EthernetClient ethClient;
static PubSubClient mqttClient;
static String mqttBuffor;
//ethernet
static String ip = "192.168";
static String MAC = "";
static uint8_t mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
// MQTT
static constexpr int mqttPort = 1883;
static const String mqttBrokerIP = "192.168.13.181";
static const String publicTopic = "iDom-client/command";
static const String subTopic = "swiatlo/output/#";
using namespace lkankowski;

#define xstr(a) str(a)
#define str(a) #a
const char *MULTI_RELAY_VERSION = xstr(SKETCH_VERSION);

#if defined(EXPANDER_PCF8574) || defined(EXPANDER_MCP23017)
#if defined(EXPANDER_PCF8574)
#include "PCF8574.h"
#define EXPANDER_PINS 8
#elif defined(EXPANDER_MCP23017)
#include "Adafruit_MCP23017.h"
#define EXPANDER_PINS 16
#endif
#define USE_EXPANDER
#include <Wire.h> // Required for I2C communication
#define E(expanderNo, ExpanderPin) (((expanderNo + 1) << 8) | (ExpanderPin))
#endif

// Configuration in separate file
#include "config.h"

#ifdef USE_EXPANDER
const int gNumberOfExpanders = sizeof(expanderAddresses);
#if defined(PCF8574_H)
PCF8574 gExpander[gNumberOfExpanders];
#elif defined(_Adafruit_MCP23017_H_)
Adafruit_MCP23017 gExpander[gNumberOfExpanders];
#endif
#endif

#ifdef DEBUG_STATS
bool debugStatsOn = false;
int loopCounter = 0;
unsigned long loopInterval = 0;
unsigned long loopCumulativeMillis = 0;
#endif
#ifdef DEBUG_STARTUP
unsigned long debugCounter = 0;
#endif

MyMessage myMessage; // MySensors - Sending Data
#if defined(DEBUG_COMMUNICATION) || defined(DEBUG_STATS)
MyMessage debugMessage(255, V_TEXT);
#endif

void callbackMqtt(char *topic, byte *payload, unsigned int length)
{
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    //Serial.print((char)payload[i]);
    mqttBuffor += (char)payload[i];
  }
  //Serial.println();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "lightClient";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //mqttClient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttClient.subscribe(subTopic.c_str());
      mqttClient.publish(publicTopic.c_str(), "log INFO polaczono arduino z MQTT");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}

#include <common.h>

Relay gRelay[gNumberOfRelays];
lkankowski::Button gButton[gNumberOfButtons];

void (*resetFunc)(void) = 0; //declare reset function at address 0

//iDom send state

void iDomSend(int releyID, int buttonID, int state)
{
  if (digitalRead(NETWORK_STATE) == HIGH || releyID == VIRTUAL_________RELAY){ // do not send for virtual reley or if network fiture is off
     //Serial.println("nie wysylam po ethernecie iDomSend()");
    return;
  }
  /* Serial3.print("state;");
  Serial3.print(releyID);
  Serial3.print(";");
  Serial3.print(buttonID);
  Serial3.print(";");
  Serial3.println(state);*/
  String msg = "light ";
  msg += "state;";
  msg += String(releyID);
  msg += ";";
  msg += String(buttonID);
  msg += ";";
  msg += String(state);
  mqttClient.publish(publicTopic.c_str(), msg.c_str());
}

void iDomSendAllBulbStatus()
{
  if(digitalRead(NETWORK_STATE) == HIGH){
    Serial.println("nie wysylam po ethernecie iDomSendAllBulbStatus()");
    return;
    }
  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++)
  {
    // myMessage.setSensor(gRelay[relayNum].getSensorId());
    // myMessage.setType(gRelay[relayNum].isSensor() ? V_TRIPPED : V_STATUS);
    // send(myMessage.set(gRelay[relayNum].getState())); // send current state
    iDomSend(gRelay[relayNum].getSensorId(), -1, gRelay[relayNum].getState());
  }
}

// MySensors - This will execute before MySensors starts up
void before()
{

#ifdef DEBUG_STARTUP
  Serial.println(String("# ") + (debugCounter++) + " Debug startup - common config: MONO_STABLE_TRIGGER=" + MONO_STABLE_TRIGGER + ", RELAY_IMPULSE_INTERVAL=" + RELAY_IMPULSE_INTERVAL + ", BUTTON_DEBOUNCE_INTERVAL=" + BUTTON_DEBOUNCE_INTERVAL + ", BUTTON_DOUBLE_CLICK_INTERVAL=" + BUTTON_DOUBLE_CLICK_INTERVAL + ", BUTTON_LONG_PRESS_INTERVAL=" + BUTTON_LONG_PRESS_INTERVAL + ", MULTI_RELAY_VERSION=" + MULTI_RELAY_VERSION);

#ifdef USE_EXPANDER
  Serial.println(String("# ") + (debugCounter++) + " Debug startup - expander config");
  for (int i = 0; i < gNumberOfExpanders; i++)
  {
    Serial.print(expanderAddresses[i]);
    Serial.print(",");
  }
  Serial.println();
#endif

  Serial.println(String("# ") + (debugCounter++) + " Debug startup - relay config");
  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++)
  {
    Serial.println(String("# ") + (debugCounter++) + " > " + gRelayConfig[relayNum].sensorId + ";" + gRelayConfig[relayNum].relayPin + ";" + gRelayConfig[relayNum].relayOptions + ";" + gRelayConfig[relayNum].relayDescription);
  }
  Serial.println(String("# ") + (debugCounter++) + " Debug startup - button config");
  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++)
  {
    Serial.println(String("# ") + (debugCounter++) + " > " + gButtonConfig[buttonNum].buttonPin + ";" + gButtonConfig[buttonNum].buttonType + ";" + gButtonConfig[buttonNum].clickRelayId + ";" + gButtonConfig[buttonNum].longClickRelayId + ";" + gButtonConfig[buttonNum].doubleClickRelayId + ";" + gButtonConfig[buttonNum].buttonDescription);
  }
  Serial.println(String("# ") + (debugCounter++) + " Debug startup - EEPROM (first value is version, relay state starts at " + RELAY_STATE_STORAGE + ")");
  Serial.print(String("# ") + (debugCounter++) + " ");
  for (int relayNum = 0; relayNum < gNumberOfRelays + 1; relayNum++)
  {
    Serial.print(EEPROM.read(relayNum));
    Serial.print(",");
  }
  Serial.println();
  Serial.println(String("# ") + (debugCounter++) + " Debug startup - buttons pin state");
  Serial.print(String("# ") + (debugCounter++) + " ");
  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++)
  {
    pinMode(gButtonConfig[buttonNum].buttonPin, INPUT_PULLUP);
  }
  delay(200);
  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++)
  {
    Serial.print(digitalRead(gButtonConfig[buttonNum].buttonPin));
    Serial.print(",");
  }
  Serial.println();
#endif

// validate config
#ifdef USE_EXPANDER
  //TODO: check if I2C pins are not used
  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++)
  {
    int pin = gRelayConfig[relayNum].relayPin;
    if (pin & 0xff00)
    {
      if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS))
      {
        Serial.println(String("Configuration failed - expander no or number of pins out of range for relay: ") + relayNum);
        delay(1000);
        assert(0);
      }
    }
  }
#endif
  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++)
  {
#ifdef USE_EXPANDER
    int pin = gButtonConfig[buttonNum].buttonPin;
    if (pin & 0xff00)
    {
      if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS))
      {
        Serial.println(String("Configuration failed - expander no or number of pins out of range for button: ") + buttonNum);
        delay(1000);
        assert(0);
      }
    }
#endif
    const char *failAction[] = {"OK", "click", "long-press", "double-click"};
    int fail = 0;
    if ((gButtonConfig[buttonNum].clickRelayId != -1) && (getRelayNum(gButtonConfig[buttonNum].clickRelayId) == -1))
      fail = 1;
    if ((gButtonConfig[buttonNum].longClickRelayId != -1) && (getRelayNum(gButtonConfig[buttonNum].longClickRelayId) == -1))
      fail = 2;
    if ((gButtonConfig[buttonNum].doubleClickRelayId != -1) && (getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId) == -1))
      fail = 3;
    if (fail)
    {
      Serial.println(String("Configuration failed - invalid '") + failAction[fail] + " relay ID' for button: " + buttonNum);
      delay(1000);
      assert(0);
    }
    // TODO: validate if pin is correct to the current board
  }

  // if version has changed, reset state of all relays
  bool versionChangeResetState = (CONFIG_VERSION == EEPROM.read(0)) ? false : true;

#ifdef USE_EXPANDER
  /* Start I2C bus and PCF8574 instance */
  for (int i = 0; i < gNumberOfExpanders; i++)
  {
    gExpander[i].begin(expanderAddresses[i]);
  }

  Relay::expanderInit(gExpander);
  lkankowski::Button::expanderInit(gExpander);
#endif

  // initialize relays
  Relay::setImpulseInterval(RELAY_IMPULSE_INTERVAL);

  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++)
  {

    gRelay[relayNum].initialize(relayNum, gRelayConfig[relayNum].sensorId, gRelayConfig[relayNum].relayDescription);
    gRelay[relayNum].attachPin(gRelayConfig[relayNum].relayPin);
    gRelay[relayNum].setModeAndStartupState(gRelayConfig[relayNum].relayOptions, versionChangeResetState);
    gRelay[relayNum].start();
  }

  // Setup buttons
  lkankowski::Button::Button::setEventIntervals(BUTTON_DOUBLE_CLICK_INTERVAL, BUTTON_LONG_PRESS_INTERVAL);
  lkankowski::Button::Button::setMonoStableTrigger(MONO_STABLE_TRIGGER);

  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++)
  {

    gButton[buttonNum].initialize(gButtonConfig[buttonNum].buttonType, gButtonConfig[buttonNum].buttonDescription);
    int clickActionRelayNum = getRelayNum(gButtonConfig[buttonNum].clickRelayId);
    gButton[buttonNum].setAction(clickActionRelayNum,
                                 getRelayNum(gButtonConfig[buttonNum].longClickRelayId),
                                 getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
    gButton[buttonNum].setDebounceInterval(BUTTON_DEBOUNCE_INTERVAL);
    gButton[buttonNum].attachPin(gButtonConfig[buttonNum].buttonPin);
    if (((gButtonConfig[buttonNum].buttonType & 0x0f) == REED_SWITCH) && (clickActionRelayNum > -1))
    {
      gRelay[clickActionRelayNum].reportAsSensor();
      gRelay[clickActionRelayNum].changeState(gButton[buttonNum].getRelayState(false));
    }
    else if (((gButtonConfig[buttonNum].buttonType & 0x0f) == DING_DONG) && (clickActionRelayNum > -1))
    {
      gRelay[clickActionRelayNum].changeState(gButton[buttonNum].getRelayState(false));
    }
  }

  if (versionChangeResetState)
  {
    // version has changed, so store new version in eeporom
    EEPROM.write(0, CONFIG_VERSION);
  }
}; // before()

// executed AFTER mysensors has been initialised
void setup()
{

  Serial.begin(115200);
  //Serial3.begin(9600);
  pinMode(NETWORK_STATE, INPUT_PULLUP); //init config button

  if (digitalRead(NETWORK_STATE) == LOW)
  {
    // setup ethernet communication using DHCP
    Serial.println("ETHERNET1");
    if (Ethernet.begin(mac) == 0)
    {
      Serial.println(F("Unable to configure Ethernet using DHCP"));
      for (;;)
        ;
    }

    Serial.println("ETHERNET2");
    // ip="192.168";
    for (int i = 2; i < 4; i++)
    {
      ip = ip + ".";
      ip = ip + String(Ethernet.localIP()[i]);
    }

    //---mac
    for (int i = 0; i < 6; i++)
    {
      if ((mac[i]) <= 0x0F)
      {
        MAC = MAC + "0"; //zet er zonodig een '0' voor
      }
      MAC = MAC + String((mac[i]), HEX);
      MAC = MAC + ":";
    }
    MAC[(MAC.length()) - 1] = '\0'; //verwijder laatst toegevoegde ":"

    Serial.println(F("Ethernet configured via DHCP"));
    Serial.print("IP address: ");
    Serial.println(Ethernet.localIP());
    Serial.println();

    // setup mqtt client
    mqttClient.setClient(ethClient);
    mqttClient.setServer(mqttBrokerIP.c_str(), mqttPort);

    mqttClient.setCallback(callbackMqtt);

    wdt_enable(WDTO_8S); //aktywujemy watchdog z argumentem czasu - w tej sytuacji 1 sekunda
    //wstawiamy w dowolnym miejscu w setup...od tego momentu watchdog już działa;)

    // Send initial state to MySensor Controller
    myMessage.setType(V_STATUS);

    Serial.println("konczymy setup");
  }
};

void loop()
{
  wdt_reset();
  if (digitalRead(NETWORK_STATE) == LOW)
  {
    if (!mqttClient.connected())
    {
      reconnect();
      iDomSendAllBulbStatus();
    }
    mqttClient.loop();
  }
  if (mqttBuffor.length() > 1)
  {
    //0;125;1;0;2;0
    /* auto command = Serial3.readStringUntil(';');

    if(command == "all"){
      Serial3.readStringUntil('\n');
      iDomSendAllBulbStatus();
    }

    int bulbID = Serial3.readStringUntil(';').toInt();
    (void)Serial3.readStringUntil(';');
    (void)Serial3.readStringUntil(';');
    (void)Serial3.readStringUntil(';');
    int state = Serial3.readStringUntil('\n').toInt();
    int relayNum = getRelayNum(bulbID);
    if (relayNum == -1)
      return;*/

    int bulbID;
    int state;
    int pos3, pos4, pos5;
    //0;125;1;0;2;0
    int cmd;
    //Serial.println(mqttBuffor);
    int n = sscanf(mqttBuffor.c_str(), "%i;%i;%i;%i;%i;%i", &cmd, &bulbID, &pos3, &pos4, &pos5, &state);
    if (cmd == 777)
    {
      Serial.println("komenda all");
      iDomSendAllBulbStatus();
      mqttBuffor = "";
      return;
    }
    int relayNum = getRelayNum(bulbID);
    if (relayNum == -1)
      return;

    //Serial.print("bulbID: ");
    //Serial.println(bulbID);
    gRelay[relayNum].changeState(state);
    // myMessage.setType(gRelay[relayNum].isSensor() ? V_TRIPPED : V_STATUS);
    // myMessage.setSensor(message.getSensor());
    // send(myMessage.set(message.getBool())); // support for OPTIMISTIC=FALSE (Home Asistant) //cyniu
    mqttBuffor = "";
    iDomSend(gRelay[relayNum].getSensorId(), -1, gRelay[relayNum].getState());
  }
#ifdef DEBUG_STATS
  unsigned long loopStartMillis = millis();
  if (loopCounter == 0)
  {
    loopCumulativeMillis = 0;
    loopInterval = loopStartMillis;
  }
#endif

  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++)
  {

    int relayNum = gButton[buttonNum].updateAndGetRelayNum(&mqttClient, publicTopic);
    if (relayNum > -1)
    {
      // mono/bi-stable button toggles the relay, ding-dong/reed-switch switch to exact state
      bool relayState = gButton[buttonNum].getRelayState(gRelay[relayNum].getState());

#ifdef IGNORE_BUTTONS_START_MS
      if (millis() > IGNORE_BUTTONS_START_MS)
      {
#endif
        if (gRelay[relayNum].changeState(relayState))
        {
          // myMessage.setType(gRelay[relayNum].isSensor() ? V_TRIPPED : V_STATUS);
          // myMessage.setSensor(gRelay[relayNum].getSensorId());
          // send(myMessage.set(relayState));
          iDomSend(gRelay[relayNum].getSensorId(), gButton[buttonNum]._pinCyniu, relayState);
        }
#ifdef IGNORE_BUTTONS_START_MS
      }
#endif
    }
  }

  if (Relay::isImpulsePending())
  {
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++)
    {
      if (gRelay[relayNum].impulseProcess())
      {
        myMessage.setSensor(gRelay[relayNum].getSensorId());
        // send(myMessage.set(0));
      }
    }
  }

#ifdef DEBUG_STATS
  if (debugStatsOn)
  {
    unsigned long loopIntervalCurrent = millis();
    loopCumulativeMillis += loopIntervalCurrent - loopStartMillis;
    loopCounter++;
    if (loopCounter > DEBUG_STATS)
    {
      Serial.println(String("# ") + DEBUG_STATS + " loop stats: (end-start)=" + (loopIntervalCurrent - loopInterval) + "ms, cumulative_loop_millis=" + loopCumulativeMillis + "ms");
      loopCounter = 0;
      debugStatsOn = false;
    }
  }
#endif
};

// MySensors - Presentation - Your sensor must first present itself to the controller.
// Executed after "before()" and before "setup()"
void presentation()
{
  // sendSketchInfo(MULTI_RELAY_DESCRIPTION, MULTI_RELAY_VERSION);

  // Register every relay as separate sensor
  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++)
  {
    //  present(gRelay[relayNum].getSensorId(),
    //         gRelay[relayNum].isSensor() ? S_DOOR : S_BINARY,
    //         gRelay[relayNum].getDescription());
  }
};

// MySensors - Handling incoming messages
// Nodes that expects incoming data, must implement the receive() function to handle the incoming messages.
void receive(const MyMessage &message)
{

#ifdef DEBUG_COMMUNICATION
  Serial.println(String("# Incoming message: sensorId=") + message.getSensor() + ", command=" + message.getCommand() + ", ack=" + message.isAck() + ", echo=" + message.isEcho() + ", type=" + message.getType() + ", payload=" + message.getString());
#endif
  if (message.getCommand() == C_SET)
  {
    if (message.getType() == V_STATUS)
    {
      int relayNum = getRelayNum(message.getSensor());
      if (relayNum == -1)
        return;
      gRelay[relayNum].changeState(message.getBool());
      // myMessage.setType(gRelay[relayNum].isSensor() ? V_TRIPPED : V_STATUS);
      // myMessage.setSensor(message.getSensor());
      // send(myMessage.set(message.getBool())); // support for OPTIMISTIC=FALSE (Home Asistant) //cyniu
      iDomSend(gRelay[relayNum].getSensorId(), -1, gRelay[relayNum].getState());
#ifdef DEBUG_STATS
    }
    else if (message.getType() == V_VAR1)
    {
      int debugCommand = message.getInt();
      if (debugCommand == 1)
      {
        debugStatsOn = message.getBool();
        loopCounter = 0;
        send(debugMessage.set("toogle debug stats"));
      }
      else if (debugCommand == 2)
      {
        for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++)
        {
          Serial.println(String("# Sensor ") + gRelay[relayNum].getSensorId() + " state=" + gRelay[relayNum].getState() + "; " + gRelay[relayNum].getDescription());
        }
      }
      else if (debugCommand == 3)
      {
        for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++)
        {
          Serial.println(String("# Button ") + buttonNum + ": " + gButton[buttonNum].toString());
        }
      }
      else if (debugCommand == 4)
      { // dump EEPROM
        Serial.print("# Dump EEPROM: ");
        for (int relayNum = 0; relayNum < gNumberOfRelays + RELAY_STATE_STORAGE; relayNum++)
        {
          Serial.print(EEPROM.read(relayNum));
          Serial.print(",");
        }
        Serial.println();
      }
      else if (debugCommand == 5)
      { // clear EEPROM & reset
        for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++)
        {
          EEPROM.write(RELAY_STATE_STORAGE + relayNum, 0);
        }
        resetFunc();
      }
      else if (debugCommand == 6)
      { // reset
        resetFunc();
      }
#endif
    }
  }
};
