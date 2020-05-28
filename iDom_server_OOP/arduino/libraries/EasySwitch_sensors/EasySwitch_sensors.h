#ifndef EasySwitch_sensors_h
#define EasySwitch_senosrs_h
#include <EEPROM.h>
#include "Arduino.h"
#include <avr/pgmspace.h> 
#include <Wire.h>    
#include "Adafruit_MCP23017.h"
const PROGMEM uint16_t sensorId_t[] = {
100,101,101,102,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,999,};

const PROGMEM uint16_t relay_t[] = {
127,128,128,129,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,38,};

const PROGMEM uint8_t relayOptions_t[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

const PROGMEM uint8_t relayStartup_t[] = {
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,};

const PROGMEM uint8_t relayType_t[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

const PROGMEM int button_t[] = {
-1,49,48,52,44,-1,40,28,35,21,-1,-1,47,46,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,36,41,-1,-1,-1,-1,-1,-1,};

const PROGMEM uint16_t button_type_t[] = {
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,};

const PROGMEM int button_next_t[] = {
-1,49,48,52,44,-1,40,28,35,21,-1,-1,47,46,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,36,41,-1,-1,-1,-1,-1,-1,};

const PROGMEM uint16_t button_click_t[] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,};

const PROGMEM byte one_button_or_more_t[] = {
1,0,0,0,0,1,0,0,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,};

const char s_0[] PROGMEM = "";
const char s_1[] PROGMEM = "maly_pokoj";
const char s_2[] PROGMEM = "maly_pokoj";
const char s_3[] PROGMEM = "korytarz";
const char s_4[] PROGMEM = "korytarz";
const char s_5[] PROGMEM = "";
const char s_6[] PROGMEM = "lazienka";
const char s_7[] PROGMEM = "salon_nad_stolem";
const char s_8[] PROGMEM = "wiatrolap";
const char s_9[] PROGMEM = "Kanciapa";
const char s_10[] PROGMEM = "";
const char s_11[] PROGMEM = "";
const char s_12[] PROGMEM = "pokojDzieci";
const char s_13[] PROGMEM = "pokojDzieci";
const char s_14[] PROGMEM = "";
const char s_15[] PROGMEM = "";
const char s_16[] PROGMEM = "";
const char s_17[] PROGMEM = "";
const char s_18[] PROGMEM = "";
const char s_19[] PROGMEM = "";
const char s_20[] PROGMEM = "";
const char s_21[] PROGMEM = "";
const char s_22[] PROGMEM = "";
const char s_23[] PROGMEM = "";
const char s_24[] PROGMEM = "";
const char s_25[] PROGMEM = "";
const char s_26[] PROGMEM = "";
const char s_27[] PROGMEM = "salon_kinkiet";
const char s_28[] PROGMEM = "lazienka";
const char s_29[] PROGMEM = "";
const char s_30[] PROGMEM = "";
const char s_31[] PROGMEM = "";
const char s_32[] PROGMEM = "";
const char s_33[] PROGMEM = "";
const char s_34[] PROGMEM = "gniazdko_taras";

const char *const relayDescription[] PROGMEM = {
  s_0,  s_1,  s_2,  s_3,  s_4,  s_5,  s_6,  s_7,  s_8,  s_9,  s_10,  s_11,  s_12,  s_13,  s_14,  s_15,  s_16,  s_17,  s_18,  s_19,  s_20,  s_21,  s_22,  s_23,  s_24,  s_25,  s_26,  s_27,  s_28,  s_29,  s_30,  s_31,  s_32,  s_33,  s_34,};

class EasySwitch_sensors {
public:
	EasySwitch_sensors();
	void begin();
	void sendPresentation(String version , String name);
	void sendState(int id, bool state, bool ack, int buttonPin = -1);
	void sendPresentationId();
	void sendAllState();
	void saveState(int relay, byte state);
	void checkEEprom();
	void EEpromSetAllZero();
	int readState(int relay); 
	void setAllPinMode();
	void checking();
	void createButton(int pin, int id);
	void Click(int i, int button, byte click);
	void buttonType3(int i, int button);
	void buttonType2(int i, int button);
	uint16_t check();
	uint8_t getState();
	void SetOutputArdu(int pin, bool state);
	void SetOutputMCP(int pin, bool state);
	void MCP_begin();
	int GetMcpEx(int in); 
	int GetMcpPin(int ex,int in); 
	bool getTrigger(int i , int relay, bool relayState);
	void loadStateAndSetOutput();
	void serialIn();
	void parseIn();
	void parseAll();
private:
	unsigned long _lastTime; 
	unsigned long _lastTime1s; 
	uint8_t _buttonTimeStart = 12; 
	uint8_t _buttonOneDebounce = 5;
	uint8_t _buttonDoubleStart = 9;
	uint8_t _buttonDoubleStop = 2;
	uint8_t _buttonDoubleDebounce = 3;
	uint8_t _buttonLongDebounce = 10;
	String _serialIn; 
	uint8_t _serialCount;
	unsigned char _frame_in[50];
	uint16_t _node_id;
	uint16_t _child_sensor_id;
	uint16_t _command;
	uint8_t _ack;
	uint16_t _type;
	uint16_t _payload;
	char _charBuf[50]; 
	uint8_t expanderAddresses[8];
	const int numberOfExpanders = sizeof(expanderAddresses);
	Adafruit_MCP23017 expander[8];
byte _eepromerrase =74;
uint8_t _buttonTime[35] ;
uint8_t _buttonState[35];
uint16_t _numberOfRelayButtons = 35;
bool _oneClickPress[35];
String g_sketch_name = "EasySwitch";
};
#endif
