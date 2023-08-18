// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Async/AsyncWork.h"
#include "Async/Async.h"

#include <Ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>


#pragma warning(disable:4996) 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")

using namespace std;
/**
 * 
 */
class NETWORKTEST_API SocketRep
{
	class Server
	{
	private:

		string SERVERIP;
		int SERVERPORT;

		const int BUFFERLENGTH = 2048;

		u_long iMode = 0;

		string username;
		string id;
		WSADATA wsaData;
		SOCKADDR_IN serverAddr;
		SOCKET connectSocket;
		SOCKADDR_IN clientAddr;
		SOCKADDR_IN remoteAddr;

		int	remoteAddrLen;

		void SetUp()
		{
			WSAStartup(MAKEWORD(2, 2), &wsaData);

			serverAddr.sin_port = htons(SERVERPORT);
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_addr.s_addr = inet_addr(SERVERIP.c_str());

			connectSocket = socket(AF_INET, SOCK_DGRAM, 0);

			clientAddr.sin_port = 0;
			clientAddr.sin_family = AF_INET;
			clientAddr.sin_addr.s_addr = INADDR_ANY;

			::bind(connectSocket, (LPSOCKADDR)&clientAddr, sizeof(clientAddr));

			int val = 64 * 1024;
			setsockopt(connectSocket, SOL_SOCKET, SO_SNDBUF, (char*)&val, sizeof(val));

			remoteAddrLen = sizeof(remoteAddr);

			ioctlsocket(connectSocket, FIONBIO, &iMode);
		}

	public:

		Server(int serverPort, string serverIp)
		{
			SERVERPORT = serverPort;
			SERVERIP = serverIp;
			SetUp();
		}
		~Server()
		{
			closesocket(connectSocket);
			WSACleanup();
		}

		int ReciveChar(char buf[])
		{
			int iResult = recvfrom(connectSocket, buf, BUFFERLENGTH, 0, (sockaddr*)&remoteAddr, &remoteAddrLen);
			return iResult;
		}

		void SendChar(char* buf, int length)
		{
			sendto(connectSocket, buf, length, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		}
	};
	class Client
	{
	private:

		int PORT = 1688;
		u_long iMode = 0;


		SOCKET serverSocket;

		SOCKADDR_IN sockAddr;
		SOCKADDR_IN clientAddr;
		int clientSize;

		char buffer[1024];
		int bufferLength = 1024;

		void SetUp()
		{
			WSADATA winsock_data;
			WSAStartup(0x202, &winsock_data);

			sockAddr.sin_port = htons(PORT);
			sockAddr.sin_family = AF_INET;
			sockAddr.sin_addr.s_addr = INADDR_ANY;

			serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

			::bind(serverSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));

			int val = 64 * 1024;
			setsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&val, sizeof(val));

			listen(serverSocket, 1000);

			clientSize = sizeof(clientAddr);

			ioctlsocket(serverSocket, FIONBIO, &iMode);
		}

	public:

		Client(int clientPort)
		{
			PORT = clientPort;
			SetUp();
		}
		~Client()
		{
			closesocket(serverSocket);
			WSACleanup();
		}

		int ReciveChar(char buf[])
		{
			int bytes_received = recvfrom(serverSocket, buf, bufferLength, 0, (SOCKADDR*)&clientAddr, &clientSize);
			return bytes_received;
		}

		void SendChar(char* buf, int length)
		{
			sendto(serverSocket, buf, length, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
		}
	};

	class BackAsyncTask : public FNonAbandonableTask
	{
		Client* client;
		Server* server;
		SocketRep* socketRep;

	public:
		BackAsyncTask(Client* client, Server* server, SocketRep* socketRep)
		{
			this->client = client;
			this->server = server;
			this->socketRep = socketRep;
		}
		~BackAsyncTask()
		{
			//GLog->Log("destr1");
		}

		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(RecivingAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
		}

		void DoWork()
		{
			char bufferBack[1024];
			GLog->Log("Back");
			while (socketRep->threadsRun)
			{
				int recivedLength = server->ReciveChar(bufferBack);

				if (recivedLength > 0)
				{
					client->SendChar(bufferBack, recivedLength);
					//GLog->Log("recived1");
				}
				//Sleep(1);
			}
			//GLog->Log("NeRun1");
		}
	};
	class ThereAsyncTask : public FNonAbandonableTask
	{
		Client* client;
		Server* server;
		SocketRep* socketRep;

	public:
		ThereAsyncTask(Client* client, Server* server, SocketRep* socketRep)
		{
			//GLog->Log("Constructor");
			this->client = client;
			this->server = server;
			this->socketRep = socketRep;
		}
		~ThereAsyncTask()
		{
			//GLog->Log("destr2");
		}

		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(RecivingAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
		}

		void DoWork()
		{
			GLog->Log("There");

			char therebuffer[1024];
			while (socketRep->threadsRun)
			{
				int recivedLength = client->ReciveChar(therebuffer);

				if (recivedLength > 0)
				{
					server->SendChar(therebuffer, recivedLength);
					//GLog->Log("recived2");
				}
				//GLog->Log("run2");
				//Sleep(1);
			}
			//GLog->Log("NeRun2");

		}
	};

	FAutoDeleteAsyncTask<ThereAsyncTask>* task1;
	FAutoDeleteAsyncTask<BackAsyncTask>* task2;

	bool threadsRun = true;

	Client* client;
	Server* server;

public:
	SocketRep(int clientPort, int serverPort, string serverIp)
	{
		client = new Client(clientPort);
		server = new Server(serverPort, serverIp);

		GLog->Log("SocketRep");

		task1 = new FAutoDeleteAsyncTask<ThereAsyncTask>(client, server, this);
		task1->StartBackgroundTask();

		task2 = new FAutoDeleteAsyncTask<BackAsyncTask>(client, server, this);
		task2->StartBackgroundTask();
	}
	~SocketRep() 
	{
		threadsRun = false;
		
		delete client;
		delete server;

		Sleep(1000);
		GLog->Log("SocketRepUmer");
	}

};
