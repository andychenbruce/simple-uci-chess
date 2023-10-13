#include "move.hpp"
#include "chessLogic.hpp"
#include "chessEngine.hpp"
#include "drawBoard.hpp"
#include "utils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int myOpenGLWindow::windowWidth;
int myOpenGLWindow::windowHeight;
bool myOpenGLWindow::mouseDown = false;
double myOpenGLWindow::mouseX = 0;
double myOpenGLWindow::mouseY = 0;
int myOpenGLWindow::clickedBoardPosX = 0;
int myOpenGLWindow::clickedBoardPosY = 0;
int myOpenGLWindow::releasedBoardPosX = 0;
int myOpenGLWindow::releasedBoardPosY = 0;
bool myOpenGLWindow::hasMouseData = false;
bool myOpenGLWindow::hasKeyboardCommand = false;
int myOpenGLWindow::keyboardCommand;
glm::mat4 myOpenGLWindow::perspectiveMatrix;

static const struct vertData{
  const glm::vec3 square[6] = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
  };
  const glm::vec3 line[2] = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
  };
} dataToGPU;


void myOpenGLWindow::glfwCallback_error(int error, const char* desc){
  printf("glfw err: %d :\"%s\"\n", error, desc);
}

void myOpenGLWindow::glfwCallback_keyboardClicked(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(action == GLFW_PRESS){
    switch(key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      break;
    case GLFW_KEY_LEFT_BRACKET:
      hasKeyboardCommand = true;
      keyboardCommand = ENUM_undoKeyPressed;
      break;
    case GLFW_KEY_RIGHT_BRACKET:
      hasKeyboardCommand = true;
      keyboardCommand = ENUM_redoKeyPressed;
      break;
    case GLFW_KEY_Q:
      hasKeyboardCommand = true;
      keyboardCommand = ENUM_setMousePromotion_Queen;
      break;
    case GLFW_KEY_R:
      hasKeyboardCommand = true;
      keyboardCommand = ENUM_setMousePromotion_Rook;
      break;
    case GLFW_KEY_B:
      hasKeyboardCommand = true;
      keyboardCommand = ENUM_setMousePromotion_Bishop;
      break;
    case GLFW_KEY_N:
      hasKeyboardCommand = true;
      keyboardCommand = ENUM_setMousePromotion_Knight;
      break;
    default:
      break;
    }
  }
}

void myOpenGLWindow::glfwCallback_mouseMoved(GLFWwindow* window, double xpos, double ypos)
{
  mouseX = xpos;
  mouseY = ypos;
}

void myOpenGLWindow::glfwCallback_mouseClicked(GLFWwindow* window, int button, int action, int mods)
{
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    screenPosToBoardPos(mouseX, mouseY, &clickedBoardPosX, &clickedBoardPosY);
    printf("mouse pressed on square %d, %d\n", clickedBoardPosX, clickedBoardPosY);
    mouseDown = true;
  }
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    if(mouseDown){
      screenPosToBoardPos(mouseX, mouseY, &releasedBoardPosX, &releasedBoardPosY);
      printf("mouse released on square %d, %d\n", releasedBoardPosX, releasedBoardPosY);
      hasMouseData = true;
    }
    mouseDown = false;
  }
}

void myOpenGLWindow::glfwCallback_windowSizeChanged(GLFWwindow* window, int width, int height){
  windowWidth = width;
  windowHeight = height;
  glViewport(0, 0, windowWidth, windowHeight);
  setOrthoRatioCorrect();
}

void myOpenGLWindow::screenPosToBoardPos(double screenX, double screenY, int* boardX, int* boardY){
  int squareSide = 0;
  if(windowWidth > windowHeight){
    double diff = windowWidth-windowHeight;
    screenX -= (diff/2.0);
    squareSide = windowHeight;
  }else{
    double diff = windowHeight-windowWidth;
    screenY -= (diff/2.0);
    squareSide = windowWidth;
  }
  screenX /= squareSide;
  screenY /= squareSide;
  screenX *= 8;
  screenY *= 8;
  
  *boardX = floor(screenX);
  *boardY = floor(screenY);
}


void
myOpenGLWindow::setOrthoRatioCorrect(void)
{
  if(windowWidth > windowHeight){
    double ratio = windowWidth*(1.0/windowHeight);
    double diff = ratio-1;
    perspectiveMatrix = glm::ortho(0.0-(diff/2.0), 1.0+(diff/2), 0.0, 1.0, -100.0, 100.0);
  }else{
    double ratio = windowHeight*(1.0/windowWidth);
    double diff = ratio-1;
    perspectiveMatrix = glm::ortho(0.0, 1.0, 0.0-(diff/2), 1.0+(diff/2), -100.0, 100.0);
  }
}

GLuint myOpenGLWindow::LoadShaders(const char * vertex_file_path,const char * fragment_file_path){//TODO: SPLIT UP THIS BIG FUNCTION
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  char* vertexShaderString = readFileIntoString(vertex_file_path);
  char* fragmentShaderString = readFileIntoString(fragment_file_path);
  
  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  glShaderSource(VertexShaderID, 1, &vertexShaderString , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if(InfoLogLength > 0){
    char* VertexShaderErrorMessage = (char*)malloc(InfoLogLength+1);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    printf("%s\n", VertexShaderErrorMessage);
    free(VertexShaderErrorMessage);
  }

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  glShaderSource(FragmentShaderID, 1, &fragmentShaderString, NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    char* FragmentShaderErrorMessage = (char*)malloc(InfoLogLength+1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, FragmentShaderErrorMessage);
    printf("%s\n", FragmentShaderErrorMessage);
  }

  // Link the program
  printf("Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    char* ProgramErrorMessage = (char*)malloc(InfoLogLength+1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage);
    printf("%s\n", ProgramErrorMessage);
  }
  
  glDetachShader(ProgramID, VertexShaderID);
  glDetachShader(ProgramID, FragmentShaderID);
  
  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}



void
myOpenGLWindow::drawSquare(int ENUM, glm::vec3 pos, glm::vec3 size){
  glUniform1i(objEnumUniformLocation, ENUM);

  glm::mat4 viewMatrix = glm::mat4(1);
  glm::mat4 modelMatrix = glm::mat4(1);
  modelMatrix = glm::translate(modelMatrix, pos);
  modelMatrix = glm::scale(modelMatrix, size);

  glm::mat4 MVP = perspectiveMatrix*viewMatrix*modelMatrix;

  glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &(MVP[0][0]));
  
  glEnableVertexAttribArray(inputPosAttributeLocation);
  glVertexAttribPointer(inputPosAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  int offset = offsetof(vertData, square)/sizeof(glm::vec3);
    
  glDrawArrays(GL_TRIANGLES, offset, 6);
  glDisableVertexAttribArray(inputPosAttributeLocation);
}

void
myOpenGLWindow::drawThickLine(int ENUM, glm::vec3 start, glm::vec3 end, double thickness){
  glUniform1i(objEnumUniformLocation, ENUM);

  glm::mat4 viewMatrix = glm::mat4(1);
  glm::mat4 modelMatrix = glm::mat4(1);
  
  glm::vec3 diff = end-start;
  
  modelMatrix = glm::translate(modelMatrix, start);

  modelMatrix = glm::rotate(modelMatrix, atan2(diff.y, diff.x), glm::vec3(0, 0, 1));
  
  modelMatrix = glm::scale(modelMatrix, glm::vec3(glm::length(diff), 1, 1));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(1, thickness, 1));
  modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.5, 0));
  
  glm::mat4 MVP = perspectiveMatrix*viewMatrix*modelMatrix;
  
  glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &(MVP[0][0]));
  
  glEnableVertexAttribArray(inputPosAttributeLocation);
  glVertexAttribPointer(inputPosAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  int offset = offsetof(vertData, square)/sizeof(glm::vec3);
    
  glDrawArrays(GL_TRIANGLES, offset, 6);
  glDisableVertexAttribArray(inputPosAttributeLocation);
}

void
myOpenGLWindow::drawLine(int ENUM, glm::vec3 start, glm::vec3 end){
  glUniform1i(objEnumUniformLocation, ENUM);

  glm::mat4 viewMatrix = glm::mat4(1);
  glm::mat4 modelMatrix = glm::mat4(1);
  modelMatrix = glm::translate(modelMatrix, start);
  modelMatrix = glm::scale(modelMatrix, end-start);

  glm::mat4 MVP = perspectiveMatrix*viewMatrix*modelMatrix;

  glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &(MVP[0][0]));
  
  glEnableVertexAttribArray(inputPosAttributeLocation);
  glVertexAttribPointer(inputPosAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  int offset = offsetof(vertData, line)/sizeof(glm::vec3);;
    
  glDrawArrays(GL_LINE_LOOP, offset, 2);
  glDisableVertexAttribArray(inputPosAttributeLocation);
}

void
myOpenGLWindow::setInputCallbacks(void)
{
  glfwSetKeyCallback(mainWindow, myOpenGLWindow::glfwCallback_keyboardClicked);
  glfwSetFramebufferSizeCallback(mainWindow, myOpenGLWindow::glfwCallback_windowSizeChanged);
  glfwSetCursorPosCallback(mainWindow, myOpenGLWindow::glfwCallback_mouseMoved);
  glfwSetMouseButtonCallback(mainWindow, myOpenGLWindow::glfwCallback_mouseClicked);
}

void
myOpenGLWindow::initGLFW(void)
{
  glewExperimental = true;
  if(!glfwInit()){
    printf("glfw failed to init\n");
    assert(false);
  }
  glfwSetErrorCallback(glfwCallback_error);
  
  //3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  mainWindow = glfwCreateWindow(640, 480, "poopwindow", NULL, NULL);
  glfwGetFramebufferSize(mainWindow, &windowWidth, &windowHeight);
  glViewport(0, 0, windowWidth, windowHeight);
  if(!mainWindow){
    printf("window failed to init\n");
    assert(false);
  }

  setInputCallbacks();
  
  glfwMakeContextCurrent(mainWindow);
  glfwSwapInterval(1);
  if(glewInit() != GLEW_OK){
    printf("glew failed to init\n");
    assert(false);
  }
  printf("GLFW and GLEW init done\n");
}

void
myOpenGLWindow::closeGLFW(void)
{
  glfwDestroyWindow(mainWindow);
  glfwTerminate();
}

void
myOpenGLWindow::loopStuffGLFW(double input_timeout)
{
  glfwSwapBuffers(mainWindow);
  glfwWaitEventsTimeout(input_timeout);//so only continues when cursor moves and stuff
  //glfwPollEvents();//basically timeout 0
}

unsigned char*
myOpenGLWindow::readTexture(const char* filename, int* width, int* height, int* numChannels)
{
  unsigned char *data = stbi_load(filename, width, height, numChannels, 0);
  return data;

  if(data == NULL){
    printf("texture \"%s\" failed to load\n", filename);
    exit(1);
  }
}

GLint
myOpenGLWindow::getProgramTextureUniformLoc(int textureNum){
  char uniformName[0x100];

  sprintf(uniformName, "texture_%d", textureNum);
  
  int textureLoc = glGetUniformLocation(shaderProgramID, uniformName);
  printf("texLoc = %d\n", textureLoc);
  if(textureLoc == -1){
    printf("unable to find uniform %s in shader\n", uniformName);
    exit(1);
  }

  return textureLoc;
}

void
myOpenGLWindow::initTextures(int numTextures){
  if(textureIDs != NULL){
    printf("textures already initialized\n");
    exit(1);
  }
  numTexturesInitialized = numTextures;
  textureIDs = (unsigned int*)andyMalloc(numTextures*sizeof(unsigned int));
  glGenTextures(numTextures, textureIDs);

  textureUniformsLocs = (GLint*)andyMalloc(numTextures*sizeof(GLint));
  for(int i = 0; i < numTextures; i++){
    textureUniformsLocs[i] = getProgramTextureUniformLoc(i);
  }
}

void
myOpenGLWindow::loadTexture(const char* filename, int textureUnit_num)
{
  if(textureIDs == NULL){
    printf("you must initialize textures before loading\n");
    exit(1);
  }
  if(textureUnit_num >= numTexturesInitialized){
    printf("not enough texture ID's initialized: %d", numTexturesInitialized);
  }
  
  int width, height, numChannels;
  unsigned char* data = readTexture(filename, &width, &height, &numChannels);
  printf("texture file %s has w h c = %d, %d, %d\n", filename, width, height, numChannels);

  glActiveTexture(GL_TEXTURE0+textureUnit_num);
  glBindTexture(GL_TEXTURE_2D, textureIDs[textureUnit_num]);

  glUniform1i(textureUniformsLocs[textureUnit_num], textureUnit_num);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  
  GLenum pixelDataFormat;
  switch(numChannels){
  case 3:
    pixelDataFormat = GL_RGB;
    break;
  case 4:
    pixelDataFormat = GL_RGBA;
    break;
  default:
    printf("unknow amount of channels format: %d\n", numChannels);
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, pixelDataFormat, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  stbi_image_free(data);
  
}

void
myOpenGLWindow::enableTransparency(void){
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
myOpenGLWindow::initBuffers(void)
{
  glGenVertexArrays(1, &vaoThingID);
  glBindVertexArray(vaoThingID);
  
  glGenBuffers(1, &vboThingID);
  glBindBuffer(GL_ARRAY_BUFFER, vboThingID);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(dataToGPU), (void*)&dataToGPU, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, vboThingID);
  
  shaderProgramID = LoadShaders("shaders/vertShader.txt", "shaders/fragShader.txt");
  glUseProgram(shaderProgramID);
  inputPosAttributeLocation = glGetAttribLocation(shaderProgramID, "inputPosition");
  MVPUniformLocation = glGetUniformLocation(shaderProgramID, "MVP");
  objEnumUniformLocation = glGetUniformLocation(shaderProgramID, "objEnum");
  
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  setOrthoRatioCorrect();
  //perspectiveMatrix = glm::ortho(0, 1, 0, 1, -100, 100);

  enableTransparency();
  
  printf("openGl ready\n");
}



int
myOpenGLWindow::pieceToEnum(char piece){
  switch (piece){
  case 'r':
    return ENUM_piece_blackRook;
  case 'n':
    return ENUM_piece_blackKnight;
  case 'b':
    return ENUM_piece_blackBishop;
  case 'q':
    return ENUM_piece_blackQueen;
  case 'k':
    return ENUM_piece_blackKing;
  case 'p':
    return ENUM_piece_blackPawn;

  case 'R':
    return ENUM_piece_whiteRook;
  case 'N':
    return ENUM_piece_whiteKnight;
  case 'B':
    return ENUM_piece_whiteBishop;
  case 'Q':
    return ENUM_piece_whiteQueen;
  case 'K':
    return ENUM_piece_whiteKing;
  case 'P':
    return ENUM_piece_whitePawn;
    
  case EMPTY:
    return -1;

  default:
    assert(false);
  }
}

void
myOpenGLWindow::drawEngineInfo(engine* engineToDraw){
  for(int i = 0; i < engineToDraw->numPvMoves; i++){
    int x0 = engineToDraw->info_pv[i][0] - 'a';
    int y0 = engineToDraw->info_pv[i][1] - '1';
    int x1 = engineToDraw->info_pv[i][2] - 'a';
    int y1 = engineToDraw->info_pv[i][3] - '1';
    drawThickLine(-1, glm::vec3((x0+0.5)/8.0, (y0+0.5)/8.0, 0), glm::vec3((x1+0.5)/8.0, (y1+0.5)/8, 0), 0.002);
  }
}

void
myOpenGLWindow::drawStuffOpenGL(chessGame* currentGame, engine* engine1, engine* engine2)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glm::mat4 viewMatrix = glm::mat4(1);
  glm::mat4 modelMatrix = glm::mat4(1);
  //double time = glfwGetTime();
  //modelMatrix = glm::rotate(modelMatrix, (float)time, glm::vec3(0, 1, 0));
  
  glm::mat4 MVP = perspectiveMatrix*viewMatrix*modelMatrix;
  glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &(MVP[0][0]));

  //glBindBuffer(GL_ARRAY_BUFFER, vboThingID);//only redo if buffer changes (i think)
  drawSquare(ENUM_chessBoard, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
  
  for(int y = 0; y < 8; y++){
    for(int x = 0; x < 8; x++){
      int index = x+(y*8);
      int pieceEnum = pieceToEnum(currentGame->currentState.board[index]);
      if(pieceEnum != -1){
	drawSquare(pieceEnum, glm::vec3(0.125*x, 0.125*(7-y), 0), glm::vec3(0.125, 0.125, 0));
      }
    }
  }
  
  drawEngineInfo(engine1);
  drawEngineInfo(engine2);

  //drawThickLine(-1, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 0.1);
}


myOpenGLWindow::myOpenGLWindow(void){
}
