#ifndef EKARD_CLIENT_HPP_INCLUDED
#define EKARD_CLIENT_HPP_INCLUDED

#include <winsock2.h>
#include <ws2tcpip.h>
#include "pch.h"
//#include <winsock.h>
#include "iostream"
#include <bitset>

#define EKARD_IP "192.168.178.201"
#define EKARD_PORT 44444
#define ServoNum 6
#define Min_Servo 500
#define Max_Servo 2500
#define FirstPosAxisTCPStream 3
#define TCPStreamLength 17

#pragma comment(lib, "ws2_32.lib")

class Servo {
private:
	//axis value in �
	int angle;

	//Servo pulse width in �S (min 500, max 2500) -> values to trim servo behavior
	int minPulse;
	int maxPulse;

public: 
	Servo();
	void set_angle(int agl);
	void set_minPulse(int min);
	void set_maxPulse(int max);
	int get_angle();
	int get_minPulse();
	int get_maxPulse();
};

class EKARD_Client {
private:

	//interface variables
	bool ConnectionStatus;
	int Command; //Commands values
	bool enter_Command;
	int MenuStatus; //Interface values
	int SubMenuStatus; //Interface values
	//int MenuMode; //Commands values

	//EKARD variables
	int Axis0_Time;
	int Axis0_Speed;
	int MAX_TimeDiff;
	int CalibrationFactor;
	int Axes_Speed;
	char TCPStream[TCPStreamLength];
	uint8_t TCPRecvStream[TCPStreamLength];//for typecasting to unsigned, otherwise the comparisons get messed up
	int PosBuffer[ServoNum];
	Servo EKARD_Axis[ServoNum];
	SOCKET EKARD_ClientSocket;
	bool transmit_EKARDData();
	void create_TCPStream();
	void output_EKARDAngles();
	void output_EKARDMinValues();
	void output_EKARDMaxValues();
	void get_User_Input();

public:
	EKARD_Client();
	void execute_EKARD();
	bool connect_ToServer();
	bool get_ConnectionStatus();
	int* get_EKARD_Position();
	bool get_EKARDServerData();
};


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

enum Interface {
	IN_MAIN_MENU				= 0,
	IN_SET_NEW_POS				= 1,
	IN_AUTO_HOME				= 2,
	IN_RUN_DEMO					= 3,
	IN_DEMO_0					= 4,
	IN_DEMO_1					= 5,
	IN_DEMO_2					= 6,
	IN_TRIM_AXIS				= 7,
	IN_AXIS_0					= 8,
	IN_AXIS_1					= 9,
	IN_AXIS_2					= 10,
	IN_AXIS_3					= 11,
	IN_AXIS_4					= 12,
	IN_AXIS_5					= 13,
	IN_MIN						= 14,
	IN_MAX						= 15,
	IN_RETURN					= 16,
	IN_SAVE_ON_EKARD			= 17,
	IN_SAVE_HOME				= 18,
	IN_SAVE_AXIS				= 19,
	IN_RECALIBRATE_AXIS_0		= 20,
	IN_ENTER_COMANDS			= 22,
	IN_SET_AXIS0_SPEED			= 23,
	IN_SET_MAX_TIME_DIFF		= 24,
	IN_SET_CALIBRATION_FACTOR	= 25,
	IN_SET_AXES_SPEED			= 26,
	IN_SAVE_AXIS0_TIME			= 27,
	IN_SAVE_AXIS0_SPEED			= 28,
	IN_SAVE_MAX_TIME_DIFF		= 29,
	IN_SAVE_CALIBRATION_FACTOR	= 30,
	IN_SAVE_AXES_SPEED			= 31,
	IN_SET_HOME_POS				= 32,
	IN_ERROR					= 50
};

#endif // !EKARD_CLIENT_HPP_INCLUDED
