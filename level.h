#pragma once

#include <map> // related type (tileset storing, order is important)
#include "nlohmann_external.hpp" // parsing from JSON, 'nlohmann::json' type

#include "geometry_utils.h" // geometry types
#include "tile_base.h" // 'Tile' base class
#include "entity_base.h" // 'Entity' base class
#include "script_base.h" // 'Script' base class
#include "player.h" // 'Player' base class
#include "collection.hpp" // 'Collection' class
#include "timer.h" // 'Milliseconds' type



// # Level #
// - Holds all tiles, entities and scripts present on a map
// - Holds all tilesets, tiles and their hitboxes on the level
// - Holds level background
// - Handles updating and drawing of all aforementioned objects
class Level {
public:
	Level() {};

	Level(const std::string &mapName, const std::string &mapVersion);
	Level(const std::string &mapName, const std::string &mapVersion, std::unique_ptr<Player> &&player);
		// also inits player upon construction

	//Level& operator=(const Level &other) = delete;

	void update(Milliseconds elapsedTime); // updates all logic, scripts and tile animations of the level
	void draw() const;

	// Construction
	void loadLevel(const std::string &mapName);
	void initPlayer(std::unique_ptr<Player> &&player);

	void parseFromJSON(const std::string &filePath);

	// Getters
	const Vector2& getSize() const;
	const std::string& getName() const;
	const std::string& getVersion() const;

	std::unique_ptr<Player> player;

	Collection<Tile> tiles; // holds all individual tiles for a level, polimorphism allows convenient storage and usage of different types of tiles
	Collection<Entity> entities; // holds all entities present on a level
	Collection<Script> scripts;

	// Utility
	bool unfreezed(const Tile& tile) const;
	bool unfreezed(const Entity& entity) const;

	void damageInArea(const Rectangle &area, const Damage &damage);
		// deals damage to every entity in given area (unless fraction is the same)

private:
	void clearDeadEntities();

	// Tile functions
	void parse_tilelayer(const nlohmann::json &tilelayer_node); // does all tilelayer parsing

	// Entity parsing
	void parse_objectgroup(const nlohmann::json &objectgroup_node); // redirects to parse_entity() or parse_script()
	void parse_objectgroup_entity(const nlohmann::json &objectgroup_node);

	// Scripts parsing
	void parse_objectgroup_script_levelChange(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_levelSwitch(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_PlayerInArea(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_AND(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_OR(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_XOR(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_NAND(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_NOR(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_XNOR(const nlohmann::json &objectgroup_node);


	void add_Tile(Tileset &tileset, int id, const Vector2 position); // adds tile to the level with respect to its interactions and etc
	void add_Entity(const std::string &type, const std::string &name, Vector2d position);
	// no need for 'add_Item()' as items can't exist outside of inventories
	// no need for 'add_Script()' as scripts can't be standardiez under the same constructor parameters

	std::map<std::string, Tileset> tilesets; // holds all tilesets for given level   NOTE: map is ordered because we iterate through tilesets when determining tileset by gid

	SDL_Texture* background;

	Vector2 mapSize;

	std::string levelName;
	std::string levelVersion;
};