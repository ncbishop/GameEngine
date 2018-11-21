#include <iostream>

#define SDL_MAIN_HANDLED

#include "SDL.h"
#include "Game.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

int main(int argc, char** argv)
{

	//auto console = spdlog::stdout_color_mt("EngineLogger");
	auto console = spdlog::basic_logger_mt("EngineLogger", "logs/basic-log.txt");

	// Set message level to include debugs
	console->set_level(spdlog::level::trace);

	// Uncomment to limit level to info messages
	//console->set_level(spdlog::level::info);

	console->info("Main engine start...");

	// Instance of Game
	Game::Game g(std::string("configfile.txt"), true);

	// Set up main loop
	console->info("Begin event loop");

	g.Run();
	
	return 0;
}