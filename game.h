#pragma once

#include "timer.h" // 'Timer' class, 'Milliseconds' type
#include "input.h" // 'Input' class
#include "level.h" // 'Level' class




// # Game #
// - Can be accessed wherever #include'ed through static 'READ' and 'ACCESS' fields
// - Holds the game loop
// - Handles most high-level logic
class Game {
public:
	Game(); // inits SDL

	~Game(); // quits SDL

	static const Game* READ; // used for aka 'global' access
	static Game* ACCESS;

	void changeLevel(const std::string &mapName, const Vector2d newPosition, int delay); // changes level to given, version is loaded from save
	bool levelChangeInProgress() const; // returns whether level change is in progress

	double timescale = 1;

	Level level;

	//std::unique_ptr<Player> player;

	Input input;

	Milliseconds _true_time_elapsed = 0;
		// used by some GUI things that calculate time independent from timescale
		// mostly here for FPS counter

private:
	void gameLoop();

	void updateGame(Milliseconds elapsedTime); // updates everything
	void drawGame() const; // draws everything

	// DEVELOPER METHODS, used for debugging and testing!
	void _drawHitboxes() const; // shows a red outline of all hitboxes
	void _drawEmits() const; // shows content of EmitStorage

	bool level_change_requested = false; // true if level change was requested
	std::string level_change_target_name; // name of the level to change
	std::string level_change_target_version; // version of the level loaded from save
	Vector2d level_change_position; // player position on a new level
	Timer level_change_timer; // waits for level change animation to finish
};