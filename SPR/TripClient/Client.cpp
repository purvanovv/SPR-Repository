#include <iostream>
#include <string>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

#define ADD_OPTIONS_COUNT 8

using namespace std;

int chooseMainMenuOption();
void addTrip(SOCKET sock);
void showAllTrips(SOCKET sock);
void showTrip(SOCKET sock);
void showShortestTrips(SOCKET sock);
void showLongestTrips(SOCKET sock);

struct TripPoint {
	int x;
	int y;
	char trip_name[20];
};

struct Trip {
	int id;
	struct TripPoint start_point;
	struct TripPoint end_point;
	int avarage_speed;
	int time;
};

void main()
{
	string ipAddress = "192.168.0.101";		// Ip Address of the server
	int port = 54000;						// Listening port ]# on the server

	// Initialize WinSock
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return;
	}
	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}
	// Fill in hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	//Connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return;
	}

	// Do-while loop to send and receive data
	int option;
	do
	{
		option = chooseMainMenuOption();

		switch (option)
		{
		case 1: {
			addTrip(sock);
			break;
		}
		case 2: {
			showAllTrips(sock);
			break;
		}
		case 3: {
			showTrip(sock);
			break;
		}
		case 4: {	
			showShortestTrips(sock);
			break;
		}
		case 5: {
			showLongestTrips(sock);
			break;
		}
		case 6: {
			break;
		}
		default:
			break;
		}
		//prompt the user for some text

		//echo response to console
	} while (option < 6);
	// Gracefully close down everything

}

int chooseMainMenuOption() {
	system("CLS");
	printf("*****************************\n");
	printf("* 1.Add trip                *\n");
	printf("* 2.Show all trips          *\n");
	printf("* 3.Show trip by id         *\n");
	printf("* 4.Show top shortest trips *\n");
	printf("* 5.Show top longest trips  *\n");
	printf("* 6.Exit		    *\n");
	printf("*****************************\n");

	int option;

	printf("Choose option:");
	cin >> option;
	return option;
}

void addTrip(SOCKET sock) {
	string options[] = { "Start point name:","x:","y:","End point name:","x:","y:","avarage speed:","time:" };
	system("CLS");
	printf("********Add Trip Menu*******\n");
	char buf[4096];
	char userInput[100];
	int options_count = 8;

	//send to know for our option
	send(sock, "1", sizeof("1") + 1, 0);
	for (int i = 0; i < ADD_OPTIONS_COUNT; i++)
	{
		cout << options[i];
		cin >> userInput;
		int sendResult = send(sock, userInput, sizeof(userInput) + 1, 0);
	}

	ZeroMemory(buf, 4096);
	int bytesReceived = recv(sock, buf, 4096, 0);
	if (bytesReceived > 0)
	{
		// Echo response to console
		cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
		system("pause");
	}
}

void showAllTrips(SOCKET sock) {
	char buf[4096];
	system("CLS");
	printf("****Show all trips menu*****\n");

	send(sock, "2", sizeof("2") + 1, 0);
	while (true) {
		ZeroMemory(buf, 4096);
		int bytesReceived = recv(sock, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error in recv(). Quiting" << endl;
			return;
		}

		if (bytesReceived == 0)
		{
			cout << "Server disconnected " << endl;
			return;
		}

		if (strncmp(buf, "END", strlen("END")) == 0) {
			break;
		}
		if (strncmp(buf, "N", strlen("N")) == 0) {
			printf("****************************\n");
		}
		else {
			cout << buf << endl;
			send(sock, "1", sizeof("1"), 0);
		}
		
	}
	system("pause");
}

void showTrip(SOCKET sock) {
	char tripIdText[3];
	char buf[4096];
	system("CLS");
	printf("*********Show trip**********\n");
	printf("Enter trip id:");
	int tripId;
	cin >> tripId;

	send(sock, "3", sizeof("3") + 1, 0);

	sprintf(tripIdText, "%ld", tripId);
	send(sock, tripIdText, sizeof(tripIdText) + 1, 0);
	while (true) {
		ZeroMemory(buf, 4096);
		int bytesReceived = recv(sock, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error in recv(). Quiting" << endl;
			return;
		}

		if (bytesReceived == 0)
		{
			cout << "Server disconnected " << endl;
			return;
		}

		if (strncmp(buf, "END", strlen("END")) == 0) {
			break;
		}

		else {
			cout << buf << endl;
			send(sock, "1", sizeof("1"), 0);
		}

	}
	system("pause");
}

void showShortestTrips(SOCKET sock) {
	char buf[4096];
	char top[3];
	system("CLS");
	printf("****Show shortest trips*****\n");

	printf("Enter top:");
	int tripId;
	cin >> tripId;

	send(sock, "4", sizeof("4") + 1, 0);

	sprintf(top, "%ld", tripId);
	send(sock, top, sizeof(top) + 1, 0);
	while (true) {
		ZeroMemory(buf, 4096);
		int bytesReceived = recv(sock, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error in recv(). Quiting" << endl;
			return;
		}

		if (bytesReceived == 0)
		{
			cout << "Server disconnected " << endl;
			return;
		}

		if (strncmp(buf, "END", strlen("END")) == 0) {
			break;
		}
		if (strncmp(buf, "N", strlen("N")) == 0) {
			printf("****************************\n");
		}
		else {
			cout << buf << endl;
			send(sock, "1", sizeof("1"), 0);
		}

	}
	system("pause");
}

void showLongestTrips(SOCKET sock) {
	char buf[4096];
	char top[3];
	system("CLS");
	printf("****Show longest trips******\n");

	printf("Enter top:");
	int tripId;
	cin >> tripId;

	send(sock, "5", sizeof("5") + 1, 0);

	sprintf(top, "%ld", tripId);
	send(sock, top, sizeof(top) + 1, 0);
	while (true) {
		ZeroMemory(buf, 4096);
		int bytesReceived = recv(sock, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error in recv(). Quiting" << endl;
			return;
		}

		if (bytesReceived == 0)
		{
			cout << "Server disconnected " << endl;
			return;
		}

		if (strncmp(buf, "END", strlen("END")) == 0) {
			break;
		}
		if (strncmp(buf, "N", strlen("N")) == 0) {
			printf("****************************\n");
		}
		else {
			cout << buf << endl;
			send(sock, "1", sizeof("1"), 0);
		}

	}
	system("pause");
}



