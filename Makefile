CFLAGS = -g -std=c++14
INCLUDES = -I.
OBJS = gui/imgui.o gui/imgui_draw.o gui/imgui-sfml.o tinyxml2.o SimplexNoise.o TextureManager.o AnimationHandler.o bfr.o

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

all: $(OBJS)
	g++ $(OBJS) $(CFLAGS) -o bfr -lsfml-graphics -lsfml-window -lsfml-system -lglut -lGL

clean:
	rm -fr gui/*.o
	rm -fr *.o
	rm bfr