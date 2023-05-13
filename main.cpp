#include <iostream>
#include <vector>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

#define MAX_CLIENTS 10

using namespace std;

int main() {
    // Winsock initialization
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOK = WSAStartup(ver, &wsData);
    if (wsOK != 0)
    {
        cerr << "Can't initialize winsock! Aborting" << endl;
        return 1;
    }

    // Socket creation
    SOCKET listening = socket(AF_INET6, SOCK_STREAM, 0); // Use AF_INET6 for IPv6
    if (listening == INVALID_SOCKET)
    {
        cerr << "Can't create a socket! Aborting" << endl;
        return 1;
    }

    // Bind socket to ip and port
    sockaddr_in6 hint;
    hint.sin6_family = AF_INET6; // Use AF_INET6 for IPv6
    hint.sin6_port = htons(54000);
    inet_pton(AF_INET6, "::1", &hint.sin6_addr); // Use "::1" for localhost in IPv6

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Socket will be listening
    if (listen(listening, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "Can't listen on socket! Error: " << WSAGetLastError() << endl;
        closesocket(listening);
        WSACleanup();
        return 1;
    }

    // Set up the set of sockets
    fd_set master;
    FD_ZERO(&master);
    FD_SET(listening, &master);

    // Initialize the vector of clients
    vector<SOCKET> clients(MAX_CLIENTS, INVALID_SOCKET);

    while (true) {
        // Copy the master set to a temporary set
        fd_set copy = master;

        // Wait for some socket to become ready
        int socketCount = select(0, &copy, nullptr, nullptr, nullptr);
        if (socketCount == SOCKET_ERROR)
        {
            cerr << "Can't select sockets! Aborting" << endl;
            break;
        }

        // Check which sockets are ready and handle them accordingly
        for (int i = 0; i < socketCount; ++i)
        {
            SOCKET sock = copy.fd_array[i];

            if (sock == listening)
            {
                // Accept a new client and add it to the set of sockets
                sockaddr_in6 client; // Use sockaddr_in6 for IPv6
                ZeroMemory(&client, sizeof(client)); // Initialize to zero
                int clientSize = sizeof(client);
                SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
                if (clientSocket == INVALID_SOCKET)
                {
                    cerr << "Can't accept client! Aborting" << endl;
                    continue;
                }
                // Add the client socket to the set of sockets
                FD_SET(clientSocket, &master);

                // Add the client socket to the vector of clients
                for (int i = 0; i < MAX_CLIENTS; ++i)
                {
                    if (clients[i] == INVALID_SOCKET)
                    {
                        clients[i] = clientSocket;
                        break;
                    }
                }
                // Print the remote name and port of the client
                char host[NI_MAXHOST];
                char service[NI_MAXSERV];
                ZeroMemory(host, NI_MAXHOST);
                ZeroMemory(service, NI_MAXSERV);
                if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
                    cout << host << " connected on port " << service << endl;
                }
                else {
                    if (client.sin6_family == AF_INET6) {
                        inet_ntop(AF_INET6, &(((sockaddr_in6*)&client)->sin6_addr), host, NI_MAXHOST);
                        cout << host << " connected on port " << ntohs(((sockaddr_in6*)&client)->sin6_port) << endl;
                    }
                    else {
                        inet_ntop(AF_INET, &(((sockaddr_in*)&client)->sin_addr), host, NI_MAXHOST);
                        cout << host << " connected on port " << ntohs(((sockaddr_in*)&client)->sin_port) << endl;
                    }
                }



            }
            else {
                // Handle data from a client
                char buf[4096];
                ZeroMemory(buf, 4096);
                int bytesReceived = recv(sock, buf, 4096, 0);
                if (bytesReceived <= 0)
                {
                    // Connection closed or error
                    closesocket(sock);
                    FD_CLR(sock, &master);
                    for (int i = 0; i < MAX_CLIENTS; ++i)
                    {
                        if (clients[i] == sock)
                        {
                            clients[i] = INVALID_SOCKET;
                            break;
                        }
                    }
                }
                else {
                    // Send the message to all other clients
                    for (int i = 0; i < MAX_CLIENTS; ++i)
                    {
                        if (clients[i] != INVALID_SOCKET && clients[i] != sock)
                            send(clients[i], buf, bytesReceived, 0);
                    }
                }
            }
        }
    }


    // Close the listening socket
    closesocket(listening);

    // Clean up winsock
    WSACleanup();

    return 0;
    }