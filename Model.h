#pragma once

#include <vector>
#include <bitset>

class View;
class Controller;
class Action;

class Actors
{
public:
	Actors(size_t num_actors);

	enum class ActorType { Player, Rock, Stick };

	struct PositionData
	{
		int bx, by;
		float x, y;
		int w, h;
	};

	struct MovementData
	{
		float vx, vy;
		float ax, ay;
	};

	std::vector<PositionData>	m_pd;
	std::vector<MovementData>	m_md;
	std::vector<ActorType>		m_types;
	std::vector<std::bitset<32>> m_attributes;

	void Reserve(size_t num_actors);
	void Push(const Actors::PositionData& pd, const Actors::MovementData& md, const Actors::ActorType& at, const std::bitset<32>& attrib);
	void Swap(const size_t first, const size_t second);
	void Pop();

	size_t GetLength() const { return m_length; }

private:
	size_t m_length;
};

/// Model base class, knows about the data, but not how to view
///  it.  Owns all the data in the model world.  Applies the requested
///  transformations or commands to it on request.
///

class Model
{
public:
	
	Model(size_t num_actors);
	~Model();
	void Attach(View* v) noexcept;
	void Detach(View* v) noexcept;
	void Notify() const;

	void Simulate(Controller& control) const;
	void Apply(const std::vector<std::unique_ptr<Action>>& actions);

protected:
	
	friend class Game;
	void AddActor(const Actors::PositionData& pd, const Actors::MovementData& md, const Actors::ActorType& at, const std::bitset<32>& attrib);
	void RemoveActor(const size_t index);
	
private:

	// Invariants: 
	//		all vectors are the same length
	//		indices refer to the same object
	//		
	Actors				m_actors;
	std::vector<View*>	m_views;

};
