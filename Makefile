CFLAGS = -g -std=c++14
INCLUDES = -I.
IMGUI_OBJS = gui/imgui.o gui/imgui_draw.o gui/imgui-sfml.o
PARTICLES_OBJS = Particles/ParticleData.o Particles/ParticleSpawner.o Particles/ParticleUpdater.o Particles/ParticleGenerator.o Particles/ParticleSystem.o
OBJS = tinyxml2.o SimplexNoise.o bfr.o

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ -c $<

bfr: $(OBJS) $(PARTICLES_OBJS) $(IMGUI_OBJS)
	$(CXX) $(OBJS) $(PARTICLES_OBJS) $(IMGUI_OBJS) $(CFLAGS) -o bfr -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

all: bfr

particle_demo: $(PARTICLES_OBJS) $(IMGUI_OBJS)
	$(CXX) $(CFLAGS) -I. $(PARTICLES_OBJS) $(IMGUI_OBJS) tests/imgui-custom.cpp tests/demo.cpp -o tests/demo -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

clean:
	rm -fr Particles/*.o
	rm -fr gui/*.o
	rm -fr *.o
	rm -f bfr
	rm -f tests/demo