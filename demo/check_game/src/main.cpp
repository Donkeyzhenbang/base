#include <graphics.h>
#include <iostream>


/*读取操作 读取鼠标输入
* 绘制画面 当前落子类型；X与O的绘制；3X3网格
* 处理数据 char board[3][3]表示棋盘，初始为'-'；对胜利条件进行枚举；平局则为无人获胜+每个变量均不是'-'
*/

int draw_ball()
{
	initgraph(1200, 720);
	int x = 300;
	int y = 300;
	BeginBatchDraw();

	while (true) {
		ExMessage msg;
		while (peekmessage(&msg)) {
			if (msg.message == WM_MOUSEMOVE) {
				x = msg.x;
				y = msg.y;
			}
		}
		cleardevice();
		solidcircle(x, y, 100);
		FlushBatchDraw();
	}
	EndBatchDraw();
	return 0;

}

/*
MessageBox(
hWnd,	父窗口句柄 windows系统中用于指向窗口对象的指针
text,	提示内容
caption,弹窗标题
type	弹窗样式
);
*/

/* 先行后列 y在前 x在后
00 01 02
10 11 12
20 21 22
*/
char board_data[3][3] =
{
	{'-', '-', '-'},
	{'-', '-', '-'},
	{'-', '-', '-'},
};

char current_piece = 'O';

bool CheckWin(char c)
{
	if (board_data[0][0] == c && board_data[0][1] == c && board_data[0][2] == c)
		return true;
	if (board_data[1][0] == c && board_data[1][1] == c && board_data[1][2] == c)
		return true;
	if (board_data[2][0] == c && board_data[2][1] == c && board_data[2][2] == c)
		return true;
	if (board_data[0][0] == c && board_data[1][0] == c && board_data[2][0] == c)
		return true;
	if (board_data[0][1] == c && board_data[1][1] == c && board_data[2][1] == c)
		return true;
	if (board_data[0][2] == c && board_data[1][2] == c && board_data[2][2] == c)
		return true;
	if (board_data[0][0] == c && board_data[1][1] == c && board_data[2][2] == c)
		return true;
	if (board_data[0][2] == c && board_data[1][1] == c && board_data[2][0] == c)
		return true;

	return false;
}

bool CheckDraw()
{
	for (size_t i = 0; i < 3; i++) 
	{
		for (size_t j = 0; j < 3; j++) 
		{
			if (board_data[i][j] == '-')
				return false;
		}
	}

	return true;
}

void DrawBoard()
{
	line(0, 200, 600, 200);
	line(0, 400, 600, 400);
	line(200, 0, 200, 600);
	line(400, 0, 400, 600);

}

void DrawPiece()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			switch (board_data[i][j])
			{
			case 'O':
				circle(200 * j + 100, 200 * i + 100, 100);
				break;
			case 'X':
				line(200 * j, 200 * i, 200 * (j + 1), 200 * (i + 1));
				line(200 * (j + 1), 200 * i, 200 * j , 200 * (i + 1)); 
				break;
			case '-':
				break;
			}
		}
	}
}

void DrawTipText()
{
	static TCHAR str[64];
	_stprintf_s(str, _T("The current piece type : %c"), current_piece);
	settextcolor((RGB(225, 175, 45)));
	outtextxy(0, 0, str);
}
int main() 
{
	initgraph(600,600);
	bool running = true;

	ExMessage msg;

	BeginBatchDraw();

	while (running) 
	{
		DWORD start_time = GetTickCount();
		while (peekmessage(&msg))
		{
			if (msg.message == WM_LBUTTONDOWN)
			{
				int x = msg.x;
				int y = msg.y;
				int index_x = x / 200;
				int index_y = y / 200;
				//尝试落子
				if (board_data[index_y][index_x] == '-')
				{
					board_data[index_y][index_x] = current_piece;

					if (current_piece == 'O')
						current_piece = 'X';
					else
						current_piece = 'O';
				}

			}

		}

		cleardevice();//先清屏
		DrawBoard();
		DrawPiece();
		DrawTipText();

		FlushBatchDraw();//双缓冲  再把绘制内容显示上去
		 
		if (CheckWin('X')) {
			MessageBox(GetHWnd(), _T("X win"), _T("Game Over"), MB_OK);
			running = false;
		}
		else if (CheckWin('O')) {
			MessageBox(GetHWnd(), _T("O win"), _T("Game Over"), MB_OK);
			running = false;
		}
		else if (CheckDraw()) {
			MessageBox(GetHWnd(), _T("draw"), _T("Game Over"), MB_OK);
		}

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 60)
		{
			Sleep(1000 / 60 - delta_time); //Sleep与GetTickCount函数都是毫秒级
		}
	}
	EndBatchDraw();
	return 0;
}

