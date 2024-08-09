#
# 
#
# Instructions:
#
# 1. Add .cc files (not .h files) to the line beginning with SRCS = main.cc.
#    Files should be separated with spaces, not commas.  If all of the
#    .cc files in the directory should be included in the build, then you
#    can avoid maintaining a specific list of source files by commenting
#    out the first SRCS line and uncommenting the second (as done below).
# 2. Type 'make depend' to build or update the dependency list.  Do this
#    whenever you add this Makefile or add a .cc or .h file to a project.
# 3. Type 'make' to build the project.
# 4. Type 'make clean' to start over or prepare for submission.  This does
#    not remove the dependency information.
#
# N.B. If you're using files with .cpp extensions, then you need to search
# and replace cc with cpp.
#

CC = g++
#SRCS = main.cc
SRCS = ${wildcard *.cc}
OBJS = ${SRCS:.cc=.o}
INCLS = ${SRCS:.cc=.h}


build: $(OBJS)
	g++ -c main.cc -I/opt/homebrew/Cellar/sfml/2.6.1/include    
	g++ main.o -o chess -L/opt/homebrew/Cellar/sfml/2.6.1/lib  -lsfml-graphics -lsfml-window -lsfml-system

clean: $(OBJS)
	rm *.o chess