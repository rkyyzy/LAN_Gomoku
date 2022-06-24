
// GomokuDlg.h : header file
//

#pragma once
#include "resource.h"
#include "GomokuGameMain.h"
#include "winsockUT.h"

#define TIMER_INIT 1
#define TIMER_REPAINT 2
#define TIMER_DEFAULT_ELAPSE 40

constexpr int r_NOT_CONNECTED = 0;
constexpr int r_SERVER = 1;;
constexpr int r_CLIENT = 2;

// CGomokuDlg dialog
class CGomokuDlg : public CDialogEx
{
// Construction
public:
	CGomokuDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GOMOKU_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	int role;
	bool tSignal;
	bool isBegin;

	CStatic m_MyStaticControl;
	CStatic m_chessBoard;
	CButton m_host_create;
	CButton m_host_stop;
	CButton m_client_join;
	CButton m_client_leave;
	CEdit m_msg;

	Client rival;

	HANDLE h_host_listen_thread;
	HANDLE h_client_connect_thread;

	SOCKET s_host_listen_socket;
	SOCKET s_client_connect_socket;

	Gomoku gomoku;						// game class

	void print(CString msg);
	void updateTurn();
	void receivedMove(int x, int y);
	void SendInfo(CString strInfo, Client *pNotSend = NULL);
	void GameStart();

	afx_msg void OnIpnFieldchangedIpaddress1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedHostCreate();
	afx_msg void OnBnClickedHostStop();
	afx_msg void OnBnClickedClientJoin();
	afx_msg void OnBnClickedClientLeave();
	afx_msg void OnBnClickedButton1();

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	// afx_msg void OnBnClickedButtonStart();
};