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
	THasLessObjectsTypeThan
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
	{ "HasLessObjectsTypeThan", AITag::THasLessObjectsTypeThan}
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


class HasMoreResourcesThan : public BrainTree::Leaf
{
public:
	HasMoreResourcesThan(BrainTree::Blackboard::Ptr board, GameVault *vault, EntityID entity, double amount) : Leaf(board), vault(vault), entity(entity), amount(amount) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);
		if (player.resources > amount)
			return Node::Status::Success;
		else
			return Node::Status::Failure;
	}

private:
	GameVault *vault;
	EntityID entity;
	float amount;
};


class HasLessObjectsTypeThan : public BrainTree::Leaf
{
public:
	HasLessObjectsTypeThan(BrainTree::Blackboard::Ptr board, GameVault *vault, EntityID entity, std::string type, int qty) : Leaf(board), vault(vault), entity(entity), type(type), qty(qty) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);
		if (player.objsCount.count(type)==0 || player.objsCount[type] < qty)
			return Node::Status::Success;
		else
			return Node::Status::Failure;
	}

private:
	GameVault *vault;
	EntityID entity;
	std::string type;
	int qty;
};


class AIParser : public GameSystem {
	tinyxml2::XMLDocument doc;
public:

	void load(const char* filename) {
		tinyxml2::XMLError error = doc.LoadFile(filename);
		if (error)
			std::cout << "AI: error loading file: " << error << std::endl;
	}

	void parse(entt::Registry<EntityID> &registry, BrainTree::BehaviorTree &tree, EntityID entity) {
		auto rootSelector = std::make_shared<BrainTree::Selector>();
		auto blackboard = tree.getBlackboard();

		for (tinyxml2::XMLElement *child : doc.FirstChildElement()) {
			std::cout << "AI: add " << child->Name() << " to root" << std::endl;
			rootSelector->addChild(this->parseElement(registry, blackboard, child, entity));
		}
		tree.setRoot(rootSelector);
	}

	std::shared_ptr<BrainTree::Node> parseElement(entt::Registry<EntityID> &registry, std::shared_ptr<BrainTree::Blackboard> blackboard, tinyxml2::XMLElement *element, EntityID entity) {
		std::cout << "AI: parse node " << element->Name() << std::endl;
		switch (aiTags[element->Name()]) {
		case AITag::TSelector: {
			auto selector = std::make_shared<BrainTree::Selector>();
			for (tinyxml2::XMLElement *child : element) {
				selector->addChild(this->parseElement(registry, blackboard, child, entity));
			}
			return selector;
		}
		case AITag::TRandomSelector: {
			auto selector = std::make_shared<BrainTree::RandomSelector>();
			for (tinyxml2::XMLElement *child : element) {
				selector->addChild(this->parseElement(registry, blackboard, child, entity));
			}
			return selector;
		}
		case AITag::TSequence: {
			auto sequence = std::make_shared<BrainTree::Sequence>();
			for (tinyxml2::XMLElement *child : element) {
				sequence->addChild(this->parseElement(registry, blackboard, child, entity));
			}
			return sequence;
		}
		case AITag::TInverter: {
			auto node = std::make_shared<BrainTree::Inverter>();
			node->setChild(this->parseElement(registry, blackboard, element->FirstChildElement(), entity));
			return node;
		}
		case AITag::TSucceeder: {
			auto node = std::make_shared<BrainTree::Succeeder>();
			node->setChild(this->parseElement(registry, blackboard, element->FirstChildElement(), entity));
			return node;
		}
		case AITag::TUntilFail: {
			auto node = std::make_shared<BrainTree::UntilFail>();
			node->setChild(this->parseElement(registry, blackboard, element->FirstChildElement(), entity));
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
			auto node = std::make_shared<HasMoreResourcesThan>(blackboard, this->vault, entity, amount);
			return node;
		}
		case AITag::THasLessObjectsTypeThan: {
			int qty = element->IntAttribute("qty");
			std::string type = element->Attribute("type");
			auto node = std::make_shared<HasLessObjectsTypeThan>(blackboard, this->vault, entity, type, qty);
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
};