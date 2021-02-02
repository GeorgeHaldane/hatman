#include "saver.h"

#include <fstream> // parsing from JSON (opening a file)
#include <iomanip> // used to "beautify" savefile JSON

#include "game.h" // access to game state



// # Saver #
const Saver* Saver::READ;
Saver* Saver::ACCESS;

Saver::Saver(const std::string &filePath) :
	save_filepath(filePath)
{
	this->READ = this;
	this->ACCESS = this;

	std::ifstream inFile(this->save_filepath);
	if (inFile.good()) { this->state = nlohmann::json::parse(inFile); } // savefile is present => load
	else { this->makeNewSave(); } // savefile not present => create new
	
}

void Saver::makeNewSave() {
	const std::string firstLevel = "GrayBricks_1";
	const Vector2d playerSpawnpoint(100, 240);
	
	this->state["player"]["current_level"] = firstLevel;
	this->state["player"]["x"] = playerSpawnpoint.x;
	this->state["player"]["y"] = playerSpawnpoint.y;

	this->state["levels"][firstLevel] = "default";

	this->save();
}

void Saver::save() {
	std::ofstream file(this->save_filepath);
	file << std::setw(4) << this->state; // setw() pretifies JSON so it is no a single line
	file.close();
}

// Recorders
void Saver::record_Level(const std::string &name, const std::string &version) {
	this->state["levels"][name] = version;
}
void Saver::record_Player() {
	this->state["player"]["current_level"] = Game::ACCESS->level.getName();
	this->state["player"]["x"] = Game::ACCESS->level.player->position.x;
	this->state["player"]["y"] = Game::ACCESS->level.player->position.y;
}

// Getters
std::string Saver::get_CurrentLevel() const {
	return this->state["player"]["current_level"].get<std::string>();
}
Vector2d Saver::get_PlayerPosition() const {
	return Vector2d(this->state["player"]["x"].get<double>(), this->state["player"]["y"].get<double>());
}
std::string Saver::get_LevelVersion(const std::string &levelName) const {
	if (this->state["levels"].find(levelName) != this->state["levels"].end()) {
		return this->state["levels"][levelName].get<std::string>();
	}
	else {
		return "default";
	}
}