#include <windows.h>
#include<gdiplus.h>
#include "chess.h"
#include "resource.h"
#define DISPLAY_SIZE 640
#define MINIMAX_DEPTH 5

//GLOBAL VARIABLES
Board board;
Coordinate movedPosition, activePosition;
CoordinateList validMoves;
bool isComputerTurn = false, isGameOver = false;
Piece winner = empty;
HWND hwnd;
HINSTANCE hinst;

//FUNCTIONS
LPCWSTR getImageDir(Piece piece);
void printBoard(HWND hwnd);
void onClickCell(int x, int y);
Piece getPromotionPiece(const Coordinate &c, Board &board, int depth);
void doComputerMove();
void doPlayerMove(const Coordinate& move);
bool hasValidMoves(Piece color);
void finishGame();
LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);


int WINAPI WinMain(HINSTANCE currentInstance, HINSTANCE previousInstance, PSTR cmdLine, INT cmdCount) {
	hinst = currentInstance;
	LPCSTR CLASS_NAME = "myWin32WindowClass", WINDOW_NAME = "Chess++";

	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	// Register the window class
	WNDCLASSA wc{};
	wc.hInstance = currentInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIconW(hinst, MAKEINTRESOURCEW(MAIN_ICON));
	wc.lpfnWndProc = WindowProcessMessages;
	RegisterClassA(&wc);

	DWORD windowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
	RECT windowRect{0, 0, DISPLAY_SIZE, DISPLAY_SIZE};
	AdjustWindowRect(&windowRect, windowStyle, FALSE);

	// Create the window
	CreateWindowA(CLASS_NAME, WINDOW_NAME,
		windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,				// Window initial position
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		nullptr, 
		nullptr, 
		currentInstance, 
		nullptr);

	// Window loop
	MSG msg{};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);

	return 0;
}

LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam) {
	::hwnd = hwnd;

	switch (msg) {
	case WM_DESTROY:{
		PostQuitMessage(0);		//makes GetMessage return 0 (and this the window loop exits)
		return 0;
	}
	case WM_LBUTTONDOWN:{
		int posX = lparam & 0xffff, posY = (lparam >> 16) & 0xffff;
		onClickCell(BOARD_SIZE*posX/DISPLAY_SIZE, BOARD_SIZE*posY/DISPLAY_SIZE);
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		return 0;
	}
	case WM_PAINT:{
		printBoard(hwnd);

		if(isGameOver){
			finishGame();
		}
		//do computer move after player move is printed
		if(isComputerTurn){
			doComputerMove();
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		}
		return 0;
	}
	default:
		return DefWindowProc(hwnd, msg, param, lparam);
	}
}

LPCWSTR getImageDir(Piece piece){
	switch(piece){
		case pawn_b: return L"resources/pawn_b.png";
		case pawn_w: return L"resources/pawn_w.png";
		case rook_b: return L"resources/rook_b.png";
		case rook_w: return L"resources/rook_w.png";
		case knight_b: return L"resources/knight_b.png";
		case knight_w: return L"resources/knight_w.png";
		case bishop_b: return L"resources/bishop_b.png";
		case bishop_w: return L"resources/bishop_w.png";
		case queen_b: return L"resources/queen_b.png";
		case queen_w: return L"resources/queen_w.png";
		case king_b: return L"resources/king_b.png";
		case king_w: return L"resources/king_w.png";
	}
	return NULL;
}

void printBoard(HWND hwnd){
	// All painting occurs here, between BeginPaint and EndPaint.
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	int size = DISPLAY_SIZE/BOARD_SIZE;
	Gdiplus::Graphics graphics(hdc);

	Gdiplus::SolidBrush white(Gdiplus::Color(255, 252, 215, 157));
	Gdiplus::SolidBrush black(Gdiplus::Color(255, 171, 101, 37));
	Gdiplus::SolidBrush markerGreen(Gdiplus::Color(255, 87, 187, 0));

	for(int x = 0; x < BOARD_SIZE; ++x){
		int left = size*x;
		for(int y = 0; y < BOARD_SIZE; ++y){
			int top = size*y;
			if((x + y)%2){
				graphics.FillRectangle(&black, left, top, size, size);
			} else {
				graphics.FillRectangle(&white, left, top, size, size);
			}
			//blit piece sprites
			if(board.state[x][y] != empty){
				// Gdiplus::Bitmap *image = Gdiplus::Bitmap::FromResource(hinst, MAKEINTRESOURCEW(BISHOP_B_IMG));
				graphics.DrawImage(Gdiplus::Bitmap::FromFile(getImageDir(board.state[x][y])), left, top, size, size);
			}
		}
	}

	if(board.warnedPosition.isValid()){
		Gdiplus::SolidBrush warningRed(Gdiplus::Color(255, 180, 0, 0));
		int left = board.warnedPosition.x * size, top = board.warnedPosition.y * size;
		graphics.FillRectangle(&warningRed, left, top, size, size);
		Gdiplus::Bitmap image(getImageDir(board.state[board.warnedPosition.x][board.warnedPosition.y]));
		graphics.DrawImage(&image, left, top, size, size);
	}
	
	if(movedPosition.isValid()){
		int left = movedPosition.x * size, top = movedPosition.y * size;
		graphics.FillRectangle(&markerGreen, left, top, size, size);
		Gdiplus::Bitmap image(getImageDir(board.state[movedPosition.x][movedPosition.y]));
		graphics.DrawImage(&image, left, top, size, size);
	}

	for(auto v: validMoves){
		int printx;
		if(v.x == INFINITY_NUM)
			printx = 6 * size + size/2 - 20;
		else if(v.x == -INFINITY_NUM)
			printx = 2 * size + size/2 - 20;
		else
			printx = v.x*size + size/2 - 20;

		//int radius = 20;
		graphics.FillEllipse(&markerGreen, printx, v.y*size + size/2 - 20, 40, 40);
	}

	EndPaint(hwnd, &ps);
}

void onClickCell(int x, int y){
	if(activePosition.isValid()){
		for(auto &c : validMoves){
			if(c.y == y && (c.x == x || (c.x == INFINITY_NUM && x == 6) || (c.x == -INFINITY_NUM && x == 2))){
				doPlayerMove(c);
				break;
			}
		}
		activePosition.clear();
		validMoves.clear();

	} else if(isWhite(board.state[x][y])){
		getMoves(validMoves, Coordinate(x, y), board, true);
		if(validMoves.size() > 0){
			activePosition.set(x, y);
		}
	}
}

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 	
	if(message != WM_COMMAND)
		return FALSE;

	if(LOWORD(wParam) == IDCANCEL)
		return FALSE;

	EndDialog(hwndDlg, wParam);
    return TRUE; 
} 

Piece getPromotionPiece(){
	WPARAM picked = DialogBoxW(hinst, MAKEINTRESOURCEW(PROMOTION_DIALOG), hwnd, (DLGPROC)DialogProc);

	switch (LOWORD(picked)) 
    { 
        case QUEEN_BTN: return queen_w;
		case ROOK_BTN: return rook_w;
		case KNIGHT_BTN: return knight_w;
		case BISHOP_BTN: return bishop_w;
    }
	return empty;
}

/* shitty solution but it works
	getMoveToMake() returns promoted to piece if promoted
	movePiece() needs function to get promotion option
	so assign the return value to a global var and return that as required
*/
Piece computer_promoted;
Piece getComputerPromotion(){
	return computer_promoted;
}

void doComputerMove(){
	Coordinate comp_from, comp_to;
	computer_promoted = getMoveToMake(comp_from, comp_to, board, MINIMAX_DEPTH, -1);
	movePiece(board, comp_from, comp_to, getComputerPromotion);
	int to_x;
	if(comp_to.x == INFINITY_NUM)
		to_x = 6;
	else if(comp_to.x == -INFINITY_NUM)
		to_x = 2;
	else
		to_x = comp_to.x;
	movedPosition.set(to_x, comp_to.y);
	isComputerTurn = false;

	//check for game end
	if(!hasValidMoves(king_w)){
		if(isInCheck(king_w, board)){
			winner = king_b;
		} else {
			winner = empty;
		}
		isGameOver = true;
	}
	if(isInCheck(king_b, board)){	//leading to a checkmate. [no change in board sum. so it will just do the first possible move]
		winner = king_w;
		isGameOver = true;
	}
}

void doPlayerMove(const Coordinate& move){
	movePiece(board, activePosition, move, getPromotionPiece);
	movedPosition.set(move.x, move.y);
	isComputerTurn = true;

	//check for game end
	if(!hasValidMoves(king_b)){
		if(isInCheck(king_b, board)){
			winner = king_w;
		} else {
			winner = empty;
		}
		isGameOver = true;
	}
}

bool hasValidMoves(Piece color){
	bool (*isOfColor)(Piece)  = color == king_w ? isWhite : isBlack;

	for(int x = 0; x < BOARD_SIZE; ++x){
		for(int y = 0; y <BOARD_SIZE; ++y){
			if(isOfColor(board.state[x][y])){
				CoordinateList moves;
				getMoves(moves, Coordinate(x, y), board, true);
				if(moves.size()>0)
					return true;
			}
		}
	}

	return false;
}

void finishGame(){

	LPCSTR message = (winner == empty)?"It's a tie!" : (winner == king_w) ?"You Win!" : "Better luck next time.";
	//show dialog
	if(MessageBoxA(hwnd, message, "Game Over", MB_RETRYCANCEL) == IDRETRY){
		board = Board();
		isComputerTurn = false;
		validMoves.clear();
		activePosition.clear();
		movedPosition.clear();
		isGameOver = false;
		winner = empty;

	} else {
		PostQuitMessage(0);
	}
}