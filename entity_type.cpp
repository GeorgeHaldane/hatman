#include "entity_type.h"

#include "graphics.h" // access to texture loading
#include "game.h" // access to game state
#include "item_unique.h" // item classes (to assign loot to entities)
#include "controls.h" // access to control keys
#include "globalconsts.hpp" // physical consts


/* ### PRIMITIVE TYPES ### */

// # Creature #
entity_primitive_types::Creature::Creature(const Vector2d &position) :
	Entity(position)
{}

void entity_primitive_types::Creature::update(Milliseconds elapsedTime) {
	Entity::update(elapsedTime);

	if (this->animation_lock_timer.finished()) { // don't flip the sprite if animation lock is present
		this->sprite->flip = (this->orientation == Orientation::RIGHT) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
	}	

	for (auto &skill : this->skill_map) { skill.second->update(elapsedTime); }

	if (this->health->dead()) {
		this->onDeath();
		this->mark_for_erase();
	}
}

void entity_primitive_types::Creature::draw() const {
	Entity::draw();

	for (auto &skill : this->skill_map) { skill.second->draw(); }
}

// Module inits
void entity_primitive_types::Creature::_init_sprite(const std::string &imageFileName) {
	this->sprite = std::make_unique<ControllableSprite>(
		Graphics::ACCESS->getTexture_Entity(imageFileName),
		this->position,
		true,
		false
		);
	this->_sprite = dynamic_cast<ControllableSprite*>(this->sprite.get());
}
void entity_primitive_types::Creature::_init_solid(const Vector2 &hitboxSize, const std::initializer_list<SolidFlags> &flags, double mass, double friction) {
	this->solid = std::make_unique<SolidRectangle>(
		this->position,
		hitboxSize,
		flags,
		mass,
		friction
		);
}
void entity_primitive_types::Creature::_init_health(Fraction fraction, uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes) {
	this->health = std::make_unique<Health>(fraction, maxHp, regen, physRes, magicRes, dotRes);
}

// Member inits
void entity_primitive_types::Creature::_init_animation(const std::string &name, const std::initializer_list< std::pair<Rectangle, Milliseconds> > &frames) {
	this->_sprite->addAnimation(name, frames);
}
void entity_primitive_types::Creature::_init_animation(const std::string &name, const Rectangle &frame) {
	this->_sprite->addAnimation(name, frame);
}

void entity_primitive_types::Creature::playAnimation(const std::string &name, bool once) {
	if (this->animation_lock_timer.finished()) {
		this->_sprite->playAnimation(name, once);
	}
}

// Behaviour
void entity_primitive_types::Creature::onDeath() {}



/* ### TYPES ### */

// # ItemEntity #
entity_types::ItemEntity::ItemEntity(const Vector2d &position) :
	Entity(position)
{}

void entity_types::ItemEntity::update(Milliseconds elapsedTime) {
	Entity::update(elapsedTime);

	if (this->enabled) {
		if (this->checkActivation()) {
			this->activate();

			if (this->checkTrigger()) { this->trigger(); }
		}
		
	}
}

bool entity_types::ItemEntity::checkActivation() const {
	bool res = false;

	/// OPTIMIZE
	if (this->solid) {
		if (this->solid->getHitbox().overlapsWithRect(Game::ACCESS->level.player->solid->getHitbox())) {
			res = true;
		}
	}

	return res;
} 
bool entity_types::ItemEntity::checkTrigger() const {
	bool res = false;

	if (Game::ACCESS->input.is_KeyPressed(Controls::READ->USE)) {
		res = true;
		/// Check if player has free inventory space
	}

	return res;
}
void entity_types::ItemEntity::activate() {
	// No default effects
}
void entity_types::ItemEntity::trigger() {
	auto item = items::make_item(this->name);
	Game::ACCESS->level.player->inventory.addItem(*item);

	this->mark_for_erase();
}

// Module inits
void entity_types::ItemEntity::_init_sprite(const std::string &imageFileName, const std::initializer_list<std::pair<Rectangle, Milliseconds>> &animationFrames) {
	this->sprite = std::make_unique<AnimatedSprite>(
		Graphics::ACCESS->getTexture_Entity(imageFileName),
		this->position,
		true,
		false,
		animationFrames
		);
}
void entity_types::ItemEntity::_init_sprite(const std::string &imageFileName) {
	this->sprite = std::make_unique<StaticSprite>(
		Graphics::ACCESS->getTexture_Entity(imageFileName),
		this->position,
		true,
		false
		);
}
void entity_types::ItemEntity::_init_solid(const Vector2 &hitboxSize) {
	this->solid = std::make_unique<SolidRectangle>(
		this->position,
		hitboxSize,
		make_init_list({
			SolidFlags::AFFECTED_BY_GRAVITY,
			SolidFlags::SOLID_FOR_TILES,
			SolidFlags::SOLID_FOR_BORDER
			}),
		physics::DEFAULT_MASS_ITEMS,
		physics::DEFAULT_FRICTION_ITEMS
		);
}

// Member inits
void entity_types::ItemEntity::_init_name(const std::string &name) {
	this->name = name;
}



// # Destructible #
entity_types::Destructible::Destructible(const Vector2d &position) :
	Entity(position)
{}

void entity_types::Destructible::update(Milliseconds elapsedTime) {
	Entity::update(elapsedTime);

	if (this->health->dead()) {
		// Trigger effect/animation if not triggered yet
		if (!this->effect_triggered) {
			this->effect();
			this->effect_triggered = true;

			this->_sprite->playAnimation("death", true);

			this->timer.start(this->erasion_delay);
		}

		// Erase entity after delay
		if (this->timer.finished()) {
			this->mark_for_erase();
		}
	}
}

void entity_types::Destructible::effect() {} // nothing by default

// Module inits
void entity_types::Destructible::_init_sprite(
	const std::string &imageFileName,
	const std::initializer_list<std::pair<Rectangle, Milliseconds>> &defaulAnimationFrames,
	const std::initializer_list<std::pair<Rectangle, Milliseconds>> &deathAnimationFrames
) {
	this->sprite = std::make_unique<ControllableSprite>(
		Graphics::ACCESS->getTexture_Entity(imageFileName),
		this->position,
		true,
		false
		);
	this->_sprite = dynamic_cast<ControllableSprite*>(this->sprite.get());

	this->_sprite->addAnimation("default", defaulAnimationFrames);
	this->_sprite->addAnimation("death", deathAnimationFrames);

	this->_sprite->playAnimation("default");
}
void entity_types::Destructible::_init_solid(const Vector2 &hitboxSize) {
	this->solid = std::make_unique<SolidRectangle>(
		this->position,
		hitboxSize,
		make_init_list({
			SolidFlags::AFFECTED_BY_GRAVITY,
			SolidFlags::SOLID_FOR_TILES,
			SolidFlags::SOLID_FOR_BORDER
			}),
		physics::DEFAULT_MASS_CREATURES,
		physics::DEFAULT_FRICTION_CREATURES
		);
}
void entity_types::Destructible::_init_health(Fraction fraction, uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes) {
	this->health = std::make_unique<Health>(fraction, maxHp, regen, physRes, magicRes, dotRes);
}

// Member inits
void entity_types::Destructible::_init_delay(int erasionDelay) {
	this->erasion_delay = erasionDelay;
}



// # Enemy #
entity_types::Enemy::Enemy(const Vector2d &position) :
	Creature(position),
	aggroed(false),
	target(Game::READ->level.player.get()),
	target_relative_pos(0, 0)
{}

void entity_types::Enemy::update(Milliseconds elapsedTime) {
	Creature::update(elapsedTime);

	this->target_relative_pos = Game::READ->level.player->position - this->position;

	if (this->aggroed) {
		if (this->deaggroCheck()) { this->aggroed = false; }
		else {
			if (this->canAttack()) { this->attack(elapsedTime); }
			else { this->chase(elapsedTime); }
		}
	}
	else {
		if (this->aggroCheck()) { this->aggroed = true; }
		else { this->wander(elapsedTime); }
	}
}

// Behaviour
bool entity_types::Enemy::aggroCheck() { return false; }
bool entity_types::Enemy::deaggroCheck() { return false; }

void entity_types::Enemy::wander(Milliseconds elapsedTime) {}

bool entity_types::Enemy::canAttack() { return false; }

void entity_types::Enemy::chase(Milliseconds elapsedTime) {}
void entity_types::Enemy::attack(Milliseconds elapsedTime) {}

void entity_types::Enemy::onDeath() {}