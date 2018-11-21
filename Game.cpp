#include <iostream>
#include <exception>
#include <sstream>
#include <memory>

#include "rapidxml-1.13\rapidxml_print.hpp"
#include "SDL_image.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "Game.h"
#include "ImageLibrary.h"
#include "ConfigFileInterface.h"

class RootWindow
{
public:
	RootWindow(const ConfigFile::ConfigObject& co);
		
	int width;
	int height;
	int xpos;
	int ypos;

	std::string title;
};

RootWindow::RootWindow(const ConfigFile::ConfigObject& co)
{
	auto logger = spdlog::get("EngineLogger");

	std::stringstream stream;
	stream << co.GetAttribute("width");
	stream >> width;

	// After the extraction, "stream" state will be "eof" which is an error condition
	// so clear it for reuse
	stream.clear();
	stream << co.GetAttribute("height");
	stream >> height;

	stream.clear();
	stream << co.GetAttribute("xpos");
	stream >> xpos;

	stream.clear();
	stream << co.GetAttribute("ypos");
	stream >> ypos;

	this->title = co.GetAttribute("title");
}

Game::Game(const std::string& configfilename, bool boxymode) : cfi(configfilename), window(nullptr), running(true)
{
	// create color multi threaded logger
	auto logger = spdlog::get("EngineLogger");
	logger->info("Game created with configfile: {0}", configfilename);

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		logger->error("Could not initialize SDL2! SDL_Error: {0}", SDL_GetError());
		throw std::exception("Could not initialize SDL2.");
	}

	// 
	this->SetupRootWindow();

	if (boxymode)
	{
		// TODO FIXME temp stuff 
		this->TempSetup();
	}
	
}

void Game::Run()
{
	SDL_Event e;
	while (this->IsRunning())
	{
		// Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			this->HandleEvent(e);
		}
		this->Update();
	}
}

void Game::SetupRootWindow()
{
	auto logger = spdlog::get("EngineLogger");
	
	// Create window based on parameters stored in config file
	// Get the root window dimensions and position
	auto rw_cop = this->cfi.GetConfigObject("rootwindow");
	logger->trace("Address of rootwindow pointer: {0}", static_cast<void*>(rw_cop));
	logger->debug("Name of object \"{0}\"", rw_cop->GetName());

	auto mf_cop = this->cfi.GetConfigObject("mapfiles");
	logger->trace("Address of mapfiles pointer: {0}", static_cast<void*>(mf_cop));
	logger->debug("Name of object \"{0}\"", mf_cop->GetName());

	auto il_cop = this->cfi.GetConfigObject("imagelib");
	logger->trace("Address of imagelib pointer: {0}", static_cast<void*>(il_cop));
	logger->debug("Name of object \"{0}\"", il_cop->GetName());


	if (rw_cop != nullptr)
	{
		// Use helper class RootWindow to parse config data provided by ConfigObject
		RootWindow rw(*rw_cop);
		this->screen_rect.x = 0;
		this->screen_rect.y = 0;
		this->screen_rect.w = rw.width;
		this->screen_rect.h = rw.height;
			
		this->window = SDL_CreateWindow(rw.title.c_str(), rw.xpos, rw.ypos, rw.width, rw.height, SDL_WINDOW_SHOWN);
		if (this->window == nullptr)
		{
			logger->error("Window could not be created! SDL_Error: {0}", SDL_GetError());
			throw std::exception("Could not create main window.");
		}
	}
	else
	{
		logger->error("Root window node not found in file: {0}", this->cfi.GetFilename());
		throw std::exception("Root window configuration not found.");
	}
}

void Game::TempSetup()
{
	auto logger = spdlog::get("EngineLogger");

	// Set up boxy sprite
	logger->debug("Loading sprite: {0}", "boxy.png");
	auto loaded = IMG_Load("boxy.png");
	auto boxy_sprite = SDL_ConvertSurface(loaded, this->GetWindowSurface()->format, NULL);
	SDL_FreeSurface(loaded);
	loaded = nullptr;

	SDL_Rect boxy_rect;
	boxy_rect.x = 0;
	boxy_rect.y = 0;
	boxy_rect.w = 32;
	boxy_rect.h = 32;

	SDLActor boxy(SDLActor::Type::Player, boxy_rect, boxy_sprite);
	m_model.AddActor(boxy);
			

	// Now background
	logger->debug("Load tiles for background: {0}\t{1}\t{2}", "bluetile.png", "browntile.png", "greentile.png");
	this->gmap = new GameMap();
	std::vector<std::string> image_files;
	image_files.push_back("bluetile.png");
	image_files.push_back("browntile.png");
	image_files.push_back("greentile.png");

	this->gmap->LoadTileImages(image_files);

	// Doing it for the logger outputs
	//this->gmap->LoadMap("basic_map.tmx");

	this->gmap->LoadTestMap(this->screen_rect.w / 32, this->screen_rect.h / 32);
}

Game::~Game()
{
	auto logger = spdlog::get("EngineLogger");
	logger->info("Game::Game destructor");

	delete this->gmap;
	this->gmap = nullptr;

	logger->debug("Images destroyed");

	if (window != nullptr)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	logger->debug("Window destroyed");

	SDL_Quit();
}

SDL_Surface* Game::GetWindowSurface()
{

	return SDL_GetWindowSurface(this->window);
}

bool Game::IsRunning()
{
	return this->running;
}

void Game::HandleEvent(SDL_Event & e)
{
	//std::cerr << "Handle event " << e.type << std::endl;
	// If the user wants to quit
	if (e.type == SDL_QUIT)
	{
		this->running = false;
	}
	else if (e.type == SDL_KEYDOWN)
	{
		m_pc.HandleEvent(e, m_model);
	}
	else
	{
		// PASS
	}
	return;
}

void Game::Update()
{
	auto logger = spdlog::get("EngineLogger");
	logger->info("Game::Update");

	SDL_Surface* screen = GetWindowSurface();
	// First place the map
	this->gmap->DrawTiles(screen);

	Data& data = m_model.GetState();
	// Find player
	auto filter = [](const SDLActor& a) -> bool { return a.GetType() == SDLActor::Type::Player; };
	auto player = std::find_if(data.begin(), data.end(), filter);
	if (player == data.end())
	{
		logger->warn("Game::Update with no player actor!");
		return;
	}

	SDL_Rect dest_rect = player->GetPosition();
	SDL_BlitSurface(player->GetSprite(), NULL, screen, &dest_rect);
		
	SDL_UpdateWindowSurface(this->window);
}
