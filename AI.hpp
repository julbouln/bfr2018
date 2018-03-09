#pragma once

#include "third_party/tinyxml2.h"
#include "third_party/tixml2ex.h"

#include "Systems/GameSystem.hpp"
#include "BrainTree/BrainTree.h"

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
#ifdef AI_DEBUG
			std::cout << "AI: " << entity << " has less than " << qty << " (" << foundQty << ") " << type << std::endl;
#endif
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



class HasLessObjectsThan : public BrainTree::Leaf, public GameSystem
{
public:
	HasLessObjectsThan(BrainTree::Blackboard::Ptr board, EntityID entity, int qty) : Leaf(board), entity(entity), qty(qty) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);
		int foundQty = 0;
		for (auto o : player.objsByType) {
			foundQty += o.second.size();
		}

		if (foundQty < qty) {
#ifdef AI_DEBUG
			std::cout << "AI: " << entity << " has less than " << qty << " (" << foundQty << ") " << std::endl;
#endif
			return Node::Status::Success;
		}
		else
			return Node::Status::Failure;
	}

private:
	EntityID entity;
	int qty;
};

class HasExploredLessThan : public BrainTree::Leaf, public GameSystem
{
public:
	HasExploredLessThan(BrainTree::Blackboard::Ptr board, EntityID entity, int per) : Leaf(board), entity(entity), per(per) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);
		int explored = player.fog.visited();
		int exploredPer = (explored * 100) / (this->map->width * this->map->height) ;

		if (exploredPer < per) {
#ifdef AI_DEBUG
			std::cout << "AI: " << entity << " has explored less than " << per << " (" << exploredPer << "/" << explored << ") " << std::endl;
#endif
			return Node::Status::Success;
		}
		else
			return Node::Status::Failure;
	}

private:
	EntityID entity;
	int per;
};


class HasFoundEnemy : public BrainTree::Leaf, public GameSystem
{
public:
	HasFoundEnemy(BrainTree::Blackboard::Ptr board, EntityID entity) : Leaf(board), entity(entity) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		if (player.enemyFound) {
#ifdef AI_DEBUG
			std::cout << "AI: " << entity << " enemy found at " << player.enemyPos.x << "x" << player.enemyPos.y << std::endl;
#endif
			return Node::Status::Success;
		}
		else
			return Node::Status::Failure;
	}

private:
	EntityID entity;
};


class Explore : public BrainTree::Leaf, public GameSystem
{
public:
	Explore(BrainTree::Blackboard::Ptr board, EntityID entity, std::string name, int maxDist) : Leaf(board), entity(entity), name(name), maxDist(maxDist) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		if (player.objsByType.count(name) > 0) {
			std::vector<EntityID> explorers = player.objsByType[name];
			std::random_shuffle ( explorers.begin(), explorers.end() );
			EntityID explorer = explorers.front();

			if (this->vault->registry.valid(explorer)) {
				Tile &exTile = this->vault->registry.get<Tile>(explorer);
				sf::Vector2i explorePos = this->getRandExplorationPos(exTile);

				int cnt = 0;
				while ((!this->map->bound(explorePos.x, explorePos.y) || player.fog.get(explorePos.x, explorePos.y) != FogState::Unvisited) && cnt < 32) {
					explorePos = this->getRandExplorationPos(exTile);
					cnt++;
				}

				if (exTile.state == "idle") {
#ifdef AI_DEBUG
					std::cout << "AI: " << entity << " explore with " << explorer << " at " << explorePos.x << "x" << explorePos.y << std::endl;
#endif
					this->goTo(explorer, explorePos);
				}
			}
			return Node::Status::Success;
		} else {
			return Node::Status::Failure;
		}

	}

private:
	EntityID entity;
	std::string name;
	int maxDist;

	sf::Vector2i getRandExplorationPos(Tile &tile) {
		sf::Vector2i explorePos;

		if (this->maxDist == -1)
			explorePos =  sf::Vector2i(rand() % this->map->width, rand() % this->map->height);
		else
			explorePos = sf::Vector2i(tile.pos.x + (rand() % (this->maxDist * 2)) - this->maxDist, tile.pos.y + (rand() % (this->maxDist * 2)) - this->maxDist);

		return explorePos;
	}

};

class Build : public BrainTree::Leaf, public GameSystem
{
public:
	Build(BrainTree::Blackboard::Ptr board, EntityID entity, std::string name) : Leaf(board), entity(entity), name(name) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		if (!player.rootConstruction)
			player.rootConstruction = this->vault->factory.startBuilding(vault->registry, name, 0);

#ifdef AI_DEBUG
//		std::cout << "AI: " << entity << " build " << name << " at " << buildPos.front().x << "x" << buildPos.front().y << std::endl;
#endif
		return Node::Status::Success;
	}

private:
	EntityID entity;
	std::string name;
};



class PlaceBuilt : public BrainTree::Leaf, public GameSystem
{
public:
	PlaceBuilt(BrainTree::Blackboard::Ptr board, EntityID entity) : Leaf(board), entity(entity) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		if (player.rootConstruction && vault->registry.get<Building>(player.rootConstruction).buildTime == 0) {
			EntityID buildingEnt = this->vault->factory.finishBuilding(vault->registry, player.rootConstruction, entity, 200, 200, false);

			GameObject &obj = vault->registry.get<GameObject>(buildingEnt);
			Tile &tile = vault->registry.get<Tile>(buildingEnt);

			std::vector<sf::Vector2i> buildPos;
			for (int x = 0; x < this->map->width; x++) {
				for (int y = 0; y < this->map->height; y++) {
					if (player.fog.get(x, y) == FogState::InSight) {
						tile.pos = sf::Vector2i(x, y);
						bool intersect = false;
						for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, 2)) {
							if (this->map->objs.get(p.x, p.y) || this->map->staticBuildable.get(p.x, p.y) != 0) {
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

				std::sort( buildPos.begin( ), buildPos.end( ), [this, player ]( const auto & lhs, const auto & rhs )
				{
					return distance(player.initialPos, lhs) < distance(player.initialPos, rhs);
				});

//			std::random_shuffle ( buildPos.begin(), buildPos.end() );

				obj.mapped = true;
				tile.pos = buildPos.front();
				tile.ppos = sf::Vector2f(tile.pos) * 32.0f + 16.0f;
				player.rootConstruction = 0;

#ifdef AI_DEBUG
				std::cout << "AI: " << entity << " build " << obj.name << " at " << buildPos.front().x << "x" << buildPos.front().y << std::endl;
#endif
				return Node::Status::Success;
			} else {
#ifdef AI_DEBUG
				std::cout << "AI: " << entity << " cannot build " << obj.name << std::endl;
#endif
				this->vault->registry.remove<Tile>(buildingEnt);

				return Node::Status::Failure;
			}
		} else {
#ifdef AI_DEBUG
			std::cout << "AI: " << entity << " no building to place " << std::endl;
#endif
			return Node::Status::Success;
		}
	}

private:
	EntityID entity;
};

class Plant : public BrainTree::Leaf, public GameSystem
{
public:
	Plant(BrainTree::Blackboard::Ptr board, EntityID entity, std::string name) : Leaf(board), entity(entity), name(name) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		std::vector<EntityID> plantAround;

		if (name == "nature") {
			plantAround = player.objsByType["ferme"];
		} else {
			plantAround = player.objsByType["labo"];
		}

		if (plantAround.size() > 0) {
			std::random_shuffle ( plantAround.begin(), plantAround.end() );
			if (this->vault->registry.valid(plantAround.front())) {
				this->seedResources(name, plantAround.front());

#ifdef AI_DEBUG
				std::cout << "AI: " << entity << " plant " << name << " around " << plantAround.front() << std::endl;
#endif
				TechNode *n = this->vault->factory.getTechNode(blackboard->GetString("team"), name);
			}
		}
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

		std::string parentName = this->vault->factory.getTechNode(blackboard->GetString("team"), name)->parentType;
		if (player.objsByType.count(parentName) > 0) {
			std::vector<EntityID> trainAround = player.objsByType[parentName];

			std::random_shuffle ( trainAround.begin(), trainAround.end() );
			if (this->trainUnit(name, entity, trainAround.front() )) {
#ifdef AI_DEBUG
				std::cout << "AI: " << entity << " train " << name << " around " << trainAround.front() << std::endl;
#endif
				return Node::Status::Success;
			} else {
				return Node::Status::Failure;
			}
		} else {
			return Node::Status::Failure;
		}
	}

private:
	EntityID entity;
	std::string name;

};


class SendExpedition : public BrainTree::Leaf, public GameSystem
{
public:
	SendExpedition(BrainTree::Blackboard::Ptr board, EntityID entity, std::string name, int per) : Leaf(board), entity(entity), name(name), per(per) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		if (player.enemyFound && player.objsByType.count(name) > 0) {
			int tot = player.objsByType[name].size();
			int perCnt = (int)((float)per / 100.0 * (float)tot);

			std::vector<EntityID> attackers = player.objsByType[name];
			std::random_shuffle ( attackers.begin(), attackers.end() );

			std::vector<EntityID> group;

			for (int i = 0; i < perCnt; i++) {
				EntityID attacker = attackers[i];
				if (this->vault->registry.valid(attacker)) {
					Tile &atTile = this->vault->registry.get<Tile>(attacker);

					if (atTile.state == "idle" && distance(atTile.pos, player.enemyPos) > 8) {
#ifdef AI_DEBUG
						std::cout << "AI: " << entity << " launch expedition with " << attacker << " at " << player.enemyPos.x << "x" << player.enemyPos.y << std::endl;
#endif
						group.push_back(attacker);
//						this->goTo(attacker, player.enemyPos);
					}
				}
			}
			this->sendGroup(group, player.enemyPos, GroupFormation::Square, North, false);
			return Node::Status::Success;
		} else {
			return Node::Status::Failure;
		}

	}

private:
	EntityID entity;
	std::string name;
	int per;
};

class SendDefense : public BrainTree::Leaf, public GameSystem
{
public:
	SendDefense(BrainTree::Blackboard::Ptr board, EntityID entity, std::string name, int per) : Leaf(board), entity(entity), name(name), per(per) {}

	Status update() override
	{
		Player &player = vault->registry.get<Player>(entity);

		if (player.frontPoints.size() > 0 && player.objsByType.count(name) > 0) {
			int tot = player.objsByType[name].size();
			int perCnt = (int)((float)per / 100.0 * (float)tot);

			std::vector<EntityID> attackers = player.objsByType[name];
			std::random_shuffle ( attackers.begin(), attackers.end() );

			std::vector<EntityID> group;
			sf::Vector2i destPos = player.frontPoints.back().pos;

			for (int i = 0; i < perCnt; i++) {
				EntityID attacker = attackers[i];
				if (this->vault->registry.valid(attacker)) {
					Tile &atTile = this->vault->registry.get<Tile>(attacker);

					if (atTile.state == "idle" && distance(atTile.pos, destPos) > 8) {
#ifdef AI_DEBUG
						std::cout << "AI: " << entity << " send defense with " << attacker << " at " << destPos.x << "x" << destPos.y << std::endl;
#endif
						group.push_back(attacker);
//						this->goTo(attacker, destPos);
					}
				}
			}
			this->sendGroup(group, destPos, GroupFormation::Square, North, false);
			return Node::Status::Success;
		} else {
			return Node::Status::Failure;
		}

	}

private:
	EntityID entity;
	std::string name;
	int per;
};

typedef entt::HashedString AINode;

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
#ifdef AI_DEBUG
			std::cout << "AI: add " << child->Name() << " to root" << std::endl;
#endif
			rootSelector->addChild(this->parseElement(blackboard, child, entity));
		}
		tree.setRoot(rootSelector);
	}

	std::shared_ptr<BrainTree::Node> parseElement(std::shared_ptr<BrainTree::Blackboard> blackboard, tinyxml2::XMLElement *element, EntityID entity) {
#ifdef AI_DEBUG
		std::cout << "AI: parse node " << element->Name() << std::endl;
#endif

		switch (AINode(element->Name())) {
		case AINode("Selector"): {
			auto selector = std::make_shared<BrainTree::Selector>();
			for (tinyxml2::XMLElement *child : element) {
				selector->addChild(this->parseElement(blackboard, child, entity));
			}
			return selector;
		}
		case AINode("RandomSelector"): {
			auto selector = std::make_shared<BrainTree::RandomSelector>();
			for (tinyxml2::XMLElement *child : element) {
				selector->addChild(this->parseElement(blackboard, child, entity));
			}
			return selector;
		}
		case AINode("Sequence"): {
			auto sequence = std::make_shared<BrainTree::Sequence>();
			for (tinyxml2::XMLElement *child : element) {
				sequence->addChild(this->parseElement(blackboard, child, entity));
			}
			return sequence;
		}
		case AINode("Inverter"): {
			auto node = std::make_shared<BrainTree::Inverter>();
			node->setChild(this->parseElement(blackboard, element->FirstChildElement(), entity));
			return node;
		}
		case AINode("Succeeder"): {
			auto node = std::make_shared<BrainTree::Succeeder>();
			node->setChild(this->parseElement(blackboard, element->FirstChildElement(), entity));
			return node;
		}
		case AINode("UntilFail"): {
			auto node = std::make_shared<BrainTree::UntilFail>();
			node->setChild(this->parseElement(blackboard, element->FirstChildElement(), entity));
			return node;
		}
		case AINode("Probability"): {
			int per = element->IntAttribute("per");
			int frac = element->IntAttribute("frac");
#ifdef AI_DEBUG
			std::cout << "AI: probability " << per << "/" << frac << std::endl;
#endif
			auto node = std::make_shared<Probability>(blackboard, per, frac);
			return node;
		}
		case AINode("HasMoreResourcesThan"): {
			float amount = element->FloatAttribute("amount");
			auto node = std::make_shared<HasMoreResourcesThan>(blackboard, entity, amount);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("HasLessObjectsTypeThan"): {
			int qty = element->IntAttribute("qty");
			std::string type = element->Attribute("type");
			auto node = std::make_shared<HasLessObjectsTypeThan>(blackboard, entity, type, qty);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("HasLessObjectsThan"): {
			int qty = element->IntAttribute("qty");
			auto node = std::make_shared<HasLessObjectsThan>(blackboard, entity, qty);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("HasExploredLessThan"): {
			int per = element->IntAttribute("per");
			auto node = std::make_shared<HasExploredLessThan>(blackboard, entity, per);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("Build"): {
			std::string name = element->Attribute("type");
			auto node = std::make_shared<Build>(blackboard, entity, name);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("PlaceBuilt"): {
			auto node = std::make_shared<PlaceBuilt>(blackboard, entity);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("Plant"): {
			std::string name = element->Attribute("type");
			auto node = std::make_shared<Plant>(blackboard, entity, name);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("Train"): {
			std::string name = element->Attribute("type");
			auto node = std::make_shared<Train>(blackboard, entity, name);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("Explore"): {
			std::string name = element->Attribute("type");
			int maxDist = -1;

			if (element->Attribute("maxDist"))
				maxDist = element->IntAttribute("maxDist");

			auto node = std::make_shared<Explore>(blackboard, entity, name, maxDist);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("HasFoundEnemy"): {
			auto node = std::make_shared<HasFoundEnemy>(blackboard, entity);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("SendExpedition"): {
			std::string name = element->Attribute("type");
			int per = element->IntAttribute("per");
			auto node = std::make_shared<SendExpedition>(blackboard, entity, name, per);
			node->map = this->map;
			node->setVault(this->vault);
			return node;
		}
		case AINode("SendDefense"): {
			std::string name = element->Attribute("type");
			int per = element->IntAttribute("per");
			auto node = std::make_shared<SendDefense>(blackboard, entity, name, per);
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
		rebelAI.load("defs/ai/rebel.xml");
		nazAI.load("defs/ai/neonaz.xml");
	}

	void setShared(GameVault *vault, Map *map, int screenWidth, int screenHeight) {
		this->vault = vault;
		this->map = map;
		this->screenWidth = screenWidth;
		this->screenHeight = screenHeight;

		this->rebelAI.setShared(this->vault, map, screenWidth, screenHeight);
		this->nazAI.setShared(this->vault, map, screenWidth, screenHeight);
	}

	void init() {
		auto view = this->vault->registry.view<Player>();
		for (EntityID entity : view) {
			Player &player = view.get(entity);

			if (player.team == "rebel")
			{
				if (player.ai) {
					this->rebelAI.parse(player.team, player.aiTree, entity);
				}
			} else if (player.team == "neonaz") {
				if (player.ai) {
					this->nazAI.parse(player.team, player.aiTree, entity);
				}
			}

		}
	}

	void update(float dt) {
		auto playerView = this->vault->registry.view<Player>();
		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
			if (player.ai)
				player.aiTree.update();
		}
	}

};