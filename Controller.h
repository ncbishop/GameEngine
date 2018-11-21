#pragma once

#include <vector>
#include <memory>

class Actors;

class Action
{
public:
	Action();
	virtual ~Action();

	virtual void Execute(Actors& a) const;

private:
	size_t index;

};

/// Control base class.  Recieves events and reacts appropriately by updating
///  the model.  Intended to be implemented by adding behaviors (logic)
///
///
class Controller
{
public:
	Controller();
	Controller(const Controller& cpy);
	virtual ~Controller();
	//void HandleEvent(const SDL_Event& e, Model& dm) const;

	void Control(const Actors& a);

private:
	std::vector<std::unique_ptr<Action>> m_actions;

};
