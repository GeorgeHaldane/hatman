#pragma once

#include "entity_type.h" // 'Creature' base class
#include "inventory.h" // 'Inventory' module



class PlayerForm;

// # Forms #
enum class Forms {
	HUMAN,
	CHTULHU
};



// # Player #
// - Holds methods specific to player, but not the particular form
class Player : public entity_primitive_types::Creature {
	friend ::PlayerForm;
	friend class PlayerForm_Human;
	friend class PlayerForm_Chtulhu;
public:
	Player() = delete;

	Player(const Vector2d &position);

	virtual ~Player() = default;

	void update(Milliseconds elapsedTime) override;

	void changeForm(Forms newForm);

	Vector2d cameraTrapPos() const;

	Inventory inventory;

	Timer form_change_cooldown;
private:
	Vector2d cameraTrap;

	std::unique_ptr<PlayerForm> current_form;
};



// # PlayerForm #
class PlayerForm {
public:
	PlayerForm() = delete;

	PlayerForm(Player &parentPlayer); // bounds to the player, applies stat modifiers, changes sprite and solid

	virtual ~PlayerForm() = default; // removes applied stat modifiers

	virtual void update(Milliseconds elapsedTime) = 0;

protected:
	Player &parent_player;
};



// # PlayerForm_Human #
class PlayerForm_Human : public PlayerForm {
public:
	PlayerForm_Human() = delete;

	PlayerForm_Human(Player &parentPlayer); // bounds to the player, applies stat modifiers, changes sprite and solid

	~PlayerForm_Human() = default; // human has no on-form-change effects

	void update(Milliseconds elapsedTime);

private:
	void runLeft(Milliseconds elapsedTime);
	void runRight(Milliseconds elapsedTime);
	void stopRunning();
	void jump();
};



// # PlayerForm_Chtulhu #
class PlayerForm_Chtulhu : public PlayerForm {
public:
	PlayerForm_Chtulhu() = delete;

	PlayerForm_Chtulhu(Player &parentPlayer); // bounds to the player, applies stat modifiers, changes sprite and solid

	~PlayerForm_Chtulhu();

	void update(Milliseconds elapsedTime);

private:
	void runLeft();
	void runRight();
	void stopRunning();
	void jump();
};