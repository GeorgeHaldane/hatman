#include "script_type.h"

#include "graphics.h" // access to GUI methods
#include "game.h" // access to game state
#include "emit.h" // access to 'EmitStorage'
#include "controls.h" // access to control keys



// # LevelChange #
scripts::LevelChange::LevelChange(const Rectangle &hitbox, const std::string &goesToLevel, const Vector2 &goesToPos) :
	hitbox(hitbox),
	goes_to_level(goesToLevel),
	goes_to_pos(goesToPos)
{}

bool scripts::LevelChange::checkTrigger() const {
	return Game::ACCESS->level.player->solid->getHitbox().overlapsWithRect(this->hitbox);
}
void scripts::LevelChange::trigger() {
	if (!Game::ACCESS->levelChangeInProgress()) {
		const int FADE_DURATION = 500;

		Graphics::ACCESS->gui->Fade_on(colors::BLACK.transparent(), colors::BLACK, FADE_DURATION);

		Game::ACCESS->changeLevel(this->goes_to_level, this->goes_to_pos, FADE_DURATION);
	}
}



// # LevelSwitch #
scripts::LevelSwitch::LevelSwitch(const Rectangle &hitbox, const std::string &goesToLevel, const Vector2 &goesToPos) :
	hitbox(hitbox),
	goes_to_level(goesToLevel),
	goes_to_pos(goesToPos)
{}

bool scripts::LevelSwitch::checkTrigger() const {
	return
		Game::ACCESS->level.player->solid->getHitbox().overlapsWithRect(this->hitbox)
		&& Game::ACCESS->input.is_KeyPressed(Controls::READ->USE);
}
void scripts::LevelSwitch::trigger() {
	if (!Game::ACCESS->levelChangeInProgress()) {
		const int FADE_DURATION = 500;

		Graphics::ACCESS->gui->Fade_on(colors::BLACK.transparent(), colors::BLACK, FADE_DURATION);

		Game::ACCESS->changeLevel(this->goes_to_level, this->goes_to_pos, FADE_DURATION);
	}
}



// # PlayerInArea #
scripts::PlayerInArea::PlayerInArea(const Rectangle &hitbox) : hitbox(hitbox) {}

bool scripts::PlayerInArea::checkTrigger() const {
	return Game::ACCESS->level.player->solid->getHitbox().overlapsWithRect(this->hitbox);
}



// # AND #
scripts::AND::AND(const std::unordered_set<std::string> &emitInputs) : emit_inputs(emitInputs) {}
scripts::AND::AND(std::unordered_set<std::string> &&emitInputs) : emit_inputs(emitInputs) {}

bool scripts::AND::checkTrigger() const {
	if (!EmitStorage::ACCESS->changed()) { return false; } // no need to check if there is no change

	bool res = true;
	
	for (const auto &emit : this->emit_inputs) {
		if (!EmitStorage::ACCESS->emit_present(emit)) { res = false; break; }
	}

	return res;
}



// # OR #
scripts::OR::OR(const std::unordered_set<std::string> &emitInputs) : emit_inputs(emitInputs) {}
scripts::OR::OR(std::unordered_set<std::string> &&emitInputs) : emit_inputs(emitInputs) {}

bool scripts::OR::checkTrigger() const {
	if (!EmitStorage::ACCESS->changed()) { return false; } // no need to check if there is no change

	bool res = false;

	for (const auto &emit : this->emit_inputs) {
		if (EmitStorage::ACCESS->emit_present(emit)) { res = true; break; }
	}

	return res;
}



// # XOR #
scripts::XOR::XOR(const std::unordered_set<std::string> &emitInputs) : emit_inputs(emitInputs) {}
scripts::XOR::XOR(std::unordered_set<std::string> &&emitInputs) : emit_inputs(emitInputs) {}

bool scripts::XOR::checkTrigger() const {
	if (!EmitStorage::ACCESS->changed()) { return false; } // no need to check if there is no change

	int inputs = 0;

	for (const auto &emit : this->emit_inputs) {
		if (EmitStorage::ACCESS->emit_present(emit)) { ++inputs; }
	}

	return (inputs % 2);
}



// # NAND #
scripts::NAND::NAND(const std::unordered_set<std::string> &emitInputs) : emit_inputs(emitInputs) {}
scripts::NAND::NAND(std::unordered_set<std::string> &&emitInputs) : emit_inputs(emitInputs) {}

bool scripts::NAND::checkTrigger() const {
	if (!EmitStorage::ACCESS->changed()) { return false; } // no need to check if there is no change

	bool res = true;

	for (const auto &emit : this->emit_inputs) {
		if (EmitStorage::ACCESS->emit_present(emit)) { res = false; break; }
	}

	return res;
}



// # NOR #
scripts::NOR::NOR(const std::unordered_set<std::string> &emitInputs) : emit_inputs(emitInputs) {}
scripts::NOR::NOR(std::unordered_set<std::string> &&emitInputs) : emit_inputs(emitInputs) {}

bool scripts::NOR::checkTrigger() const {
	if (!EmitStorage::ACCESS->changed()) { return false; } // no need to check if there is no change

	bool res = false;

	for (const auto &emit : this->emit_inputs) {
		if (!EmitStorage::ACCESS->emit_present(emit)) { res = true; break; }
	}

	return res;
}



// # XNOR #
scripts::XNOR::XNOR(const std::unordered_set<std::string> &emitInputs) : emit_inputs(emitInputs) {}
scripts::XNOR::XNOR(std::unordered_set<std::string> &&emitInputs) : emit_inputs(emitInputs) {}

bool scripts::XNOR::checkTrigger() const {
	if (!EmitStorage::ACCESS->changed()) { return false; } // no need to check if there is no change

	int inputs = 1;

	for (const auto &emit : this->emit_inputs) {
		if (EmitStorage::ACCESS->emit_present(emit)) { ++inputs; }
	}

	return (inputs % 2);
}