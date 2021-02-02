#include "controls.h"



// # Controls #
const Controls* Controls::READ;
Controls* Controls::ACCESS;

Controls::Controls() {
	this->READ = this;
	this->ACCESS = this;  

	this->LEFT = SDL_SCANCODE_A;
	this->RIGHT = SDL_SCANCODE_D;
	this->UP = SDL_SCANCODE_W;
	this->DOWN = SDL_SCANCODE_S;

	this->JUMP = SDL_SCANCODE_SPACE;
	this->USE = SDL_SCANCODE_E;
	this->INVENTORY = SDL_SCANCODE_I;

	this->FORM_CHANGE = SDL_SCANCODE_RCTRL;
	this->FORM_CHANGE_LEFT = SDL_SCANCODE_LEFT;
	this->FORM_CHANGE_RIGHT = SDL_SCANCODE_RIGHT;
	this->FORM_CHANGE_UP = SDL_SCANCODE_UP;
	this->FORM_CHANGE_DOWN = SDL_SCANCODE_DOWN;
}