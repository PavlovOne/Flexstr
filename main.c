
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#include <signal.h>
#include <winsock2.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "config.h"
#include "utils.h"
#include "json/json.h"
#include "encode/encode.h"

#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef WIN32
int start_job(SOCKET sock)
#else
int start_job(int sock)
#endif
{
	int nError;
//	int readyFlag = 0;
	char *netBuff, *settingsBuff, *encodesBuff;
	int len;
	size_t settingsCount, encodesCount;

	netBuff = malloc(NET_BUFSIZE);
	if (netBuff == NULL)
	{
		printf("Can not alloc network buffer\r\n");
		return -1;
	}

	settingsBuff = malloc(SETTINGS_BUFSIZE);
	if (settingsBuff == NULL)
	{
		printf("Can not alloc settings buffer\r\n");
		return -1;
	}

	encodesBuff = malloc(ENCODES_BUFSIZE);
	if (encodesBuff == NULL)
	{
		printf("Can not alloc encodes buffer\r\n");
		return -1;
	}

	printf("Started the encode job\r\n");
	
	// job main loop
	for(;;)	{
		len = recv(sock, netBuff, NET_BUFSIZE, 0);
		if (len > 0) {
			netBuff[len] = 0;
			printf("%s\r\n", netBuff);

			json_parse(netBuff, settingsBuff, &settingsCount, encodesBuff, &encodesCount);

			strcpy(netBuff, "{\"status\":\"success\"}");
			send(sock, netBuff, strlen(netBuff), 0);
		}

		if (settingsCount > 0) {
			// TODO : init settings

			settingsCount = 0;
		}

		if (encodesCount > 0) {

		}

#ifdef WIN32
		nError=WSAGetLastError();
		if(nError!=WSAEWOULDBLOCK&&len!=0)
#else
		nError = errno;
		if(nError!=EWOULDBLOCK&&len!=0)
#endif
		{
			printf("Client disconnected!\r\n");

			// Close our socket entirely
#ifdef WIN32
			closesocket(sock);
#else
			close(sock);
#endif

			printf("\r\n---------------------------------\r\n");

			break;
		}

		sleepcp(1000);
	}

	free(netBuff);
	free(settingsBuff);
	free(encodesBuff);

	return 0;
}

#ifdef WIN32
int main()
{
	WSADATA wsaDat;
	SOCKET mainSock, tempSock;
	SOCKADDR_IN serverInf;
	u_long iMode;
	
	if(WSAStartup(MAKEWORD(2,2),&wsaDat)!=0)
	{
		printf("WSA Initialization failed!\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	mainSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(mainSock==INVALID_SOCKET)
	{
		printf("Socket creation failed.\r\n");
		WSACleanup();
		return 0;
	}

	serverInf.sin_family=AF_INET;
	serverInf.sin_addr.s_addr=INADDR_ANY;
	serverInf.sin_port=htons(NET_PORT);

	if(bind(mainSock,(SOCKADDR*)(&serverInf),sizeof(serverInf))==SOCKET_ERROR)
	{
		printf("Unable to bind socket!\r\n");
		WSACleanup();
		return 0;
	}

	listen(mainSock,1);

WAIT_CLIENT:
	tempSock=SOCKET_ERROR;
	while(tempSock==SOCKET_ERROR)
	{
		printf("Waiting for incoming connections...\r\n");
		tempSock=accept(mainSock,NULL,NULL);
	}
	
	// If iMode!=0, non-blocking mode is enabled.
	iMode=1;
	ioctlsocket(tempSock, FIONBIO, &iMode);

	printf("Client connected!\r\n\r\n");

	start_job(tempSock);

	goto WAIT_CLIENT;

	// Close main Socket
	closesocket(mainSock);
	WSACleanup();
	return 0;
}
#else
int main()
{
	int 			mainSock, tempSock;
	struct 	sockaddr_in 	my_addr;
	struct 	sockaddr_in 	their_addr;
	socklen_t		sin_size;
	
	if ((mainSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(NET_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), 0, 8);
	
	if (bind(mainSock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return -1;
	}
	
	if (listen(mainSock, 1) == -1) {
		perror("listen");
		return -1;
	}

WAIT_CLIENT:
	tempSock = -1;
	while(tempSock == -1)
	{
		printf("Waiting for incoming connections...\r\n");
		sin_size = sizeof(struct sockaddr_in);
		if ((tempSock = accept(mainSock, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("accept");
			continue;
		}
	}

	fcntl(tempSock, F_SETFL, O_NONBLOCK);
	printf("Client connected!\r\n\r\n");

	start_job(tempSock);

	goto WAIT_CLIENT;

	close(mainSock);

	return 0;
}
#endif

