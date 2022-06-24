#include "pch.h"
#include "Gomoku.h"
#include "GomokuDlg.h"
#include "winsockUT.h"

bool select_socket(SOCKET s, bool isRead) {
	fd_set set;
	FD_ZERO(&set);
	FD_SET(s, &set);
	timeval t;
	t.tv_sec = 0;
	t.tv_usec = DEFAULT_TIMEOUT_MS;
	int ret;
	if (isRead) {
		// read
		ret = select(0, &set, NULL, NULL, &t);
	}
	else {
		// write
		ret = select(0, NULL, &set, NULL, &t);
	}

	return (ret > 0 && FD_ISSET(s, &set));
}

DWORD WINAPI Listener(LPVOID param)
{
	// pointer to main dialog
	CGomokuDlg *pMainDlg = (CGomokuDlg *)param;
	ASSERT(pMainDlg != NULL);
	pMainDlg->s_host_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (pMainDlg->s_host_listen_socket == INVALID_SOCKET) {
		// failed to create socket
		pMainDlg->print(_T("Error: Failed to create socket."));
		return FALSE;
	}

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(DEFAULT_PORT);

	if (bind(pMainDlg->s_host_listen_socket, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		// failed to bind the port
		pMainDlg->print(_T("Error: Failed to bind the port."));
		closesocket(pMainDlg->s_host_listen_socket);
		return TRUE;
	}

	if (listen(pMainDlg->s_host_listen_socket, 2) == SOCKET_ERROR) {
		pMainDlg->print(_T("Error: Failed to listen to incoming connection."));
		closesocket(pMainDlg->s_host_listen_socket);
		return TRUE;
	}

	pMainDlg->print(_T("[sys]: Room created!"));

	// Assign server variable & Adjust MFC
	pMainDlg->role = r_SERVER;

	// *** button enable here *** 
	pMainDlg->m_host_create.EnableWindow(false);
	pMainDlg->m_host_stop.EnableWindow(true);
	pMainDlg->m_client_join.EnableWindow(false);
	pMainDlg->m_client_leave.EnableWindow(false);

	while (pMainDlg->tSignal) {
		if (select_socket(pMainDlg->s_host_listen_socket, true)) {
			// when client enters, goes here
			if (pMainDlg->rival.hThread != NULL) {
				// accept only 1 player
				continue;
			}
			sockaddr_in client_addr;
			int addrlen = sizeof(sockaddr_in);
			SOCKET newClient = accept(pMainDlg->s_host_listen_socket, (struct sockaddr *)&client_addr, &addrlen);
			if (newClient == INVALID_SOCKET)
				continue;
			pMainDlg->rival.Socket = newClient;
			pMainDlg->rival.pMainDlg = pMainDlg;
			pMainDlg->rival.Ip = inet_ntoa(client_addr.sin_addr);

			pMainDlg->rival.hThread = CreateThread(NULL, 0, ClientThread, pMainDlg, CREATE_SUSPENDED, NULL);
			ResumeThread(pMainDlg->rival.hThread);

			// only server receives
			pMainDlg->print(_T("Player: ") + pMainDlg->rival.Ip + _T(" has entered."));
			pMainDlg->print(_T("Game start!"));
			pMainDlg->GameStart();
			Sleep(100);
		}
	}
	closesocket(pMainDlg->s_host_listen_socket);
	return TRUE;
}

DWORD WINAPI ClientThread(LPVOID param)
{
	CGomokuDlg *pMainDlg = (CGomokuDlg *)param;
	Client host = pMainDlg->rival;
	OutputDebugString(_T("Client thread here \r\n"));
	while (host.pMainDlg->tSignal) {
		if (select_socket(host.Socket, true)) {
			OutputDebugString(_T("Client thread inner loop: if \r\n"));
			TCHAR info[BUF_SIZE] = { 0 };
			CString fullInfo;
			CString instruction;
			CString content;
			CString printMsg;
			int ret = recv(host.Socket, (char*)info, BUF_SIZE, 0);
			if (ret > 0) {
				OutputDebugString(_T(" ClientThread ret > 0  \r\n"));
				// server received info;
				fullInfo = info;
				int insPos = fullInfo.Find(_T(' '));
				instruction = fullInfo.Mid(insPos + 1, 3); //	3 letter instruction
				content = fullInfo.Mid(fullInfo.Find(_T(' '), insPos + 1) + 1);
				OutputDebugString(_T(" ClientThread ins:"));
				OutputDebugString(instruction);
				OutputDebugString(_T(" ClientThread content:"));
				OutputDebugString(content);
				// *************** INSTRUCTION PROCESS HERE ****************
				if (instruction == _T("mov")) {
					// e.g mov 2,3
					int posX = -1, posY = -1;
					int nTokenPos = 0;
					CString strToken = content.Tokenize(_T(","), nTokenPos);
					host.pMainDlg->print(_T("host received mov!"));
					while (!strToken.IsEmpty())
					{
						if (posX != -1) {
							posY = _tstoi(strToken);
							//host.pMainDlg->print(_T("catched position y:") + strToken);
						}
						else {
							posX = _tstoi(strToken);
							//host.pMainDlg->print(_T("catched position x:") + strToken);
						}
						
						strToken = content.Tokenize(_T(","), nTokenPos);
					}
					// server simulates client sent mov
					host.pMainDlg->receivedMove(posX, posY);

				}
				else if (instruction == _T("msg")) {
					// msg

				}
				else {
					// unidentified ins
				}
				pMainDlg->updateTurn();
				host.pMainDlg->print(info);
				host.pMainDlg->SendInfo(info, &host);
				//if (client.pMainDlg->test_turn_black) {
				//	client.pMainDlg->m_MyStaticControl.SetWindowText(_T("Turn: Black"));
				//}
				//else {
				//	client.pMainDlg->m_MyStaticControl.SetWindowText(_T("Turn: White"));
				//}
			}
			else {
				// server received client left
				// close socket;
				OutputDebugString(_T(" ClientThread: client left  \r\n"));
				printMsg = _T("[WHITE] has left. Game over.");
				// ***************** Game over ****************** 
				host.pMainDlg->print(printMsg);
				host.pMainDlg->rival = Client();
				break;
			}
		}
		Sleep(200);
	}
	return TRUE;
}

DWORD WINAPI Connect(LPVOID param)
{
	CGomokuDlg *pMainDlg = (CGomokuDlg*)param;
	ASSERT(pMainDlg != NULL);
	pMainDlg->s_client_connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pMainDlg->s_client_connect_socket == INVALID_SOCKET) {
		// failed to create socket
		pMainDlg->print(_T("Error: Failed to create socket."));
		return FALSE;
	}

	CString serverIP;
	char cServerIP[16];
	pMainDlg->GetDlgItemText(IDC_IPADDRESS1, serverIP);
	USES_CONVERSION;
	strcpy_s(cServerIP, 16, T2A(serverIP));

	sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(cServerIP);
	server.sin_port = htons(DEFAULT_PORT);
	server.sin_family = AF_INET;

	if (connect(pMainDlg->s_client_connect_socket, (struct sockaddr*) &server, sizeof(struct sockaddr)) == SOCKET_ERROR) {
		// connection failed
		pMainDlg->print(_T("Error: Failed to connect."));
		closesocket(pMainDlg->s_client_connect_socket);
		return TRUE;
	}
	pMainDlg->role = r_CLIENT;
	pMainDlg->print(_T("Connected to the host!"));
	pMainDlg->print(_T("Game start!"));
	pMainDlg->GameStart();

	// *** button enable here *** 
	pMainDlg->m_host_create.EnableWindow(false);
	pMainDlg->m_host_stop.EnableWindow(false);
	pMainDlg->m_client_join.EnableWindow(false);
	pMainDlg->m_client_leave.EnableWindow(true);

	while (pMainDlg->tSignal) {
		if (select_socket(pMainDlg->s_client_connect_socket, false)) {
			TCHAR info[BUF_SIZE] = { 0 };
			int ret = recv(pMainDlg->s_client_connect_socket, (char*)info, BUF_SIZE, 0);
			CString fullInfo;
			CString instruction;
			CString content;
			CString printMsg;
			if (ret > 0) {
				OutputDebugString(_T(" Connect ret > 0  \r\n"));
				// client received info from server
				fullInfo = info;
				int insPos = fullInfo.Find(_T(' '));
				instruction = fullInfo.Mid(insPos + 1, 3); //	3 letter instruction
				content = fullInfo.Mid(fullInfo.Find(_T(' '), insPos + 1) + 1);
				// *************** INSTRUCTION PROCESS HERE ****************
				if (instruction == _T("mov")) {
					// e.g mov 2,3
					int posX = -1, posY = -1;
					int nTokenPos = 0;
					CString strToken = content.Tokenize(_T(","), nTokenPos);
					// pMainDlg->print(_T("client received mov!"));
					while (!strToken.IsEmpty())
					{
						if (posX != -1) {
							posY = _tstoi(strToken);
							// pMainDlg->print(_T("catched position y:") + strToken);
						}
						else {
							posX = _tstoi(strToken);
							// pMainDlg->print(_T("catched position x:") + strToken);
						}

						strToken = content.Tokenize(_T(","), nTokenPos);
					}
					// client simulates server sent mov
					pMainDlg->receivedMove(posX, posY);

				}
				else if (instruction == _T("msg")) {
					// msg

				}
				else {
					// unidentified ins
				}
				//if (pMainDlg->test_turn_black) {
				//	pMainDlg->m_MyStaticControl.SetWindowText(_T("Turn: Black"));
				//}
				//else {
				//	pMainDlg->m_MyStaticControl.SetWindowText(_T("Turn: White"));
				//}
				pMainDlg->updateTurn();
				pMainDlg->print(info);
			}
			else {
				// server ended
				pMainDlg->print(_T("[BLACK] has ended the game. Game over."));
				Sleep(200);
				// *** button enable here *** 
				pMainDlg->OnBnClickedClientLeave();
				//pMainDlg->role = r_NOT_CONNECTED;
				//pMainDlg->m_host_create.EnableWindow(true);
				//pMainDlg->m_host_stop.EnableWindow(false);
				//pMainDlg->m_client_join.EnableWindow(true);
				//pMainDlg->m_client_leave.EnableWindow(false);
				break;
			}
		}
		Sleep(200);
	}

	closesocket(pMainDlg->s_client_connect_socket);
	return TRUE;
}