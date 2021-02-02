#pragma once

#include <SDL.h> // 'SDL_Event' type
#include <unordered_map> // related type



// # Input #
// - Holds all pressed/released/held keys for a single frame
// - Basically a wrapper for ugly SDL event system
class Input {
public:
	Input() {}

	void beginNewFrame();

	void event_KeyUp(const SDL_Event &event);
	void event_KeyDown(const SDL_Event &event);

	bool is_KeyPressed(SDL_Scancode key);
	bool is_KeyReleased(SDL_Scancode key);
	bool is_KeyHeld(SDL_Scancode key);
private:
	// Note that [] of nonexistant key initializes pair <key, FALSE> in the map (standard C++14)
	std::unordered_map<SDL_Scancode, bool> pressedKeys;
	std::unordered_map<SDL_Scancode, bool> releasedKeys;
	std::unordered_map<SDL_Scancode, bool> heldKeys;
};