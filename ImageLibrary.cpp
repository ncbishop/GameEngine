#include "ImageLibrary.h"
#include <iostream>
#include "SDL_image.h"

Image::Image(std::string filename, SDL_Surface* screen): filename(filename), surface(nullptr)
{
	SDL_Surface* loaded = IMG_Load(filename.c_str());
	this->surface = SDL_ConvertSurface(loaded, screen->format, NULL);
	SDL_FreeSurface(loaded);
	loaded = nullptr;
	std::cerr << "Image created: " << filename << std::endl;
}

Image::~Image()
{
	SDL_FreeSurface(this->surface);
	std::cerr << "Image destroyed." << std::endl;

}


ImageLibrary::ImageLibrary(std::string directory, SDL_Surface* screen)
{
	std::cerr << "Image library created at this directory: " << directory << std::endl;

}

ImageLibrary::~ImageLibrary()
{
	std::cerr << "ImageLibrary destroyed." << std::endl;

}

SDL_Surface* GetImage(int id)
{
	std::cerr << "Image reference retrieved." << std::endl;

	return NULL;
}