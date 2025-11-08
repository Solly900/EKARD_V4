#ifndef EKARD_CLIENT_HPP_INCLUDED
#define EKARD_CLIENT_HPP_INCLUDED

#include <winsock2.h>
#include <ws2tcpip.h>
#include "pch.h"
#include <winsock.h>
#include "iostream"
#include <bitset>

#define EKARD_IP "192.168.0.201"
#define EKARD_PORT 44444
#define ServoNum 6

#pragma comment(lib, "ws2_32.lib")

class Servo {
protected:
	//axis value in °
	int angle;

	//Servo pulse width in µS (min 500, max 2500) -> values to trim servo behavior
	int minPulse;
	int maxPulse;

public:
	Servo();
	void set_angle(int agl);
	void set_PulseWidth(int min, int max);
	int get_angle();
	int get_minPulse();
	int get_maxPulse();
};

class EKARD_Client : public Servo {
private:
	bool ConnectionStatus;
	int Command;
	int InterfaceStatus;
	Servo EKARD_Axis[ServoNum];
	void create_TCPStream();
	int* get_EKARD_Position();
	void get_User_Input();
	void set_ConnectionStatus(bool Status);
	bool get_ConnectionStatus();

public:
	EKARD_Client();
	void execute_EKARD();
	bool connect_ToServer();
};


enum Commands {
	COM_IDLE = 0,
	COM_SET_POSITION = 1,
	COM_SET_MIN = 2,
	COM_SET_MAX = 3,
	COM_AUTO_HOME = 4,
	COM_CALIBRATE_A0 = 5,
	COM_SAVE_POS_AS_90_DEGREES = 6,
	COM_SAVE_POS_AS_AUTO_HOME = 7,
	COM_SAVE_MIN = 8,
	COM_SAVE_MAX = 9,
	COM_RUN_DEMO_0 = 10,
	COM_RUN_DEMO_1 = 11,
	COM_RUN_DEMO_2 = 12,
	COM_REQUEST_EKARD_POS = 13
};

enum Interface {
	IN_MAIN_MENU = 0,
	IN_SET_NEW_POS = 1,
	IN_AUTO_HOME = 2,
	IN_RUN_DEMO = 3,
	IN_DEMO_0 = 4,
	IN_DEMO_1 = 5,
	IN_DEMO_2 = 6,
	IN_TRIM_AXIS = 7,
	IN_AXIS_0 = 8,
	IN_AXIS_1 = 9,
	IN_AXIS_2 = 10,
	IN_AXIS_3 = 11,
	IN_AXIS_4 = 12,
	IN_AXIS_5 = 13,
	IN_MIN = 14,
	IN_MAX = 15,
	IN_RETURN = 16,
	IN_SAVE_ON_EKARD = 17,
	IN_SAVE_HOME = 18,
	IN_SAVE_AXIS = 19,
	IN_RECALIBRATE_AXIS_0 = 20
};

#endif // !EKARD_CLIENT_HPP_INCLUDED
