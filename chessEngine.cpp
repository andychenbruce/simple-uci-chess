#include "move.hpp"
#include "chessLogic.hpp"
#include "chessEngine.hpp"

bool
waitForData(int fd, int timeoutMilliseconds){
  pollfd thingy;
  thingy.fd = fd;
  thingy.events = POLLIN;
  thingy.revents = 0;
  
  if(poll(&thingy, 1, timeoutMilliseconds) < 0){
    printf("poll failed on pipe: %s", strerror(errno));
    exit(1);
  }
  return((thingy.revents & POLLIN) != 0);
}

void
readMoveIntoBuffer(char* movesString, char buffer[5]){
  char char1 = movesString[0];
  char char2 = movesString[1];
  char char3 = movesString[2];
  char char4 = movesString[3];
  char char5 = movesString[4];

  char promotion = char5;
  if(!isalpha(promotion)){
    promotion = '\0';
  }

  buffer[0] = char1;
  buffer[1] = char2;
  buffer[2] = char3;
  buffer[3] = char4;
  buffer[4] = promotion;
  if(!(isalpha(char1)&&isdigit(char2)&&isalpha(char3)&&isdigit(char4))){
    printf("move is not in format letter-digit-letter-digit");
    exit(1);
  }
}
  
int
readStringOfMovesIntoBuffer(char* movesString, char buffer[][5], int maxMoves){
  int numRead = 0;
  while(true){
    char char1 = movesString[0];
    char char2 = movesString[1];
    char char3 = movesString[2];
    char char4 = movesString[3];
    char char5 = movesString[4];

    char promotion = char5;
    if(!isalpha(promotion)){
      promotion = '\0';
      movesString += sizeof("a0a0 ")-1;
    }else{
      movesString += sizeof("a0a0q ")-1;
    }

    buffer[numRead][0] = char1;
    buffer[numRead][1] = char2;
    buffer[numRead][2] = char3;
    buffer[numRead][3] = char4;
    buffer[numRead][4] = promotion;
    numRead++;
    if(numRead >= maxMoves){
      break;
    }
    if(!(isalpha(char1)&&isdigit(char2)&&isalpha(char3)&&isdigit(char4))){
      break;
    }
    if(char5 == '\0'){
      break;
    }
  }
  return numRead;
}

void engine::resetEngineData(void){
  memset(bestmove, 0, sizeof(bestmove));
  bestmoveHasPonder = false;
  memset(bestmove_ponder, 0, sizeof(bestmove_ponder));;

  info_depth = 0;
  info_seldepth = 0;
  info_time = 0;
  info_nodes = 0;
  numPvMoves = 0;
  info_multipv = 0;
  info_score = 0;
  info_score_cp = false;
  info_score_mate = false;
  info_score_lowerbound = false;
  info_score_upperbound = false;
  memset(info_currmove, 0, sizeof(info_currmove));
  info_currmovenumber = 0;
  info_hashfull = 0;
  info_nps = 0;
  info_tbhits = 0;
  info_cpuload = 0;
  memset(info_string, 0, 0xff);
  numRefutationMoves = 0;
  currline_cpu = 0;
  numCurrlineMoves = 0;
}
  
void engine::printEngineData(void){
  printf("=========================================\n");
  printf("depth = %d\n", info_depth);
  printf("seldepth = %d\n", info_seldepth);
  printf("time = %d\n", info_time);
  printf("nodes = %d\n", info_nodes);
  printf("pv = [");
  for(int i = 0; i < numPvMoves; i++){
    printf("%c%c%c%c%c, ", info_pv[i][0], info_pv[i][1], info_pv[i][2], info_pv[i][3], info_pv[i][4]);
  }
  printf("]\n");
  printf("multipv = %d\n", info_multipv);
  printf("score = ");
  if(info_score_cp){
    printf("cp ");
  }
  if(info_score_mate){
    printf("mate ");
  }
  printf("%d ", info_score);
  if(info_score_lowerbound){
    printf("lowerbound");
  }
  if(info_score_upperbound){
    printf("upperbound");
  }
  printf("\n");
  printf("currmove = %c%c%c%c%c\n", info_currmove[0], info_currmove[1], info_currmove[2], info_currmove[3], info_currmove[4]);
  printf("currmovenumber = %d\n", info_currmovenumber);
  printf("hashfull = %d\n", info_hashfull);
  printf("nps = %d\n", info_nps);
  printf("tbhits = %d\n", info_tbhits);
  printf("cpuload = %d\n", info_cpuload);
  printf("refutation = [");
  for(int i = 0; i < numRefutationMoves; i++){
    printf("%c%c%c%c%c, ", info_refutation[i][0], info_refutation[i][1], info_refutation[i][2], info_refutation[i][3], info_refutation[i][4]);
  }
  printf("]\n");
  printf("currline = [");
  for(int i = 0; i < numCurrlineMoves; i++){
    printf("%c%c%c%c%c, ", info_currline[i][0], info_currline[i][1], info_currline[i][2], info_currline[i][3], info_currline[i][4]);
  }
  printf("]\n");
  printf("string = \"%s\"\n", info_string);
}
  
void
engine::writeToEngine(char* string)
{
  int length = strlen(string);
  int numWrote = write(enginePipeFDWrite, string, length);
  if(numWrote != length){
    printf("wrote %d out of %d\n", numWrote, length);
    printf("failed write: %s\n", strerror(errno));
    exit(1);
  }
}

void
engine::processInfoString(char* infoString){
  printEngineData();
  char* splitString;
    
  if((splitString = strstr(infoString, "string ")) != NULL){
    char* tmp = splitString;
    splitString += sizeof("string ")-1;
    memcpy(info_string, splitString, strlen(splitString)+1);
    tmp[0] = '\0';
  }
  if((splitString = strstr(infoString, "depth ")) != NULL){
    splitString += sizeof("depth ")-1;
    if(sscanf(splitString, "%d", &info_depth) != 1){
      printf("no number after depth\n");
      exit(1);
    }
  }
  if((splitString = strstr(infoString, "seldepth ")) != NULL){
    splitString += sizeof("seldepth ")-1;
    if(sscanf(splitString, "%d", &info_seldepth) != 1){
      printf("no number after seldepth\n");
      exit(1);
    }
  }
  if((splitString = strstr(infoString, "time ")) != NULL){
    splitString += sizeof("time ")-1;
    if(sscanf(splitString, "%d", &info_time) != 1){
      printf("no number after time\n");
      exit(1);
    }
  }
  if((splitString = strstr(infoString, "nodes ")) != NULL){
    splitString += sizeof("nodes ")-1;
    if(sscanf(splitString, "%d", &info_nodes) != 1){
      printf("no number after nodes\n");
      exit(1);
    }
  }
  char pvBuffer[maxPvMoves][5];
  if((splitString = strstr(infoString, " pv ")) != NULL){
    splitString += sizeof(" pv ")-1;
    numPvMoves = readStringOfMovesIntoBuffer(splitString, pvBuffer, maxPvMoves);
    memcpy(info_pv, pvBuffer, sizeof(pvBuffer));
  }
  if((splitString = strstr(infoString, "multipv ")) != NULL){
    splitString += sizeof("multipv ")-1;
    if(sscanf(splitString, "%d", &info_multipv) != 1){
      printf("no number after multipv\n");
      exit(1);
    }
  }
  if((splitString = strstr(infoString, "score ")) != NULL){
    splitString += sizeof("score")-1;
      
    char* tmp;
    if((tmp = strstr(splitString, " cp ")) != NULL){
      info_score_cp = true;
      splitString += sizeof(" cp ")-1;
      if(sscanf(splitString, "%d", &info_score) != 1){
	printf("no number after cp\n");
	exit(1);
      }
    }else{
      info_score_cp = false;
    }
    if((tmp = strstr(splitString, " mate ")) != NULL){
      assert(info_score_cp == false);
      info_score_mate = true;
      splitString += sizeof(" mate ")-1;
      if(sscanf(splitString, "%d", &info_score) != 1){
	printf("no number after mate\n");
	exit(1);
      }
    }else{
      assert(info_score_cp == true);
      info_score_mate = false;
    }
    info_score_lowerbound = (strstr(splitString, " lowerbound ") != NULL);
    info_score_upperbound = (strstr(splitString, " upperbound ") != NULL);
  }
  if((splitString = strstr(infoString, "currmove ")) != NULL){
    splitString += sizeof("currmove ")-1;
    readMoveIntoBuffer(splitString, info_currmove);
  }
  if((splitString = strstr(infoString, "currmovenumber ")) != NULL){
    splitString += sizeof("currmovenumber ")-1;
    if(sscanf(splitString, "%d", &info_currmovenumber) != 1){
      printf("no number after currmovenumber\n");
      exit(1);
    }
  }
  if((splitString = strstr(infoString, "hashfull ")) != NULL){
    splitString += sizeof("hashfull ")-1;
    if(sscanf(splitString, "%d", &info_hashfull) != 1){
      printf("no number after hashfull\n");
      exit(1);
    }
  }
  if((splitString = strstr(infoString, "nps ")) != NULL){
    splitString += sizeof("nps ")-1;
    if(sscanf(splitString, "%d", &info_nps) != 1){
      printf("no number after nps\n");
      exit(1);
    }
  }
  if((splitString = strstr(infoString, "tbhits ")) != NULL){
    splitString += sizeof("tbhits ")-1;
    if(sscanf(splitString, "%d", &info_tbhits) != 1){
      printf("no number after tbhits\n");
      exit(1);
    }
  }
  if((splitString = strstr(infoString, "cpuload ")) != NULL){
    splitString += sizeof("cpuload ")-1;
    if(sscanf(splitString, "%d", &info_cpuload) != 1){
      printf("no number after cpuload\n");
      exit(1);
    }
  }
  char refutationBuffer[maxRefutationMoves][5];
  if((splitString = strstr(infoString, "refutation ")) != NULL){
    splitString += sizeof("refutation ")-1;
    numRefutationMoves = readStringOfMovesIntoBuffer(splitString, refutationBuffer, maxRefutationMoves);
    memcpy(info_refutation, refutationBuffer, sizeof(refutationBuffer));
  }
  char currlineBuffer[maxCurrlineMoves][5];
  if((splitString = strstr(infoString, "currline ")) != NULL){
    splitString += sizeof("currline ")-1;
    if(sscanf(splitString, "%d", &currline_cpu) != 1){
	
    }else{
      int numberLengthChars = currline_cpu/10;
      if(currline_cpu < 0){
	numberLengthChars++;
      }
      splitString += sizeof(" ")-1 + numberLengthChars + sizeof(" ")-1;
    }
    numCurrlineMoves = readStringOfMovesIntoBuffer(splitString, currlineBuffer, maxCurrlineMoves);
    memcpy(info_currline, currlineBuffer, sizeof(currlineBuffer));
  }
}

void
engine::waitForString(const char* getString){
  char buffer[0xffff];
  int timeoutmilliseconds = 120000;
  while(true){
    int isDataTimeout = waitForData(enginePipeFDRead, timeoutmilliseconds);
    if(!isDataTimeout){
      printf("no data from engine in %d milleseconds\n", timeoutmilliseconds);
      exit(1);
    }
    int r = read(enginePipeFDRead, buffer, sizeof(buffer)-1);
    if (r < 0) {
      printf("Read failed on pipe: %s", strerror(errno));
      exit(1);
    }
    if(r == 0){
      printf("pipe somehow closed??\n");
      exit(1);
    }
    buffer[r] = '\0';  
    char* checkPos = &buffer[0];
    char* stringPos;
    while(true){
      if((stringPos = strstr(checkPos,  getString)) != NULL){
	return;
      }else{
	break;
      }
    }
  }
}
  
move
engine::getBestMove(boardState* state){
  resetEngineData();
  char writeCmd[0xff];
  sprintf(writeCmd, "isready\n");
  writeToEngine(writeCmd);
  waitForString("readyok");
    
  char* fen = state->convertBoardToFen();
  sprintf(writeCmd, "position fen %s\n", fen);
  free(fen);
  writeToEngine(writeCmd);
  sprintf(writeCmd, "go depth %d\n", 25);
  writeToEngine(writeCmd);
  char buffer[0xffff];
  int timeoutmilliseconds = 120000;
  while(true){
    int isDataTimeout = waitForData(enginePipeFDRead, timeoutmilliseconds);
    if(!isDataTimeout){
      printf("no data from engine in %d milleseconds\n", timeoutmilliseconds);
      exit(1);
    }
    int r = read(enginePipeFDRead, buffer, sizeof(buffer)-1);
    if (r < 0) {
      printf("Read failed on pipe: %s", strerror(errno));
      exit(1);
    }
    if(r == 0){
      printf("pipe somehow closed??\n");
      exit(1);
    }
    buffer[r] = '\0';  
    char* checkPos = &buffer[0];
    char* infoPos;
    while(true){
      if((infoPos = strstr(checkPos,  "info")) != NULL){
	checkPos = infoPos + 1;
	char* endLinePos;
	if((endLinePos = strstr(checkPos,  "\n")) != NULL){
	  int strlength = endLinePos-infoPos;
	  char* infoString = (char*)malloc(strlength+1);
	  if(infoString == NULL){exit(1);};

	  memcpy(infoString, infoPos, strlength);
	  infoString[strlength] = '\0';
	    
	  processInfoString(infoString);
	  free(infoString);
	}else{
	  printf("no newline after info string, not uci protocol\n");
	  printf("info stirng = \"%s\"\n", checkPos);
	  exit(1);
	}
      }else{
	break;
      }
    }

    char* bestmoveString;
    if((bestmoveString = strstr(checkPos,  "bestmove ")) != NULL){
      bestmoveString += sizeof("bestmove ")-1;
      readMoveIntoBuffer(bestmoveString, bestmove);
      printf("bestmove \"%c%c%c%c%c\" ", bestmove[0], bestmove[1], bestmove[2], bestmove[3], bestmove[4]);
      int fromX = bestmove[0]-'a';
      int fromY = 7 - (bestmove[1]-'1');
      int toX = bestmove[2]-'a';
      int toY = 7 - (bestmove[3]-'1');
	
      move output(fromX + (fromY*8), toX + (toY*8));
      if(isalpha(bestmove[4])){
	output.promotion = bestmove[4];
      }else{
	output.promotion = '\0';
      }
      char* ponderString;
      if((ponderString = strstr(bestmoveString,  "ponder ")) != NULL){
	ponderString += sizeof("ponder ")-1;
	bestmoveHasPonder = true;
	readMoveIntoBuffer(ponderString, bestmove_ponder);
	printf("ponder \"%c%c%c%c%c\"", bestmove_ponder[0], bestmove_ponder[1], bestmove_ponder[2], bestmove_ponder[3], bestmove_ponder[4]);
      }else{
	bestmoveHasPonder = false;
      }
	
      printf("\n");
      resetEngineData();
      return output;
    }
  }
}
  
engine::engine(const char* filepath){
  resetEngineData();
    
  int pipeFdRead[2];
  int pipeFdWrite[2];
    
  if(pipe(pipeFdRead) != 0) {
    printf("pipe() failed: %s", strerror(errno));
    exit(1);
  }
  if(pipe(pipeFdWrite) != 0) {
    printf("pipe() failed: %s", strerror(errno));
    exit(1);
  }
    
  switch (fork()) {
  case -1:
    printf("fork() failed: %s\n", strerror(errno));
    exit(1);
  case 0://Child
    dup2(pipeFdRead[1], 1);
    dup2(pipeFdWrite[0], 0);
    close(pipeFdRead[0]);//child close output of read pipe
    close(pipeFdWrite[1]);//child close input of write pipe
    execl(filepath, filepath, NULL);
    printf("Can't exec %s: %s\n", filepath, strerror(errno));
    exit(1);
  default://Parent
    close(pipeFdRead[1]);//parent close input of read pipe
    close(pipeFdWrite[0]);//parent close output of write pipe
    break;
  }
  enginePipeFDRead = pipeFdRead[0];//output of read pipe;
  enginePipeFDWrite = pipeFdWrite[1];//input of write pipe

  char writeBuf[0xff];
  sprintf(writeBuf, "uci\n");
  writeToEngine(writeBuf);
  waitForString("uciok");
  sprintf(writeBuf, "setoption name Threads value 16\n");
  writeToEngine(writeBuf);
}
