#pragma once

#include <vector>
#include <tuple>

#include "SDL.h"
#include "Model.h"

/// View base class.  Knows nothing about the model.
///  Has an interface for dealing with events relevant
///  to the view (such as mouse clicks).
///  A derived class would e.g. know how to display a specific
///  type of Actor in the window.
///

struct Animation;

struct ScreenData
{
	SDL_Rect dest_rect;
	Animation* anim;
};

class View
{
public:
	// Must be constructed with a control for event handling
	View(int block_size, size_t num_actors);
	~View();
	
	void Offset(int bx, int by, float dx, float dy);
	void UpdateView(const Actors& modeldata);

protected:

	friend class Game;
	void AddActor(const Actors::PositionData& pd, const Actors::MovementData& md, const Actors::ActorType& at, const std::bitset<32>& attrib);
	void RemoveActor(const size_t index);

private:
	int m_blockx;
	int m_blocky;
	int m_block_size;
	float m_offx;
	float m_offy;

	std::vector<ScreenData>		m_screen;

	std::tuple<int, int> WorldToScreen(const Actors::PositionData& pd);

};
