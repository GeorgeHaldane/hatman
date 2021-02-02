#include "skill.h"

#include "graphics.h" // access to texture loading
#include "game.h" // access to the game state



// # Skill #
Skill::Skill(Parent* parent) :
	parent_creature(parent)
{}

void Skill::update(Milliseconds elapsedTime) {
	if (this->sprite) {
		if (this->parent_creature->orientation == Orientation::RIGHT) {
			*this->sprite_position = this->parent_creature->position + this->alignment;
			this->sprite->flip = SDL_FLIP_NONE;
		}
		else {
			// Flip sprite to another side
			*this->sprite_position = this->parent_creature->position + Vector2(-this->alignment.x, this->alignment.y);
			this->sprite->flip = SDL_FLIP_HORIZONTAL;
		}

		this->sprite->update(elapsedTime);
	}
}
void Skill::draw() const {
	if (!this->animation_timer.finished()) this->sprite->draw();
}

void Skill::use() {
	if (this->sprite) {
		this->sprite->replayAnimation("use", true);
		this->animation_timer.start(this->sprite->getDuration("use")); /// TEMP
	}

	this->cooldown_timer.start(this->cooldown);
}
bool Skill::ready() const {
	return this->cooldown_timer.finished();
}

// Module inits
void Skill::_init_sprite(const std::string &imageFileName, const std::initializer_list<std::pair<Rectangle, Milliseconds>> &animationFrames) {
	this->sprite_position = std::make_unique<Vector2d>();
	
	this->sprite = std::make_unique<ControllableSprite>(
		Graphics::ACCESS->getTexture_Ability(imageFileName),
		*this->sprite_position,
		true,
		false
		);

	this->sprite->addAnimation("use", animationFrames);
	this->sprite->playAnimation("use");
}

void Skill::_init_sprite() {
	this->sprite = nullptr;
}

// Member inits
void Skill::_init_cooldown(Milliseconds cooldown) {
	this->cooldown = cooldown;
}
void Skill::_init_alignment(const Vector2 &alignment) {
	this->alignment = alignment;
}

Vector2 Skill::skill_position() const {
	return this->sprite_position->toVector2();
}



// skills::
// # Slam #
skills::Slam::Slam(Parent* parent) :
	Skill(parent)
{
	const Vector2 spriteSize(64, 10);
	const Vector2 parentSize = parent->solid->getHitbox().getDimensions();

	// Init members
	this->_init_alignment(Vector2(0, (parentSize.y - spriteSize.y) / 2));
	this->_init_cooldown(500);

	// Init modules
	this->_init_sprite(
		"slam.png",
		{
			std::make_pair(Rectangle(Vector2(0, 0), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(0, 10), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(0, 20), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(0, 30), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(0, 40), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(0, 50), spriteSize), 100)
		});	
}

void skills::Slam::use() {
	Skill::use();

	const Rectangle damageArea(
		this->parent_creature->solid->getHitbox().getSideMiddle(Side::BOTTOM),
		Vector2(64, 20),
		true
	);

	const Damage damage(this->parent_creature->health->fraction, 600, 0, 0, 0);

	Game::ACCESS->level.damageInArea(damageArea, damage);
}



// # Slash #
skills::Slash::Slash(Parent* parent) :
	Skill(parent)
{
	const Vector2 spriteSize(7, 13);

	// Init members
	this->_init_alignment(Vector2(10, 0));
	this->_init_cooldown(400);

	// Init modules
	this->_init_sprite(
		"slash.png",
		{
			std::make_pair(Rectangle(Vector2(0, 0), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(7, 0), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(14, 0), spriteSize), 100)
		});
}

void skills::Slash::use() {
	Skill::use();

	const Rectangle damageArea(
		this->skill_position(),
		Vector2(20, 32),
		true
	);

	const Damage damage(this->parent_creature->health->fraction, 50);

	Game::ACCESS->level.damageInArea(damageArea, damage);
}



// # KnockbackAOE #
skills::KnockbackAOE::KnockbackAOE(Parent* parent) :
	Skill(parent)
{
	const Vector2 spriteSize(64, 10);
	const Vector2 parentSize = parent->solid->getHitbox().getDimensions();

	// Init members
	this->_init_alignment(Vector2(0, (parentSize.y - spriteSize.y) / 2));
	this->_init_cooldown(500);

	// Init modules
	this->_init_sprite();
}

void skills::KnockbackAOE::use() {
	Skill::use();

	const int RADIUS = 24;

	const Rectangle damageArea(
		this->parent_creature->position.toVector2(),
		Vector2(RADIUS, RADIUS),
		true
	);

	const Damage damage(this->parent_creature->health->fraction, 100, 0, 0, 0);

	Game::ACCESS->level.damageInArea(damageArea, damage);

	auto &player = Game::ACCESS->level.player;
	auto targetRelativePos = this->parent_creature->position - player->position;

	if (targetRelativePos.length2() < RADIUS * RADIUS) {
		player->solid->applyImpulse(
			Vector2d(-helpers::sign(targetRelativePos.x) * player->solid->mass * 200, player->solid->mass * -150)
		);
	}
}