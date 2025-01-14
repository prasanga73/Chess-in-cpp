#ifndef CHESS_H
#define CHESS_H

#define INFINITY_NUM 1000
#define BOARD_SIZE 8

#include<vector>

enum Piece{
	pawn_w = 1, pawn_b = -1,
	rook_w = 2, rook_b = -2,
	knight_w = 3, knight_b = -3,
	bishop_w = 4, bishop_b = -4,
	queen_w = 5, queen_b = -5,
	king_w = 6, king_b = -6,
	empty = 0
};

struct Coordinate{
	int x, y;
	Coordinate();
	Coordinate(int x, int y);
	bool isValid();
	void set(int x, int y);
	void clear();
};

typedef std::vector<Coordinate> CoordinateList;

struct Board{
	Piece state[8][8] = {
		{  rook_b, pawn_b, empty, empty, empty, empty, pawn_w, rook_w  },
		{knight_b, pawn_b, empty, empty, empty, empty, pawn_w, knight_w},
		{bishop_b, pawn_b, empty, empty, empty, empty, pawn_w, bishop_w},
		{ queen_b, pawn_b, empty, empty, empty, empty, pawn_w, queen_w },
		{  king_b, pawn_b, empty, empty, empty, empty, pawn_w, king_w  },
		{bishop_b, pawn_b, empty, empty, empty, empty, pawn_w, bishop_w},
		{knight_b, pawn_b, empty, empty, empty, empty, pawn_w, knight_w},
		{  rook_b, pawn_b, empty, empty, empty, empty, pawn_w, rook_w  }
	};

	Coordinate warnedPosition;
	bool castleBL = true, castleBR = true, castleWL = true, castleWR = true;

	Piece get(const Coordinate &c);
	Coordinate find(Piece p);
	int getPointSum();
};

int getPoints(Piece p);

void getMoves(CoordinateList &validMoves, const Coordinate &pos, Board &board, bool removeInvalid);
void movePiece(Board &board, const Coordinate &from, const Coordinate &to, Piece (*getPromotionChoice)());
Piece getMoveToMake(Coordinate &move_from, Coordinate &move_to, Board &board, int depth, int color_coeff);

bool isBlack(Piece p);
bool isWhite(Piece p);
bool isEmpty(Board &board, int x, int y);

bool isInCheck(Piece checkPiece, Board &board);

#endif