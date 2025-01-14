#include "chess.h"
#define max(a, b) (a > b ? a : b)

Coordinate::Coordinate(){
	clear();
}
Coordinate::Coordinate(int x, int y){
	set(x, y);
}
bool Coordinate::isValid(){
	return x != -1 && y!= -1; 
}
void Coordinate::set(int x, int y){
	this->x = x;
	this->y = y;
}
void Coordinate::clear(){
	x = y = -1;
}

Coordinate Board::find(Piece p){
	for(int i = 3, x = i; i >= 0; x = (x==i)? BOARD_SIZE-i-1 : --i){
		for(int y = 0; y < BOARD_SIZE/2; ++y){
			if(p == state[x][y])
				return Coordinate(x, y);
			else if(p == state[x][BOARD_SIZE - y - 1])
				return Coordinate(x, BOARD_SIZE - y - 1);
		}
	}
	return Coordinate();
}
Piece Board::get(const Coordinate &c){
	return state[c.x][c.y];
}
int Board::getPointSum(){
	int sum = 0;
	for(int x = 0; x < BOARD_SIZE; ++x){
		for(int y = 0; y < BOARD_SIZE; ++y){
			switch(state[x][y]){
			case pawn_b: sum += -1; break;
			case pawn_w: sum += 1; break;
			case rook_b: sum += -5; break;
			case rook_w: sum += 5; break;
			case knight_b: 
			case bishop_b: sum += -3; break;
			case knight_w:
			case bishop_w: sum += 3; break;
			case queen_b: sum += -9; break;
			case queen_w: sum += 9; break;
			case king_b: sum += -100; break;
			case king_w: sum += 100; break;
			}
		}
	}
	return sum;
}

bool isBlack(Piece p){
	return p < 0;
}
bool isWhite(Piece p){
	return p > 0;
}
bool isSameColor(Piece p1, Piece p2){
	return (p1 < 0 && p2 < 0) || (p1 > 0 && p2 > 0);
}
bool isInBounds(int x, int y){
	return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}
bool isEmpty(Board &board, int x, int y){
	return board.state[x][y] == empty;
}

void pushToList(CoordinateList &validMoves, Board& board, const Coordinate &from, const Coordinate &to, bool checkIfValid);

//adds valid moves by extrapolating move_increments till a blocking piece is encountered
void addMoveIncrements(CoordinateList &validMoves, Board &board, const Piece current, const Coordinate &pos, 
						const Coordinate move_increments[], int move_count, bool checkIfValid){
	
	for(int i = 0; i < move_count; ++i){
		const Coordinate &m = move_increments[i];
		for(int x = pos.x+m.x, y = pos.y+m.y; isInBounds(x, y); x += m.x, y += m.y){
			if(isEmpty(board, x, y)){
				pushToList(validMoves, board, pos, Coordinate(x, y), checkIfValid);
			} else if(isSameColor(board.state[x][y], current)){
				break;
			} else {
				pushToList(validMoves, board, pos, Coordinate(x, y), checkIfValid);
				break;
			}
		}
	}
}

//returns true if the king of checkPiece is in check in the passed board
bool isInCheck(Piece checkPiece, Board &board){
	bool (*isOtherColor)(Piece p);
	int dirCoeff = checkPiece > 0 ? -1 : 1;		//white pieces play up the board
	Piece check_king = (Piece)(-dirCoeff * king_w);		//pieces are inverse
	isOtherColor = checkPiece > 0 ? isBlack : isWhite;

	Coordinate kingPos = board.find(check_king);
	if(!kingPos.isValid())
		return false;

	//pawn check
	if((isInBounds(kingPos.x - 1, kingPos.y + dirCoeff) && board.state[kingPos.x - 1][kingPos.y + dirCoeff] == dirCoeff * pawn_w) || 
		(isInBounds(kingPos.x + 1, kingPos.y + dirCoeff) && board.state[kingPos.x + 1][kingPos.y + dirCoeff] == dirCoeff * pawn_w))
		return true;

	CoordinateList rays;
	//rook/queen check
	const Coordinate straight[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
	addMoveIncrements(rays, board, check_king, kingPos, straight, 4, false);
	for(auto &m : rays){
		if(board.get(m) == dirCoeff * rook_w || board.get(m) == dirCoeff * queen_w){
			return true;
		}
	}
	rays.clear();

	//bishop/queen check
	const Coordinate diagonal[] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
	addMoveIncrements(rays, board, check_king, kingPos, diagonal, 4, false);
	for(auto &m : rays){
		if(board.get(m) == dirCoeff * bishop_w || board.get(m) == dirCoeff * queen_w){
			return true;
		}
	}

	//knight check
	const Coordinate L[] = {{-2, 1}, {-2, -1}, {2, 1}, {2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
	for(auto &m : L){
		int x = kingPos.x + m.x, y = kingPos.y + m.y;
		if(isInBounds(x, y) && board.state[x][y] == dirCoeff * knight_w)
			return true;
	}

	//king check
	const Coordinate around[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
	for(auto &m : around){
		int x = kingPos.x + m.x, y = kingPos.y + m.y;
		if(isInBounds(x, y) && board.state[x][y] == -check_king)
			return true;
	}
	return false;
}

//pushes move to first argument (pass checkIfValid=true to avoid illegal moves)
void pushToList(CoordinateList &validMoves, Board& board, const Coordinate &from, const Coordinate &to, bool checkIfValid){
	if(!checkIfValid){
		validMoves.push_back(to);
		return;
	}
	
	int to_x;
	if(to.x == INFINITY_NUM)
		to_x = 6;
	else if(to.x == -INFINITY_NUM)
		to_x = 2;
	else
		to_x = to.x;

	Piece from_piece = board.get(from), to_piece = board.state[to_x][to.y];
	board.state[to_x][to.y] = from_piece;
	board.state[from.x][from.y] = empty;
	if(!isInCheck(from_piece, board)){
		
		validMoves.push_back(to);
	}
	board.state[from.x][from.y] = from_piece;
	board.state[to_x][to.y] = to_piece;
}

//Fills valid moves into first argument (pass removeInvalid=true to avoid illegal moves)
void getMoves(CoordinateList &validMoves, const Coordinate &pos, Board &board, bool removeInvalid){

	Piece current = board.get(pos);
	switch(current){
		case pawn_b:{
			//pawn cant be at y = 7 [since promotion], so no need to check if in bounds
			if(isEmpty(board, pos.x, pos.y + 1)){
				pushToList(validMoves, board, pos, Coordinate(pos.x, pos.y + 1), removeInvalid);
				if(pos.y == 1 && isEmpty(board, pos.x, 3))		//double move at beginning
					pushToList(validMoves, board, pos, Coordinate(pos.x, 3), removeInvalid);
			}
			//cut
			if(isInBounds(pos.x - 1, pos.y + 1) && isWhite(board.state[pos.x - 1][pos.y + 1]))
				pushToList(validMoves, board, pos, Coordinate(pos.x - 1, pos.y + 1), removeInvalid);
			if(isInBounds(pos.x + 1, pos.y + 1) && isWhite(board.state[pos.x + 1][pos.y + 1]))
				pushToList(validMoves, board, pos, Coordinate(pos.x + 1, pos.y + 1), removeInvalid);
			break;
		}

		case pawn_w:{
			if(isEmpty(board, pos.x, pos.y - 1)){
				pushToList(validMoves, board, pos, Coordinate(pos.x, pos.y - 1), removeInvalid);
				if(pos.y == 6 && isEmpty(board, pos.x, 4))
					pushToList(validMoves, board, pos, Coordinate(pos.x, 4), removeInvalid);
			}

			if(isInBounds(pos.x - 1, pos.y - 1) && isBlack(board.state[pos.x - 1][pos.y - 1]))
				pushToList(validMoves, board, pos, Coordinate(pos.x - 1, pos.y - 1), removeInvalid);
			if(isInBounds(pos.x + 1, pos.y - 1) && isBlack(board.state[pos.x + 1][pos.y - 1]))
				pushToList(validMoves, board, pos, Coordinate(pos.x + 1, pos.y - 1), removeInvalid);
			break;
		}

		case rook_b:
		case rook_w:{
			const Coordinate move_increments[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
			addMoveIncrements(validMoves, board, current, pos, move_increments, 4, removeInvalid);
			break;
		}
	
		case knight_b:
		case knight_w:{	
			const Coordinate moveIncrements[] = {{-2, 1}, {-2, -1}, {2, 1}, {2, -1}, 
					{1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
			//movement list
			for(const Coordinate& c: moveIncrements){
				int x = pos.x + c.x, y = pos.y + c.y;
				if(isInBounds(x, y) && (isEmpty(board, x, y) || !isSameColor(board.state[x][y],current)))
					pushToList(validMoves, board, pos, Coordinate(x, y), removeInvalid);
			}
			break;
		}

		case bishop_b:
		case bishop_w:{
			const Coordinate move_increments[] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
			addMoveIncrements(validMoves, board, current, pos, move_increments, 4, removeInvalid);
			break;
		}
	
		case queen_b:
		case queen_w:{
			const Coordinate move_increments[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
			addMoveIncrements(validMoves, board, current, pos, move_increments, 8, removeInvalid);
			break;
		}
	
		case king_b:{
			//castle
			if(board.castleBL && isEmpty(board, 1, 0) && isEmpty(board, 2, 0) && isEmpty(board, 3, 0) && board.get(board.warnedPosition) != king_b)
				pushToList(validMoves, board, pos, Coordinate(-INFINITY_NUM, 0), removeInvalid);
			if(board.castleBR && isEmpty(board, 5, 0) && isEmpty(board, 6, 0) && board.get(board.warnedPosition) != king_b)
				pushToList(validMoves, board, pos, Coordinate(INFINITY_NUM, 0), removeInvalid);

			const Coordinate move_increments[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
			for(auto &m: move_increments){
				int x = pos.x + m.x, y = pos.y + m.y;
				if(isInBounds(x, y) && (isEmpty(board, x, y) || !isSameColor(current, board.state[x][y])))
					pushToList(validMoves, board, pos, Coordinate(x, y), removeInvalid);
			}
			break;
		}
		case king_w:{
			if(board.castleWL && isEmpty(board, 1, 7) && isEmpty(board, 2, 7) && isEmpty(board, 3, 7) && board.get(board.warnedPosition) != king_w)
				pushToList(validMoves, board, pos, Coordinate(-INFINITY_NUM, 7), removeInvalid);
			if(board.castleWR && isEmpty(board, 5, 7) && isEmpty(board, 6, 7) && board.get(board.warnedPosition) != king_w)
				pushToList(validMoves, board, pos, Coordinate(INFINITY_NUM, 7), removeInvalid);

			const Coordinate move_increments[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
			for(auto &m: move_increments){
				int x = pos.x + m.x, y = pos.y + m.y;
				if(isInBounds(x, y) && (isEmpty(board, x, y) || !isSameColor(current, board.state[x][y])))
					pushToList(validMoves, board, pos, Coordinate(x, y), removeInvalid);
			}
			break;
		}
	}
}

Piece handlePromotionChoice(const Coordinate &c, Board &board, int depth, CoordinateList& white, CoordinateList& black);

//Moves piece from one position to another
void movePiece(Board &board, const Coordinate &from, const Coordinate &to, Piece (*getPromotionChoice)()){
	//castling updates
	if(board.castleWL && ((from.x == 0 && from.y == 7) || (to.x == 0 && to.y == 7)))
		board.castleWL = false;
	if(board.castleWR && ((from.x == 7 && from.y == 7) || (to.x == 7 && to.y == 7)))
		board.castleWR = false;
	if(board.castleBL && ((from.x == 0 && from.y == 0) || (to.x == 0 && to.y == 0)))
		board.castleBL = false;
	if(board.castleBR && ((from.x == 7 && from.y == 0) || (to.x == 7 && to.y == 0)))
		board.castleBR = false;
	if((board.castleWL || board.castleWR) && board.get(from) == king_w)
		board.castleWL = board.castleWR = false;
	if((board.castleBL || board.castleBR) && board.get(from) == king_b)
		board.castleBL = board.castleBR = false;

	if(to.x == INFINITY_NUM){
		board.state[6][to.y] = board.get(from);
		board.state[5][to.y] = board.state[7][to.y];
		board.state[from.x][from.y] = board.state[7][to.y] = empty;

	} else if(to.x == -INFINITY_NUM){
		board.state[2][to.y] = board.get(from);
		board.state[3][to.y] = board.state[0][to.y];
		board.state[from.x][from.y] = board.state[0][to.y] = empty;

	} else{
		board.state[to.x][to.y] = board.get(from);
		board.state[from.x][from.y] = empty;

		//promotion
		if((board.get(to) == pawn_b && to.y == 7) || (board.get(to) == pawn_w && to.y == 0))
			board.state[to.x][to.y] = getPromotionChoice();
		
		Piece check_king = isWhite(board.get(to))?king_b:king_w;
		if(isInCheck(check_king, board)){
			Coordinate kingPos(board.find(check_king));
			board.warnedPosition.set(kingPos.x, kingPos.y);
		} else {
			board.warnedPosition.clear();
		}
	}
}

void movePieceCalcPromotion(Board &board, const Coordinate &from, const Coordinate &to, int promotion_depth, CoordinateList &white, CoordinateList &black){
	if(board.castleWL && ((from.x == 0 && from.y == 7) || (to.x == 0 && to.y == 7)))
		board.castleWL = false;
	if(board.castleWR && ((from.x == 7 && from.y == 7) || (to.x == 7 && to.y == 7)))
		board.castleWR = false;
	if(board.castleBL && ((from.x == 0 && from.y == 0) || (to.x == 0 && to.y == 0)))
		board.castleBL = false;
	if(board.castleBR && ((from.x == 7 && from.y == 0) || (to.x == 7 && to.y == 0)))
		board.castleBR = false;
	if((board.castleWL || board.castleWR) && board.get(from) == king_w)
		board.castleWL = board.castleWR = false;
	if((board.castleBL || board.castleBR) && board.get(from) == king_b)
		board.castleBL = board.castleBR = false;

	if(to.x == INFINITY_NUM){
		board.state[6][to.y] = board.get(from);
		board.state[5][to.y] = board.state[7][to.y];
		board.state[from.x][from.y] = board.state[7][to.y] = empty;

	} else if(to.x == -INFINITY_NUM){
		board.state[2][to.y] = board.get(from);
		board.state[3][to.y] = board.state[0][to.y];
		board.state[from.x][from.y] = board.state[0][to.y] = empty;

	} else{
		board.state[to.x][to.y] = board.get(from);
		board.state[from.x][from.y] = empty;
		//promotion
		if((board.get(to) == pawn_b && to.y == 7) || (board.get(to) == pawn_w && to.y == 0))
			board.state[to.x][to.y] = handlePromotionChoice(to, board, promotion_depth, white, black);

		Piece check_king = isWhite(board.get(to))?king_b:king_w;
		if(isInCheck(check_king, board)){
			Coordinate kingPos(board.find(check_king));
			board.warnedPosition.set(kingPos.x, kingPos.y);
		} else {
			board.warnedPosition.clear();
		}
	}
}

bool isCapture( Board &board, const Coordinate &move, int color_coeff) {
    Piece targetPiece = board.get(move);
    return targetPiece != empty && ((color_coeff == 1 && isBlack(targetPiece)) || (color_coeff == -1 && isWhite(targetPiece)));
}

void orderMovesByCapture(Board &board, CoordinateList &moves, int color_coeff) {
    // Separate the moves into captures and non-captures
    CoordinateList captures, nonCaptures;
    
    for (auto &move : moves) {
        if (isCapture(board, move, color_coeff)) {
            captures.push_back(move);  // Move is a capture
        } else {
            nonCaptures.push_back(move);  // Move is not a capture
        }
    }

    // First add all capture moves (to the front)
    moves = captures;
    // Then add the non-capture moves
    moves.insert(moves.end(), nonCaptures.begin(), nonCaptures.end());
}


bool isGameOver(Board &board){
	return !(board.find(king_w).isValid() && board.find(king_b).isValid());
}

int negamax(Board &board, int depth, int alpha, int beta, int color_coeff, CoordinateList &white, CoordinateList &black){
	if(depth == 0 || isGameOver(board))
		return color_coeff * board.getPointSum();

	CoordinateList &pieces = color_coeff == -1? black:white;
	bool (*isColor)(Piece) = color_coeff == -1? isBlack:isWhite;

	int max_val = -INFINITY_NUM;
	for(auto &p : pieces){
		if(isColor(board.get(p))){	//recursive calls dont remove cut pieces
			CoordinateList moves;
			getMoves(moves, p, board, false);
			orderMovesByCapture(board,moves,color_coeff);
			for(auto &m : moves){
				//make backup
				Piece from_piece = board.get(p), 
					to_piece = (m.x != INFINITY_NUM && m.x != -INFINITY_NUM) ? board.get(m) : empty;
				Coordinate warnedPosition(board.warnedPosition);
				bool castleBL = board.castleBL, castleBR = board.castleBR, castleWL = board.castleWL, castleWR = board.castleWR;
				
				movePieceCalcPromotion(board, p, m, depth, white, black);

				int p_x = p.x, p_y = p.y;
				Coordinate *castle_rook = nullptr;
				if(m.x == INFINITY_NUM){
					p.x = 6;
					for(auto &pos: pieces){
						if(pos.x == 7 && pos.y == p.y){
							pos.x = 5;
							castle_rook = &pos;
							break;
						}
					}
				} else if(m.x == -INFINITY_NUM){
					p.x = 2;
					for(auto &pos: pieces){
						if(pos.x == 0 && pos.y == p.y){
							pos.x = 3;
							castle_rook = &pos;
							break;
						}
					}
				} else {
					p.set(m.x, m.y);
				}
				
				int val = -negamax(board, depth - 1, -beta, -alpha, -color_coeff, white, black);
				
				//restore backup
				if(m.x == INFINITY_NUM){
					p.x = 4;
					castle_rook->x = 7;

					board.state[4][p.y] = color_coeff==1? king_w:king_b;
					board.state[5][p.y] = empty;
					board.state[6][p.y] = empty;
					board.state[7][p.y] = color_coeff==1? rook_w:rook_b;

				} else if(m.x == -INFINITY_NUM){
					p.x = 4;
					castle_rook->x = 0;

					board.state[0][p.y] = color_coeff==1? rook_w:rook_b;
					board.state[1][p.y] = empty;
					board.state[2][p.y] = empty;
					board.state[3][p.y] = empty;
					board.state[4][p.y] = color_coeff==1? king_w:king_b;

				} else {
					p.set(p_x, p_y);
					board.state[p.x][p.y] = from_piece;
					board.state[m.x][m.y] = to_piece;
				}

				board.warnedPosition.set(warnedPosition.x, warnedPosition.y);
				board.castleBL = castleBL;
				board.castleBR = castleBR;
				board.castleWL = castleWL;
				board.castleWR = castleWR;
				
				max_val = max(val, max_val);
				alpha = max(alpha, max_val);
				if(beta <= alpha)
					return max_val;
			}
		}
	}
	
	if(max_val == -INFINITY_NUM)
		return color_coeff * board.getPointSum();	//stalemate
	
	return max_val;
}

//picks best promotion option using minimax
Piece handlePromotionChoice(const Coordinate &c, Board &board, int depth, CoordinateList& white, CoordinateList& black){
	return c.y == 7 ? queen_b : queen_w;

	int color_coeff = isWhite(board.get(c))? 1 : -1;
	Piece possible[]{(Piece)(color_coeff*queen_w), (Piece)(color_coeff*rook_w), (Piece)(color_coeff*knight_w), (Piece)(color_coeff*bishop_w)};
	Piece picked = possible[0];		//default

	int max_val = -INFINITY_NUM;
	for(Piece &m: possible){
		board.state[c.x][c.y] = m;
		// by multiplying with -1 for minimising player becomes maximising (negamax)
		int val = -negamax(board, depth, -INFINITY_NUM, INFINITY_NUM, -color_coeff, white, black);
		if (val > max_val){
			max_val = val;
			picked = m;
		}
	}
	board.state[c.x][c.y] = color_coeff==1 ? pawn_w : pawn_b;
	return picked;
}

Piece getMoveToMake(Coordinate &move_from, Coordinate &move_to, Board &board, int depth, int color_coeff){

	CoordinateList black, white;

	for(int i = 3, x = i; i >= 0; x = (x==i)? BOARD_SIZE-i-1 : --i){
		for(int y = 0; y < BOARD_SIZE; ++y){
			//white from top to bottom, black bottom to top[so pawns come first]
			if(isWhite(board.state[x][y]))
				white.push_back(Coordinate(x, y));
			if(isBlack(board.state[x][BOARD_SIZE - y - 1]))
				black.push_back(Coordinate(x, BOARD_SIZE - y - 1));
		}
	}

	CoordinateList &pieces = color_coeff == -1? black:white;

	int max_val = -INFINITY_NUM, alpha = -INFINITY_NUM;
	Piece updated_piece;

	for(auto &p : pieces){
		CoordinateList moves;
		getMoves(moves, p, board, false);
		for(auto &m : moves){
			//make backup
			Piece just_moved = empty, from_piece = board.get(p), 
				to_piece = (m.x != INFINITY_NUM && m.x != -INFINITY_NUM) ? board.get(m) : empty;
			Coordinate warnedPosition(board.warnedPosition);
			bool castleBL = board.castleBL, castleBR = board.castleBR, castleWL = board.castleWL, castleWR = board.castleWR;
			
			movePieceCalcPromotion(board, p, m, depth, white, black);

			int p_x = p.x, p_y = p.y;
			Coordinate *castle_rook = nullptr;
			if(m.x == INFINITY_NUM){
				p.x = 6;
				for(auto &pos: pieces){
					if(pos.x == 7 && pos.y == p.y){
						pos.x = 5;
						castle_rook = &pos;
						break;
					}
				}
			} else if(m.x == -INFINITY_NUM){
				p.x = 2;
				for(auto &pos: pieces){
					if(pos.x == 0 && pos.y == p.y){
						pos.x = 3;
						castle_rook = &pos;
						break;
					}
				}
			} else {
				just_moved = board.get(m);
				p.set(m.x, m.y);
			}

			int val = -negamax(board, depth - 1, -INFINITY_NUM, -alpha, -color_coeff, white, black);

			//restore backup
			if(m.x == INFINITY_NUM){
				p.x = 4;
				castle_rook->x = 7;

				board.state[4][p.y] = color_coeff==1? king_w:king_b;
				board.state[5][p.y] = empty;
				board.state[6][p.y] = empty;
				board.state[7][p.y] = color_coeff==1? rook_w:rook_b;
			} else if(m.x == -INFINITY_NUM){
				p.x = 4;
				castle_rook->x = 0;

				board.state[0][p.y] = color_coeff==1? rook_w:rook_b;
				board.state[1][p.y] = empty;
				board.state[2][p.y] = empty;
				board.state[3][p.y] = empty;
				board.state[4][p.y] = color_coeff==1? king_w:king_b;
			} else {
				p.set(p_x, p_y);
				board.state[p.x][p.y] = from_piece;
				board.state[m.x][m.y] = to_piece;
			}
			board.warnedPosition.set(warnedPosition.x, warnedPosition.y);
			board.castleBL = castleBL;
			board.castleBR = castleBR;
			board.castleWL = castleWL;
			board.castleWR = castleWR;

			if (val > max_val){
				max_val = val;
				move_from.set(p.x, p.y);
				move_to.set(m.x, m.y);
				updated_piece = just_moved;		//promotion
			}
			alpha = max(alpha, max_val);
		}
	}
	return updated_piece;
}
