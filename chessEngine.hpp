#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

bool waitForData(int fd, int timeoutMilliseconds);
class engine {
public:
  int enginePipeFDRead;
  int enginePipeFDWrite;

  char bestmove[5];
  bool bestmoveHasPonder;
  char bestmove_ponder[5];

  int info_depth;

  int info_seldepth;
  int info_time;
  
  int info_nodes;

  static const int maxPvMoves = 0xff;
  int numPvMoves;
  char info_pv[maxPvMoves][5];
  int info_multipv;

  int info_score;
  bool info_score_cp;
  bool info_score_mate;
  bool info_score_lowerbound;
  bool info_score_upperbound;
  
  char info_currmove[5];
  int info_currmovenumber;

  int info_hashfull;

  int info_nps;
  int info_tbhits;

  int info_cpuload;

  char info_string[0xff];

  static const int maxRefutationMoves = 0xff;
  int numRefutationMoves;
  char info_refutation[maxRefutationMoves][5];

  
  int currline_cpu;
  static const int maxCurrlineMoves = 0xff;
  int numCurrlineMoves;
  char info_currline[maxCurrlineMoves][5];

  void resetEngineData(void);
  void printEngineData(void);
  void writeToEngine(char* string);
  void processInfoString(char* infoString);
  void waitForString(const char* getString);
  move getBestMove(boardState* state);

  engine(const char* filepath);
};
