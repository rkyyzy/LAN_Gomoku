#include "GomokuGameMain.h"

const std::string G_ERR_NOT_YOUR_TURN = "err: not your turn";
const std::string G_ERR_X_Y_OUTTA_RANGE = "err: x,y out of range";
const std::string G_ERR_POS_TAKEN = "err: position already taken";

void Gomoku::updateTurn() {
	if (isOver)
		return;
	turn = (turn == BLACK) ? WHITE : BLACK;
}

bool Gomoku::inRange(int x, int y) {
	return x >= 0 && y >= 0 && x < BOARD_X && y < BOARD_Y;
}

bool Gomoku::horizontalCheck(int x, int y, int target) {
	int cnt = 0;
	for (int x_curr = x, i = 0; i < 5; x_curr--, i++) {
		// left
		if (!inRange(x_curr, y) || chessBoard[x_curr][y] != target)
			break;
		cnt++;
	}
	if (cnt == 5)
		return true;
	for (int x_curr = x + 1, i = 0; i < 5; x_curr++, i++) {
		// right
		if (!inRange(x_curr, y) || chessBoard[x_curr][y] != target)
			break;
		cnt++;
		if (cnt >= 5)
			return true;
	}
	return cnt >= 5;
}

bool Gomoku::verticalCheck(int x, int y, int target) {
	int cnt = 0;
	for (int y_curr = y, i = 0; i < 5; y_curr--, i++) {
		// up
		if (!inRange(x, y_curr) || chessBoard[x][y_curr] != target)
			break;
		cnt++;
	}
	if (cnt == 5)
		return true;
	for (int y_curr = y + 1, i = 0; i < 5; y_curr++, i++) {
		// down
		if (!inRange(x, y_curr) || chessBoard[x][y_curr] != target)
			break;
		cnt++;
		if (cnt >= 5)
			return true;
	}
	return cnt >= 5;
}

bool Gomoku::diagonalCheck(int x, int y, int target) {
	// 2 parts: left-up to right-down, left-down to right-up, separate counter
	int cnt = 0;
	for (int x_curr = x, y_curr = y, i = 0; i < 5; x_curr--, y_curr--, i++) {
		// left-up
		if (!inRange(x_curr, y_curr) || chessBoard[x_curr][y_curr] != target)
			break;
		cnt++;
	}
	if (cnt == 5)
		return true;
	for (int x_curr = x + 1, y_curr = y + 1, i = 0; i < 5; x_curr++, y_curr++, i++) {
		// right-down
		if (!inRange(x_curr, y_curr) || chessBoard[x_curr][y_curr] != target)
			break;
		cnt++;
		if (cnt >= 5)
			return true;
	}

	cnt = 0;

	for (int x_curr = x, y_curr = y, i = 0; i < 5; x_curr++, y_curr--, i++) {
		// left-down
		if (!inRange(x_curr, y_curr) || chessBoard[x_curr][y_curr] != target)
			break;
		cnt++;
	}
	if (cnt == 5)
		return true;
	for (int x_curr = x - 1, y_curr = y + 1, i = 0; i < 5; x_curr--, y_curr++, i++) {
		// right-up
		if (!inRange(x_curr, y_curr) || chessBoard[x_curr][y_curr] != target)
			break;
		cnt++;
		if (cnt >= 5)
			return true;
	}

	return cnt >= 5;
}

bool Gomoku::checkWinner(int x, int y) {
	// if winner found, winner = turn

	if (horizontalCheck(x, y, chessBoard[x][y])) {
		OutputDebugString(_T("checkWinner - horizontal \r\n"));
		return true;
	}
	if (verticalCheck(x, y, chessBoard[x][y])) {
		OutputDebugString(_T("checkWinner - veritical \r\n"));
		return true;
	}
	if (diagonalCheck(x, y, chessBoard[x][y])) {
		OutputDebugString(_T("checkWinner - diagonal \r\n"));
		return true;
	}
	return false;
}
Gomoku::Gomoku() {
	// initialize all values with 0 - empty
	memset(chessBoard, EMPTY, sizeof(chessBoard));

	// black goes first
	turn = BLACK;

	isOver = false;

	result = ST_NOT_FINISHED;

	count = 0;
}

void Gomoku::p_DrawTable(CDC *pDC, CRect m_rect) {
	CRect m_background;
	m_background.top = m_rect.top;
	m_background.bottom = m_rect.bottom;
	m_background.left = m_rect.left;
	m_background.right = m_rect.right;
	pDC->FillSolidRect(&m_background, COLOR_BOARD);
	// subGridSize -> based on shortest side
	p_subGridSize = min(((m_rect.right - m_rect.left) / (BOARD_X + 1)), ((m_rect.bottom - m_rect.top) / (BOARD_Y + 1)));

	// centered and leave space from edge
	p_board_top = (m_rect.bottom - m_rect.top) / 2 - p_subGridSize * (BOARD_X - 1) / 2;
	p_board_bottom = p_board_top + p_subGridSize * (BOARD_X - 1);
	p_board_left = (m_rect.right - m_rect.left) / 2 - p_subGridSize * (BOARD_Y - 1) / 2;
	p_board_right = p_board_left + p_subGridSize * (BOARD_Y - 1);

	for (int x = p_board_left; x <= p_board_right; x += p_subGridSize)
	{
		// up down
		pDC->MoveTo(x, p_board_top);
		pDC->LineTo(x, p_board_bottom);
	}
	for (int y = p_board_top; y <= p_board_bottom; y += p_subGridSize)
	{
		// left right
		pDC->MoveTo(p_board_left, y);
		pDC->LineTo(p_board_right, y);
	}

	// p_vairable initialize here
	p_selectRect.first = SELECT_RECT_INIT, p_selectRect.second = SELECT_RECT_INIT;
}

void Gomoku::p_Repaint(CDC *pDC)
{
	if (count) {
		// only repaint if chessboard has pieces
		int x = -1, y = -1;
		for (int i = 0; i < whiteCount.size(); i++) {
			// drawing white piece
			x = whiteCount[i].first;
			y = whiteCount[i].second;
			CBrush m_brush;
			m_brush.CreateSolidBrush(COLOR_WHITE);
			CBrush *m_oldBrush = pDC->SelectObject(&m_brush);
			pDC->Ellipse(x - 10, y - 10, x + 10, y + 10);
			pDC->SelectObject(m_oldBrush);
			m_brush.DeleteObject();
		}
		for (int i = 0; i < blackCount.size(); i++) {
			// drawing black piece
			x = blackCount[i].first;
			y = blackCount[i].second;
			CBrush m_brush;
			m_brush.CreateSolidBrush(COLOR_BLACK);
			CBrush *m_oldBrush = pDC->SelectObject(&m_brush);
			pDC->Ellipse(x - 10, y - 10, x + 10, y + 10);
			pDC->SelectObject(m_oldBrush);
			m_brush.DeleteObject();
		}
	}
}
void Gomoku::p_DrawSelectRectHelper(CDC *pDC, int x, int y) {
	// rect over box
	pDC->MoveTo(x + 10, y + 1);
	pDC->LineTo(x + 10, y + 10);
	pDC->LineTo(x + 1, y + 10);

	pDC->MoveTo(x + 10, y - 1);
	pDC->LineTo(x + 10, y - 10);
	pDC->LineTo(x + 1, y - 10);

	pDC->MoveTo(x - 10, y + 1);
	pDC->LineTo(x - 10, y + 10);
	pDC->LineTo(x - 1, y + 10);

	pDC->MoveTo(x - 10, y - 1);
	pDC->LineTo(x - 10, y - 10);
	pDC->LineTo(x - 1, y - 10);
}

void Gomoku::p_DrawSelectRect(CDC *pDC, CPoint point)
{
	CPen m_pen;
	m_pen.CreatePen(PS_SOLID, 1, COLOR_WHITE);
	CPen *m_oldPen = pDC->SelectObject(&m_pen);

	// cursor position to chess board pixels
	int grid_x, grid_y; // in pixels
	int arrIdx_x, arrIdx_y; // pos in chessboard, 0 - 14
	arrIdx_x = p_ScreenToBoard_x(point.x);
	grid_x = arrIdx_x < 0 ? p_board_left : p_board_left + p_subGridSize * arrIdx_x;
	grid_x = min(grid_x, p_board_right);

	arrIdx_y = p_ScreenToBoard_x(point.y);
	grid_y = arrIdx_y < 0 ? p_board_top : p_board_top + p_subGridSize * arrIdx_y;
	grid_y = min(grid_y, p_board_bottom);

	if (arrIdx_x < 0 || arrIdx_x >= BOARD_X || arrIdx_y < 0 || arrIdx_y >= BOARD_Y)
		return;

	int rect_prev_x = p_selectRect.first, rect_prev_y = p_selectRect.second;

	// only draw select rectancle if no piece
	if (chessBoard[arrIdx_x][arrIdx_y] == EMPTY && (p_selectRect.first != grid_x || p_selectRect.second != grid_y)) {
		// white rect over selected box
		p_DrawSelectRectHelper(pDC, grid_x, grid_y);

		p_selectRect.first = grid_x;
		p_selectRect.second = grid_y;

		// clear previous select box
		if (rect_prev_x != SELECT_RECT_INIT || rect_prev_y != SELECT_RECT_INIT) {
			CPen m_restore_pen;
			m_restore_pen.CreatePen(PS_SOLID, 1, COLOR_BOARD);
			CPen *m_old_pen_restore = pDC->SelectObject(&m_restore_pen);

			p_DrawSelectRectHelper(pDC, rect_prev_x, rect_prev_y);
		}
	}
	else {
		//OutputDebugString(_T("Not drawing. arrIdx_x: "));
		//CString t;
		//t.Format(_T("%d"), arrIdx_x);
		//OutputDebugString(t);
		//OutputDebugString(_T(" arrIdx_y: "));
		//t.Format(_T("%d"), arrIdx_y);
		//OutputDebugString(t + _T("\r\n"));

		//OutputDebugString(_T(" p_selectRect.first: "));
		//t.Format(_T("%d"), p_selectRect.first);
		//OutputDebugString(t);
		//OutputDebugString(_T(" p_selectRect.second: "));
		//t.Format(_T("%d"), p_selectRect.second);
		//OutputDebugString(t + _T("\r\n"));

		//OutputDebugString(_T(" grid_x: "));
		//t.Format(_T("%d"), grid_x);
		//OutputDebugString(t);
		//OutputDebugString(_T(" grid_y: "));
		//t.Format(_T("%d"), grid_y);
		//OutputDebugString(t + _T("\r\n"));
	}

	pDC->SelectObject(m_oldPen);
	m_pen.DeleteObject();
}

void Gomoku::p_DrawPiece(CDC *pDC, CPoint point, int piece)
{
	// cursor position to chess board pixels
	int grid_x, grid_y; // in pixels
	int arrIdx_x, arrIdx_y; // pos in chessboard, 0 - 14
	arrIdx_x = p_ScreenToBoard_x(point.x);
	grid_x = arrIdx_x < 0 ? p_board_left : p_board_left + p_subGridSize * arrIdx_x;
	grid_x = min(grid_x, p_board_right);

	arrIdx_y = p_ScreenToBoard_x(point.y);
	grid_y = arrIdx_y < 0 ? p_board_top : p_board_top + p_subGridSize * arrIdx_y;
	grid_y = min(grid_y, p_board_bottom);

	if (arrIdx_x < 0 || arrIdx_x >= BOARD_X || arrIdx_y < 0 || arrIdx_y >= BOARD_Y)
		return;

	if (chessBoard[arrIdx_x][arrIdx_y] == EMPTY) {
		CBrush m_brush;
		if (piece == WHITE)
			m_brush.CreateSolidBrush(COLOR_WHITE);
		else
			m_brush.CreateSolidBrush(COLOR_BLACK);
		CBrush *m_old_brush = pDC->SelectObject(&m_brush);
		pDC->Ellipse(grid_x - 10, grid_y - 10, grid_x + 10, grid_y + 10);
		pDC->SelectObject(m_old_brush);
		m_brush.DeleteObject();
	}
}

void Gomoku::restart() {
	// initialize all values with 0 - empty
	memset(chessBoard, EMPTY, sizeof(chessBoard));

	// black goes first
	turn = BLACK;

	isOver = false;

	result = ST_NOT_FINISHED;

	count = 0;
}

int Gomoku::whoseTurn() {
	return turn;
}

int Gomoku::getCount() {
	return count;
}
bool Gomoku::canMove(int player_lccal) {
	return (player_lccal == turn);
}

bool Gomoku::gameOver() {
	return isOver;
}

// move by input of x, y on relative cursor position
bool Gomoku::move(CDC *pDC, int player_local, CPoint point, std::string& errMsg) {
	if (!canMove(player_local)) {
		// not player's turn
		errMsg = G_ERR_NOT_YOUR_TURN;
		return false;
	}

	// cursor position to chess board pixels
	int grid_x, grid_y; // in pixels
	int arrIdx_x, arrIdx_y; // pos in chessboard, 0 - 14
	arrIdx_x = p_ScreenToBoard_x(point.x);
	grid_x = arrIdx_x < 0 ? p_board_left : p_board_left + p_subGridSize * arrIdx_x;
	grid_x = min(grid_x, p_board_right);

	arrIdx_y = p_ScreenToBoard_x(point.y);
	grid_y = arrIdx_y < 0 ? p_board_top : p_board_top + p_subGridSize * arrIdx_y;
	grid_y = min(grid_y, p_board_bottom);

	if (arrIdx_x < 0 || arrIdx_x >= BOARD_X || arrIdx_y < 0 || arrIdx_y >= BOARD_Y) {
		errMsg = G_ERR_X_Y_OUTTA_RANGE;
		return false;
	}

	if (chessBoard[arrIdx_x][arrIdx_y] != EMPTY) {
		// already occupied
		errMsg = G_ERR_POS_TAKEN;
		return false;
	}

	CBrush m_brush;
	if (player_local == WHITE)
		m_brush.CreateSolidBrush(COLOR_WHITE);
	else
		m_brush.CreateSolidBrush(COLOR_BLACK);
	CBrush *m_old_brush = pDC->SelectObject(&m_brush);
	pDC->Ellipse(grid_x - 10, grid_y - 10, grid_x + 10, grid_y + 10);
	pDC->SelectObject(m_old_brush);
	m_brush.DeleteObject();

	count++;

	chessBoard[arrIdx_x][arrIdx_y] = player_local;

	if (checkWinner(arrIdx_x, arrIdx_y)) {
		// winner = turn
		isOver = true;
		result = turn;
		return true;
	}

	if (count == BOARD_X * BOARD_Y) {
		// tie
		isOver = true;
		result = ST_TIE;
		return true;
	}
	updateTurn();
	return true;
}

// move by input of x, y on chessboard
bool Gomoku::move(CDC *pDC, int player_local, int x, int y, std::string& errMsg) {
	if (!canMove(player_local)) {
		// not player's turn
		errMsg = G_ERR_NOT_YOUR_TURN;
		return false;
	}

	if (x < 0 || x >= BOARD_X || y < 0 || y >= BOARD_Y) {
		errMsg = G_ERR_X_Y_OUTTA_RANGE;
		return false;
	}

	if (chessBoard[x][y] != EMPTY) {
		// already occupied
		errMsg = G_ERR_POS_TAKEN;
		return false;
	}

	CBrush m_brush;
	if (player_local == WHITE)
		m_brush.CreateSolidBrush(COLOR_WHITE);
	else
		m_brush.CreateSolidBrush(COLOR_BLACK);
	CBrush *m_old_brush = pDC->SelectObject(&m_brush);
	int grid_x, grid_y;
	grid_x = x < 0 ? p_board_left : p_board_left + p_subGridSize * x;
	grid_x = min(grid_x, p_board_right);
	grid_y = y < 0 ? p_board_top : p_board_top + p_subGridSize * y;
	grid_y = min(grid_y, p_board_bottom);
	pDC->Ellipse(grid_x - 10, grid_y - 10, grid_x + 10, grid_y + 10);
	pDC->SelectObject(m_old_brush);
	m_brush.DeleteObject();

	count++;

	chessBoard[x][y] = player_local;

	if (checkWinner(x, y)) {
		// winner = turn
		isOver = true;
		result = turn;
		return true;
	}

	if (count == BOARD_X * BOARD_Y) {
		// tie
		isOver = true;
		result = ST_TIE;
		return true;
	}
	updateTurn();
	return true;
}


// get 2d board index x from cursor pos x
int Gomoku::p_ScreenToBoard_x(long screenPosX) {
	return int(double(screenPosX - p_board_left) / p_subGridSize + 0.5);
}

// get 2d board index y from cursor pos y
int Gomoku::p_ScreenToBoard_y(long screenPosY) {
	return int(double(screenPosY - p_board_top) / p_subGridSize + 0.5);
}