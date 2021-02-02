#include "game.h"

#include <SDL.h> // 'SDL_Init()' and SDL event system

#include "graphics.h" // access to rendering updating
#include "saver.h" // access to save loading
#include "emit.h" // acess to 'EmitStorage' (DEV method _drawEmits())
#include "entity_unique.h" /// TEMP



// # Game #
const Game* Game::READ;
Game* Game::ACCESS;

Game::Game() { // initializes SDL subsystems, starts the game loop
	this->READ = this;
	this->ACCESS = this;

	SDL_Init(SDL_INIT_EVERYTHING); /// !!! DETERMINE WHICH INITS ARE NECESSARY !!!

	// Load save file
	const std::string currentLevelName = Saver::ACCESS->get_CurrentLevel();
	const std::string currentLevelVersion = Saver::ACCESS->get_LevelVersion(currentLevelName);
	Vector2d currentPlayerPos = Saver::ACCESS->get_PlayerPosition();

	this->level = Level(
		currentLevelName,
		currentLevelVersion,
		std::make_unique<Player>(currentPlayerPos)
		); // init level and player

	// Turn on some GUI objects
	Graphics::ACCESS->gui->FPSCounter_on();
	Graphics::ACCESS->gui->Healthbar_on();
	Graphics::ACCESS->gui->CDbar_on();
	Graphics::ACCESS->gui->Portrait_on(Forms::HUMAN); /// TEMP

	// Start game loop
	this->gameLoop();
}

Game::~Game() {
	SDL_Quit();
}

void Game::changeLevel(const std::string &mapName, const Vector2d newPosition, int delay) {
	this->level_change_requested = true;
	this->level_change_target_name = mapName;
	this->level_change_target_version = Saver::ACCESS->get_LevelVersion(mapName);
	this->level_change_position = newPosition;
	this->level_change_timer.start(delay);
}

bool Game::levelChangeInProgress() const {
	return this->level_change_requested;
}

void Game::gameLoop() {
	// The game loop itself
	SDL_Event event;
	int LAST_UPDATE_TIME = SDL_GetTicks();
	while (true) {
		// Poll user input to the input object
		this->input.beginNewFrame();
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				this->input.event_KeyDown(event);
			}
			else if (event.type == SDL_KEYUP) {
				if (!event.key.repeat) input.event_KeyUp(event);
			}
			else if (event.type == SDL_QUIT) { // SDL_QUIT produced by pressing red X on the window 
				return;
			}
		}

		// Top-level input handling goes here
		if (this->input.is_KeyPressed(SDL_SCANCODE_ESCAPE)) { // Esc exits the game
			return;
		}
		//if (this->input.is_KeyPressed(SDL_SCANCODE_I)) { /// TEMP
		//	Graphics::ACCESS->gui->inventoryGUI.toggle();
		//}
		

		// Measure frame time (in ms) and update 
		const int CURRENT_TIME = SDL_GetTicks();
		int ELAPSED_TIME = CURRENT_TIME - LAST_UPDATE_TIME;
		LAST_UPDATE_TIME = CURRENT_TIME;

		if (ELAPSED_TIME > 50) { ELAPSED_TIME = 50; } // fix for physics bugging out in low FPS moments
			// this means below 1000/50=20 FPS physics start to slow down 

		this->_true_time_elapsed = static_cast<Milliseconds>(ELAPSED_TIME);

		updateGame(ELAPSED_TIME * this->timescale); // this is all there is to timescale mechanic
		drawGame();
	}
}

void Game::updateGame(const Milliseconds elapsedTime) {
	if (this->level_change_requested && this->level_change_timer.finished()) { // handle level change
		auto player = std::move(this->level.player); // extract player
		this->level = Level(
			this->level_change_target_name,
			this->level_change_target_version,
			std::move(player)
		); // constuct new level and transfer player to it

		this->level.player->position = this->level_change_position; // set player position

		this->level_change_requested = false;
	}

	this->level.update(elapsedTime);


	Graphics::ACCESS->gui->update(elapsedTime);

	EmitStorage::ACCESS->update(elapsedTime);

	TimerController::ACCESS->update(elapsedTime);

	Graphics::ACCESS->camera->position = this->level.player->cameraTrapPos();
}

void Game::drawGame() const {
	this->level.draw();

	//this->_drawHitboxes(); // enable to display hitboxes
	this->_drawEmits(); // enable to display emits

	Graphics::ACCESS->gui->draw();

	Graphics::ACCESS->camera->zoom = 1;

	// Render to screen
	Graphics::ACCESS->camera->cameraToRenderer(); // draw camera content first
	Graphics::ACCESS->gui->GUIToRenderer(); // draw GUI content on top
	Graphics::ACCESS->rendererToWindow(); // apply renderer to screen

	// Clear all rendering textures
	Graphics::ACCESS->camera->cameraClear(); // clear camera backbuffer
	Graphics::ACCESS->gui->GUIClear(); // clear GUI backbuffer
	Graphics::ACCESS->rendererClear(); // clear renderer
}


// DEV methods
void Game::_drawHitboxes() const {
	SDL_Texture* redBorder = Graphics::ACCESS->getTexture("content/textures/hitbox_border.png");

	// Draw tile hitboxes
	for (const auto &tile : this->level.tiles) {
		if (tile.hitbox)
			for (const auto& hitboxRect : tile.hitbox->rectangles) {
				const SDL_Rect destRect = hitboxRect.toSDLRect();
				Graphics::ACCESS->camera->textureToCamera(redBorder, NULL, &destRect);
			}
	}

	// Draw entity hitboxes
	for (const auto &entity : this->level.entities) {
		if (entity.solid) {
			const SDL_Rect destRect = entity.solid->getHitbox().toSDLRect();
			Graphics::ACCESS->camera->textureToCamera(redBorder, NULL, &destRect);
		}
	}

	// Draw player hitbox
	Graphics::ACCESS->camera->textureToCamera(redBorder, NULL, &this->level.player->solid->getHitbox().toSDLRect());
}

void Game::_drawEmits() const {
	// Bypasses GUI text system and draws text directly through font

	Font* const font = Graphics::ACCESS->gui->fonts.at("BLOCKY").get();

	/// TEMP
	font->color_set(RGBColor(255, 0, 0));
	font->draw_line(Vector2(200, 2), std::to_string(this->level.player->solid->speed.x));
	font->draw_line(Vector2(400, 2), std::to_string(this->level.player->solid->speed.y));
	/// TEMP


	font->color_set(RGBColor(0, 0, 0));

	const Vector2 start = Vector2(2, 2);
	Vector2 cursor = start;

	for (const auto &emit : EmitStorage::ACCESS->emits) {
		//const std::string &line = emit.first;

		font->draw_line(cursor, emit.first);
		//for (const auto &letter : line) { cursor = font->draw_symbol(cursor, letter); }
		cursor.x = start.x;
		cursor.y += font->get_monospace().y;
	}
}