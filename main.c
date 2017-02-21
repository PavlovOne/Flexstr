
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
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
#include  <signal.h>
#endif

#include "config.h"
#include "utils.h"
#include "encode/encode.h"
#include "json/json.h"

#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef WIN32
#define WOULDBLCOK WSAEWOULDBLOCK
#else
#define WOULDBLCOK EWOULDBLOCK
#endif

ENCODE_SETTINGS gEncodeSettings;
char *gNetBuff, *gEncodesBuff;
size_t gEncodesCount;
size_t gEncodesPtr;

int init_buff(void)
{
	gEncodesCount = 0;
	gEncodesPtr = 0;

	gNetBuff = malloc(NET_BUFSIZE);
	if (gNetBuff == NULL)
		return -1;

	gEncodesBuff = malloc(ENCODES_BUFSIZE);
	if (gEncodesBuff == NULL)
		return -1;

	return 0;
}

void release_buff(void)
{
	if (gNetBuff) free(gNetBuff);
	if (gEncodesBuff) free(gEncodesBuff);
}

#ifndef WIN32
void     INThandler(int);
void  INThandler(int sig)
{
	char  c;

	signal(sig, SIG_IGN);
	printf("Do you really want to quit? [y/n] ");
	c = getchar();
	if (c == 'y' || c == 'Y') {
		release_buff();
		exit(0);
	}
	else
		signal(SIGINT, INThandler);
	getchar(); // Get new line character
}
#endif

int get_network_error(void)
{
#ifdef WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

#ifdef WIN32
void close_network_socket(SOCKET sock)
{
	closesocket(sock);
}
#else
void close_network_socket(int sock)
{
	close(sock);
}
#endif

int main()
{
	int netLen, ret;
	struct 	sockaddr_in serverInf;
#ifdef WIN32
	WSADATA wsaDat;
	SOCKET mainSock, newSock, tempSock;
	u_long iMode = 1;
#else
	int mainSock, newSock, tempSock;

	signal(SIGINT, INThandler);
#endif

	// Init Buffers
	if (init_buff() < 0) {
		printf("Memory alloc error!\r\n");
		return -1;
	}

	// init Encoder
	if (init_encode("tmr://localhost") < 0) {
		printf("Init Encode error!\r\n");
		return -1;
	}

#ifdef WIN32
	// Init network
	if(WSAStartup(MAKEWORD(2,2),&wsaDat)!=0)
	{
		printf("WSA Initialization failed!\r\n");
		WSACleanup();
		system("PAUSE");
		return -1;
	}
#endif

	if ((mainSock = socket(AF_INET, SOCK_STREAM, 0)) == SOCK_ERROR) {
	#ifdef WIN32
		WSACleanup();
	#endif
		return -1;
	}
	
	serverInf.sin_family=AF_INET;
	serverInf.sin_addr.s_addr=INADDR_ANY;
	serverInf.sin_port=htons(NET_PORT);
	memset(&(serverInf.sin_zero), 0, 8);

	if(bind(mainSock,(struct sockaddr*)(&serverInf),sizeof(serverInf))==SOCK_ERROR)
	{
		printf("Unable to bind socket!\r\n");
	#ifdef WIN32
		WSACleanup();
	#endif
		return -1;
	}

	listen(mainSock,1);

#ifdef WIN32
	ioctlsocket(mainSock, FIONBIO, &iMode);
#else
	fcntl(mainSock, F_SETFL, O_NONBLOCK);
#endif

	tempSock=SOCK_ERROR;
	newSock=SOCK_ERROR;
WAIT_CLIENT:
	tempSock=accept(mainSock,NULL,NULL);
	if (tempSock != SOCK_ERROR)
	{
		if (newSock != SOCK_ERROR)
		{
			close_network_socket(newSock);
		}

		newSock = tempSock;
		tempSock = SOCK_ERROR;
		
		printf("\r\n-------------Client connected--------------------\r\n");
		netLen = 0;

		// If iMode!=0, non-blocking mode is enabled.
#ifdef WIN32
		ioctlsocket(newSock, FIONBIO, &iMode);
#else
		fcntl(newSock, F_SETFL, O_NONBLOCK);
#endif
	}

	if (newSock != SOCK_ERROR)
	{
		if (NET_BUFSIZE == netLen)
		{
			netLen = 0;
			printf("Client error!\r\n");

			// Close our socket entirely
			close_network_socket(newSock);

			printf("\r\n---------------------------------\r\n");

			newSock = SOCK_ERROR;
		}
		else
		{
			int len;
			len = recv(newSock, &gNetBuff[netLen], NET_BUFSIZE-netLen, 0);
			if (len > 0) {
				netLen += len;
				gNetBuff[netLen] = 0;
				ret = json_parse(gNetBuff, &gEncodeSettings, gEncodesBuff, &gEncodesCount);
				if (ret != JSMN_ERROR_PART) {
					netLen = 0;
					gEncodesPtr = 0;

					if (ret == 0) { // success
						printf("///// Recevied New settings and Encodes\r\n");
						encode_print_status();

						strcpy(gNetBuff, "{\"status\":\"success\"}");
						send(newSock, gNetBuff, strlen(gNetBuff), 0);
					}
					else {
						printf("Error settings & Encodes\n");
						strcpy(gNetBuff, "{\"status\":\"Invalid JSON string\"}");
						send(newSock, gNetBuff, strlen(gNetBuff), 0);
					}
				}
			}
		}
	}

	if (gEncodesPtr < gEncodesCount) {
//		encode_loop();
	}

	goto WAIT_CLIENT;

	release_buff();

	// Close main Socket
	closesocket(mainSock);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}
