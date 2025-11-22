 //EKARD client to connect to robot

#include "EKARD_Client.hpp"


using namespace std;

//Servo functions
Servo::Servo() {
	angle = 90;
	minPulse = 500;
	maxPulse = 2500;
}

void Servo::set_angle(int agl) {
	angle = agl; 
}

void Servo::set_minPulse(int min) {
	minPulse = min;
}

void Servo::set_maxPulse(int max) {
	maxPulse = max;
}


int Servo::get_angle() {
	return angle;
}

int Servo::get_minPulse() {
	return minPulse;
}

int Servo::get_maxPulse() {
	return maxPulse;
}

//EKARD functions
EKARD_Client::EKARD_Client() {
	ConnectionStatus = false;
	Command = COM_IDLE;            
	//MenuMode = COM_IDLE;
	enter_Command = false;
	MenuStatus = IN_MAIN_MENU;
	SubMenuStatus = IN_MAIN_MENU;
	EKARD_ClientSocket = INVALID_SOCKET;
	TCPStream[0] = 0xAA;
	TCPStream[1] = 0xBB;
	TCPStream[TCPStreamLength - 2] = 0xDD;
	TCPStream[TCPStreamLength - 1] = 0xEE;
	for (int i = 2; i < 10; i++) {
		TCPStream[i] = 90;
	}
}

void EKARD_Client::create_TCPStream() {
	int j = 0;
	TCPStream[0] = 0xAA;
	TCPStream[1] = 0xBB;
	TCPStream[TCPStreamLength - 2] = 0xDD;
	TCPStream[TCPStreamLength - 1] = 0xEE;
	TCPStream[FirstPosAxisTCPStream - 1] = (char)Command;
	if (Command == COM_SET_POSITION) {
		for (int i = 0; i < ServoNum; i++) {
			TCPStream[FirstPosAxisTCPStream + j++] = (char)(EKARD_Axis[i].get_angle() >> 8);
			TCPStream[FirstPosAxisTCPStream + j++] = (char)(EKARD_Axis[i].get_angle() & 0xFF);
		}
	}
	else if (Command == COM_SET_MAX) {
		for (int i = 0; i < ServoNum; i++) {
			TCPStream[FirstPosAxisTCPStream + j++] = (char)(EKARD_Axis[i].get_maxPulse() >> 8);
			TCPStream[FirstPosAxisTCPStream + j++] = (char)(EKARD_Axis[i].get_maxPulse() & 0xFF);
		}
	}
	else if (Command == COM_SET_MIN) {
		for (int i = 0; i < ServoNum; i++) {
			TCPStream[FirstPosAxisTCPStream + j++] = (char)(EKARD_Axis[i].get_minPulse() >> 8);
			TCPStream[FirstPosAxisTCPStream + j++] = (char)(EKARD_Axis[i].get_minPulse()& 0xFF);
		}
	}
	else if (Command == COM_SET_AXIS0_SPEED) {
		TCPStream[FirstPosAxisTCPStream] = (char)Axis0_Speed;
	}
	else if (Command == COM_SET_MAX_TIME_DIFF) {
		TCPStream[FirstPosAxisTCPStream] = (char)(MAX_TimeDiff >> 8);
		TCPStream[FirstPosAxisTCPStream + 1] = (char)(MAX_TimeDiff & 0xFF);
	}
	else if (Command == COM_SET_CALIBRATION_FACTOR) {
		TCPStream[FirstPosAxisTCPStream] = (char)(CalibrationFactor >> 8);
		TCPStream[FirstPosAxisTCPStream + 1] = (char)(CalibrationFactor & 0xFF);
	}
	else if (Command == COM_SET_AXES_SPEED) {
		TCPStream[FirstPosAxisTCPStream] = (char)(Axes_Speed>> 8);
		TCPStream[FirstPosAxisTCPStream + 1] = (char)(Axes_Speed & 0xFF);
	}
	else {
		
			for (int i = 0; i < ServoNum; i++) {
				TCPStream[FirstPosAxisTCPStream + j++] = (char)0;
				TCPStream[FirstPosAxisTCPStream + j++] = (char)0;
			}
	}
	return;
}

bool EKARD_Client::transmit_EKARDData() {

	int sendCount = send(EKARD_ClientSocket, TCPStream, TCPStreamLength, 0);
	if(sendCount == 0) {
		cout << "Client sending failed" << endl;
		WSACleanup();
		return 0;
	}
	cout << "did sent data" << endl;
	int recvCount = recv(EKARD_ClientSocket, TCPStream, TCPStreamLength, 0);
	for (int i = 0; i < TCPStreamLength; i++) {
		TCPRecvStream[i] = static_cast<uint8_t>(TCPStream[i]);
		cout << static_cast<int>(TCPRecvStream[i]) << endl;
	}
	if (recvCount == 0) {
		cout << "Client receive failed" << endl;
		WSACleanup();
		return 0;
	}
	if (!(TCPRecvStream[0] == 0xAA && TCPRecvStream[1] == 0xBB && TCPRecvStream[FirstPosAxisTCPStream - 1] == COM_RECEIVED_DATA && TCPRecvStream[TCPStreamLength - 2] == 0xDD && TCPRecvStream[TCPStreamLength - 1] == 0xEE)) {
		cout << "Transmitting data to EKARD failed" << endl;
		WSACleanup();
		return 0;
	}
	cout << "Successfully transmitted data to EKARD" << endl;
	return 1;
}

bool EKARD_Client::connect_ToServer() {
	//initializing variables
	WSADATA wsaData;
	int wsaerr;
	WORD wVersionRequested = MAKEWORD(2, 2);

	//initializing dll
	wsaerr = WSAStartup(wVersionRequested, &wsaData);
	if (wsaerr != 0) {
		cout << "Error: DLL not found!" << endl;
		return 0;
	}
	else {
		cout << "DLL found" << endl;
		cout << "Status is:" << wsaData.szSystemStatus << endl;
	}

	//initializing socket
	EKARD_ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (EKARD_ClientSocket == INVALID_SOCKET) {
		cout << "Error: Could´nt initialize Socket! error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}
	else {
		cout << "socket() is OK!" << endl;
	}

	//connect to server
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	InetPton(AF_INET, TEXT(EKARD_IP), &clientService.sin_addr.s_addr);
	clientService.sin_port = htons(EKARD_PORT);
	if (connect(EKARD_ClientSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		cout << "Error: Failed to connect to Server! (IP: " << EKARD_IP << ", PortNr: " << EKARD_PORT << ")" << endl;
		closesocket(EKARD_ClientSocket);
		WSACleanup();
		return 0;
	}
	else {
		cout << "Client: connect() is OK!" << endl;
		cout << "Client start sending and receiving..." << endl;
	}
	if (get_EKARDServerData()) {
		ConnectionStatus = true;
		cout << "got EKARD data successfully" << endl;
		return 1;
	}
	else return 0;
}

void EKARD_Client::get_User_Input() {
	int xInput;

	switch (MenuStatus) {

	case IN_MAIN_MENU:
		cout << endl << "--------------------------------------------------------------" << endl;
		cout << endl << "Welcome to EKARD the IV. main menu!" << endl;
		cout << "What would you like to do?" << endl << endl;
		cout << "1: Set a new position for an axis" << endl;
		cout << "2: AutoHome the EKARD" << endl;
		cout << "3: Run a Demo to show movement" << endl;
		cout << "4: Set the current position as new AutoHome position" << endl;
		cout << "5: Recalibrate Axis 0" << endl;
		cout << "6: Trim an axis position" << endl;
		cout << "7: Save a value on the EKARD Harddrive" << endl << endl;
		cout << "Please type in the number of the task you want to perform!" << endl << endl;
		//Command = COM_IDLE;
		cin >> xInput;
		switch (xInput) {

		case 1:
			MenuStatus = IN_SET_NEW_POS;
			break;
		case 2:
			MenuStatus = IN_AUTO_HOME;
			break;
		case 3:
			MenuStatus = IN_RUN_DEMO;
			break;
		case 4:
			MenuStatus = IN_SET_HOME_POS;
			break;
		case 5:
			MenuStatus = IN_RECALIBRATE_AXIS_0;
			break;
		case 6:
			MenuStatus = IN_TRIM_AXIS;
			break;
		case 7:
			MenuStatus = IN_SAVE_ON_EKARD;
			break;
		}
		system("CLS");
		break;

	case IN_SET_NEW_POS:
		cout << endl << "--------------------------------------------------------------" << endl;
		cout << endl << "Which Axis you want to set to a new Value?" << endl << endl;
		cout << "1: Axis 0" << endl;
		cout << "2: Axis 1" << endl;
		cout << "3: Axis 2" << endl;
		cout << "4: Axis 3" << endl;
		cout << "5: Axis 4" << endl;
		cout << "6: Axis 5" << endl;
		cout << "7: Return" << endl;
		cout << "8: Commit Commands" << endl << endl;
		cout << "Please type in the number of the axis you want to set!" << endl << endl;
		cout << "(if you have already set a new value, you have to enter your commands in order to return to the main menu)" << endl << endl;
		cin >> xInput;
		switch (xInput) {

		case 1:
			MenuStatus = IN_AXIS_0;
			break;
		case 2:
			MenuStatus = IN_AXIS_1;
			break;
		case 3:
			MenuStatus = IN_AXIS_2;
			break;
		case 4:
			MenuStatus = IN_AXIS_3;
			break;
		case 5:
			MenuStatus = IN_AXIS_4;
			break;
		case 6:
			MenuStatus = IN_AXIS_5;
			break;
		case 7:
			MenuStatus = IN_RETURN;
			break;
		case 8:
			MenuStatus = IN_ENTER_COMANDS;
			break;
		}
		SubMenuStatus = IN_SET_NEW_POS;
		system("CLS");
		break;

	case IN_AUTO_HOME:
		Command = COM_AUTO_HOME;
		MenuStatus = IN_ENTER_COMANDS;
		SubMenuStatus = IN_MAIN_MENU;
		break;

	case IN_RUN_DEMO:
		cout << endl << "--------------------------------------------------------------" << endl;
		cout << endl << "Which DemoNr do you want to run?" << endl << endl;
		cout << "1: Demo 0" << endl;
		cout << "2: Demo 1" << endl;
		cout << "3: Demo 2" << endl;
		cout << "4: Return" << endl;
		cout << "Please type in the number of Demo u want to run!" << endl << endl;
		cin >> xInput;
		switch (xInput) {

		case 1:
			MenuStatus = IN_DEMO_0;
			break;
		case 2:
			MenuStatus = IN_DEMO_1;
			break;
		case 3:
			MenuStatus = IN_DEMO_2;
			break;
		case 4:
			MenuStatus = IN_RETURN;
			break;
		}
		system("CLS");
		break;

	case IN_DEMO_0:
		Command = COM_RUN_DEMO_0;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_DEMO_1:
		Command = COM_RUN_DEMO_1;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_DEMO_2:
		Command = COM_RUN_DEMO_2;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_TRIM_AXIS:
		cout << endl << "--------------------------------------------------------------" << endl;
		cout << endl << "You are trimming the angle of an axis, do you want to set the max or min value?" << endl << endl;
		cout << "1: Trim min value" << endl;
		cout << "2: Trim max value" << endl;
		cout << "3: Set a new speed for Axis 0" << endl;
		cout << "4: Set a new calibration precision factor for Axis 0" << endl;
		cout << "5: Set a new calibration step size factor for Axis 0" << endl;
		cout << "6: Set a new speed for all axes except Axis 0" << endl;
		cout << "7: Return" << endl;
		//Command = COM_IDLE;
		cin >> xInput;
		switch (xInput) {

		case 1:
			MenuStatus = IN_MIN;
			break;
		case 2:
			MenuStatus = IN_MAX;
			break;
		case 3:
			MenuStatus = IN_SET_AXIS0_SPEED;
			break;
		case 4:
			MenuStatus = IN_SET_MAX_TIME_DIFF;
			break;
		case 5:
			MenuStatus = IN_SET_CALIBRATION_FACTOR;
			break;
		case 6:
			MenuStatus = IN_SET_AXES_SPEED;
			break;
		case 7:
			MenuStatus = IN_RETURN;
			break;
		}
		SubMenuStatus = IN_TRIM_AXIS;
		system("CLS");
		break;

	case IN_AXIS_0:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_SET_NEW_POS) {
			cout << "Current value is: " << EKARD_Axis[0].get_angle() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "You are setting a new angle for axis 0, please type in a value between 0 - 360" << endl << endl;
			cin >> xInput;
			if (0 <= xInput && xInput <= 360) {
				EKARD_Axis[0].set_angle(xInput);
				Command = COM_SET_POSITION;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MIN) {
			cout << "Current values are: MIN: " << EKARD_Axis[0].get_minPulse() << "; MAX: " << EKARD_Axis[0].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new min value (20-4000 & smaller then max value)" << endl << endl;
			cin >> xInput;
			if (20 <= xInput && xInput < EKARD_Axis[0].get_maxPulse()) {
				EKARD_Axis[0].set_minPulse(xInput);
				Command = COM_SET_MIN;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MAX) {
			cout << "Current values are: MIN: " << EKARD_Axis[0].get_minPulse() << "; MAX: " << EKARD_Axis[0].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new max value (20 - 4000 & greater then min value)" << endl << endl;
			cin >> xInput;
			if (EKARD_Axis[0].get_minPulse() < xInput && xInput <= 4000) {
				EKARD_Axis[0].set_maxPulse(xInput);
				Command = COM_SET_MAX;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		system("CLS");
		break;

	case IN_AXIS_1:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_SET_NEW_POS) {
			cout << "Current value is: " << EKARD_Axis[1].get_angle() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "You are setting a new angle for axis 1, please type in a value between 0 - 180" << endl << endl;
			cin >> xInput;
			if (0 <= xInput && xInput <= 180) {
				EKARD_Axis[1].set_angle(xInput);
				Command = COM_SET_POSITION;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MIN) {
			cout << "Current values are: MIN: " << EKARD_Axis[1].get_minPulse() << "; MAX: " << EKARD_Axis[1].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new min value (500-2500 & smaller then max value)" << endl << endl;
			cin >> xInput;
			if (Min_Servo <= xInput && xInput < EKARD_Axis[1].get_maxPulse()) {
				EKARD_Axis[1].set_minPulse(xInput);
				Command = COM_SET_MIN;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MAX) {
			cout << "Current values are: MIN: " << EKARD_Axis[1].get_minPulse() << "; MAX: " << EKARD_Axis[1].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new max value (500-2500 & greater then min value)" << endl << endl;
			cin >> xInput;
			if (EKARD_Axis[1].get_minPulse() < xInput && xInput <= Max_Servo) {
				EKARD_Axis[1].set_maxPulse(xInput);
				Command = COM_SET_MAX;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		system("CLS");
		break;

	case IN_AXIS_2:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_SET_NEW_POS) {
			cout << "Current value is: " << EKARD_Axis[2].get_angle() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "You are setting a new angle for axis 2, please type in a value between 0 - 180" << endl << endl;
			cin >> xInput;
			if (0 <= xInput && xInput <= 180) {
				EKARD_Axis[2].set_angle(xInput);
				Command = COM_SET_POSITION;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MIN) {
			cout << "Current values are: MIN: " << EKARD_Axis[2].get_minPulse() << "; MAX: " << EKARD_Axis[2].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new min value (500-2500 & smaller then max value)" << endl << endl;
			cin >> xInput;
			if (Min_Servo <= xInput && xInput < EKARD_Axis[2].get_maxPulse()) {
				EKARD_Axis[2].set_minPulse(xInput);
				Command = COM_SET_MIN;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MAX) {
			cout << "Current values are: MIN: " << EKARD_Axis[2].get_minPulse() << "; MAX: " << EKARD_Axis[2].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new max value (500-2500 & greater then min value)" << endl << endl;
			cin >> xInput;
			if (EKARD_Axis[2].get_minPulse() < xInput && xInput <= Max_Servo) {
				EKARD_Axis[2].set_maxPulse(xInput);
				Command = COM_SET_MAX;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		system("CLS");
		break;

	case IN_AXIS_3:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_SET_NEW_POS) {
			cout << "Current value is: " << EKARD_Axis[3].get_angle() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "You are setting a new angle for axis 3, please type in a value between 0 - 180" << endl << endl;
			cin >> xInput;
			if (0 <= xInput && xInput <= 180) {
				EKARD_Axis[3].set_angle(xInput);
				Command = COM_SET_POSITION;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MIN) {
			cout << "Current values are: MIN: " << EKARD_Axis[3].get_minPulse() << "; MAX: " << EKARD_Axis[3].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new min value (500-2500 & smaller then max value)" << endl << endl;
			cin >> xInput;
			if (Min_Servo <= xInput && xInput < EKARD_Axis[3].get_maxPulse()) {
				EKARD_Axis[3].set_minPulse(xInput);
				Command = COM_SET_MIN;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MAX) {
			cout << "Current values are: MIN: " << EKARD_Axis[3].get_minPulse() << "; MAX: " << EKARD_Axis[3].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new max value (500-2500 & greater then min value)" << endl << endl;
			cin >> xInput;
			if (EKARD_Axis[3].get_minPulse() < xInput && xInput <= Max_Servo) {
				EKARD_Axis[3].set_maxPulse(xInput);
				Command = COM_SET_MAX;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		system("CLS");
		break;

	case IN_AXIS_4:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_SET_NEW_POS) {
			cout << "Current value is: " << EKARD_Axis[4].get_angle() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "You are setting a new angle for axis 4, please type in a value between 0 - 180" << endl << endl;
			cin >> xInput;
			if (0 <= xInput && xInput <= 180) {
				EKARD_Axis[4].set_angle(xInput);
				Command = COM_SET_POSITION;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MIN) {
			cout << "Current values are: MIN: " << EKARD_Axis[4].get_minPulse() << "; MAX: " << EKARD_Axis[4].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new min value (500-2500 & smaller then max value)" << endl << endl;
			cin >> xInput;
			if (Min_Servo <= xInput && xInput < EKARD_Axis[4].get_maxPulse()) {
				EKARD_Axis[4].set_minPulse(xInput);
				Command = COM_SET_MIN;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MAX) {
			cout << "Current values are: MIN: " << EKARD_Axis[4].get_minPulse() << "; MAX: " << EKARD_Axis[4].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new max value (500-2500 & greater then min value)" << endl << endl;
			cin >> xInput;
			if (EKARD_Axis[4].get_minPulse() < xInput && xInput <= Max_Servo) {
				EKARD_Axis[4].set_maxPulse(xInput);
				Command = COM_SET_MAX;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		system("CLS");
		break;

	case IN_AXIS_5:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_SET_NEW_POS) {
			cout << "Current value is: " << EKARD_Axis[5].get_angle() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new angle for axis 5, please type in a value between 0 - 180" << endl << endl;
			cin >> xInput;
			if (0 <= xInput && xInput <= 180) {
				EKARD_Axis[5].set_angle(xInput);
				Command = COM_SET_POSITION;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MIN) {
			cout << "Current values are: MIN: " << EKARD_Axis[5].get_minPulse() << "; MAX: " << EKARD_Axis[5].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new min value (500-2500 & smaller then max value)" << endl << endl;
			cin >> xInput;
			if (Min_Servo <= xInput && xInput < EKARD_Axis[5].get_maxPulse()) {
				EKARD_Axis[5].set_minPulse(xInput);
				Command = COM_SET_MIN;
			}
			else MenuStatus = IN_ERROR;
		}
		else if (SubMenuStatus == IN_MAX) {
			cout << "Current values are: MIN: " << EKARD_Axis[5].get_minPulse() << "; MAX: " << EKARD_Axis[5].get_maxPulse() << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new max value (500-2500 & greater then min value)" << endl << endl;
			cin >> xInput;
			if (EKARD_Axis[5].get_minPulse() < xInput && xInput <= Max_Servo) {
				EKARD_Axis[5].set_maxPulse(xInput);
				Command = COM_SET_MAX;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		system("CLS");
		break;

	case IN_MIN:
		cout << endl << "--------------------------------------------------------------" << endl;
		cout << endl << "Which Axis you want to set the min value?" << endl << endl;
		cout << IN_AXIS_0 << ": Axis 0" << endl;
		cout << IN_AXIS_1 << ": Axis 1" << endl;
		cout << IN_AXIS_2 << ": Axis 2" << endl;
		cout << IN_AXIS_3 << ": Axis 3" << endl;
		cout << IN_AXIS_4 << ": Axis 4" << endl;
		cout << IN_AXIS_5 << ": Axis 5" << endl;
		cout << IN_RETURN << ": Return" << endl;
		cout << IN_ENTER_COMANDS << ": Commit Commands" << endl << endl;
		cout << "Please type in the number of the axis you want to set!" << endl << endl;
		cout << "(if you have already set a new value, you have to enter your commands in order to return to the main menu)" << endl << endl;
		cin >> xInput;
		switch (xInput) {

		case 1:
			MenuStatus = IN_AXIS_0;
			break;
		case 2:
			MenuStatus = IN_AXIS_1;
			break;
		case 3:
			MenuStatus = IN_AXIS_2;
			break;
		case 4:
			MenuStatus = IN_AXIS_3;
			break;
		case 5:
			MenuStatus = IN_AXIS_4;
			break;
		case 6:
			MenuStatus = IN_AXIS_5;
			break;
		case 7:
			MenuStatus = IN_RETURN;
			break;
		case 8:
			MenuStatus = IN_ENTER_COMANDS;
			break;
		}
		SubMenuStatus = IN_MIN;
		system("CLS");
		break;
			
	case IN_MAX:
		cout << endl << "--------------------------------------------------------------" << endl;
		cout << endl << "Which Axis you want to set the max value?" << endl << endl;
		cout << IN_AXIS_0 << ": Axis 0" << endl;
		cout << IN_AXIS_1 << ": Axis 1" << endl;
		cout << IN_AXIS_2 << ": Axis 2" << endl;
		cout << IN_AXIS_3 << ": Axis 3" << endl;
		cout << IN_AXIS_4 << ": Axis 4" << endl;
		cout << IN_AXIS_5 << ": Axis 5" << endl;
		cout << IN_RETURN << ": Return" << endl;
		cout << IN_ENTER_COMANDS << ": Commit Commands" << endl << endl;
		cout << "Please type in the number of the axis you want to set!" << endl << endl;
		cout << "(if you have already set a new value, you have to enter your commands in order to return to the main menu)" << endl << endl;
		cin >> xInput;
		switch (xInput) {

		case 1:
			MenuStatus = IN_AXIS_0;
			break;
		case 2:
			MenuStatus = IN_AXIS_1;
			break;
		case 3:
			MenuStatus = IN_AXIS_2;
			break;
		case 4:
			MenuStatus = IN_AXIS_3;
			break;
		case 5:
			MenuStatus = IN_AXIS_4;
			break;
		case 6:
			MenuStatus = IN_AXIS_5;
			break;
		case 7:
			MenuStatus = IN_RETURN;
			break;
		case 8:
			MenuStatus = IN_ENTER_COMANDS;
			break;
		}
		SubMenuStatus = IN_MAX;
		system("CLS");
		break;

	case IN_RETURN:
		if (SubMenuStatus == IN_MIN || SubMenuStatus == IN_MAX) {
			MenuStatus = IN_TRIM_AXIS;
			SubMenuStatus = IN_MAIN_MENU;
		}
		else if (SubMenuStatus == IN_SET_NEW_POS) {
			MenuStatus = IN_MAIN_MENU;
			SubMenuStatus = IN_MAIN_MENU;
		}
		else {
			MenuStatus = IN_MAIN_MENU;
			SubMenuStatus = IN_MAIN_MENU;
			//Command = COM_IDLE;
		}
		system("CLS");
		break;

	case IN_SAVE_ON_EKARD:
		cout << endl << "--------------------------------------------------------------" << endl;
		cout << endl << "What do you want to save on EKARD?" << endl;
		cout << IN_SAVE_HOME << ": Save current position as HomePosition" << endl;
		cout << IN_SAVE_AXIS << ": Save current MIN / MAX values" << endl;
		cout << IN_SAVE_AXIS0_TIME << ": Save current calibrated Axis 0 time (for 360°)" << endl;
		cout << IN_SAVE_AXIS0_SPEED << ": Save the current speed of Axis 0" << endl;
		cout << IN_SAVE_MAX_TIME_DIFF << ": Save the current precision factor of the calibration" << endl;
		cout << IN_SAVE_CALIBRATION_FACTOR << ": Save the current factor for the size of the calibration steps" << endl;
		cout << IN_SAVE_AXES_SPEED << ": Save the current speed of all axes except Axis 0" << endl;
		cout << IN_RETURN << ": Return" << endl << endl;
		cin >> xInput;
		switch (xInput) {

		case 1:
			MenuStatus = IN_SAVE_HOME;
			break;
		case 2:
			MenuStatus = IN_SAVE_AXIS;
			break;
		case 3:
			MenuStatus = IN_SAVE_AXIS0_TIME;
			break;
		case 4:
			MenuStatus = IN_SAVE_AXIS0_SPEED;
			break;
		case 5:
			MenuStatus = IN_SAVE_MAX_TIME_DIFF;
			break;
		case 6:
			MenuStatus = IN_SAVE_CALIBRATION_FACTOR;
			break;
		case 7:
			MenuStatus = IN_SAVE_AXES_SPEED;
			break;
		case 8:
			MenuStatus = IN_RETURN;
			break;
		}
		SubMenuStatus = IN_SAVE_ON_EKARD;
		system("CLS");
		break;

	case IN_SAVE_HOME:
		Command = COM_SAVE_CURRENT_HOME_POS;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_SAVE_AXIS:
		Command = COM_SAVE_MIN_MAX;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_RECALIBRATE_AXIS_0:
		Command = COM_CALIBRATE_A0;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_ENTER_COMANDS:
		if (Command == COM_IDLE) {
			cout << "Please enter a command first, before you commit it" << endl;
			MenuStatus = SubMenuStatus;
		}
		else enter_Command = 1;
		break;

	case IN_SET_AXIS0_SPEED:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_TRIM_AXIS) {
			cout << "Current value is: " << Axis0_Speed << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new value for the speed of Axis 0 (0 - 90), where 0 is no movement at all and 90 is the fastest" << endl << endl;
			cin >> xInput;
			if (0 <= xInput && xInput <= 90) {
				Axis0_Speed = xInput;
				Command = COM_SET_AXIS0_SPEED;
				MenuStatus = IN_ENTER_COMANDS;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		break;

	case IN_SET_MAX_TIME_DIFF:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_TRIM_AXIS) {
			cout << "Current value is: " << MAX_TimeDiff << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new value for the max time difference (in ms) between the amount of time it takes Axis 0 to turn 360° forward and backward (value > 5)" << endl << endl;
			cin >> xInput;
			if (5 <= xInput) {
				MAX_TimeDiff = xInput;
				Command = COM_SET_MAX_TIME_DIFF;
				MenuStatus = IN_ENTER_COMANDS;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		break;

	case IN_SET_CALIBRATION_FACTOR:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_TRIM_AXIS) {
			cout << "Current value is: " << CalibrationFactor << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new value for the step size (step size = timediff / calibrationFactor) of the calibration process of Axis 0 (value > 50)" << endl << endl;
			cin >> xInput;
			if (50 <= xInput) {
				CalibrationFactor = xInput;
				Command = COM_SET_CALIBRATION_FACTOR;
				MenuStatus = IN_ENTER_COMANDS;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		break;

	case IN_SET_AXES_SPEED:
		cout << endl << "--------------------------------------------------------------" << endl;
		if (SubMenuStatus == IN_TRIM_AXIS) {
			cout << "Current value is: " << Axes_Speed << endl;
			cout << "(to return to submenu type in a value out of the specified range)" << endl;
			cout << endl << "Set a new value for the amount of time (in ms) it takes the Axes to move to a new position (value > 5)" << endl << endl;
			cin >> xInput;
			if (5 <= xInput) {
				Axes_Speed = xInput;
				Command = COM_SET_AXES_SPEED;
				MenuStatus = IN_ENTER_COMANDS;
			}
			else MenuStatus = IN_ERROR;
		}
		else MenuStatus = IN_ERROR;
		break;

	case IN_SAVE_AXIS0_TIME:
		Command = COM_SAVE_AXIS0_TIME;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_SAVE_AXIS0_SPEED:
		Command = COM_SAVE_AXIS0_SPEED;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_SAVE_MAX_TIME_DIFF:
		Command = COM_SAVE_MAX_TIME_DIFF;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_SAVE_CALIBRATION_FACTOR:
		Command = COM_SAVE_CALIBRATION_FACTOR;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_SAVE_AXES_SPEED:
		Command = COM_SAVE_AXES_SPEED;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	case IN_SET_HOME_POS:
		Command = COM_SET_CURRENT_POS_AS_HOME;
		MenuStatus = IN_ENTER_COMANDS;
		break;

	default:
		cout << endl << "--------------------------------------------------------------" << endl;
		cout << endl << "(Value not accepted) -> Returning to Menu" << endl;
		MenuStatus = SubMenuStatus;
		SubMenuStatus = IN_MAIN_MENU;

		/*
		cout << "Do you want to commit your given commands to EKARD?" << endl;
		cout << "0: NO" << endl;
		cout << "1: YES" << endl << endl;
		cin >> xInput;
		if (xInput == 1) {
			MenuStatus = IN_ENTER_COMANDS;
		}
		*/
		//system("CLS");
		break;
	}

	
}

void EKARD_Client::execute_EKARD() {

	if (!get_ConnectionStatus()) {
		if (!connect_ToServer()) {
			cout << "unable to reconnect" << endl;
			return;
		}
	}
	get_User_Input();
	if (enter_Command) {
		create_TCPStream();
		if (transmit_EKARDData()) {
			enter_Command = 0;
			Command = COM_IDLE;
			MenuStatus = IN_MAIN_MENU;
			SubMenuStatus = IN_MAIN_MENU;
			//MenuMode = COM_IDLE;
			cout << "Status RESET" << endl;
		}
		else {
			cout << "Connection Error occured, unable to send data to EKARD!" << endl;
			ConnectionStatus = 0;
			enter_Command = 0;
			Command = COM_IDLE;
			MenuStatus = IN_MAIN_MENU;
			SubMenuStatus = IN_MAIN_MENU;
			//MenuMode = COM_IDLE;
			cout << "Connection RESET" << endl;
		}

	}
	cout << "did one execute" << endl;
}

bool EKARD_Client::get_ConnectionStatus() {
	return ConnectionStatus;
}

int* EKARD_Client::get_EKARD_Position() {
	for (int i = 0; i < ServoNum; i++) {
		PosBuffer[i] = EKARD_Axis[i].get_angle();
	}
	return PosBuffer;
}

void EKARD_Client::output_EKARDAngles() {
	cout << endl << "The current angles of the EKARD axis are: {";
	cout << EKARD_Axis[0].get_angle() << ", ";
	cout << EKARD_Axis[1].get_angle() << ", ";
	cout << EKARD_Axis[2].get_angle() << ", ";
	cout << EKARD_Axis[3].get_angle() << ", ";
	cout << EKARD_Axis[4].get_angle() << ", ";
	cout << EKARD_Axis[5].get_angle() << ", ";
	cout << "}" << endl;
}

void EKARD_Client::output_EKARDMinValues() {
	cout << endl << "The current min trimming values of the EKARD axis are: {";
	cout << EKARD_Axis[0].get_minPulse() << ", ";
	cout << EKARD_Axis[1].get_minPulse() << ", ";
	cout << EKARD_Axis[2].get_minPulse() << ", ";
	cout << EKARD_Axis[3].get_minPulse() << ", ";
	cout << EKARD_Axis[4].get_minPulse() << ", ";
	cout << EKARD_Axis[5].get_minPulse() << ", ";
	cout << "}" << endl;
}

void EKARD_Client::output_EKARDMaxValues() {
	cout << endl << "The current max trimming values of the EKARD axis are: {";
	cout << EKARD_Axis[0].get_maxPulse() << ", ";
	cout << EKARD_Axis[1].get_maxPulse() << ", ";
	cout << EKARD_Axis[2].get_maxPulse() << ", ";
	cout << EKARD_Axis[3].get_maxPulse() << ", ";
	cout << EKARD_Axis[4].get_maxPulse() << ", ";
	cout << EKARD_Axis[5].get_maxPulse() << ", ";
	cout << "}" << endl;
}

bool EKARD_Client::get_EKARDServerData() {
	Command = COM_REQUEST_EKARD_DATA;
	create_TCPStream();

	const int localStreamLength = 45;
	char localStreamBuff[localStreamLength];
	uint8_t localRecvStream[localStreamLength];
	int PosArray[ServoNum];
	int j = FirstPosAxisTCPStream;

	int sendCount = send(EKARD_ClientSocket, TCPStream, TCPStreamLength, 0);
	if (sendCount == 0) {
		cout << "Client sending failed" << endl;
		WSACleanup();
		return 0;
	}
	cout << "did request, sent data" << endl;
	int recvCount = recv(EKARD_ClientSocket, localStreamBuff, localStreamLength, 0);
	for (int i = 0; i < localStreamLength; i++) {
		localRecvStream[i] = static_cast<uint8_t>(localStreamBuff[i]);
	}
	if (recvCount == 0) {
		cout << "Client receive failed" << endl;
		WSACleanup();
		return 0;
	}
	if (!(localRecvStream[0] == 0xAA && localRecvStream[1] == 0xBB && localRecvStream[FirstPosAxisTCPStream - 1] == COM_REQUEST_EKARD_DATA && localRecvStream[localStreamLength - 2] == 0xDD && localRecvStream[localStreamLength - 1] == 0xEE)) {
		cout << "Transmitting data to EKARD failed" << endl;
		WSACleanup();
		return 0;
	}
	
	//extracting data
	PosArray[0] = (localRecvStream[j++] << 8);
	PosArray[0] |= localRecvStream[j++];
	for (int i = 1; i < ServoNum; i++) {
		PosArray[i] = localRecvStream[j++];
	}
	for (int i = 0; i < ServoNum; i++) {
		EKARD_Axis[i].set_angle(PosArray[i]);
	}

	for (int i = 0; i < ServoNum; i++) {
		PosArray[i] = (localRecvStream[j++] << 8);
		PosArray[i] |= localRecvStream[j++];
	}
	for (int i = 0; i < ServoNum; i++) {
		EKARD_Axis[i].set_minPulse(PosArray[i]);
	}

	for (int i = 0; i < ServoNum; i++) {
		PosArray[i] = (localRecvStream[j++] << 8);
		PosArray[i] |= localRecvStream[j++];
	}
	for (int i = 0; i < ServoNum; i++) {
		EKARD_Axis[i].set_maxPulse(PosArray[i]);
	}
	
	Axis0_Time = (localRecvStream[j++] << 8);
	Axis0_Time |= localRecvStream[j++];

	Axis0_Speed = localRecvStream[j++];

	MAX_TimeDiff = (localRecvStream[j++] << 8);
	MAX_TimeDiff |= localRecvStream[j++];

	CalibrationFactor = (localRecvStream[j++] << 8);
	CalibrationFactor |= localRecvStream[j++];

	Axes_Speed = (localRecvStream[j++] << 8);
	Axes_Speed |= localRecvStream[j++];


	Command = COM_RECEIVED_DATA;
	create_TCPStream();

	sendCount = send(EKARD_ClientSocket, TCPStream, TCPStreamLength, 0);
	if (sendCount == 0) {
		cout << "Client sending failed" << endl;
		WSACleanup();
		return 0;
	}
	cout << "did sent acknowledge" << endl;
	recvCount = recv(EKARD_ClientSocket, TCPStream, TCPStreamLength, 0); 
	for (int i = 0; i < TCPStreamLength; i++) {
		TCPRecvStream[i] = static_cast<uint8_t>(TCPStream[i]);
		cout << static_cast<int>(TCPRecvStream[i]) << endl;
	}
	if (recvCount == 0) {
		cout << "Client receive failed" << endl;
		WSACleanup();
		return 0;
	}
	if (!(TCPRecvStream[0] == 0xAA && TCPRecvStream[1] == 0xBB && TCPRecvStream[FirstPosAxisTCPStream - 1] == COM_RECEIVED_DATA && TCPRecvStream[TCPStreamLength - 2] == 0xDD && TCPRecvStream[TCPStreamLength - 1] == 0xEE)) {
		cout << "Transmitting acknowledge to EKARD failed" << endl;
		WSACleanup();
		return 0;
	}
	cout << "Successfully got data from EKARD" << endl;
	return 1;
}
