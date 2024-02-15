// MyWalkClient.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <iostream.h>

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cerr << "Usage: MyWalkClient hostname portnumber" << endl;
		return 1;
	}

	unsigned short port = atoi(argv[2]);

	WSADATA wsaData;
	WSAStartup(MAKEWORD(1,1), &wsaData);

	struct sockaddr_in _service;
	SOCKET _socket;

	_service.sin_family = AF_INET;
	_service.sin_port = htons(port);

	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int on = 1;
	setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

	struct hostent* entry = ::gethostbyname(argv[1]);
	memcpy(&_service.sin_addr, entry->h_addr, entry->h_length);

	connect(_socket, (struct sockaddr*)&_service, sizeof(_service));

	closesocket(_socket);

	WSACleanup();

	return 0;
}

