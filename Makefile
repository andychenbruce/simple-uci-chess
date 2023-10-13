CC :=	 g++
CFLAGS :=	-Wall
LFLAGS :=	$(shell pkg-config --cflags --libs glew glfw3)

SRCS :=	main.cpp
SRCS += chessEngine.cpp
SRCS += chessLogic.cpp
SRCS += drawBoard.cpp
SRCS += move.cpp
SRCS += utils.cpp

OBJS :=	main.o
OBJS +=	chessLogic.o
OBJS +=	chessEngine.o
OBJS += drawBoard.o
OBJS +=	move.o
OBJS += utils.o


MAIN_HDRS :=		move.hpp chessLogic.hpp chessEngine.hpp drawBoard.hpp
CHESSLOGIC_HDRS :=	chessLogic.hpp move.hpp
CHESSENGINE_HDRS :=	chessEngine.hpp chessLogic.hpp move.hpp
DRAWBOARD_HDRS :=	move.hpp drawBoard.hpp chessLogic.hpp chessEngine.hpp stb_image.h
MOVE_HDRS :=		move.hpp
UTILS_HDRS :=		utils.hpp


foo:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LFLAGS) -o $(@)

main.o:		main.cpp $(MAIN_HDRS)
chessLogic.o:	chessLogic.cpp $(CHESSLOGIC_HDRS)
chessEngine.o:	chessEngine.cpp $(CHESSENGINE_HDRS)
drawBoard.o:	drawBoard.cpp $(DRAWBOARD_HDRS)
move.o:		move.cpp $(MOVE_HDRS)
utils.o:	utils.cpp $(UTILS_HDRS)

clean:
	rm *~ *.o foo
