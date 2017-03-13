#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>

struct CMenedger
{
	SOCKET Connect;
	SOCKET *Connections;
	SOCKET Listen;

	int ClientCount;

	sockaddr_in client_addr;
	int client_addr_size;

	CMenedger()
	{
		ClientCount = 0;
		Connections = (SOCKET *)calloc(64, sizeof(SOCKET));
		client_addr_size = sizeof(client_addr);
	}
} g_pCM;

DWORD WINAPI SendMessageToClients(int ID)
{
	char buffer[1024];
	int CurCount = (g_pCM.ClientCount - 1);
	send(g_pCM.Connections[ID], "Connected in server!\n", 25, NULL);

	int bytes_recv;
	for(int i = 0; i < sizeof(buffer); i++)
	{
		buffer[i] = '\0';
	}
	while((bytes_recv = recv(g_pCM.Connections[ID], buffer, 1024, NULL)) && bytes_recv != SOCKET_ERROR)
	{
		printf(buffer);
		if(CurCount)
		{
			for(int i(0); i <= CurCount; i++)
			{
				if(g_pCM.Connections[i] != g_pCM.Connections[ID])
				{
					send(g_pCM.Connections[i], buffer, strlen(buffer), NULL);
				}
			}
		}
		for(int i = 0; i < sizeof(buffer); i++)
		{
			buffer[i] = '\0';
		}
	}
	closesocket(g_pCM.Connections[ID]);
	g_pCM.ClientCount--;
	printf("Client is disconect\n");
	return 0;
}

int main()
{
	setlocale(LC_ALL, "RUS");
	WSAData date;

	if(WSAStartup(WINSOCK_VERSION, &date) != NULL)
	{
		printf("Socket starting is error [%d]!", WSAGetLastError());
		closesocket(g_pCM.Listen);
		system("PAUSE");
		return -1;
	}

	addrinfo hints;
	addrinfo *result;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_flags	= AI_PASSIVE;
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_protocol	= IPPROTO_TCP;

	getaddrinfo(NULL, "7770", &hints, &result);

	g_pCM.Listen = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(g_pCM.Listen == INVALID_SOCKET)
	{
		printf("Socket is error [%d]!", WSAGetLastError());
		closesocket(g_pCM.Listen);
		system("PAUSE");
		return -1;
	}

	if(bind(g_pCM.Listen, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
	{
		printf("Socket bind is error [%d]!", WSAGetLastError());
		closesocket(g_pCM.Listen);
		system("PAUSE");
		return -1;
	}
	listen(g_pCM.Listen, SOMAXCONN);

	freeaddrinfo(result);

	printf("Server started!\n");

	while((g_pCM.Connect = accept(g_pCM.Listen, (sockaddr *)&g_pCM.client_addr, &g_pCM.client_addr_size)))
	{		
		g_pCM.ClientCount++;
		HOSTENT *hst;
		hst = gethostbyaddr((char *)&g_pCM.client_addr.sin_addr.S_un.S_addr, 4, AF_INET);
		
		printf("Client %s [%s] is conected\n", (hst) ? hst->h_name : "", inet_ntoa(g_pCM.client_addr.sin_addr));

		g_pCM.Connections[g_pCM.ClientCount] = g_pCM.Connect;
		DWORD thID;
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendMessageToClients, (LPVOID)g_pCM.ClientCount, NULL, &thID);
	}
	system("PAUSE");
	return EXIT_SUCCESS;
}