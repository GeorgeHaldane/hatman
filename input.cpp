#include "input.h"



// # Input #
void Input::beginNewFrame() { // pressed/released keys matter only for current 1 frame => we clear them each frame
	this->pressedKeys.clear();
	this->releasedKeys.clear();
}
void Input::event_KeyDown(const SDL_Event &event) {
	this->pressedKeys[event.key.keysym.scancode] = true;
	this->heldKeys[event.key.keysym.scancode] = true;
}
void Input::event_KeyUp(const SDL_Event &event) {
	this->releasedKeys[event.key.keysym.scancode] = true;
	this->heldKeys[event.key.keysym.scancode] = false;
}
bool Input::is_KeyPressed(const SDL_Scancode key) {
	return this->pressedKeys[key];
}
bool Input::is_KeyReleased(const SDL_Scancode key) {
	return this->releasedKeys[key];
}
bool Input::is_KeyHeld(const SDL_Scancode key) {
	return this->heldKeys[key];
}
