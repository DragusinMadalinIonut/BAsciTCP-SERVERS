#include <iostream>
#include <WS2tcpip.h>
//w2stcpip includes header files and the windows socket

#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main() {

	//Winsock initialization
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsOK = WSAStartup(ver, &wsData);
	if (wsOK != 0)
	{
		cerr << "CAn't initializie winsock! Aborting" << endl;
		return;
	}
	//socket creation
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Aborting" << endl;
	}

	//Bind socket to ip and port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; //configuration with inet_pton also possible

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	//Socket will be listening
	listen(listening, SOMAXCONN);
	//while waitting for conection
	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	char host[NI_MAXHOST]; //cleint remote name
	char service[NI_MAXSERV]; //Service/port for when the client is connected

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << "connected on port" << service << endl;

	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << "connected on port" <<
			ntohs(client.sin_port) << endl;
	}

	//close listening socket
	closesocket(listening);

	//while loop acept and echo messages back to client
	char buff[4096];

	while (true) {
		ZeroMemory(buff, 4096);

		//Wait for client to send data
		int bytesReceived = recv(clientSocket, buff, 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "ERROR in recv(). Aborting" << endl;
			break;
		}

		if (bytesReceived == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}
		//Echo back to client
		send(clientSocket, buff, bytesReceived + 1, 0);

	}
	//Close socket
	closesocket(clientSocket);

	//Winsock Shutdown
	WSACleanup();
}

