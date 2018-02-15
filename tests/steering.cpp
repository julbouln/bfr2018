#include <iostream>
#include "Helpers.hpp"

struct Agent {
	sf::Vector2f pos;
	sf::Vector2f vel;
	float speed;
	float view;
};

sf::Vector2f seek(Agent ag, Agent target) {
	sf::Vector2f seekVel = vectorNormalize(target.pos - ag.pos);

	seekVel = vectorRound(seekVel);
	std::cout << "seek " << ag.pos.x << "x" << ag.pos.y << " -> " << target.pos.x << "x" << target.pos.y << " : " << seekVel.x << "x" << seekVel.y << std::endl;
	return seekVel;
}

sf::Vector2f flee(Agent ag, Agent target) {
	return seek(target, ag);
}

sf::Vector2f pursue(Agent ag, Agent target) {
	float maxPrediction = 5.5f;
	float prediction = 0.f;
	sf::Vector2f direction = target.pos - ag.pos;
	float distance = vectorLength(direction);

	if (ag.speed <= distance / maxPrediction)
	{
		prediction = maxPrediction;
	}
	else
	{
		prediction = distance / ag.speed;
	}

	std::cout << "pursue distance:" << distance << " prediction:" << prediction << std::endl;
	target.pos += target.vel * prediction;

	target.pos = vectorRound(target.pos);

	return seek(ag, target);
}

sf::Vector2f followPath(Agent ag, std::vector<sf::Vector2f> path, int idx) {
	int i = 0, index = 0;
	float dist = std::numeric_limits<float>::max();

	for (sf::Vector2f vect : path)
	{
		sf::Vector2f temp = (vect - ag.pos);
		float len = fabs(vectorLength(temp));
		if (len < dist)
		{
			index = i;
			dist = len;
		}
		i++;
	}

	std::cout << "followPath: index(1):" << index << std::endl;
	// arrive on last pos
	if (index == path.size() - 1)
	{
		std::cout << "followPath: at destination" << std::endl;
		return sf::Vector2f(0, 0);
	}

//	index += idx;
	index++;
	std::cout << "followPath: index:" << index << std::endl;

	return seek(ag, Agent{path[index]});
}

sf::Vector2f collisionAvoid(Agent ag, std::vector<Agent> others) {
	sf::Vector2f direction;
	sf::Vector2f orientation = ag.vel;

	for (Agent nb : others)
	{
		direction = nb.pos - ag.pos;

		if (vectorLength(direction) != 0.f)  // check for check the agent itself
		{
			if (fabs(vectorDot(orientation, direction)) > ag.view && vectorLength(direction) < 50.0f)
				//if (direction.dot(nb.getorientation().asVector()) > agent.getFOV() && direction.getLength() < agent.getseparationRadius())
			{
				std::cout << "collisionAvoid dot:" << fabs(vectorDot(orientation, direction)) << " " << orientation.x << "x" << orientation.y << " " << direction.x << "x" << direction.y << std::endl;
				ag.pos = vectorRound(ag.pos);
				nb.pos = vectorRound(nb.pos);
				return pursue(nb, ag);
			}
		}
	}
	return sf::Vector2f(0.0, 0.0);
}

int main() {
	Agent agent1 = Agent{sf::Vector2f(10.0, 10.0), sf::Vector2f(0.0, 0.0), 3.0, 3.0};
	Agent agent2 = Agent{sf::Vector2f(10.0, 14.0), sf::Vector2f(1.0, 1.0), 3.0, 3.0};

	sf::Vector2f pos(10.0, 10.0);
	sf::Vector2f target(10.0, 14.0);

	seek(agent1, agent2);
	flee(agent1, agent2);
	pursue(agent1, agent2);

	std::vector<sf::Vector2f> path;
	path.push_back(sf::Vector2f(13.0, 10.0));
	path.push_back(sf::Vector2f(14.0, 10.0));
	path.push_back(sf::Vector2f(15.0, 10.0));
	path.push_back(sf::Vector2f(16.0, 10.0));
	path.push_back(sf::Vector2f(17.0, 11.0));

	int idx = 0;
	for (int i = 0; i < 16; i++) {
		sf::Vector2f vel = followPath(agent1, path, idx);
		agent1.vel = vel;

		std::vector<Agent> others;
		others.push_back(agent1);
		others.push_back(agent2);
		sf::Vector2f avel = collisionAvoid(agent1, others);

		if (vectorLength(avel) > 0) {
			agent1.vel = avel;
			agent1.pos += avel;
		}
		else {
			agent1.pos += vel;
		}

		std::cout << "PATH NEW POS " << idx << " " << agent1.pos.x << "x" << agent1.pos.y << std::endl;
		if (agent1.pos == path[idx] && idx < path.size() - 1) {
			idx++;
		}
	}


}