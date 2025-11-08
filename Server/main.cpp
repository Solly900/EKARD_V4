/* -- includes -------------------------------------------- */
/* -------------------------------------------------------- */
#include "EKARD.h"
#include <WiFi.h>
#include <ESP32Servo.h>
#include <EEPROM.h>

/* --- function declarations ------------------------------ */
void EKARD_MoveAxes();
bool EKARD_ProcessReceivedData();
void EKARD_CalibrateAxis0();
void EKARD_AutoHome();
void EKARD_ReadDataFromMemory();
void EKARD_SaveDataToMemory(uint8_t SaveCommand);
void EKARD_SendEKARDData(WiFiClient *client);
void EKARD_Demo0();
void EKARD_Demo1();
void EKARD_Demo2();

//only to be called to initialize or reset the memory area of each microcontroller
//WARNING: overwrites saved EKARD values
void EKARD_OneTimeMemoryInit();

/* --- variables ------------------------------------------ */
/* -------------------------------------------------------- */

/* --- Wifi settings --- */
const char* EKARD_ssid = "EKARD_the_IV";
const char* EKARD_pass = "123456789";
WiFiServer EKARD_server(EKARD_Port);

#ifdef inWlan
IPAddress EKARD_ip(192, 168, 178, 201);
IPAddress EKARD_gateway(192, 168, 0, 1);
IPAddress EKARD_subnet(255, 255, 0, 0);
const char* ssid = "FRITZ!Box WGLAN";
const char* password = "1chMoecht31nt3rn3t<3";
#endif


/* --- global working variables --- */
EKARD EKARD_; //= data from flash memory 
Servo Axis[ServoNum];
uint8_t TCPStream[TCPStreamLength];
const int AxisPins[ServoNum] = {Servo0Pin, Servo1Pin, Servo2Pin, Servo3Pin, Servo4Pin, Servo5Pin};
int Command = COM_IDLE;
float OldPos[ServoNum]; //holds old axes positions



/* --- setup ---------------------------------------------- */
/* -------------------------------------------------------- */
void setup() {

  Serial.begin(115200);
/* --- init EKARD server --- */
#ifdef inWlan
  // Configures static IP address
  if (!WiFi.config(EKARD_ip, EKARD_gateway, EKARD_subnet)) {
    Serial.println("STA Failed to configure");
  }
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#else
  WiFi.softAP(EKARD_ssid, EKARD_pass); //ESP32 with IP-Address 192.168.4.1 ->standard gateway
  IPAddress IPcheck = WiFi.softAPIP(); 
  Serial.print("IP address is:");
  Serial.println(IPcheck);
#endif
  EKARD_server.begin();
  EEPROM.begin(EEPROM_DataSize);

//only to be called to initialize or reset the memory area of each microcontroller
//WARNING: overwrites saved EKARD values
/*
#warning "your doing a memory INITIALIZATION / RESET"
 EKARD_OneTimeMemoryInit();
//*/

  //read in saved data
  //!!needs to get initialized on microcontroller (before µC is attached to EKARD) 
  //-> InitOneTimeFunc needs to be called once per Microcontroller to initialize EEPROM values
  //otherwise Axis 0 Time gets initialized with uint16_t max value (65535) ->AutoHome takes forever and Axis values are not valid 
  EKARD_ReadDataFromMemory();

/* --- init EKARD servos --- */
  for(int i = 1; i < ServoNum; i++){
    Axis[i].setPeriodHertz(50);
    Axis[i].attach(AxisPins[i], EKARD_.EKARD_Axis[i].minPulse, EKARD_.EKARD_Axis[i].maxPulse);
  }
  Axis[0].setPeriodHertz(50);
  pinMode(HallSensorPin, INPUT);
  EKARD_AutoHome();

  for(int i = 0; i < ServoNum; i++){
    OldPos[i] = (float)EKARD_.EKARD_Axis[i].angle;
  }

}



/* --- loop ----------------------------------------------- */
/* -------------------------------------------------------- */
void loop() {
  WiFiClient EKARD_Client = EKARD_server.available();
  if (EKARD_Client) {
    if(EKARD_Client.connected())
    {
      Serial.println("Client Connected");
    }
    
/* --- start of communication loop --- */
    while(EKARD_Client.connected()){  
      //receive data    
      if(EKARD_Client.available() > 0){
        Serial.println("Client is available");
        Serial.println("----------------------------------------------------------------------------");
        for(int i = 0; i < TCPStreamLength; i++){
          TCPStream[i] = EKARD_Client.read();
        } 
        if(EKARD_ProcessReceivedData()){ //if data was valid
          //send back acknowledge
          if(Command == COM_REQUEST_EKARD_DATA){
            EKARD_SendEKARDData(&EKARD_Client);
          }
          TCPStream[FirstDataPosTCPStream - 1] = Command;

        }
        else{
          TCPStream[FirstDataPosTCPStream - 1] = COM_RECEIVED_INCORRECT_DATA;
        }

        EKARD_Client.write(TCPStream, TCPStreamLength);
        Serial.println("send data back");
      }
    }
    
/* --- end of communication loop --- */  
    EKARD_Client.stop();
    Serial.println("Client disconnected");  
    Serial.println("------------------------------------------------------------------------");  
  }
  Serial.println("waiting for connection...");
  delay(1000);
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------ function definitions --------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_MoveAxes (){
  Serial.println("Moving EKARD axes in new positions");

  int PosDiffs[ServoNum];
  float StepSize[ServoNum];
  for(int i = 1; i < ServoNum; i++){
    PosDiffs[i] = EKARD_.EKARD_Axis[i].angle - OldPos[i];
    StepSize[i] = (float)PosDiffs[i] / EKARD_.Axes_Speed;
  } 

  
  Serial.println("moving axes");
  Serial.println(EKARD_.Axes_Speed);
  for(int i = 0; i < EKARD_.Axes_Speed; i++){
    for(int j = 1; j < ServoNum; j++){
      OldPos[j] += StepSize[j];
      Axis[j].write((int)OldPos[j]);
      delay(5);
    }
  }

  Serial.println("moving axis0");
  Axis[0].attach(Servo0Pin, EKARD_.EKARD_Axis[0].minPulse, EKARD_.EKARD_Axis[0].maxPulse);
  if(PosDiffs[0] < 0){
    Axis[0].write(90 + EKARD_.Axis_0Speed); //negative Drehrichtung
    delay(abs(PosDiffs[0]) * EKARD_.Axis_0Time);
    Axis[0].write(90);
  }
  else if(PosDiffs[0] > 0){
    Axis[0].write(90 - EKARD_.Axis_0Speed);  //positive Drehrichtung
    delay(PosDiffs[0] * EKARD_.Axis_0Time);
    Axis[0].write(90);
  }
  Axis[0].detach();
  Serial.println("got into new position");
  return;
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_SetPulseValues(){
  Serial.println("Setting new Pulse values");

  for(int i = 1; i > ServoNum; i++){
    Axis[i].detach();
    Axis[i].attach(AxisPins[i], EKARD_.EKARD_Axis[i].minPulse, EKARD_.EKARD_Axis[i].maxPulse);
  }
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
bool EKARD_ProcessReceivedData(){
   //check if data is valid
      if(TCPStream[0] == 0xAA && TCPStream[1] == 0xBB && TCPStream[TCPStreamLength - 2] == 0xDD && TCPStream[TCPStreamLength - 1] == 0xEE){
        Serial.println("received data correctly");

        //processing data
        Command = TCPStream[FirstDataPosTCPStream - 1];
        int j = 0;
        switch(Command){
          case COM_SET_POSITION:
            for(int i = 0; i < ServoNum; i++){
              EKARD_.EKARD_Axis[i].angle = (uint16_t)(TCPStream[FirstDataPosTCPStream + j++] << 8);
              EKARD_.EKARD_Axis[i].angle |= (uint16_t)(TCPStream[FirstDataPosTCPStream + j++]);
              Serial.print("Angle for Axis ");
              Serial.print(i);
              Serial.print(" is: ");
              Serial.println(EKARD_.EKARD_Axis[i].angle);
            }
            EKARD_MoveAxes();
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SET_MIN:
            for(int i = 0; i < ServoNum; i++){
              EKARD_.EKARD_Axis[i].minPulse = (uint16_t)(TCPStream[FirstDataPosTCPStream + j++] << 8);
              EKARD_.EKARD_Axis[i].minPulse |= (uint16_t)(TCPStream[FirstDataPosTCPStream + j++]);
              Serial.print("Min value for Axis ");
              Serial.print(i);
              Serial.print(" is: ");
              Serial.println(EKARD_.EKARD_Axis[i].minPulse);
            }
            EKARD_SetPulseValues();
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SET_MAX:
            for(int i = 0; i < ServoNum; i++){
              EKARD_.EKARD_Axis[i].maxPulse = (uint16_t)(TCPStream[FirstDataPosTCPStream + j++] << 8);
              EKARD_.EKARD_Axis[i].maxPulse |= (uint16_t)(TCPStream[FirstDataPosTCPStream + j++]);
              Serial.print("Max value for Axis ");
              Serial.print(i);
              Serial.print(" is: ");
              Serial.println(EKARD_.EKARD_Axis[i].maxPulse);
            }
            EKARD_SetPulseValues();
            Command = COM_RECEIVED_DATA;
            break;
          case COM_AUTO_HOME:
            EKARD_AutoHome();
            Command = COM_RECEIVED_DATA;
            break;
          case COM_CALIBRATE_A0:
            EKARD_CalibrateAxis0();
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SAVE_CURRENT_HOME_POS:
            EKARD_SaveDataToMemory(COM_SAVE_CURRENT_HOME_POS);
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SAVE_MIN_MAX:
            EKARD_SaveDataToMemory(COM_SAVE_MIN_MAX);
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SAVE_AXIS0_TIME:
            EKARD_SaveDataToMemory(COM_SAVE_AXIS0_TIME);
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SAVE_AXIS0_SPEED:
            EKARD_SaveDataToMemory(COM_SAVE_AXIS0_SPEED);
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SAVE_MAX_TIME_DIFF:
            EKARD_SaveDataToMemory(COM_SAVE_MAX_TIME_DIFF);
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SAVE_CALIBRATION_FACTOR:
            EKARD_SaveDataToMemory(COM_SAVE_CALIBRATION_FACTOR);
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SAVE_AXES_SPEED:
            EKARD_SaveDataToMemory(COM_SAVE_AXES_SPEED);
            Command = COM_RECEIVED_DATA;
            break;
          case COM_RUN_DEMO_0:
            EKARD_Demo0();
            Command = COM_RECEIVED_DATA;
            break;
          case COM_RUN_DEMO_1:
            EKARD_Demo1();
            Command = COM_RECEIVED_DATA;
            break;
          case COM_RUN_DEMO_2:
            EKARD_Demo2();
            Command = COM_RECEIVED_DATA;
            break;
          case COM_REQUEST_EKARD_DATA:
            //EKARD_SendEKARDData();
            Command = COM_REQUEST_EKARD_DATA;
            break;
          case COM_RECEIVED_DATA:
            Serial.println("Data got correctly received");
            Command = COM_RECEIVED_DATA;
            break;
          case COM_SET_AXIS0_SPEED:
            EKARD_.Axis_0Speed = TCPStream[FirstDataPosTCPStream];
            Command = COM_RECEIVED_DATA;
            Serial.print("Set a new speed for Axis 0: ");
            Serial.println(EKARD_.Axis_0Speed);
            break;
          case COM_SET_MAX_TIME_DIFF:
            EKARD_.MaxTimeDiff = (TCPStream[FirstDataPosTCPStream] << 8);
            EKARD_.MaxTimeDiff |= TCPStream[FirstDataPosTCPStream + 1];
            Command = COM_RECEIVED_DATA;
            Serial.print("Set a new max time diff: ");
            Serial.println(EKARD_.MaxTimeDiff);
            break;
          case COM_SET_CALIBRATION_FACTOR:
            EKARD_.CalibrationFactor = (TCPStream[FirstDataPosTCPStream] << 8);
            EKARD_.CalibrationFactor |= TCPStream[FirstDataPosTCPStream + 1];
            Command = COM_RECEIVED_DATA;
            Serial.print("Set a new calibration factor for Axis 0: ");
            Serial.println(EKARD_.CalibrationFactor);
            break;
          case COM_SET_AXES_SPEED:
            EKARD_.Axes_Speed = (TCPStream[FirstDataPosTCPStream] << 8);
            EKARD_.Axes_Speed |= TCPStream[FirstDataPosTCPStream + 1];
            Command = COM_RECEIVED_DATA;
            Serial.print("Set a new speed for Axes 1 - 5: ");
            Serial.println(EKARD_.Axes_Speed);
            break;
          case COM_SET_CURRENT_POS_AS_HOME:
            Serial.print("Set a new Home Position: ");
            for(int i = 0; i < ServoNum; i++){
              EKARD_.HomePosition[i] = EKARD_.EKARD_Axis[i].angle;
              Serial.print(EKARD_.HomePosition[i]);
              Serial.print(", ");
            }
            Serial.println(" ");
            Command = COM_RECEIVED_DATA;
            break;
          case COM_RECEIVED_INCORRECT_DATA:
            Serial.println("Data was incorrectly received");
            Command = COM_REQUEST_EKARD_DATA;
            break;
            //???!
          default:
            Serial.println("Command unknown");
            break;
        } 
        return 1;    
      }
      else{
        Command = COM_RECEIVED_INCORRECT_DATA;
        Serial.println("did not receive the correct data"); 
        return 0;
      }
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_CalibrateAxis0(){
  Serial.println("calibrating axis 0");

  float timeCounter0 = 0;
  float timeCounter1 = 0;

  Axis[0].attach(Servo0Pin, EKARD_.EKARD_Axis[0].minPulse, EKARD_.EKARD_Axis[0].maxPulse);
  Axis[0].write(90 - EKARD_.Axis_0Speed);
  while (digitalRead(HallSensorPin) == HIGH){
    Serial.println(digitalRead(HallSensorPin));
  }
  Axis[0].write(90);
  delay(10);

  /*calib forward time*/
  Axis[0].write(90 - EKARD_.Axis_0Speed);
  while (digitalRead(HallSensorPin) == HIGH){
    timeCounter0++;
    delay(1);
  }
  Axis[0].write(90);
  delay(10);

  /*calib backwards time*/
  Axis[0].write(90 + EKARD_.Axis_0Speed);
  while (digitalRead(HallSensorPin) == HIGH){
    timeCounter1++;
    delay(1);
  }
  Axis[0].write(90);
  delay(10);

  int timediff = timeCounter0 - timeCounter1;
  if(timediff < (-EKARD_.MaxTimeDiff) || timediff > EKARD_.MaxTimeDiff){
    if(timediff < 0){ //min value is too small
      EKARD_.EKARD_Axis[0].minPulse += (timediff / (-EKARD_.CalibrationFactor));
    }
    else if(timediff > 0){ //max value is too great
      EKARD_.EKARD_Axis[0].maxPulse -= (timediff / (EKARD_.CalibrationFactor));
    }
    EKARD_CalibrateAxis0(); //recursive calibration
  }
  else{
    Axis[0].detach();
    EKARD_.Axis_0Time = (uint16_t) timeCounter0 / 360.0f;
  }
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_ReadDataFromMemory(){
  /*
  Order of saved data as follows (34 B in total):

  0-1: Axis 0 HomePosition
  2-6: Axis 1-5 HomePositions
  7-18: Axis 0-5 MIN PulseWidth values (two consecutive bytes holding one value)
  19-30: Axis 0-5 MAX PulseWidth values (two consecutive bytes holding one value)
  31-32: Axis 0 Time
  33: Axis 0 Speed
  34-35: MaxTimeDiff
  36-37: Calibration factor
  38-39: Axes move time

  */
  Serial.println("Reading data from memory");

  //reading HomePosition
  EKARD_.HomePosition[0] = (EEPROM.read(0) << 8);
  EKARD_.HomePosition[0] |= EEPROM.read(1);
  for(int i = 0; i < (ServoNum - 1); i++){
    EKARD_.HomePosition[i + 1] = EEPROM.read(i + 2);
  }


  //reading MIN / MAX values
  int j = 0;
  int k = 0;
  for(int i = 0; i < ServoNum; i++){
    EKARD_.EKARD_Axis[i].minPulse = (EEPROM.read(7 + j++) << 8);
    EKARD_.EKARD_Axis[i].minPulse |= EEPROM.read(7 + j++);
    EKARD_.EKARD_Axis[i].maxPulse = (EEPROM.read(19 + k++) << 8);
    EKARD_.EKARD_Axis[i].maxPulse |= EEPROM.read(19 + k++);
  }

  //reading Axis 0 time
  EKARD_.Axis_0Time = (EEPROM.read(31) << 8);
  EKARD_.Axis_0Time |= EEPROM.read(32);

  //reading Axis 0 speed
  EKARD_.Axis_0Speed = EEPROM.read(33);

  //reading MaxTimeDiff
  EKARD_.MaxTimeDiff = (EEPROM.read(34) << 8);
  EKARD_.MaxTimeDiff |= EEPROM.read(35);

  //reading calibration factor
  EKARD_.CalibrationFactor = (EEPROM.read(36) << 8);
  EKARD_.CalibrationFactor |= EEPROM.read(37);

  //reading Axes_Speed
  EKARD_.Axes_Speed = (EEPROM.read(38) << 8);
  EKARD_.Axes_Speed |= EEPROM.read(39);
  
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_SaveDataToMemory(uint8_t SaveCommand){
  /*
  Order of saved data as follows (40 B in total):

  0-1: Axis 0 HomePosition
  2-6: Axis 1-5 HomePositions
  7-18: Axis 0-5 MIN PulseWidth values (two consecutive bytes holding one value)
  19-30: Axis 0-5 MAX PulseWidth values (two consecutive bytes holding one value)
  31-32: Axis 0 Time
  33: Axis 0 Speed
  34-35: MaxTimeDiff
  36-37: Calibration factor
  38-39: Axes move time

  */
  Serial.print("Saving Data to memory with Command: ");
  Serial.println(Command);

  if(SaveCommand == COM_SAVE_CURRENT_HOME_POS){
    if(EEPROM.read(0) != (EKARD_.HomePosition[0] >> 8)){
      EEPROM.write(0, (EKARD_.HomePosition[0] >> 8));
    }
    if(EEPROM.read(1) != (EKARD_.HomePosition[0] & 0xFF)){
      EEPROM.write(1, (EKARD_.HomePosition[0] & 0xFF));
    }
    for(int i = 0; i < (ServoNum - 1); i++){
      if(EEPROM.read(i + 2) != EKARD_.HomePosition[i + 1]){
        EEPROM.write((i + 2), EKARD_.HomePosition[i + 1]);
      }
    }
  }
  
  else if(SaveCommand == COM_SAVE_MIN_MAX){
    int j = 7;
    for(int i = 0; i < ServoNum; i++){
      if(EEPROM.read(j) != (EKARD_.EKARD_Axis[i].minPulse >> 8)){
        EEPROM.write(j++, (EKARD_.EKARD_Axis[i].minPulse >> 8));
      }
      else j++;
      if(EEPROM.read(j) != (EKARD_.EKARD_Axis[i].minPulse & 0xFF)){
        EEPROM.write(j++, (EKARD_.EKARD_Axis[i].minPulse & 0xFF));
      }
      else j++;
    }
    for(int i = 0; i < ServoNum; i++){
      if(EEPROM.read(j) != (EKARD_.EKARD_Axis[i].maxPulse >> 8)){
        EEPROM.write(j++, (EKARD_.EKARD_Axis[i].maxPulse >> 8));
      }
      else j++;
      if(EEPROM.read(j) != (EKARD_.EKARD_Axis[i].maxPulse & 0xFF)){
        EEPROM.write(j++, (EKARD_.EKARD_Axis[i].maxPulse & 0xFF));
      }
      else j++;
    }
  }

  else if(SaveCommand == COM_SAVE_AXIS0_TIME){
    if(EEPROM.read(31) != (EKARD_.Axis_0Time >> 8)){
      EEPROM.write(31, (EKARD_.Axis_0Time >> 8));
    }
    if(EEPROM.read(32) != (EKARD_.Axis_0Time & 0xFF)){
      EEPROM.write(32, (EKARD_.Axis_0Time & 0xFF));
    }
  }

  else if(SaveCommand == COM_SAVE_AXIS0_SPEED){
    if(EEPROM.read(33) != EKARD_.Axis_0Time){
      EEPROM.write(33, EKARD_.Axis_0Time);
    }
  }

  else if(SaveCommand == COM_SAVE_MAX_TIME_DIFF){
    if(EEPROM.read(34) != (EKARD_.MaxTimeDiff >> 8)){
      EEPROM.write(34, (EKARD_.MaxTimeDiff >> 8));
    }
    if(EEPROM.read(35) != (EKARD_.MaxTimeDiff & 0xFF)){
      EEPROM.write(35, (EKARD_.MaxTimeDiff & 0xFF));
    }
  }

  else if(SaveCommand == COM_SAVE_CALIBRATION_FACTOR){
    if(EEPROM.read(36) != (EKARD_.CalibrationFactor >> 8)){
      EEPROM.write(36, (EKARD_.CalibrationFactor >> 8));
    }
    if(EEPROM.read(37) != (EKARD_.CalibrationFactor & 0xFF)){
      EEPROM.write(37, (EKARD_.CalibrationFactor & 0xFF));
    }
  }

  else if(SaveCommand == COM_SAVE_AXES_SPEED){
    if(EEPROM.read(38) != (EKARD_.Axes_Speed >> 8)){
      EEPROM.write(38, (EKARD_.Axes_Speed >> 8));
    }
    if(EEPROM.read(39) != (EKARD_.Axes_Speed & 0xFF)){
      EEPROM.write(39, (EKARD_.Axes_Speed & 0xFF));
    }
  }
  EEPROM.commit();
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_AutoHome(void){
  Serial.println("Setting HomePosition");
  for(int i = 0; i < ServoNum; i++){
    EKARD_.EKARD_Axis[i].angle = EKARD_.HomePosition[i];
  }
  EKARD_MoveAxes();
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_SendEKARDData(WiFiClient *client){
  Serial.println("Sending EKARD data to Client");

  uint8_t DataStream[EEPROM_DataSize + 5];
  int j = 0;

  DataStream[j++] = 0xAA;
  DataStream[j++] = 0xBB;
  DataStream[j++] = Command;

  DataStream[j++] = (EKARD_.EKARD_Axis[0].angle >> 8);
  DataStream[j++] = (EKARD_.EKARD_Axis[0].angle & 0xFF);
  for(int i = 1; i < (ServoNum); i++){
    DataStream[j++] = EKARD_.EKARD_Axis[i].angle;
  }
  for(int i = 0; i < ServoNum; i++){
  DataStream[j++] = (EKARD_.EKARD_Axis[i].minPulse >> 8);
  DataStream[j++] = (EKARD_.EKARD_Axis[i].minPulse & 0xFF);    
  }
  for(int i = 0; i < ServoNum; i++){
  DataStream[j++] = (EKARD_.EKARD_Axis[i].maxPulse >> 8);
  DataStream[j++] = (EKARD_.EKARD_Axis[i].maxPulse & 0xFF);    
  } 
  DataStream[j++] = (EKARD_.Axis_0Time >> 8);
  DataStream[j++] = (EKARD_.Axis_0Time & 0xFF); 
  
  DataStream[j++] = EKARD_.Axis_0Speed;

  DataStream[j++] = (EKARD_.MaxTimeDiff >> 8);
  DataStream[j++] = (EKARD_.MaxTimeDiff & 0xFF);   

  DataStream[j++] = (EKARD_.CalibrationFactor >> 8);
  DataStream[j++] = (EKARD_.CalibrationFactor & 0xFF);   

  DataStream[j++] = (EKARD_.Axes_Speed >> 8);
  DataStream[j++] = (EKARD_.Axes_Speed & 0xFF);  

  DataStream[j++] = 0xDD;
  DataStream[j++] = 0xEE;

  client->write(DataStream, (EEPROM_DataSize + 5));
  bool argument = 1;
  while(argument){
    if(client->available()){
      for(int i = 0; i < TCPStreamLength; i++){
        TCPStream[i] = client->read();
      }
      argument = 0;
    }
  }
  EKARD_ProcessReceivedData();
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_Demo0(){
  Serial.println("Running Demo 0");
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_Demo1(){
  Serial.println("Running Demo 1");
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_Demo2(){
  Serial.println("Running Demo 2"); 
}
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------------------------------*/
void EKARD_OneTimeMemoryInit(){
  Serial.println("!!!memory init!!!");
  int j = 0;
  //16 Bit Axis 0 
  EEPROM.write(j++, 0);
  EEPROM.write(j++, 0);
  //8 Bit Axes 1 - 5
  EEPROM.write(j++, 90);
  EEPROM.write(j++, 90);
  EEPROM.write(j++, 90);
  EEPROM.write(j++, 90);
  EEPROM.write(j++, 90);
  //16 Bit Axes Min values
  EEPROM.write(j++, (50 >> 8));
  EEPROM.write(j++, (50 & 0xFF));
  EEPROM.write(j++, (650 >> 8));
  EEPROM.write(j++, (650 & 0xFF));
  EEPROM.write(j++, (500 >> 8));
  EEPROM.write(j++, (500 & 0xFF));
  EEPROM.write(j++, (500 >> 8));
  EEPROM.write(j++, (500 & 0xFF));
  EEPROM.write(j++, (500 >> 8));
  EEPROM.write(j++, (500 & 0xFF));
  EEPROM.write(j++, (500 >> 8));
  EEPROM.write(j++, (500 & 0xFF));
  //16 Bit Axes Max values
  EEPROM.write(j++, (3500 >> 8));
  EEPROM.write(j++, (3500 & 0xFF));
  EEPROM.write(j++, (1950 >> 8));
  EEPROM.write(j++, (1950 & 0xFF));
  EEPROM.write(j++, (2500 >> 8));
  EEPROM.write(j++, (2500 & 0xFF));
  EEPROM.write(j++, (2500 >> 8));
  EEPROM.write(j++, (2500 & 0xFF));
  EEPROM.write(j++, (2500 >> 8));
  EEPROM.write(j++, (2500 & 0xFF));
  EEPROM.write(j++, (2500 >> 8));
  EEPROM.write(j++, (2500 & 0xFF));
  //16 Bit Axis0 time
  EEPROM.write(j++, (0 >> 8));
  EEPROM.write(j++, (0 & 0xFF));
  //8 Bit Axis0 Speed
  EEPROM.write(j++, 90);
  //16 Bit MaxTimeDiff
  EEPROM.write(j++, (100 >> 8));
  EEPROM.write(j++, (100 & 0xFF));
  //16 Bit Calibration Factor
  EEPROM.write(j++, (100 >> 8));
  EEPROM.write(j++, (100 & 0xFF));
  //16 Bit Axes Speed
  EEPROM.write(j++, (160 >> 8));
  EEPROM.write(j++, (160 & 0xFF));
  EEPROM.commit();
}
