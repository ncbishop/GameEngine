#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <memory>

#include "Model.h"
#include "View.h"
#include "Controller.h"

Actors::Actors(size_t num_actors)
{
	this->Reserve(num_actors);
}

void Actors::Reserve(size_t num_actors)
{
	m_pd.reserve(num_actors);
	m_md.reserve(num_actors);
	m_types.reserve(num_actors);
	m_attributes.reserve(num_actors);

	return;
}

void Actors::Push(const Actors::PositionData& pd, const Actors::MovementData& md, const Actors::ActorType& at, const std::bitset<32>& attrib)
{
	m_pd.push_back(pd);
	m_md.push_back(md);
	m_types.push_back(at);
	m_attributes.push_back(attrib);

	m_length++;
}

void Actors::Swap(const size_t first, const size_t second)
{
	if (first < m_length && second < m_length)
	{
		std::iter_swap(std::next(m_pd.begin(), first), std::next(m_pd.begin(), second));
		std::iter_swap(std::next(m_md.begin(), first), std::next(m_md.begin(), second));
		std::iter_swap(std::next(m_types.begin(), first), std::next(m_types.begin(), second));
		std::iter_swap(std::next(m_attributes.begin(), first), std::next(m_attributes.begin(), second));
	}
	else
	{
		throw std::out_of_range("Cannot swap out of valid range!");
	}
}

void Actors::Pop()
{
	m_pd.pop_back();
	m_md.pop_back();
	m_types.pop_back();
	m_attributes.pop_back();

	m_length--;
}

Model::Model(size_t num_actors): m_actors(num_actors)
{
	// RAII
	// If we're going to throw, do it now!
	m_views.reserve(16);
}

Model::~Model() {}

void Model::Attach(View* v) noexcept
{
	m_views.push_back(v);
}

void Model::Detach(View* v) noexcept
{
	auto i = std::find(m_views.begin(), m_views.end(), v);

	if (i != m_views.end())
	{
		m_views.erase(i);
	}
	else
	{
		// Pass
	}
}

void Model::Notify() const
{
	for (View* vp : m_views)
	{
		vp->UpdateView(m_actors);
	}
}

void Model::Simulate(Controller& c) const
{
	c.Control(m_actors);
}

void Model::Apply(const std::vector<std::unique_ptr<Action>>& actions)
{
	for (const auto& a: actions)
	{
		a->Execute(m_actors);
	}
}

void Model::AddActor(const Actors::PositionData& pd, const Actors::MovementData& md, const Actors::ActorType& at, const std::bitset<32>& attrib)
{
	m_actors.Push(pd, md, at, attrib);
}

void Model::RemoveActor(const size_t index)
{
	m_actors.Swap(index, m_actors.GetLength());
	m_actors.Pop();
}