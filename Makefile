CFLAGS = -g -std=c++14 -DWITHGPERFTOOLS
INCLUDES = -I.
THIRD_PARTY_OBJS = third_party/imgui/imgui.o third_party/imgui/imgui_draw.o third_party/imgui/imgui-sfml.o third_party/imgui/ImguiWindowsFileIO.o third_party/Particles/ParticleData.o third_party/Particles/ParticleSpawner.o third_party/Particles/ParticleUpdater.o third_party/Particles/ParticleGenerator.o third_party/Particles/ParticleSystem.o third_party/dbscan/dbscan.o third_party/tinyxml2.o third_party/SimplexNoise.o 
OBJS = Options.o Helpers.o Map.o \
Stages/MainMenu.o Stages/PlayMenu.o Stages/GameOver.o \
Systems/GameSystem.o \
Systems/ConstructionSystem.o Systems/DrawMapSystem.o Systems/TileAnimSystem.o Systems/CombatSystem.o \
Systems/FxSystem.o Systems/GameGeneratorSystem.o Systems/MinimapSystem.o \
Systems/MapLayersSystem.o Systems/PathfindingSystem.o Systems/SteeringSystem.o \
Systems/ResourcesSystem.o Systems/SoundSystem.o Systems/VictorySystem.o \
bfr.o

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ -c $<

bfr: $(THIRD_PARTY_OBJS) $(OBJS)
	$(CXX) $(THIRD_PARTY_OBJS) $(OBJS) $(CFLAGS) -o bfr -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL -lprofiler

all: bfr

particle_editor: $(THIRD_PARTY_OBJS) ShaderOptions.o
	$(CXX) $(CFLAGS) -I. $(THIRD_PARTY_OBJS) tools/imgui-custom.cpp ShaderOptions.o tools/particles_editor.cpp -o tools/particles_editor -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

particle_demo: $(THIRD_PARTY_OBJS) ShaderOptions.o
	$(CXX) $(CFLAGS) -I. $(THIRD_PARTY_OBJS) tests/imgui-custom.cpp ShaderOptions.o tests/demo.cpp -o tests/demo -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

steering_test: Helpers.o
	$(CXX) $(CFLAGS) $(INCLUDES) Helpers.o tests/steering.cpp -o tests/steering -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lglut -lGL

hashedstring_test:
	$(CXX) $(CFLAGS) $(INCLUDES) tests/hashedstring.cpp -o tests/hashedstring

prof:
	google-pprof --callgrind ./bfr ./bfr_prof.log > profile.callgrind

clean:
	rm -fr Particles/*.o
	rm -fr third_party/*.o
	rm -fr third_party/*/*.o
	rm -fr Stages/*.o
	rm -fr Systems/*.o
	rm -fr *.o
	rm -f bfr
	rm -f tests/demo