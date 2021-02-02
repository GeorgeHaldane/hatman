#include "level.h"

#include <fstream> // parsing from JSON (opening a file)

#include "graphics.h" // access to rendering (background)
#include "saver.h" // access to savefile info (level version)
#include "tags.h" // tag utility
#include "tile_unique.h" // creation of unique tiles
#include "entity_unique.h" // creation of unique entities
#include "script_type.h" // creation of scripts




// # Level #
Level::Level(const std::string &mapName, const std::string &mapVersion) :
	levelName(mapName),
	levelVersion(mapVersion),
	player(nullptr)
{
	this->loadLevel(tags::makeTag(mapName, mapVersion));
	
	Graphics::ACCESS->gui->Fade_on(colors::BLACK, colors::BLACK.transparent(), 500);

	Saver::ACCESS->record_Level(mapName, mapVersion);
}

Level::Level(const std::string &mapName, const std::string &mapVersion, std::unique_ptr<Player> &&player) :
	Level(mapName, mapVersion)
{
	this->initPlayer(std::move(player));
}

void Level::update(Milliseconds elapsedTime) {
	for (auto &tile : this->tiles) { if (this->unfreezed(tile)) tile.update(elapsedTime); } // update all tiles
	for (auto &entity : this->entities) { if (this->unfreezed(entity)) entity.update(elapsedTime); }
	for (auto &script : this->scripts) { script.update(elapsedTime); }

	this->player->update(elapsedTime);

	this->clearDeadEntities();
}

void Level::draw() const {
	// Backround first
	Graphics::ACCESS->copyTextureToRenderer(this->background, NULL, NULL); // background bypasses camera !!!

	// Then tiles
	for (const auto &tile : this->tiles) { if (this->unfreezed(tile)) tile.draw(); }

	// Then entities
	for (const auto &entity : this->entities) { if (this->unfreezed(entity)) entity.draw(); }

	this->player->draw();
}

// Construction
void Level::loadLevel(const std::string &mapName) {
	this->parseFromJSON("content/levels/" + mapName + ".json");
}

void Level::initPlayer(std::unique_ptr<Player> &&player) {
	this->player = std::move(player);
}


// Getters
const Vector2& Level::getSize() const { return this->mapSize; }
const std::string& Level::getName() const { return this->levelName; }
const std::string& Level::getVersion() const { return this->levelVersion; }

// Internal
void Level::clearDeadEntities() {
	for (auto iter = this->entities.begin(); iter != this->entities.end();) {
		if (iter->marked_for_erase()) {
			this->entities.erase(iter++);
		}
		else {
			++iter;
		}
	}
}

void Level::add_Tile(Tileset &tileset, int id, const Vector2 position) {
	this->tiles.insert(tiles::make_tile(tileset, id, position));
}
void Level::add_Entity(const std::string &type, const std::string &name, Vector2d position) {
	this->entities.insert(entities::make_entity(type, name, position));
}

// Utility
const int UNFREEZE_DISTANCE_TILE = 600 * 600; // square of distance where objects are unfreezed
const int UNFREEZE_DISTANCE_ENTITY = 400 * 400; // less than for tiles to prevent objects falling through terrain

bool Level::unfreezed(const Tile& tile) const {
	return (this->player->position - tile.position).length2_rough() < UNFREEZE_DISTANCE_TILE;
}
bool Level::unfreezed(const Entity& entity) const {
	return (this->player->position - entity.position).length2_rough() < UNFREEZE_DISTANCE_ENTITY;
}

void Level::damageInArea(const Rectangle &area, const Damage &damage) {
	// Look for entities that should be damaged
	for (auto &entity : this->entities) {
		// Deal damage if entity has health+hitbox, is in the area, and of the enemy faction
		if (entity.health && entity.solid && area.overlapsWithRect(entity.solid->getHitbox())) {
			entity.health->applyDamage(damage);
		}
	}

	// Check the same for player
	if (area.overlapsWithRect(this->player->solid->getHitbox())) {
		this->player->health->applyDamage(damage);
	}
}


// Parsing
void Level::parseFromJSON(const std::string &filePath) {
	// Load JSON doc
	std::ifstream ifStream(filePath);
	nlohmann::json JSON = nlohmann::json::parse(ifStream);

	// Parse map properties
	for (const auto &property_node : JSON["properties"]) {
		const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

		if (prefix == "background") {
			this->background = Graphics::ACCESS->getTexture_Background(property_node["value"].get<std::string>());
		}
		// new properties go there
	}

	// Parse tilesets
	const nlohmann::json &tilesets_array_node = JSON["tilesets"];
	for (const auto &tileset_node : tilesets_array_node) {
		// Extract tileset file name
		std::string fileName = tileset_node["source"].get<std::string>();
		fileName = fileName.substr(fileName.rfind("/") + 1); // cut before '/'
		fileName = fileName.substr(fileName.rfind("\\") + 1); // cut before '\'

		Tileset tileset = TilesetStorage::ACCESS->getTileset(fileName);
		tileset.firstGid = tileset_node["firstgid"].get<int>(); // firstgid is map-dependant (that's also why we copy tilesets)

		this->tilesets[fileName] = std::move(tileset);
	}

	// Parse map properties (size and etc)
	this->mapSize.x = JSON["width"].get<int>();
	this->mapSize.y = JSON["height"].get<int>();

	// Parse tile layers
	const nlohmann::json &layers_array_node = JSON["layers"];
	for (const auto &layer_node : layers_array_node) {
		const std::string layer_type = layer_node["type"].get<std::string>(); // can be "tilelayer" or "objectgroup"

		// Parse data depending on layer_type
		if (layer_type == "tilelayer") {
			this->parse_tilelayer(layer_node);
		}
		else if (layer_type == "objectgroup") {
			this->parse_objectgroup(layer_node);
		}
	}

}

void Level::parse_tilelayer(const nlohmann::json &tilelayer_node) {
	int tileCount = 0; // used to determine tile position

	const nlohmann::json &data_array_node = tilelayer_node["data"];
	for (const auto &data_node : data_array_node) {
		const int gid = data_node.get<int>();
		if (gid) { // if tile is present
			// Calculate tile position
			const Vector2 tilePosition(
				(tileCount % this->mapSize.x) * 32,
				(tileCount / this->mapSize.x) * 32
			);

			// Determine which tileset tile belongs to (based on gid)
			std::string correspondingTileset;
			for (auto const &tileset : this->tilesets) {
				if (gid >= tileset.second.firstGid) { correspondingTileset = tileset.second.getFileName(); }
			}

			const int tileId = gid - this->tilesets.at(correspondingTileset).firstGid;

			// Construct tile and add it to the level
			Tileset &tileTileset = this->tilesets.at(correspondingTileset);

			// Add a tile based on its interactive_type (if present)
			this->add_Tile(tileTileset, tileId, tilePosition);
		}
		++tileCount;
	}
}

void Level::parse_objectgroup(const nlohmann::json &objectgroup_node) {
	// get layer prefix and suffix
	const std::string layer_prefix = tags::getPrefix(objectgroup_node["name"].get<std::string>());
	const std::string layer_suffix = tags::getSuffix(objectgroup_node["name"].get<std::string>());

	if (layer_prefix == "entity") {
		this->parse_objectgroup_entity(objectgroup_node);
	}
	else if (layer_prefix == "script") {
		if (layer_suffix == "level_change") {
			this->parse_objectgroup_script_levelChange(objectgroup_node);
		}
		else if (layer_suffix == "level_switch") {
			this->parse_objectgroup_script_levelSwitch(objectgroup_node);
		}
		else if (layer_suffix == "player_in_area") {
			this->parse_objectgroup_script_PlayerInArea(objectgroup_node);
		}
		else if (layer_suffix == "AND") {
			this->parse_objectgroup_script_AND(objectgroup_node);
		}
		else if (layer_suffix == "OR") {
			this->parse_objectgroup_script_OR(objectgroup_node);
		}
		else if (layer_suffix == "XOR") {
			this->parse_objectgroup_script_XOR(objectgroup_node);
		}
		else if (layer_suffix == "NAND") {
			this->parse_objectgroup_script_NAND(objectgroup_node);
		}
		else if (layer_suffix == "NOR") {
			this->parse_objectgroup_script_NOR(objectgroup_node);
		}
		else if (layer_suffix == "XNOR") {
			this->parse_objectgroup_script_XNOR(objectgroup_node);
		}
		// new script types go there
	}
}

// Entity types parsing
void Level::parse_objectgroup_entity(const nlohmann::json &objectgroup_node) {
	const std::string entity_type = tags::getSuffix(objectgroup_node["name"].get<std::string>());

	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse position
		const Vector2 entityPosition(
			object_node["x"].get<int>(),
			object_node["y"].get<int>()
		);

		// Parse custom properties
		std::string entityName;

		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "name") {
				entityName = property_node["value"].get<std::string>();
			}
		}

		this->add_Entity(entity_type, entityName, entityPosition);
	}
}

// Scripts parsing
void Level::parse_objectgroup_script_levelChange(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse hitbox
		const Rectangle hitbox = Rectangle(
			object_node["x"].get<int>(), object_node["y"].get<int>(),
			object_node["width"].get<int>(), object_node["height"].get<int>()
		);

		std::string goes_to_level;
		Vector2 goes_to_pos;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "goes_to_level") {
				goes_to_level = property_node["value"].get<std::string>();
			}
			else if (prefix == "goes_to_x") {
				goes_to_pos.x = property_node["value"].get<int>();
			}
			else if (prefix == "goes_to_y") {
				goes_to_pos.y = property_node["value"].get<int>();
			}
		}

		this->scripts.insert(std::make_unique<scripts::LevelChange>(hitbox, goes_to_level, goes_to_pos));
	}
}
void Level::parse_objectgroup_script_levelSwitch(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse hitbox
		const Rectangle hitbox = Rectangle(
			object_node["x"].get<int>(), object_node["y"].get<int>(),
			object_node["width"].get<int>(), object_node["height"].get<int>()
		);

		std::string goes_to_level;
		Vector2 goes_to_pos;

		std::string emit_output = ""; // optional
		int emit_output_lifetime = 0; // optional

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "goes_to_level") {
				goes_to_level = property_node["value"].get<std::string>();
			}
			else if (prefix == "goes_to_x") {
				goes_to_pos.x = property_node["value"].get<int>();
			}
			else if (prefix == "goes_to_y") {
				goes_to_pos.y = property_node["value"].get<int>();
			}
		}

		this->scripts.insert(std::make_unique<scripts::LevelSwitch>(hitbox, goes_to_level, goes_to_pos));
	}
}
void Level::parse_objectgroup_script_PlayerInArea(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse hitbox
		const Rectangle hitbox = Rectangle(
			object_node["x"].get<int>(), object_node["y"].get<int>(),
			object_node["width"].get<int>(), object_node["height"].get<int>()
		);

		std::string emit_output = ""; // optional
		int emit_output_lifetime = 0; // optional

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "emit_output") {
				emit_output = property_node["value"].get<std::string>();
			}
			else if (prefix == "emit_output_lifetime") {
				emit_output_lifetime = property_node["value"].get<int>();
			}
		}

		auto script = std::make_unique<scripts::PlayerInArea>(hitbox);
		script->setOutput(emit_output, emit_output_lifetime);

		this->scripts.insert(std::move(script));
	}
}
void Level::parse_objectgroup_script_AND(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {

		std::string emit_output = ""; // optional
		int emit_output_lifetime = 0; // optional

		std::unordered_set<std::string> emit_inputs;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "emit_output") {
				emit_output = property_node["value"].get<std::string>();
			}
			else if (prefix == "emit_output_lifetime") {
				emit_output_lifetime = property_node["value"].get<int>();
			}
			else if (prefix == "emit_input") {
				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
				emit_inputs.insert(property_node["value"].get<std::string>());
			}
		}

		auto script = std::make_unique<scripts::AND>(emit_inputs);
		script->setOutput(emit_output, emit_output_lifetime);

		this->scripts.insert(std::move(script));
	}
}
void Level::parse_objectgroup_script_OR(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {

		std::string emit_output = ""; // optional
		int emit_output_lifetime = 0; // optional

		std::unordered_set<std::string> emit_inputs;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "emit_output") {
				emit_output = property_node["value"].get<std::string>();
			}
			else if (prefix == "emit_output_lifetime") {
				emit_output_lifetime = property_node["value"].get<int>();
			}
			else if (prefix == "emit_input") {
				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
				emit_inputs.insert(property_node["value"].get<std::string>());
			}
		}

		auto script = std::make_unique<scripts::OR>(emit_inputs);
		script->setOutput(emit_output, emit_output_lifetime);

		this->scripts.insert(std::move(script));
	}
}
void Level::parse_objectgroup_script_XOR(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {

		std::string emit_output = ""; // optional
		int emit_output_lifetime = 0; // optional

		std::unordered_set<std::string> emit_inputs;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "emit_output") {
				emit_output = property_node["value"].get<std::string>();
			}
			else if (prefix == "emit_output_lifetime") {
				emit_output_lifetime = property_node["value"].get<int>();
			}
			else if (prefix == "emit_input") {
				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
				emit_inputs.insert(property_node["value"].get<std::string>());
			}
		}

		auto script = std::make_unique<scripts::XOR>(emit_inputs);
		script->setOutput(emit_output, emit_output_lifetime);

		this->scripts.insert(std::move(script));
	}
}
void Level::parse_objectgroup_script_NAND(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {

		std::string emit_output = ""; // optional
		int emit_output_lifetime = 0; // optional

		std::unordered_set<std::string> emit_inputs;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "emit_output") {
				emit_output = property_node["value"].get<std::string>();
			}
			else if (prefix == "emit_output_lifetime") {
				emit_output_lifetime = property_node["value"].get<int>();
			}
			else if (prefix == "emit_input") {
				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
				emit_inputs.insert(property_node["value"].get<std::string>());
			}
		}

		auto script = std::make_unique<scripts::NAND>(emit_inputs);
		script->setOutput(emit_output, emit_output_lifetime);

		this->scripts.insert(std::move(script));
	}
}
void Level::parse_objectgroup_script_NOR(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {

		std::string emit_output = ""; // optional
		int emit_output_lifetime = 0; // optional

		std::unordered_set<std::string> emit_inputs;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "emit_output") {
				emit_output = property_node["value"].get<std::string>();
			}
			else if (prefix == "emit_output_lifetime") {
				emit_output_lifetime = property_node["value"].get<int>();
			}
			else if (prefix == "emit_input") {
				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
				emit_inputs.insert(property_node["value"].get<std::string>());
			}
		}

		auto script = std::make_unique<scripts::NOR>(emit_inputs);
		script->setOutput(emit_output, emit_output_lifetime);

		this->scripts.insert(std::move(script));
	}
}
void Level::parse_objectgroup_script_XNOR(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {

		std::string emit_output = ""; // optional
		int emit_output_lifetime = 0; // optional

		std::unordered_set<std::string> emit_inputs;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "emit_output") {
				emit_output = property_node["value"].get<std::string>();
			}
			else if (prefix == "emit_output_lifetime") {
				emit_output_lifetime = property_node["value"].get<int>();
			}
			else if (prefix == "emit_input") {
				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
				emit_inputs.insert(property_node["value"].get<std::string>());
			}
		}

		auto script = std::make_unique<scripts::XNOR>(emit_inputs);
		script->setOutput(emit_output, emit_output_lifetime);

		this->scripts.insert(std::move(script));
	}
}