# Makefile for Hex-a-hop, Copyright Oliver Pearce 2004
NAME		= hexahop.gpe
VERSION		= 1.0.0
CC		= arm-linux-gcc
CXXFLAGS		+= -D_VERSION=\"$(VERSION)\" -g -D_GP2X
GCC = arm-linux-g++
CXXSOURCES	= gfx.cpp hex_puzzzle.cpp
#INCLUDES	= 


#############################################################

OBJS=$(CXXSOURCES:.cpp=.o)

%.o	: %.cpp
	$(GCC) -static $(CXXFLAGS)  `sdl-config --cflags`  -c -o $@ $<	

$(NAME) : $(OBJS)
		$(GCC) -static -D_GP2X $(CXXFLAGS) $(OBJS)  `sdl-config --libs` -lm  \
		-o $(NAME)
		arm-linux-strip $(NAME)

clean :
	rm -f *~ $(OBJS) $(NAME)
