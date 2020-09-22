/*
 ##########################################
 #####   Bayang Tx using arduino +   ######
 #####  nRF24L01 to get PPM output   ######
 ##########################################
 #             by 0zoon0                  #
 #                                        #
 #   Most of this project is derived      #
 #     from bikemike/nrf24_multipro.      #
 #   The PPM code got from                # 
 #http://forum.arduino.cc/index.php?topic=309672.0#
 ##########################################
 */

#include "iface_nrf24l01.h"
#include "Joystick.h"

// uncomment to get some debug info
#define DEBUG
//#define USE_POS_VALUES

// ############ Wiring ################
//SPI Comm.pins with nRF24L01
//IF you change pin numbers you'll need to adjust SPI outputs too
#define CE_pin    9  
#define CS_pin    10 
#define LED_pin 24
#define LED_pin_connected 17

#define CE_on digitalWrite(CS_pin, 1)   
#define CE_off digitalWrite(CS_pin, 0)
#define CS_on digitalWrite(CE_pin, 1)  
#define CS_off digitalWrite(CE_pin, 0)
#define RF_POWER TX_POWER_158mW 
//TX_POWER_5mW  80 20 158

////////////////////// PPM CONFIGURATION//////////////////////////
#define channel_number 6  //set the number of channels
#define sigPin 2  //set PPM signal output pin on the arduino
#define PPM_FrLen 27000  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_PulseLen 400  //set the pulse length
//////////////////////////////////////////////////////////////////

//TODO: transmitter ID is not really used
uint8_t transmitterID[4];
uint8_t packet[32];
static uint8_t emptyPacketsCount = 320;
struct MyData {
  uint16_t throttle;
  uint16_t yaw;
  uint16_t pitch;
  uint16_t roll;
  byte aux1;
  uint8_t button0;
  uint8_t button1;
  uint8_t button2;
  uint8_t button3;
  // can add four more auxs
};

MyData data;
MyData tmp;
Joystick_ Joystick;
    
void setup()
{
    pinMode(CS_pin, OUTPUT);
    pinMode(CE_pin, OUTPUT);
    pinMode(LED_pin, OUTPUT);
    pinMode(LED_pin_connected, OUTPUT);
    digitalWrite(LED_pin, 1);
    
    delay(5000);
#ifdef DEBUG
    Serial.begin(115200);
    Serial.println("starting radio");
#endif

    Joystick.setXAxisRange(0, 1024);//Channel 1
    Joystick.setYAxisRange(0, 1024);//Channel 2
    Joystick.setRxAxisRange(0,1024);//Channel 3
    Joystick.setZAxisRange(0, 1024);//Channel 4

    
    Joystick.begin();
    resetData();
    delay(50);
    SPI_Begin(); 
    NRF24L01_Reset();
    NRF24L01_Initialize();
    Bayang_init();
    Bayang_bind();
    digitalWrite(LED_pin, 0);
}

void loop()
{
    // process protocol
    Bayang_recv_packet(&tmp);
    if (tmp.throttle==0 && tmp.yaw==0 && tmp.pitch==0 && tmp.roll==0) {
      emptyPacketsCount++;
      if (emptyPacketsCount >= 320) {
        // signal lost?
        resetData(); //set invalid values so that fc can enter fail safe
        emptyPacketsCount = 320;
      }
    }
    else {
      
      data = tmp;
      emptyPacketsCount = 0;
      static int iii;
      if (++iii==200)
      {
          iii=0;
      }
      digitalWrite(LED_pin_connected, (iii<100));
      digitalWrite(LED_pin, (iii>100));
      setPPMValuesFromData();
    }
}

void resetData() 
{
  //TODO: check safe values
  // 'safe' values to use when no radio input is detected
  data.throttle = 0;
  data.yaw = 511;
  data.pitch = 511;
  data.roll = 511;
  data.aux1 = 0;
  
  setPPMValuesFromData();
}

void setPPMValuesFromData()
{
  #ifdef USE_POS_VALUES
  Joystick.setXAxis(512+data.roll/2); //Channel 1
  Joystick.setYAxis(1024-data.pitch/2); //Channel 2
  Joystick.setZAxis(512+data.throttle/2); //Channel 3
  Joystick.setRxAxis(512+data.yaw/2); //Channel 4

  #else
  Joystick.setXAxis(data.roll); //Channel 1
  Joystick.setYAxis(1024-data.pitch); //Channel 2
  Joystick.setZAxis(data.throttle); //Channel 3
  Joystick.setRxAxis(data.yaw); //Channel 4
  #endif
  
  Joystick.setButton(0, data.button0);
  Joystick.setButton(1, data.button1);
  Joystick.setButton(2, data.button2);
  Joystick.setButton(3, data.button3);
}

