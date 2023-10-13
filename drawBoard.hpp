#include <stdio.h>
#include <cassert>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <errno.h>

const int ENUM_chessBoard = 20;
const int ENUM_piece_whitePawn = 11;
const int ENUM_piece_whiteBishop = 10;
const int ENUM_piece_whiteKnight = 9;
const int ENUM_piece_whiteRook = 8;
const int ENUM_piece_whiteKing = 7;
const int ENUM_piece_whiteQueen = 6;
const int ENUM_piece_blackPawn = 5;
const int ENUM_piece_blackBishop = 4;
const int ENUM_piece_blackKnight = 3;
const int ENUM_piece_blackRook = 2;
const int ENUM_piece_blackKing = 1;
const int ENUM_piece_blackQueen = 0;

const int ENUM_undoKeyPressed = 0;
const int ENUM_redoKeyPressed = 1;
const int ENUM_setMousePromotion_Queen = 2;
const int ENUM_setMousePromotion_Rook = 3;
const int ENUM_setMousePromotion_Bishop = 4;
const int ENUM_setMousePromotion_Knight = 5;


class myOpenGLWindow{
public:
  bool endGui = false;

  static int windowWidth;
  static int windowHeight;

  static bool mouseDown;
  static double mouseX;
  static double mouseY;

  static int clickedBoardPosX;
  static int clickedBoardPosY;

  static int releasedBoardPosX;
  static int releasedBoardPosY;

  static bool hasMouseData;
  static bool hasKeyboardCommand;

  static int keyboardCommand;


  static glm::mat4 perspectiveMatrix;
  GLuint inputPosAttributeLocation;
  GLuint MVPUniformLocation;
  GLuint objEnumUniformLocation;
  GLFWwindow* mainWindow;

  GLuint shaderProgramID = -1;

  int numTexturesInitialized = 0;
  unsigned int* textureIDs = NULL;
  GLint* textureUniformsLocs;
  
  GLuint vaoThingID;
  GLuint vboThingID;

  static void glfwCallback_error(int error, const char* desc);
  static void glfwCallback_keyboardClicked(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void glfwCallback_mouseMoved(GLFWwindow* window, double xpos, double ypos);
  static void glfwCallback_mouseClicked(GLFWwindow* window, int button, int action, int mods);
  static void glfwCallback_windowSizeChanged(GLFWwindow* window, int width, int height);

  void setInputCallbacks(void);


  void drawSquare(int ENUM, glm::vec3 pos, glm::vec3 size);
  void drawThickLine(int ENUM, glm::vec3 start, glm::vec3 end, double thickness);
  void drawLine(int ENUM, glm::vec3 start, glm::vec3 end);

  static void screenPosToBoardPos(double screenX, double screenY, int* boardX, int* boardY);
  static void setOrthoRatioCorrect(void);
  
  void enableTransparency(void);
  GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
  void initGLFW(void);
  void initBuffers(void);
  void initTextures(int numTextures);
  void loadTexture(const char* filename, int textureUnit_num);
  unsigned char* readTexture(const char* filename, int* width, int* height, int* numChannels);
  GLint getProgramTextureUniformLoc(int textureNum);

  void drawStuffOpenGL(chessGame* currentGame, engine* engine1, engine* engine2);
  void loopStuffGLFW(double input_timeout);

  void drawEngineInfo(engine* engineToDraw);
  
  void closeGLFW(void);

  int pieceToEnum(char piece);

  myOpenGLWindow(void);
};
