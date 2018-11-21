#pragma once

#include <map>
#include <string>

#include "SDL.h"

class Image
{
public:
	Image(std::string filename, SDL_Surface* screen);
	~Image();

private:
	std::string filename;
	SDL_Surface* surface;
};

class ImageLibrary
{
public:
	ImageLibrary(std::string directory, SDL_Surface* screen);
	~ImageLibrary();

	SDL_Surface* GetImage(int id);
	
private:
	std::map<int, SDL_Surface*> image_by_id;
};
