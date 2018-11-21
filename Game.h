#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "SDL.h"

//#include "ConfigFileInterface.h"
//#include "GameMap.h"
#include "Model.h"
#include "View.h"
#include "Control.h"

class Game
{
public:
	Game(const std::string& configfilename, bool boxymode);
	~Game();

	// Main game loop is here
	void Run();
	size_t AddActor(const Actors::PositionData& pd, const Actors::MovementData& md, const Actors::ActorType at, const std::bitset<32> attrib);
		
protected:
	void SetupRootWindow();
	SDL_Surface* GetWindowSurface();
	bool IsRunning();
	void HandleEvent(SDL_Event& e);
	void Update();

private:

	// Window we render to
	SDL_Rect screen_rect;
	SDL_Window* window;

	// File containing many configuration items for startup
	//ConfigFile::ConfigFileInterface cfi;
	
	// Fall out of main loop if done
	bool running;

	Model m_model;
	PlayerControl m_pc;
	View m_view;

	//std::map<int, std::vector<Control*>> m_eventmap;
	//GameMap* m_gmap;

	// Helper functions
	void TempSetup();
};
