# Dokumentation EKARD Version IV Client
Der EKARD_Client ist eine Anwendung, die sich beim Start automatisch mit dem EKARD_Server, also dem Greifarm-Roboter verbindet. 
WICHTIG: Um eine Verbindung zum EKARD_Server aufbauen zu können, muss der EKARD eingeschaltet sein, darf nicht bereits eine Verbindung zu einem anderen Client aufgebaut haben und muss sich im selben Netzwerk befinden. Um letzteres zu erreichen, muss man sich entweder mit dem Gerät, auf dem die EKARD_Client Anwendung ausgeführt wird, in das Netzwerk des EKARD einwählen (Zugangsdaten siehe EKARD_Server Sourcecode), oder im Sourcecode des EKARD_Client sowie des EKARD_Server #inWlan definieren und in letzterem auch die Einwahldaten für das Netzwerk hinterlegen, in dem sich das Client-Gerät befindet (eventuell können dabei IP-Konflikte entstehen, falls im genutzten Netzwerk die Adresse 192.168.178.201 bereits vergeben ist oder eine andere Subnetzmaske verwendet wird 
--> ggf. die im EKARD_Server hinterlegte IP, Subnetzmaske und Gateway anpassen. WICHTIG: In diesem Fall muss auch die im EKARD_Client Header unter #inWlan definierte IP-Adresse angepasst werden (muss der neuen EKARD_Server IP entsprechen)).

Der EKARD_Client wurde in der Umgebung Visual Studio unter Windows10 geschrieben. Um den Sourcecode in einer anderen Programmierungumgebung oder unter einem anderen Betriebssystem zu compilieren, kann es nötig werden, andere Bibliotheken in dem EKARD_Client Header einzubinden. In der akutellen Version, wird die winsock2.h Bibliothek von Windows genutzt, um eine TCP Verbindung aufzubauen. Das "#pragma comment" ist ein spezieller Visual Studio Befehl.  

## main

In der main-Funktion wird zu Anfang ein Objekt der EKARD_Client Klasse initialisiert, mit dem das Programm arbeitet. 
Nachdem dann erfolgreich eine Verbindung zum EKARD-Server hergestellt wurde, muss noch noch die execute-Funktion der EKARD_Client Klasse in Dauerschleife ausgeführt werden, diese handelt das Interface sowie den Wiederaufbau der Verbindung bei Abbrüchen.


## EKARD_Client Header File
Der EKARD_Client Header enthält:
- Definition spezieller Parameter:
> - EKARD_Server IP
> - EKARD_Server Port-Nr.
> - Anzahl der im EKARD verbauten Servos
> - Eckdaten der verwendeten Servos zur Eingabebegrenzung
> - KommunikationsProtokoll Parameter
- Deklaration der verwendeten Klassen Servo und EKARD_Client
- Definition der Befehle zur Kommunikation mit dem EKARD_Server 
- Definition der Befehle zur User-Interface Steuerung


## EKARD_Client Source File
Enthält die Definitionen der Klassenfunktionen im namespace std.


## class Servo

### Konstuktor
Initialisiert die Attribute mit den Standard-Werten, diese werden jedoch zu Beginn der Kommunikation mit dem EKARD_Server von den auf dem Server gespeicherten Daten überschrieben.

### Attribute
#### int angle 
- hält die akutelle Winkelposition des Servos in °
#### int minPulse/maxPulse
- bestimmen die Grenzen, in denen die Pulsweiten des PWM Signals zur Servoansteuerung liegen 
- -> bestimmen also das Übersetzungverhältnis von eingegebenem Winkel in Realwinkel. Die Werte 500 und 2500 haben sich experimentell als gute Näherung für Standard-180°-ServoMotoren herausgestellt. Um die Übersetzung von vorgegebenem Winkel zu vom Servo angefahrenen Winkel genauer zu kalibrieren, können diese Werte angepasst werden. minPulse beeinflusst dabei den kleinsten Winkel, maxPulse den größten Winkel. Für rundum drehende 360° Motoren, bei denen kein Winkel angefahren werden kann, bestimmen diese Werte stattdessen die maximalen Drehgeschwindigkeiten in beide Richtungen. 
WICHTIG: werden andere Motoren (bspw. 270°/360° verwendet, müssen diese Grenzen ebenfalls angepasst werden, daher sind für Servo0 und Servo1 auch abweichende Standardwerte definiert)

### Methoden
#### Set-Funktionen
Ermöglichen es, den Wert des jeweiligen Attributes zu verändern.

#### Get-Funktionen
Geben den aktuellen Wert des jeweiligen Attributes zurück.


## class EKARD_Client

### Konstruktor
- Initialisiert die Attribute, die zur Programmsteuerung benötigt werden, mit NULL
- Initialisiert die ersten und letzten beiden Positionen des Kommunikationsbuffers (TCPStream) mit festen Werten. Diese vier Bytes dienen als Prüfsumme, um eine korrekte Übertragung der Daten detektieren zu können. 

### Attribute
#### bool ConnectionStatus
- true, wenn eine Verbindung zum Server besteht
#### int Command
- Befehl an den Server -> bestimmt, wie die gesendeten Daten vom EKARD_Server verarbeitet werden, welche Funktion vom Server ausgeführt wird
#### bool enter_Command
- true, wenn ein Befehl zur Übertragung an den Server eingegeben wurde
#### MenuStatus
- gibt für die nächste Menü-Iteration an, welche Menüseite anzuzeigen ist
#### Submenu
- gibt die Menüseite der vorherigen Menü-Iteration an, um Verwechslungen zu vermeiden (veraltet)

#### int Axis0_Time
- Zeit, die Servo0 für eine 360° Drehung benötigt
#### int Axis0_Speed
- Wert für die Geschwindigkeit von Servo0, bei der die Zeit für 360° gemessen wurde
#### int MAX_TimeDiff 
- zulässige maximale Zeitdifferenz zwischen Drehung nach rechts und Drehung nach links von Servo0
- wird bei der Kalibrierung von Servo0 berücksichtigt
- je kleiner der Wert, desto genauer die Kalibrierung, dafür jedoch mehr Iterationen nötig -> verlängerte Kalibrierzeit
#### int CalibrationFactor
- Faktor, um den die Geschwindigkeit von Servo0 in jedem Iterationsschritt angepasst wird
- je höher der Wert, desto größer die Kalibrierschrittweite, kann die Kalibrierung beschleunigen, ein zu großer Wert in Verbindung mit einer sehr kleinen MAX-TimeDiff kann jedoch auch dazu führen, dass kein passender Wert gefunden wird und sich eine Endlosschleife einstellt.
#### int Axes_Speed
- Zeit, die alle Servos außer Servo0 benötigen, um ihre vorgegebene Position anzufahren
#### char TCPStream[TCPStreamLength]
- Befehls- und Datenbuffer, der an den Server gesendet wird
#### uint8_t TCPRecvStream[TCPStreamLength]
- dient als Übergangsbuffer zum Senden und Empfangen von Daten, um negative Werte richtig darzustellen, da das TCP-Protokoll nur unsigned integer unterstützt
#### int PosBuffer[ServoNum]
- gibt die Positionen aller Servos in Bufferform
#### Servo EKARD_Axis[ServoNum]
- Array mit einem Servo-Objekt für jeden ServoMotor des EKARD
#### SOCKET EKARD_ClientSocket
- Socket Instanz, auf dem die TCP-Verbindung aufgebaut wird


### Methoden
#### bool transmit_EKARDData()
Sendet die Daten aus dem TCPStream-Buffer an den EKARD_Server und empfängt die Antwort des Servers. Die Serverantwort wird über die Prüfsumme auf Korrektheit überprüft. War der Datenaustausch erfolgreich, ist der Rückgabewert 1, ansonsten 0 und die TCP-Verbindung zum Server wird getrennt.
#### void create_TCPStream()
Füllt den TCPStream-Buffer mit den aktuell im EKARD_Client eingegebenen Daten, jenachdem welcher Kommunikationsbefehl gerade aktiv ist.
#### void output_EKARDAngles()
Gibt die aktuell im EKARD_Client eingestellten Winkelpositionen auf dem Bildschirm aus.
#### void output_EKARDMinValues()
Gibt die aktuell im EKARD_Client eingestellten minPulse-Werte auf dem Bildschirm aus.
#### void output_EKARDMaxValues()
Gibt die aktuell im EKARD_Client eingestellten maxPulse-Werte auf dem Bildschirm aus.
#### void get_User_Input()
Zeigt je nachdem in welchem Untermenü (wird durch MenuStatus-Wert angegeben) sich die Nutzerin befindet das jeweilige Interface an und fragt einen Eingabewert ab. Dieser wird auf Korrektheit überprüft und dann abgespeichert. Je nach Befehlseingabe werden die Werte für MenuStatus, SubMenuStatus, Command und enter_Command gesetzt, die sich auf den nächsten Schleifendurchlauf auswirken (bestimmen, welche Menüseite angezeigt wird).
#### void execute_EKARD()
Überprüft als erstes die Verbindung zum EKARD_Server und versucht, sich erneut zu verbinden, falls die Verbindung abgebrochen ist. 
Besteht eine Verbindung zu Server wird über die get_User_Input-Methode die nächste Befehlseingabe abgefragt.
Anschließend wird, falls ein Befehl zur Datenübertragung an den Server eingegeben wurde, über die create_TCPStream-Methode der Buffer zur Datenübertragung gefüllt und anschließend an den Server gesendet. Falls dabei ein Fehler in der Kommunikation mit dem EKARD_Server auftritt, werden die Client-Einstellungen resetet und die Verbindung zum Server getrennt.
#### bool connect_ToServer()
Erstellt ein Socket-Objekt und versucht darauf basierend eine TCP-Verbindung zum EKARD_Server aufzubauen. Dazu muss dieser sich im gleichen Netzwerk wie das Clientgerät befinden und die im Client definierte IP-Adresse besitzen. 
Wurde erfolgreich eine Verbindung zur definierten IP-Adresse aufgebaut, werden vom EKARD_Server die im EEPROM abgelegten Daten angefordert und die Attribute des erstellten EKARD_Client-Objektes mit diesen Werten initialisiert. Erst wenn dieser Datenaustausch erfolgreich stattgefunden hat, wird der "ConnectionStatus" auf 1 gesetzt, es wird ebenfalls 1 zurückgegeben, Rückgabewert NULL, falls etwas beim Verbindungsaufbau fehlgeschlagen ist.
#### bool get_ConnectionStatus()
Gibt den aktuellen Wert von "ConnectionStatus" zurück (1: Verbindung besteht; 0: keine Verbindung).
#### int* get_EKARD_Position()
Gibt einen Zeiger auf ein Array mit den aktuellen Winkelpositionen der Achsen des EKARD zurück.
#### bool get_EKARDServerData()
Fragt die im EEPROM des EKARD gespeicherten Daten an und liest diese in den Client ein (1: Erfolgreich; 0: Kommunikation fehlgeschlagen).