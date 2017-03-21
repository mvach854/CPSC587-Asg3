CC=g++
OBJDIR=./obj
SRCDIR=./src

INCDIR=-I/usr/local/include -I/usr/include -I/usr/X11/inlcude
LIBDIR=-L/usr/X11R6/lib -L/usr/local/lib -L/usr/X11R6/lib64

CFLAGS=-c -std=c++0x -O3 -Wall
LIBS=\
	 -lglfw \
	 -lGLEW \
	 -lXi \
	 -lm \
	 -lGL \
	 -lstdc++
	 #-framework Cocoa \
	 #-framework OpenGL \
	 #-framework IOKit \
	#-framework CoreVideo

SOURCES=$(wildcard $(SRCDIR)/*cpp)
OBJECTS=$(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.cpp=.o)))
EXECUTABLE=MassSpringSim

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LINKFLAGS) $(OBJECTS) -o $@ $(LIBS) $(LIBDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $< -o $@ $(INCLDIR)

clean:
	rm $(OBJDIR)/*.o $(EXECUTABLE)
