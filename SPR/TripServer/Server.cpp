#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable:4996)

#define ADD_OPTIONS_COUNT 8

using namespace std;

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

//////////////declare functions//////////////////////
void processAddTrip(SOCKET clientSocket);
void writeTrip(struct Trip trip);
int getLastId();


void main() 
{
	// Initialize winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsok = WSAStartup(ver, &wsData);
	if (wsok != 0) {
		cerr << "Can't Initialize winsock! Quiting" << endl;
		return;
	}
	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quiting" << endl;
		return;
	}

	// Bind the socket to an ip address and port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));
	// Tell winsock the socket is for listening
	listen(listening, SOMAXCONN);

	// wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	char host[NI_MAXHOST];	//Client's remote name
	char service[NI_MAXHOST];	//Service(i.e port) the client is connected on

	ZeroMemory(host, NI_MAXHOST);	//same as memset(hostm 0 mNI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connected on port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected in port " << ntohs(client.sin_port) << endl;
	}
	// close listening socket
	closesocket(listening);

	// while loop: accept and echo message back to client
	char buf[4096];

	while (true) {
		ZeroMemory(buf, 4096);

		//Wait for client to send data
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error in recv(). Quiting" << endl;
			break;
		}

		if (bytesReceived == 0)
		{
			cout << "Client disconnected " << endl;
			break;
		}

		cout << buf << endl; 
		switch (buf[0])
		{
		case '1': {
			processAddTrip(clientSocket);
		}
		case '2': {
			break;
		}
		case '3': {

		}
		default:
			break;
		}
		//Echo message back to client
		//send(clientSocket, buf, bytesReceived + 1, 0);
	}

	// close the sock
	closesocket(clientSocket);

	// Cleanup winsock
	WSACleanup();
}

void processAddTrip(SOCKET clientSocket) {
	char data[ADD_OPTIONS_COUNT][100];

	for (size_t i = 0; i < ADD_OPTIONS_COUNT; i++)
	{
		int bytesReceived = recv(clientSocket, data[i], 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error in recv(). Quiting" << endl;
			return;
		}

		if (bytesReceived == 0)
		{
			cout << "Client disconnected " << endl;
			return;
		}
	}

	struct TripPoint startPoint;
	strcpy(startPoint.trip_name, data[0]);
	startPoint.x = stoi(data[1]);
	startPoint.y = stoi(data[2]);

	struct TripPoint endPoint;
	strcpy(endPoint.trip_name, data[3]);
	endPoint.x = stoi(data[4]);
	endPoint.y = stoi(data[5]);

	struct Trip trip;
	trip.start_point = startPoint;
	trip.end_point = endPoint;
	trip.avarage_speed = stoi(data[6]);
	trip.time = stoi(data[7]);

	writeTrip(trip);

	char message[] = { "SUCCESSFULL" };
	send(clientSocket, message, sizeof(message) + 1, 0);
}

int getLastId() {
	FILE * file;
	struct Trip trip;
	int lastId = 0;

	file = fopen("trips.txt", "rb");
	if (file == NULL) {
		return lastId;
	}

	while (fread(&trip, sizeof(struct Trip), 1, file)) {
		if (trip.id > lastId) {
			lastId = trip.id;
		}
	}
	fclose(file);
	return lastId;
}

void writeTrip(struct Trip trip) {
	FILE * file;

	int tripId = getLastId() + 1;
	trip.id = tripId;

	file = fopen("trips.txt", "ab");
	if (file == NULL) {
		fprintf(stderr, "\nError opend file!\n");
		exit(1);
	}

	fwrite(&trip, sizeof(struct Trip), 1, file);

	if (fwrite != 0)
		printf("contents to file written successfully !\n");
	else
		printf("error writing file !\n");

	fclose(file);
}