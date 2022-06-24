
// GomokuDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Gomoku.h"
#include "GomokuDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGomokuDlg dialog



CGomokuDlg::CGomokuDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GOMOKU_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	role = r_NOT_CONNECTED;
	tSignal = true;
	h_host_listen_thread = NULL;

	h_client_connect_thread = NULL;
	s_host_listen_socket = INVALID_SOCKET;
	s_client_connect_socket = INVALID_SOCKET;
}

void CGomokuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MSG, m_msg);
	DDX_Control(pDX, IDC_HOST_CREATE, m_host_create);
	DDX_Control(pDX, IDC_HOST_STOP, m_host_stop);
	DDX_Control(pDX, IDC_CLIENT_JOIN, m_client_join);
	DDX_Control(pDX, IDC_CLIENT_LEAVE, m_client_leave);
	DDX_Control(pDX, IDC_STATIC1, m_MyStaticControl);
	DDX_Control(pDX, IDC_CHESSBOARD, m_chessBoard);
}

BEGIN_MESSAGE_MAP(CGomokuDlg, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS1, &CGomokuDlg::OnIpnFieldchangedIpaddress1)
	ON_BN_CLICKED(IDC_HOST_CREATE, &CGomokuDlg::OnBnClickedHostCreate)
	ON_BN_CLICKED(IDC_HOST_STOP, &CGomokuDlg::OnBnClickedHostStop)
	ON_BN_CLICKED(IDC_CLIENT_JOIN, &CGomokuDlg::OnBnClickedClientJoin)
	ON_BN_CLICKED(IDC_CLIENT_LEAVE, &CGomokuDlg::OnBnClickedClientLeave)
	ON_BN_CLICKED(IDC_BUTTON1, &CGomokuDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CGomokuDlg message handlers

BOOL CGomokuDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_client_leave.EnableWindow(false);
	m_host_stop.EnableWindow(false);

	m_MyStaticControl.SetWindowText(_T("Turn: Black"));
	isBegin = false;
	SetTimer(TIMER_INIT, TIMER_DEFAULT_ELAPSE, NULL);
	std::string err;
	std::string IP;
	if (getLocalIP(IP, err)) {
		//CString str = CString(IP.c_str());
		//AfxMessageBox(_T("your ip: ") + str);
		print(_T("Your LAN IP: ") + CString(IP.c_str()));
	}
	else {
		//AfxMessageBox(CString(err.c_str()));
		print(_T("Failed to get LAN IP!"));
		print(CString(err.c_str()));
	}

	if (IP == "127.0.0.1") {
		// possibly not connected
		print(_T("[You're likely not connected to LAN!]"));
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGomokuDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
		
	}
	else
	{
		CDialogEx::OnPaint();
	}

	if (isBegin) {
		// repaint only if game starts
		CDC *pDC;
		CRect m_rect;
		pDC = m_chessBoard.GetDC();
		m_chessBoard.GetClientRect(&m_rect);
		gomoku.p_DrawTable(pDC, &m_rect);
		ReleaseDC(pDC);
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGomokuDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGomokuDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (role == r_SERVER && rival.Socket == INVALID_SOCKET) {
		print(_T("[sys]: No player has entered yet"));
		return;
	}
	CRect m_rect;
	CPoint curr_pt;
	GetDlgItem(IDC_CHESSBOARD)->GetWindowRect(&m_rect);
	GetCursorPos(&curr_pt);
	// only check if cursor is in the board
	if (m_rect.PtInRect(curr_pt))
	{
		if (isBegin) {
			// window pos -> board position
			curr_pt.x -= m_rect.left;
			curr_pt.y -= m_rect.top;
			CDC *pDC = m_chessBoard.GetDC();

			// server sync mov
			CString strMsg;
			CString mov_x;
			CString mov_y;
			mov_x.Format(_T("%d"), gomoku.p_ScreenToBoard_x(curr_pt.x));
			mov_y.Format(_T("%d"), gomoku.p_ScreenToBoard_y(curr_pt.y));
			CString ins = _T("mov ") + mov_x + _T(",") + mov_y;
			std::string errMsg;

			// turn check
			if (role == r_SERVER) {
				// as host
				if (gomoku.whoseTurn() != BLACK) {
					// server = black
					print(_T("[sys]: Not your turn!"));
					goto error_end;
				}
			}
			else if (role == r_CLIENT) {
				// as client
				if (gomoku.whoseTurn() != WHITE) {
					// client = white
					print(_T("[sys]: Not your turn!"));
					goto error_end;
				}
			}

			
			if (!gomoku.move(pDC, gomoku.whoseTurn(), curr_pt, errMsg)) {
				CString CerrMsg(errMsg.c_str());
				// 
				print(_T("[sys]: ") + CerrMsg);
				goto error_end;
			}

			if (role == r_SERVER) {
				updateTurn();
				strMsg = _T("[BLACK]: ") + ins;
				print(strMsg);
				SendInfo(strMsg);
			}
			else {
				updateTurn();
				CString strTmp = _T("[WHITE]: ") + ins;
				print(strTmp);
				send(s_client_connect_socket, (char*)strTmp.GetBuffer(), strTmp.GetLength() * sizeof(TCHAR), 0);
				strTmp.ReleaseBuffer();
			}

			if (gomoku.gameOver()) {
				AfxMessageBox(_T("game over!"));
				if (gomoku.result == ST_BLACKWIN) {
					print(_T("[sys]: BLACK wins!"));
				}
				else if (gomoku.result == ST_WHITEWIN) {
					print(_T("[sys]: WHITE wins!"));
				}
				else {
					print(_T("[sys]: TIE!"));
				}
				isBegin = false;
				KillTimer(TIMER_REPAINT);
			}
error_end:
			ReleaseDC(pDC);
		}
		else {
			print(_T("[sys]: Not started yet!"));
		}
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

void CGomokuDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// select box move with mouse move
	CPoint curr_pt;
	CRect m_rect;
	GetDlgItem(IDC_CHESSBOARD)->GetWindowRect(&m_rect);
	GetCursorPos(&curr_pt);
	if (m_rect.PtInRect(curr_pt))
	{
		curr_pt.x -= m_rect.left;
		curr_pt.y -= m_rect.top;
		CDC *pDC = m_chessBoard.GetDC();
		if (isBegin)
		{
			gomoku.p_DrawSelectRect(pDC, curr_pt);
		}
		ReleaseDC(pDC);
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CGomokuDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDC *pDC = m_chessBoard.GetDC();
	CRect m_rect;
	GetDlgItem(IDC_CHESSBOARD)->GetWindowRect(&m_rect);
	switch (nIDEvent)
	{
	case TIMER_INIT:
		m_chessBoard.GetClientRect(&m_rect);
		gomoku.p_DrawTable(pDC, &m_rect);
		ReleaseDC(pDC);
		KillTimer(TIMER_INIT);
		break;
	case TIMER_REPAINT:
		gomoku.p_Repaint(pDC);
		break;
	}
	ReleaseDC(pDC);
	CDialogEx::OnTimer(nIDEvent);
}

void CGomokuDlg::OnIpnFieldchangedIpaddress1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CGomokuDlg::SendInfo(CString strInfo, Client *pNotSend)
{
	/*
		INS: 
		mov 01,01		e.g mov 11,12
		msg content		e.g msg game start!
	*/
	//CString finalStr = Instruction + _T(" ") + Content;
	TCHAR info[BUF_SIZE] = { 0 };
	_tcscpy_s(info, BUF_SIZE, strInfo);
	
	if (!pNotSend || pNotSend->Socket != rival.Socket || pNotSend->hThread != rival.hThread ||
		pNotSend->Ip != rival.Ip) {
		send(rival.Socket, (char*)info, _tcslen(info) * sizeof(TCHAR), 0);
	}
}
void CGomokuDlg::print(CString msg)
{
	m_msg.SetSel(m_msg.GetWindowTextLengthW(), m_msg.GetWindowTextLengthW());
	m_msg.ReplaceSel(msg + _T("\r\n"));
}

void CGomokuDlg::updateTurn() {
	if (gomoku.whoseTurn() == WHITE) {
		// (prev) black->white (curr)
		m_MyStaticControl.SetWindowText(_T("Turn: White"));
	}
	else {
		m_MyStaticControl.SetWindowText(_T("Turn: Black"));
	}
}

void CGomokuDlg::OnBnClickedHostCreate()
{
	// TODO: Add your control notification handler code here
	
	h_host_listen_thread = CreateThread(NULL, 0, Listener, this, 0, NULL);
}


void CGomokuDlg::OnBnClickedHostStop()
{
	// TODO: Add your control notification handler code here
	bool hasClient = (rival.Socket != INVALID_SOCKET);
	HANDLE *pHandles = new HANDLE[1 + hasClient];
	pHandles[0] = h_host_listen_thread;
	if (hasClient) {
		pHandles[1] = rival.hThread;
	}
	tSignal = false;
	DWORD dwRet = WaitForMultipleObjects(1 + hasClient, pHandles, TRUE, 1000);
	if (dwRet != WAIT_OBJECT_0) {
		if (hasClient) {
			TerminateThread(rival.hThread, -1);
			closesocket(rival.Socket);
		}
		TerminateThread(h_host_listen_thread, -1);
		closesocket(s_host_listen_socket);
	}
	delete[] pHandles;
	h_host_listen_thread = NULL;
	s_host_listen_socket = INVALID_SOCKET;
	role = r_NOT_CONNECTED;
	tSignal = true;
	print(_T("[sys]: Server has stopped."));
	m_host_create.EnableWindow(true);
	m_host_stop.EnableWindow(false);
	m_client_join.EnableWindow(true);
	m_client_leave.EnableWindow(false);
	// test_turn_black = true;
}


void CGomokuDlg::OnBnClickedClientJoin()
{
	// TODO: Add your control notification handler code here
	h_client_connect_thread = CreateThread(NULL, 0, Connect, this, 0, NULL);
}


void CGomokuDlg::OnBnClickedClientLeave()
{
	// TODO: Add your control notification handler code here
	tSignal = false;
	DWORD dwRet = WaitForSingleObject(h_client_connect_thread, 1000);
	if (dwRet != WAIT_OBJECT_0) {
		TerminateThread(h_client_connect_thread, -1);
		closesocket(s_client_connect_socket);
	}
	h_client_connect_thread = NULL;
	s_client_connect_socket = INVALID_SOCKET;
	role = r_NOT_CONNECTED;
	tSignal = true;

	print(_T("[sys]: You have left the game."));
	m_host_create.EnableWindow(true);
	m_host_stop.EnableWindow(false);
	m_client_join.EnableWindow(true);
	m_client_leave.EnableWindow(false);
	// test_turn_black = true;
}

void CGomokuDlg::GameStart() {
	isBegin = true;
	// if window out of range, board will not disappear
	OnPaint();
	gomoku.restart();
	SetTimer(TIMER_REPAINT, TIMER_DEFAULT_ELAPSE, NULL);

	//OutputDebugString(_T("restarting... count: "));
	//CString t;
	//t.Format(_T("%d \r\n"), gomoku.getCount());
	//OutputDebugString(t);

	print(_T("[sys]: Black goes first"));
}

void CGomokuDlg::receivedMove(int x, int y) {
	CDC *pDC = m_chessBoard.GetDC();
	std::string errMsg;
	if (!gomoku.move(pDC, gomoku.whoseTurn(), x, y, errMsg)) {
		CString cs(errMsg.c_str());
		AfxMessageBox(cs);
	}

	if (gomoku.gameOver()) {
		AfxMessageBox(_T("game over!"));
		isBegin = false;
		KillTimer(TIMER_REPAINT);
	}
	ReleaseDC(pDC);
}

void CGomokuDlg::OnBnClickedButton1()
// deprecated
// used to test send instruction
{
	/*
	CString strMsg;
	GetDlgItemText(IDC_EDIT1, strMsg);
	if (role == r_SERVER) {
		// as host
		if (rival.Socket == INVALID_SOCKET) {
			print(_T("[sys]: No player has entered yet"));
			return;
		}
		if (!test_turn_black) {
			// server = black
			print(_T("[sys]: Not your turn!"));
			return;
		}
		updateTurn();
		strMsg = _T("[BLACK]: ") + strMsg;
		print(strMsg);
		SendInfo(strMsg);
		
	}
	else if (role == r_CLIENT) {
		// as client
		if (test_turn_black) {
			// client = white
			print(_T("[sys]: Not your turn!"));
			return;
		}
		updateTurn();
		CString strTmp = _T("[WHITE]: ") + strMsg;
		print(strTmp);
		send(s_client_connect_socket, (char*)strTmp.GetBuffer(), strTmp.GetLength() * sizeof(TCHAR), 0);
		strTmp.ReleaseBuffer();
	}
	else {
		OutputDebugString(_T("Not started yet \r\n"));
	}
	*/
	// TODO: Add your control notification handler code here
}
