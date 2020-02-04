#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdexcept>
#include <string>
#include <fstream>

const size_t DEFAULT_BUFLEN = 512;
const PCSTR printer_port = "9100";

int PrintZPL(std::string IP, std::string ZPL_Code)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	int iResult;

	const char *sendbuf = ZPL_Code.c_str();

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult)
	{

		printf("WSAStartup failed with error: %d\n", iResult);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(IP.c_str(), printer_port, &hints, &result);
	if(iResult != 0 )
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
	}

	// Attempt to connect to an address until one succeeds
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if(ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
	}

	// Send an initial buffer
	iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
	if(iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	
	}

	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}

int quit_err_msg(const std::string msg, bool print_usage=false)
{
	if(print_usage)
	{
		std::cerr << "Usage: [IP] [ZPL File]\n";
	}

	std::cerr << msg << '\n';
	return 1;
}

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		quit_err_msg("Not enough arguments.", true);
	}

	std::string ZPL_Code;
	std::string line;
	std::ifstream label_file(argv[2]);
	
	if(label_file.is_open())
	{
		while(std::getline(label_file, line))
		{
			ZPL_Code += line;
		}
		label_file.close();
	}

	else
	{
		exit(1);
	}

	return PrintZPL(argv[1], ZPL_Code.c_str());
}