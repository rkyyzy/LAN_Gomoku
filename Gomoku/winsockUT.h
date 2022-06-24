#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 1024
#define WM_TRAYICON_MSG (WM_USER+100)
#define DEFAULT_PORT 8888
#define DEFAULT_TIMEOUT_MS 100
class CGomokuDlg;
class Client {
public:
	CString Ip;
	SOCKET Socket;
	HANDLE hThread;
	CGomokuDlg *pMainDlg;
	Client() {
		pMainDlg = NULL;
		Socket = INVALID_SOCKET;
		hThread = NULL;
	}
};

DWORD WINAPI Listener(LPVOID param);
DWORD WINAPI ClientThread(LPVOID param);
DWORD WINAPI Connect(LPVOID param);
bool select_socket(SOCKET s, bool isRead);

extern const std::string WSASTARTUP_FAIL;
extern const std::string WSASTARTUP_FAIL;
extern const std::string WINSOCK_NOT_FOUND;
extern const std::string HOST_NAME_NOT_FOUND;
extern const std::string HOST_IP_NOT_FOUND;

bool getLocalIP(std::string& IP, std::string& errMsg);