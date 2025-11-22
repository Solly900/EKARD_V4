#include "EKARD_Client.hpp"


int main() {

	EKARD_Client EKARD_IV; //EKARD Objekt instanziieren

	while (1) {
		if (EKARD_IV.connect_ToServer()) { //erste Verbindung zum EKARD aufbauen
			while (1) {
				std::cout << "gonna execute main" << std::endl;
				EKARD_IV.execute_EKARD(); //in Dauerschleife die EKARD ausführen
				std::cout << "executed main" << std::endl;
			}
		}
	}
	 return 0;
}