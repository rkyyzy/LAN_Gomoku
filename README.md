# Project: LAN_Gomoku
- Classic Gomoku chess, support local area network playing
- Visual Studio 2017 C++
- Win32 program
- Microsoft Foundation Class (MFC) interface
- Socket TCP LAN connection
![MAIN](https://user-images.githubusercontent.com/73468884/175721265-11c3aa31-7908-4de3-a127-5a32d308c5cd.png)
![dual](https://user-images.githubusercontent.com/73468884/175721277-48923873-61fc-47e4-b993-c9809c15d81c.png)


# GomokuGameMain.cpp
Implementing game logic and chessboard painting functions

# GomokuDlg.cpp
Implementing dialog event message handler (Button click, Textbox update etc.)

# LAN.cpp, winsockUT.cpp
Implementing function to be executed by the threads (listener, connection, client) and LAN IP fetching.

# Misc
There are bugs to be fixed:
1) Game is supposed to sync on the host side.
2) Piece position is sometimes off when communicating through LAN.

To be improved:
1) Restart with the same rival
2) Print chess board piece position [0, 14] in place
