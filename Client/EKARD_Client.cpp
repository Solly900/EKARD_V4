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

void Servo::set_PulseWidth(int min, int max) {
	minPulse = min;
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
	ConnectionStatus = 0;
	Command = COM_IDLE;
}

bool EKARD_Client::connect_ToServer() {
	//initializing variables
	SOCKET clientSocket;
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
	clientSocket = INVALID_SOCKET;
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
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
	if (connect(clientSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		cout << "Error: Failed to connect to Server! (IP: "<<EKARD_IP<<", PortNr: "<<EKARD_PORT << endl;
		closesocket(clientSocket);
		WSACleanup();
		return 0;
	}
	else {
		cout << "Client: connect() is OK!" << endl;
		cout << "Client start sending and receiving..." << endl;
	}

	//test send and receive data
	const int num = 200;

	//sending data
	char sendbuffer[num];
	char inputbuff[30];
	int x;
	cin >> x;
	if (x == 12) {
		cout << " is = 12" << endl;

	}
	bitset<32> bits(x);
	cout << bits << endl;
	uint8_t test;
	uint8_t in;
	byte inn;

	cin >> test;
	in = (x & 0xFF);
	if (in == 12) {
		cout << "transform correctly" << endl;

	}
	test = 12;
	cout << "input: " << in << endl;


	for (int i = 0; i < 1; i++) {
		cout << "Type in a message to send..." << endl;
		//cin.getline(inputbuff, 30);
		//cin >> x;
		//strcpy_s((sendbuffer + i*30), 30, inputbuff);
		//strcpy_s((sendbuffer + ), 12, "Hello there");
	}
	sendbuffer[0] = 1;
	cout << sendbuffer[0] << endl;

	int sendCount = send(clientSocket, sendbuffer, num, 0);
	if (sendCount > 0) {
		cout << "Client send message" << endl;
	}
	else {
		cout << "Client sending failed" << endl;
		WSACleanup();
		return 0;
	}

	//receiving data
	char recvbuffer[num];

	int recvCount = recv(clientSocket, recvbuffer, num, 0);
	if (recvCount > 0) {
		cout << "Client received " << recvCount << " bytes" << endl;
		for (int i = 0; i < 10; i++) {
			cout << "byte " << i << " = " << recvbuffer[i] << endl;
		}
		if (recvbuffer[0] == 0xAA) {
			cout << "received successfully" << endl;
		}
		cout << "Message received: " << recvbuffer << endl;
	}
	else {
		cout << "Client receive failed" << endl;
		WSACleanup();
	}





	//end main and cleanup
	system("pause");
	WSACleanup();
	return 1;
}

void EKARD_Client::get_User_Input() {
	int input_buff[7];
	int menuIput = 0; 

	switch (InterfaceStatus) {
	case IN_MAIN_MENU:
		cout << "Welcome to EKARD the IV." << endl;
		cout << "What would you like to do?" << endl << endl;
		cout << "Set a new position for an axis: " << IN_MAIN_MENU << endl;
		cout << "AutoHome the EKARD: " << IN_AUTO_HOME << endl;

	}
	
}