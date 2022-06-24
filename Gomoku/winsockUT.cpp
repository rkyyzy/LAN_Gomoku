#include "pch.h"
#include "winsockUT.h"

const std::string WSASTARTUP_FAIL = "Error: WSAStartup failed. ";
const std::string WINSOCK_NOT_FOUND = "Error: Usable version of Winsock.dll not found.";
const std::string HOST_NAME_NOT_FOUND = "Error: Failed to get host name.";
const std::string HOST_IP_NOT_FOUND = "Error: no host found by host name.";

bool getLocalIP(std::string& IP, std::string& errMsg) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		errMsg = WSASTARTUP_FAIL + "Error code: " + std::to_string(err);
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		errMsg = WINSOCK_NOT_FOUND;
		WSACleanup();
		return false;
	}
	// std::cout << "Winsock initialized..." << std::endl;

	char hostName[80];
	if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
		errMsg = HOST_NAME_NOT_FOUND;
		WSACleanup();
		return false;
	}
	struct hostent *pHost = gethostbyname(hostName);
	if (!pHost) {
		errMsg = HOST_IP_NOT_FOUND;
		WSACleanup();
		return false;
	}

	struct in_addr addr;
	memcpy(&addr, pHost->h_addr_list[0], sizeof(struct in_addr));
	IP = std::string(inet_ntoa(addr));

	// std::cout << "Winsock cleaning up..." << std::endl;
	WSACleanup();
	return true;
}
