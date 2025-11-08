#include "EKARD_Client.hpp"


int main() {
	bool status = 0;
	EKARD_Client test;
	status = test.connect_ToServer();
	return 0;
}