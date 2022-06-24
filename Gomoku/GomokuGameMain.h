#pragma once
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include "pch.h"


#define EMPTY 0
#define BLACK 1
#define WHITE 2
#define BOARD_X 15
#define BOARD_Y 15

#define ST_NOT_FINISHED -1
#define ST_TIE 0
#define ST_BLACKWIN 1
#define ST_WHITEWIN 2

#define SELECT_RECT_INIT -1

#define COLOR_BOARD RGB(127,96,13)
#define COLOR_BLACK RGB(0,0,0)
#define COLOR_WHITE RGB(255,255,255)

extern const std::string G_ERR_NOT_YOUR_TURN;
extern const std::string G_ERR_X_Y_OUTTA_RANGE;
extern const std::string G_ERR_POS_TAKEN;

class Gomoku
{
private:
	// game data
	// 15 x 15 chess board, 0 - empty, 1 - black, 2 - white
	int chessBoard[BOARD_X][BOARD_Y];
	int turn;
	int count;
	bool isOver;
	
	std::vector<std::pair<int, int>> whiteCount;
	std::vector<std::pair<int, int>> blackCount;

	void updateTurn();
	bool inRange(int x, int y);
	bool horizontalCheck(int x, int y, int target);
	bool verticalCheck(int x, int y, int target);
	bool diagonalCheck(int x, int y, int target);
	bool checkWinner(int x, int y);


	// painting data
	int p_subGridSize;
	int p_board_left;
	int p_board_right;
	int p_board_top;
	int p_board_bottom;
	std::pair<int, int> p_selectRect;
	void p_DrawSelectRectHelper(CDC *pDC, int x, int y);
public:
	// game data
	bool result;

	// game func
	Gomoku();
	void restart();
	int whoseTurn();
	int getCount();
	bool canMove(int player_lccal);
	bool gameOver();
	bool move(CDC *pDC, int player_local, CPoint point, std::string& errMsg);
	bool move(CDC *pDC, int player_local, int x, int y, std::string& errMsg);

	// paint func
	void p_DrawTable(CDC *pDC, CRect m_rect);
	void p_Repaint(CDC *pDC);
	void p_DrawSelectRect(CDC *pDC, CPoint point);
	void p_DrawPiece(CDC *pDC, CPoint point, int piece);
	int p_ScreenToBoard_x(long screenPos);
	int p_ScreenToBoard_y(long screenPos);
};
