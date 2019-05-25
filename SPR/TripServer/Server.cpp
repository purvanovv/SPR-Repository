#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable:4996)

#define ADD_OPTIONS_COUNT 8
#define TRIP_FIELDS_COUNT 9

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
void processShowAllTrips(SOCKET clientSocket);
void writeTrip(struct Trip trip);
void readAllTrips(Trip *trips);
void sendTripsToClient(Trip *trips, int count_elements, SOCKET clientSocket);
void processShowTrip(SOCKET clientSocket);
int getLastId();
void sendTripToClient(struct Trip trip, SOCKET clientSocket);
bool getTripById(int tripId, struct Trip *tripIn);
void processShowTopShortestTrips(SOCKET clientSocket);
void sortTripsShortest(Trip *trips, int count_elements);
void sortTripsBiggest(Trip *trips, int count_elements);
bool firstTripBigger(Trip firstTrip, Trip secondTrip);
void swapTrips(Trip *firstTrip, Trip *secondTrip);
int countTrips(Trip *trips);
void processShowTopLongestTrips(SOCKET clientSocket);


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
			break;
		}
		case '2': {
			processShowAllTrips(clientSocket);
			break;
		}
		case '3': {
			processShowTrip(clientSocket);
			break;
		}
		case '4': {
			processShowTopShortestTrips(clientSocket);
			break;
		}
		case '5': {
			processShowTopLongestTrips(clientSocket);
			break;
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

void processShowAllTrips(SOCKET clientSocket) {
	char end[] = "END";
	Trip trips[20];
	readAllTrips(trips);
	int count_elements = sizeof(trips) / sizeof(Trip);
	sendTripsToClient(trips, count_elements, clientSocket);
	send(clientSocket, end, sizeof(end), 0);
}

void readAllTrips(Trip *trips) {
	FILE * file;
	struct Trip trip;

	file = fopen("trips.txt", "rb");
	if (file == NULL) {
		fprintf(stderr, "\nError opend file!\n");
		exit(1);
	}

	int counter = 0;
	while (fread(&trip, sizeof(struct Trip), 1, file)) {
		trips[counter] = trip;
		counter++;
	}

	fclose(file);
}
void sendTripsToClient(Trip *trips, int count_elements, SOCKET clientSocket) {

	char newLine[] = "N";
	for (int i = 0; i < count_elements; i++)
	{
		if (trips[i].id > 0) {
			sendTripToClient(trips[i], clientSocket);
			send(clientSocket, newLine, sizeof(newLine), 0);
		}
		
	}
}


void processShowTrip(SOCKET clientSocket) {
	char buf[5];
	char end[] = "END";
	struct Trip foundedTrip;
	int bytesReceived = recv(clientSocket, buf, 5, 0);

	int tripId = atoi(buf);
	if (getTripById(tripId, &foundedTrip)) {
		sendTripToClient(foundedTrip,clientSocket);
	}
	else {
		//printf("Can't find trip with id: %d\n", 5);
	}
	send(clientSocket, end, sizeof(end), 0);
}

bool getTripById(int tripId, struct Trip *tripIn) {
	FILE * file;
	struct Trip trip;

	file = fopen("trips.txt", "rb");
	if (file == NULL) {
		return false;
	}

	while (fread(&trip, sizeof(struct Trip), 1, file)) {
		if (trip.id == tripId) {
			tripIn->id = trip.id;
			tripIn->start_point = trip.start_point;
			tripIn->end_point = trip.end_point;
			tripIn->avarage_speed = trip.avarage_speed;
			tripIn->time = trip.time;
			return true;
		}
	}

	fclose(file);

	return false;

}

void sendTripToClient(struct Trip trip, SOCKET clientSocket) {
	char arr[TRIP_FIELDS_COUNT][100];
	char buf[4096];

	ZeroMemory(arr,sizeof(arr));
	sprintf(arr[0], "id:%ld", trip.id);

	strcat(arr[1], "Start city:");
	strcat(arr[1], trip.start_point.trip_name);
	sprintf(arr[2], " start.x:%ld", trip.start_point.x);
	sprintf(arr[3], " start.y%ld", trip.start_point.y);

	strcat(arr[4], "End city:");
	strcat(arr[4], trip.end_point.trip_name);
	sprintf(arr[5], " end.x:%ld", trip.end_point.x);
	sprintf(arr[6], " end:y:%ld", trip.end_point.y);

	sprintf(arr[7], "Avarage speed:%ld", trip.avarage_speed);

	sprintf(arr[8], "Trip time:%ld", trip.time);

	for (int i = 0; i < TRIP_FIELDS_COUNT; i++)
	{
		ZeroMemory(buf, 4096);
		send(clientSocket, arr[i], sizeof(arr[i]), 0);
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
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
	}
}
void processShowTopShortestTrips(SOCKET clientSocket) {
	char buf[4096];
	char end[] = "END";
	recv(clientSocket, buf, 4096, 0);
	int top = atoi(buf);

	Trip trips[20];
	readAllTrips(trips);
	int count_elements = countTrips(trips);
	sortTripsShortest(trips,count_elements);

	top = min(top, count_elements);
	sendTripsToClient(trips, top, clientSocket);

	send(clientSocket, end, sizeof(end), 0);
}

void sortTripsShortest(Trip *trips,int count_elements) {
	for (int i = 0; i < count_elements-1; i++)
	{
		for (int j = 0; j < count_elements-i-1; j++)
		{
			if (firstTripBigger(trips[j], trips[j + 1])) {
				swapTrips(&trips[j], &trips[j + 1]);
			}
		}
	}
}

bool firstTripBigger(Trip firstTrip, Trip secondTrip) {
	int firstTripDistance = sqrt(pow((firstTrip.end_point.x - firstTrip.start_point.x), 2) + pow((firstTrip.end_point.y - firstTrip.start_point.y), 2));
	int secondTripDistance = sqrt(pow((secondTrip.end_point.x - secondTrip.start_point.x), 2) + pow((secondTrip.end_point.y - secondTrip.start_point.y), 2));
	if (firstTripDistance > secondTripDistance) {
		return true;
	}
	return false;
}

void swapTrips(Trip *firstTrip, Trip *secondTrip) {
	Trip temp = *firstTrip;
	*firstTrip = *secondTrip;
	*secondTrip = temp;
}

int countTrips(Trip* trips) {
	int count = 0;
	while (trips[count].id > 0) {
		count++;
	}
	return count;
}

void processShowTopLongestTrips(SOCKET clientSocket) {
	char buf[4096];
	char end[] = "END";
	recv(clientSocket, buf, 4096, 0);
	int top = atoi(buf);

	Trip trips[20];
	readAllTrips(trips);
	int count_elements = countTrips(trips);
	sortTripsBiggest(trips, count_elements);

	top = min(top, count_elements);
	sendTripsToClient(trips, top, clientSocket);

	send(clientSocket, end, sizeof(end), 0);
}

void sortTripsBiggest(Trip *trips, int count_elements) {
	for (int i = 0; i < count_elements - 1; i++)
	{
		for (int j = 0; j < count_elements - i - 1; j++)
		{
			if (!firstTripBigger(trips[j], trips[j + 1])) {
				swapTrips(&trips[j], &trips[j + 1]);
			}
		}
	}
}
