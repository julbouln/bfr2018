CFLAGS = -g -std=c++14
INCLUDES = -I.
DBSCAN_OBJS = dbscan/dbscan.o
IMGUI_OBJS = gui/imgui.o gui/imgui_draw.o gui/imgui-sfml.o
PARTICLES_OBJS = Particles/ParticleData.o Particles/ParticleSpawner.o Particles/ParticleUpdater.o Particles/ParticleGenerator.o Particles/ParticleSystem.o
OBJS = third_party/tinyxml2.o third_party/SimplexNoise.o ShaderOptions.o Helpers.o bfr.o

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ -c $<

bfr: $(OBJS) $(PARTICLES_OBJS) $(IMGUI_OBJS) $(DBSCAN_OBJS)
	$(CXX) $(OBJS) $(PARTICLES_OBJS) $(IMGUI_OBJS) $(DBSCAN_OBJS) $(CFLAGS) -o bfr -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

all: bfr

particle_demo: $(PARTICLES_OBJS) $(IMGUI_OBJS)
	$(CXX) $(CFLAGS) -I. $(PARTICLES_OBJS) $(IMGUI_OBJS) tests/imgui-custom.cpp tests/demo.cpp -o tests/demo -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

steering_test: Helpers.o
	$(CXX) $(CFLAGS) $(INCLUDES) Helpers.o tests/steering.cpp -o tests/steering -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

clean:
	rm -fr Particles/*.o
	rm -fr Steering/*.o
	rm -fr gui/*.o
	rm -fr third_party/*.o
	rm -fr *.o
	rm -f bfr
	rm -f tests/demo