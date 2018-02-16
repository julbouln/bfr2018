CFLAGS = -O3 -std=c++14
INCLUDES = -I.
THIRD_PARTY_OBJS = third_party/imgui/imgui.o third_party/imgui/imgui_draw.o third_party/imgui/imgui-sfml.o third_party/Particles/ParticleData.o third_party/Particles/ParticleSpawner.o third_party/Particles/ParticleUpdater.o third_party/Particles/ParticleGenerator.o third_party/Particles/ParticleSystem.o third_party/dbscan/dbscan.o third_party/tinyxml2.o third_party/SimplexNoise.o 
OBJS = ShaderOptions.o Helpers.o bfr.o

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ -c $<

bfr: $(THIRD_PARTY_OBJS) $(OBJS)
	$(CXX) $(THIRD_PARTY_OBJS) $(OBJS) $(CFLAGS) -o bfr -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

all: bfr

particle_demo: $(THIRD_PARTY_OBJS)
	$(CXX) $(CFLAGS) -I. $(THIRD_PARTY_OBJS) tests/imgui-custom.cpp tests/demo.cpp -o tests/demo -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

steering_test: Helpers.o
	$(CXX) $(CFLAGS) $(INCLUDES) Helpers.o tests/steering.cpp -o tests/steering -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

clean:
	rm -fr Particles/*.o
	rm -fr third_party/*.o
	rm -fr third_party/*/*.o
	rm -fr *.o
	rm -f bfr
	rm -f tests/demo