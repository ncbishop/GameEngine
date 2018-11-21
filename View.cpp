#include "View.h"

View::View(int block_size, size_t num_actors): m_blockx(0), m_blocky(0), m_block_size(block_size), m_offx(0.0), m_offy(0.0)
{
	m_screen.reserve(num_actors);
}

void View::Offset(int bx, int by, float dx, float dy)
{
	m_blockx += bx;
	m_blocky += by;
	m_offx += dx;
	m_offy += dy;
}

void View::UpdateView(const Actors& modeldata)
{
	size_t length = modeldata.GetLength();
	for (size_t index = 0; index < length; index++)
	{
		auto& sd = m_screen[index];
		std::tie(sd.dest_rect.x, sd.dest_rect.y) = WorldToScreen(modeldata.m_pd[index]);
	}
	
}

std::tuple<int, int> View::WorldToScreen(const Actors::PositionData& pd)
{
	int sx = static_cast<int>(pd.x - m_offx);
	int sy = static_cast<int>(pd.y - m_offy);

	sx += (pd.bx - m_blockx)*m_block_size;
	sy += (pd.by - m_blocky)*m_block_size;

	return std::make_tuple(sx, sy);
}

void View::AddActor(const Actors::PositionData& pd, const Actors::MovementData& md, const Actors::ActorType& at, const std::bitset<32>& attrib)
{
	ScreenData sd;
	std::tie(sd.dest_rect.x, sd.dest_rect.y) = WorldToScreen(pd);
	sd.dest_rect.w = pd.w;
	sd.dest_rect.h = pd.h;

	sd.anim = nullptr;

	m_screen.push_back(sd);
}