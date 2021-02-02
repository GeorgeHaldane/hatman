#include "player.h"

#include "graphics.h" // access to texture loading
#include "game.h" // access to inputs
#include "controls.h" // access to control keys
#include "globalconsts.hpp" // physical consts



// # Player #
namespace player_consts {
	// Human form consts
	const Vector2 HUMAN_HITBOX_DIMENSIONS = Vector2(12, 31);
	const double HUMAN_RUNNING_SPEED = 150;
	const double HUMAN_JUMP_SPEED = 500;

	const int HUMAN_BASE_HP = 1000;
	const int HUMAN_BASE_REGEN = 10;
	const int HUMAN_BASE_PHYS_RES = 0;
	const int HUMAN_BASE_MAGIC_RES = 0;
	const int HUMAN_BASE_DOT_RES = 0;
}

Player::Player(const Vector2d &position) :
	Creature(position),
	current_form(std::make_unique<PlayerForm_Human>(*this)) // start in human form
{
	// Init members
	this->cameraTrap = this->position;
}

void Player::update(Milliseconds elapsedTime) {
	Creature::update(elapsedTime);

	/// TEMP, FIX MAGIC NUMBERS!
	if (this->position.x > this->cameraTrap.x + 30) {
		this->cameraTrap.x = this->position.x - 30;
	}
	if (this->position.x < this->cameraTrap.x - 30) {
		this->cameraTrap.x = this->position.x + 30;
	}
	if (this->position.y > this->cameraTrap.y + 10) {
		this->cameraTrap.y = this->position.y - 10;
	}
	if (this->position.y < this->cameraTrap.y - 80) {
		this->cameraTrap.y = this->position.y + 80;
	}

	this->current_form->update(elapsedTime);
}

void Player::changeForm(Forms newForm) {
	switch (newForm) {
	case Forms::HUMAN:
		this->current_form = std::make_unique<PlayerForm_Human>(*this);
		break;

	case Forms::CHTULHU:
		///this->current_form = std::make_unique<PlayerForm_Chtulhu>(*this);
		break;
	}
}

Vector2d Player::cameraTrapPos() const {
	return this->cameraTrap;
}



/* ### FORMS ### */

// # PlayerForm #
PlayerForm::PlayerForm(Player &parentPlayer) :
	parent_player(parentPlayer)
{}



// # PlayerForm_Human #
PlayerForm_Human::PlayerForm_Human(Player &parentPlayer) :
	PlayerForm(parentPlayer)
{
	// Init sprite
	this->parent_player._init_sprite("form_human.png");

	const Vector2 spriteSize(32, 34);

	this->parent_player._init_animation(
		"idle",
		Rectangle(Vector2(0, 0), spriteSize)
	);

	this->parent_player._init_animation(
		"hatless",
		Rectangle(Vector2(32, 0), spriteSize)
	);

	this->parent_player._init_animation(
		"run",
		Rectangle(Vector2(0, 0), spriteSize) /// Replace when running animation is ready 
		/*{
			std::make_pair(Rectangle(Vector2(32, 0), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(64, 0), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(96, 0), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(128, 0), spriteSize), 100)
		}*/);

	this->parent_player.playAnimation("idle");

	// Init solid
	this->parent_player._init_solid(
		player_consts::HUMAN_HITBOX_DIMENSIONS,
		{
			SolidFlags::SOLID_FOR_TILES,
			SolidFlags::SOLID_FOR_BORDER,
			SolidFlags::AFFECTED_BY_GRAVITY
		},
		physics::DEFAULT_MASS_PLAYER,
		physics::DEFAULT_FRICTION_PLAYER
	);

	// Init health
	this->parent_player._init_health(
		Fraction::PLAYER,
		player_consts::HUMAN_BASE_HP,
		player_consts::HUMAN_BASE_REGEN,
		player_consts::HUMAN_BASE_PHYS_RES,
		player_consts::HUMAN_BASE_MAGIC_RES,
		player_consts::HUMAN_BASE_DOT_RES
	);
}

void PlayerForm_Human::update(Milliseconds elapsedTime) {
	Input &input = Game::ACCESS->input;

	// Input handling
	// Running
	if (input.is_KeyHeld(Controls::READ->LEFT) && Game::ACCESS->input.is_KeyHeld(Controls::READ->RIGHT)) {
		this->stopRunning();
	}
	else if (input.is_KeyHeld(Controls::READ->LEFT)) {
		this->runLeft(elapsedTime);
	}
	else if (input.is_KeyHeld(Controls::READ->RIGHT)) {
		this->runRight(elapsedTime);
	}
	if (!input.is_KeyHeld(Controls::READ->LEFT) && !input.is_KeyHeld(Controls::READ->RIGHT)) {
		this->stopRunning();
	}

	// Jumping
	if (input.is_KeyPressed(Controls::READ->JUMP)) {
		this->jump();
	}

	// GUI
	if (input.is_KeyPressed(Controls::READ->INVENTORY)) {
		Graphics::ACCESS->gui->inventoryGUI.toggle();
	}
	if (input.is_KeyPressed(Controls::READ->FORM_CHANGE)) {
		this->parent_player.playAnimation("idle_nohat");
		this->parent_player.animation_lock_timer.start(1000000);
		// a little clutch: upon press animation is locked for an anreasonably long time
		// and upon release animation is unlocked
		// ! visual bugs may arise if player holds the button for over 16 minutes !

		Graphics::ACCESS->gui->FormSelection_on();
	}
	else if (input.is_KeyReleased(Controls::READ->FORM_CHANGE)) {
		this->parent_player.playAnimation("idle");
		this->parent_player.animation_lock_timer.stop();

		Graphics::ACCESS->gui->FormSelection_off();
	}
}

// Movement
void PlayerForm_Human::runLeft(Milliseconds elapsedTime) {
	if (parent_player.solid->speed.x > -player_consts::HUMAN_RUNNING_SPEED) {
		this->parent_player.solid->applyForce(
			Vector2d(-physics::DEFAULT_MASS_PLAYER * 3000, 0)
			);
	}
	else {
		parent_player.solid->speed.x = -player_consts::HUMAN_RUNNING_SPEED;
	}
	
	this->parent_player.orientation = Orientation::LEFT;
	this->parent_player.playAnimation("run");
}
void PlayerForm_Human::runRight(Milliseconds elapsedTime) {
	if (parent_player.solid->speed.x < player_consts::HUMAN_RUNNING_SPEED) {
		this->parent_player.solid->applyForce(
			Vector2d(physics::DEFAULT_MASS_PLAYER * 3000, 0)
		);
	}
	else {
		parent_player.solid->speed.x = player_consts::HUMAN_RUNNING_SPEED;
	}

	this->parent_player.orientation = Orientation::RIGHT;
	this->parent_player.playAnimation("run");
}
void PlayerForm_Human::stopRunning() {
	this->parent_player.playAnimation("idle");
}
void PlayerForm_Human::jump() {
	if (this->parent_player.solid->isGrounded) {
		///this->parent_player.solid->speed.y = -player_consts::HUMAN_JUMP_SPEED;
		this->parent_player.solid->applyImpulse(Vector2d(0, -physics::DEFAULT_MASS_PLAYER * player_consts::HUMAN_JUMP_SPEED));
		this->parent_player.solid->isGrounded = false;

		this->parent_player.form_change_cooldown.start(1000); /// !!! TEMP !!!
	}
}