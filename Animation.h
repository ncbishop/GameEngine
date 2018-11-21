#pragma once

#include <SDL.h>

struct Animation
{
	SDL_Surface* spritemap;
	const int step_size;
	SDL_Rect src_rect;
	int frame;
	const int max_frames;
};
