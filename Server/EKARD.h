#ifndef EKARD_H_
#define EKARD_H_

#include <Arduino.h>


/*------- defines -----------------------------------------------------------------*/
#define Servo0Pin 27
#define Servo1Pin 14
#define Servo2Pin 26
#define Servo3Pin 13
#define Servo4Pin 33
#define Servo5Pin 32
#define HallSensorPin 4

#define StepCount 160.0f
//#define CalibrationFactor 100
//#define MaxTimeDiff 100 //speed difference forward/backward axis 0 in ms
//the smaller this value the more accuracy in axis0 position but it needs more calibration time
#define EKARD_Port 44444
#define ServoNum 6
#define Min_Servo 500
#define Max_Servo 2500
#define FirstDataPosTCPStream 3
#define TCPStreamLength 17

#define EEPROM_DataSize 40 //in byte

#define inWlan
/*---------------------------------------------------------------------------------*/



/*------- struct definitions ------------------------------------------------------*/


typedef struct{

    uint16_t angle;
    uint16_t minPulse;
    uint16_t maxPulse;

}Servos;



typedef struct{

    Servos EKARD_Axis[ServoNum]; //current axes angles in °
	uint16_t HomePosition[ServoNum]; //angle of each axis in HomePosition in °
	uint16_t Axis_0Time; //calibrated time in which axis 0 moves for 360°
    uint16_t Axis_0Speed; //determines how fast Axis 0 moves
	uint16_t MaxTimeDiff; //determines the max time difference (in ms) for axis 0 between forward and backward movement (for 360°) --> calibration precision
	uint16_t CalibrationFactor; //determines the size of the calibration steps for axis 0
	uint16_t Axes_Speed; //in 5ms  --> determines how fast the Servo moves to the new Position

}EKARD;

enum Commands {
	COM_IDLE					= 0,
	COM_SET_POSITION			= 1,
	COM_SET_MIN					= 2,
	COM_SET_MAX					= 3,
	COM_AUTO_HOME				= 4,
	COM_CALIBRATE_A0			= 5,
	COM_SAVE_CURRENT_HOME_POS	= 6,
	COM_SAVE_MIN_MAX			= 7,
	COM_SAVE_AXIS0_TIME			= 9,
	COM_SAVE_AXIS0_SPEED		= 10,
	COM_SAVE_MAX_TIME_DIFF		= 11,
	COM_SAVE_CALIBRATION_FACTOR = 12,
	COM_SAVE_AXES_SPEED			= 13,
	COM_RUN_DEMO_0				= 14,
	COM_RUN_DEMO_1				= 15,
	COM_RUN_DEMO_2				= 16,
	COM_REQUEST_EKARD_DATA		= 17,
	COM_RECEIVED_DATA			= 18,
	COM_SET_AXIS0_SPEED			= 19,
	COM_SET_MAX_TIME_DIFF		= 20,
	COM_SET_CALIBRATION_FACTOR	= 21,
	COM_SET_AXES_SPEED			= 22,
	COM_SET_CURRENT_POS_AS_HOME = 23,
	COM_RECEIVED_INCORRECT_DATA = 24
};
/*---------------------------------------------------------------------------------*/



/*------- API function declarations -----------------------------------------------*/
EKARD EKARD_Init(void);

void EKARD_AutoHome(void);

uint16_t EKARD_CalibrateAxis0(uint8_t ServoSpeed); //speed 1-180, increase in speed leads to decrease in calibration granularity

void EKARD_SetServos(EKARD *EKARD_instancePointer);

void EKARD_SaveSettings(EKARD *EKARD_instancePointer);
/*---------------------------------------------------------------------------------*/


#endif
