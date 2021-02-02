#pragma once

#include <SDL_events.h> // 'SDL_Scancode' type



class Controls {
public:
	Controls();

	static const Controls* READ;
	static Controls* ACCESS;

	SDL_Scancode LEFT;
	SDL_Scancode RIGHT;
	SDL_Scancode UP;
	SDL_Scancode DOWN;

	SDL_Scancode JUMP;
	SDL_Scancode USE;
	SDL_Scancode INVENTORY;

	SDL_Scancode FORM_CHANGE;
	SDL_Scancode FORM_CHANGE_LEFT;
	SDL_Scancode FORM_CHANGE_RIGHT;
	SDL_Scancode FORM_CHANGE_UP;	
	SDL_Scancode FORM_CHANGE_DOWN;
	
};