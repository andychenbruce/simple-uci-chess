#include <vector>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char UInt8;
enum {
  BP = 'p',
  BR = 'r',
  BN = 'n',
  BB = 'b',
  BQ = 'q',
  BK = 'k',
  
  WP = 'P',
  WR = 'R',
  WN = 'N',
  WB = 'B',
  WK = 'K',
  WQ = 'Q',

  EMPTY = '\0'
};

#define __ EMPTY
static const UInt8 startingBoard[64] = {
				  BR, BN, BB, BQ, BK, BB, BN, BR,
				  BP, BP, BP, BP, BP, BP, BP, BP,
				  __, __, __, __, __, __, __, __,
				  __, __, __, __, __, __, __, __,
				  __, __, __, __, __, __, __, __,
				  __, __, __, __, __, __, __, __,
				  WP, WP, WP, WP, WP, WP, WP, WP,
				  WR, WN, WB, WQ, WK, WB, WN, WR
};
#undef __

const int BoxSize = 150;

const int dirLeft = -1;
const int dirRight = 1;
const int dirUp = -8;
const int dirDown = 8;
const int dirUpLeft = dirUp+dirLeft;
const int dirUpRight = dirUp+dirRight;
const int dirDownLeft = dirDown+dirLeft;
const int dirDownRight = dirDown+dirRight;

const int directions[8] = {dirLeft, dirRight, dirUp, dirDown, dirUpLeft, dirUpRight, dirDownLeft, dirDownRight};

const int actualDirections[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};



class boardState
{
public:
  UInt8 board[64];
  bool isWhitesTurn;
  bool whiteCanCastleQueenSide;
  bool whiteCanCastleKingSide;
  bool blackCanCastleQueenSide;
  bool blackCanCastleKingSide;
  int  enPassantPos;
  int halfMoves;
  int fullMoves;

  boardState(void);

  bool isEmpty(int pos);
  bool isPawn(int pos);
  bool isRook(int pos);
  bool isKnight(int pos);
  bool isBishop(int pos);
  bool isQueen(int pos);
  bool isKing(int pos);
  bool isBlack(int pos);
  bool isWhite(int pos);
  bool isAttackable(int pos);
  char* convertBoardToFen(void);
};

class chessGame
{
public:
  boardState currentState;

  static const int maxBackup = 300;
  boardState backupInfo[maxBackup];
  int farthestBackup = 0;
  int currentBackup = 0;

  chessGame(void);
  
  void undo();
  void redo();
  void backupCurrentState(void);
  
  static void forceMove(move forcedMove, boardState* state);
  static std::vector<move> generatePseudoLegalMoves(boardState* state);
  static bool isInCheck(boardState* state, bool isWhite);
  static std::vector<move> generateLegalMoves(boardState* state);
  
  bool attemptMove(move attemptThisMove);
};


int nodeTree(int depth, boardState state);
int nodeTest(int depth, boardState state);
void generateDistanceToEdge(void);
