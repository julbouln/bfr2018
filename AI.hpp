#pragma once

#include "tinyxml2.h"
#include "tixml2ex.h"

#include "GameSystems/GameSystem.hpp"
#include "BrainTree/BrainTree.h"


enum class AITag {
	TSelector,
	TRandomSelector,
	TSequence,
	TInverter,
	TSucceeder,
	TUntilFail,
	TProbability,
	THasMoreResourcesThan,
	THasLessObjectsTypeThan,
	TBuild,
	TPlant,
	TTrain
};

static std::map<std::string, AITag> aiTags =
{
	{ "Selector", AITag::TSelector },
	{ "RandomSelector", AITag::TRandomSelector },
	{ "Sequence", AITag::TSequence },
	{ "Inverter", AITag::TInverter },
	{ "Succeeder", AITag::TSucceeder },
	{ "UntilFail", AITag::TUntilFail },
	{ "Probability", AITag::TProbability},
	{ "HasMoreResourcesThan", AITag::THasMoreResourcesThan},
	{ "HasLessObjectsTypeThan", AITag::THasLessObjectsTypeThan},
	{ "Build", AITag::TBuild},
	{ "Plant", AITag::TPlant},
	{ "Train", AITag::TTrain}
};

class Probability : public BrainTree::Leaf
{
public:
	Probability(BrainTree::Blackboard::Ptr board, int per, int frac) : Leaf(board), per(per), frac(frac) {}

	Status update() override
	{
		double rnd = (double)rand() / RAND_MAX;

		if (rnd < (double)per / (double)frac) {
			return Node::Status::Success;
		}
		else {
			return Node::Status::Failure;
		}
	}

private:
	int per;
	int frac;
};


class HasMoreResourcesThan : public BrainTree::Leaf, public GameSystem
{
public:
	HasMoreResourcesThan(BrainTree::Blackboard::Ptr board, EntityID entity, double amount) : Leaf(board), entity(entity), amount(amount) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);
		if (player.resources > amount)
			return Node::Status::Success;
		else
			return Node::Status::Failure;
	}

private:
	EntityID entity;
	float amount;
};


class HasLessObjectsTypeThan : public BrainTree::Leaf, public GameSystem
{
public:
	HasLessObjectsTypeThan(BrainTree::Blackboard::Ptr board, EntityID entity, std::string type, int qty) : Leaf(board), entity(entity), type(type), qty(qty) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);
		int foundQty = 0;
		if (player.objsByType.count(type) == 0)
			foundQty = 0;
		else
			foundQty = player.objsByType[type].size();

		if (foundQty < qty) {
			std::cout << "AI: " << entity << " has less than " << qty << " (" << foundQty << ") " << type << std::endl;
			return Node::Status::Success;
		}
		else
			return Node::Status::Failure;
	}

private:
	EntityID entity;
	std::string type;
	int qty;
};

class Build : public BrainTree::Leaf, public GameSystem
{
public:
	Build(BrainTree::Blackboard::Ptr board, EntityID entity, std::string name) : Leaf(board), entity(entity), name(name) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		EntityID buildingEnt = this->vault->factory.createBuilding(vault->registry, entity, name, 200, 200, false);
		GameObject &obj = vault->registry.get<GameObject>(buildingEnt);
		Tile &tile = vault->registry.get<Tile>(buildingEnt);

		std::vector<sf::Vector2i> buildPos;
		for (int x = 0; x < this->map->width; x++) {
			for (int y = 0; y < this->map->height; y++) {
//				std::cout << "FOG:" << (int)player.fog.get(x,y) << std::endl;
				if (player.fog.get(x, y) != FogState::Unvisited) {
					tile.pos = sf::Vector2i(x, y);
					bool intersect = false;
					for (sf::Vector2i p : this->tileSurface(tile)) {
						if (this->map->objs.get(p.x, p.y)) {
							intersect = true;
						}
					}

					if (!this->map->bound( x - tile.size.x, y - tile.size.y) || !this->map->bound(x + tile.size.x, y + tile.size.y))
						intersect = true;

					if (!intersect)
						buildPos.push_back(sf::Vector2i(x, y));

				}
			}
		}

		if (buildPos.size() > 0) {
			std::random_shuffle ( buildPos.begin(), buildPos.end() );

			obj.mapped = true;
			tile.pos = buildPos.front();
			tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

			std::cout << "AI:" << name << " build at " << buildPos.front().x << "x" << buildPos.front().y << std::endl;
			return Node::Status::Success;
		} else {
			std::cout << "AI:" << name << " cannot be built !"<<std::endl;
			this->vault->registry.destroy(buildingEnt);
			return Node::Status::Failure;

		}

	}

private:
	EntityID entity;
	std::string name;
};

class Plant : public BrainTree::Leaf, public GameSystem
{
public:
	Plant(BrainTree::Blackboard::Ptr board, EntityID entity, std::string name) : Leaf(board), entity(entity), name(name) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		std::vector<EntityID> plantAround;
		ResourceType type;
		if (name == "nature") {
			plantAround = player.objsByType["ferme"];
			type = ResourceType::Nature;
		} else {
			plantAround = player.objsByType["labo"];
			type = ResourceType::Pollution;
		}

		std::random_shuffle ( plantAround.begin(), plantAround.end() );
		this->seedResources(type, plantAround.front());

		std::cout << "AI:" << name << " plant " << name << " around " << plantAround.front() << std::endl;
		TechNode *n = this->vault->factory.getTechNode(blackboard->GetString("team"), name);
//		std::cout << "NODE "<< n->parentType << std::endl;
//		std::cout << ->parent->type << std::endl;

		return Node::Status::Success;
	}

private:
	EntityID entity;
	std::string name;

};


class Train : public BrainTree::Leaf, public GameSystem
{
public:
	Train(BrainTree::Blackboard::Ptr board, EntityID entity, std::string name) : Leaf(board), entity(entity), name(name) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		std::vector<EntityID> trainAround = player.objsByType[this->vault->factory.getTechNode(blackboard->GetString("team"), name)->parentType];

		std::random_shuffle ( trainAround.begin(), trainAround.end() );
		if (trainUnit(name, entity, trainAround.front() )) {
			std::cout << "AI:" << name << " train " << name << " around " << trainAround.front() << std::endl;

			return Node::Status::Success;

		} else {
			return Node::Status::Failure;

		}


	}

private:
	EntityID entity;
	std::string name;

};


class AIParser : public GameSystem {
	tinyxml2::XMLDocument doc;
public:

	void load(const char* filename) {
		tinyxml2::XMLError error = doc.LoadFile(filename);
		if (error)
			std::cout << "AI: error loading file: " << error << std::endl;
	}

	void parse(std::string team, BrainTree::BehaviorTree &tree, EntityID entity) {
		auto rootSelector = std::make_shared<BrainTree::Selector>();
		auto blackboard = tree.getBlackboard();
		blackboard->SetString("team", team);

		for (tinyxml2::XMLElement *child : doc.FirstChildElement()) {
			std::cout << "AI: add " << child->Name() << " to root" << std::endl;
			rootSelector->addChild(this->parseElement(blackboard, child, entity));
		}
		tree.setRoot(rootSelector);
	}

	std::shared_ptr<BrainTree::Node> parseElement(std::shared_ptr<BrainTree::Blackboard> blackboard, tinyxml2::XMLElement *element, EntityID entity) {
		std::cout << "AI: parse node " << element->Name() << std::endl;
		switch (aiTags[element->Name()]) {
		case AITag::TSelector: {
			auto selector = std::make_shared<BrainTree::Selector>();
			for (tinyxml2::XMLElement *child : element) {
				selector->addChild(this->parseElement(blackboard, child, entity));
			}
			return selector;
		}
		case AITag::TRandomSelector: {
			auto selector = std::make_shared<BrainTree::RandomSelector>();
			for (tinyxml2::XMLElement *child : element) {
				selector->addChild(this->parseElement(blackboard, child, entity));
			}
			return selector;
		}
		case AITag::TSequence: {
			auto sequence = std::make_shared<BrainTree::Sequence>();
			for (tinyxml2::XMLElement *child : element) {
				sequence->addChild(this->parseElement(blackboard, child, entity));
			}
			return sequence;
		}
		case AITag::TInverter: {
			auto node = std::make_shared<BrainTree::Inverter>();
			node->setChild(this->parseElement(blackboard, element->FirstChildElement(), entity));
			return node;
		}
		case AITag::TSucceeder: {
			auto node = std::make_shared<BrainTree::Succeeder>();
			node->setChild(this->parseElement(blackboard, element->FirstChildElement(), entity));
			return node;
		}
		case AITag::TUntilFail: {
			auto node = std::make_shared<BrainTree::UntilFail>();
			node->setChild(this->parseElement(blackboard, element->FirstChildElement(), entity));
			return node;
		}
		case AITag::TProbability: {
			int per = element->IntAttribute("per");
			int frac = element->IntAttribute("frac");
			std::cout << "AI: probability " << per << "/" << frac << std::endl;
			auto node = std::make_shared<Probability>(blackboard, per, frac);
			return node;
		}
		case AITag::THasMoreResourcesThan: {
			float amount = element->FloatAttribute("amount");
			auto node = std::make_shared<HasMoreResourcesThan>(blackboard, entity, amount);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AITag::THasLessObjectsTypeThan: {
			int qty = element->IntAttribute("qty");
			std::string type = element->Attribute("type");
			auto node = std::make_shared<HasLessObjectsTypeThan>(blackboard, entity, type, qty);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AITag::TBuild: {
			std::string name = element->Attribute("type");
			auto node = std::make_shared<Build>(blackboard, entity, name);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AITag::TPlant: {
			std::string name = element->Attribute("type");
			auto node = std::make_shared<Plant>(blackboard, entity, name);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AITag::TTrain: {
			std::string name = element->Attribute("type");
			auto node = std::make_shared<Train>(blackboard, entity, name);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		}
		std::cout << "AI: unknown tag " << element->Name() << std::endl;
		return nullptr;

	}

};


class AI : public GameSystem
{
public:
	AIParser rebelAI;
	AIParser nazAI;

	AI() {
		nazAI.load("defs/new/ai/neonaz.xml");
	}
};