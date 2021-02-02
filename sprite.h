#pragma once

/* Contains modules: 'StaticSprite', 'AnimatedSprite', 'ControllableSprite' */

#include <SDL.h> // 'SDL_texture' and related types
#include <string> // related type
#include <initializer_list> // related type
#include <unordered_map> // related type

#include "timer.h" // 'Milliseconds' type
#include "geometry_utils.h" // geometry types



// # Animation #
// - Contains frames of an animation and delays between these frames
// - Does NOT handle time recording, updating of frame indexes and etc
struct Animation {
	Animation() = default;

	Animation(const std::vector< std::pair<Rectangle, Milliseconds> > &frames);
	Animation(const std::initializer_list< std::pair<Rectangle, Milliseconds> > &frames);
	Animation(const Rectangle &frame); // creates single-frame animation

	bool isSingleFrame() const; // if "Animation" has a single frame returns true

	std::vector< std::pair<Rectangle, Milliseconds> > frames; // holds source rectangles and display time of all frames of animation
};



// # Sprite #
// - ABSTRACT
class Sprite {
public:
	Sprite() = delete;

	Sprite(
		SDL_Texture *spritesheet,
		const Vector2d &parentPosition,
		bool centered,
		bool overlay
	);

	virtual ~Sprite() = default;

	virtual void update(Milliseconds elapsedTime); // does nothing
	void draw() const; // draws from source_rect to dest_rect

	SDL_RendererFlip flip = SDL_FLIP_NONE; // change to flip textures

protected:
	SDL_Texture* spritesheet; // spritesheet texture

	const Vector2d &parent_position; // position of the object module is attached to
	bool centered; // decides if sprite is centered around parent_position
	bool overlay; // decides if sprite is rendered to GUI or to Camera

	SDL_Rect source_rect; // source rect used by 'draw()'
};



// # StaticSprite #
// - Sprite with no animation
class StaticSprite : public Sprite {
public:
	StaticSprite() = delete;

	StaticSprite(
		SDL_Texture *spritesheet,
		const Vector2d &parentPosition,
		bool centered,
		bool overlay
		// if source rect is not specified it's assumed to be the whole texture
	);

	StaticSprite(
		SDL_Texture *spritesheet,
		const Vector2d &parentPosition,
		bool centered,
		bool overlay,
		SDL_Rect sourceRect // source rect on the spritesheet
	);

	void update(Milliseconds elapsedTime) override; // does nothing
};



// # AnimatedSprite #
// - Sprite with a single looped animation
class AnimatedSprite : public Sprite {
public:
	AnimatedSprite() = delete;

	AnimatedSprite(
		SDL_Texture *spritesheet,
		const Vector2d &parentPosition,
		bool centered,
		bool overlay,
		const std::vector< std::pair<Rectangle, Milliseconds> > &frames
	);
	AnimatedSprite(
		SDL_Texture *spritesheet,
		const Vector2d &parentPosition,
		bool centered,
		bool overlay,
		std::vector< std::pair<Rectangle, Milliseconds> > &&frames
	); // move semantics
	AnimatedSprite(
		SDL_Texture *spritesheet,
		const Vector2d &parentPosition,
		bool centered,
		bool overlay,
		const std::initializer_list<std::pair<Rectangle, Milliseconds>> &frames
	);
	AnimatedSprite(
		SDL_Texture *spritesheet,
		const Vector2d &parentPosition,
		bool centered,
		bool overlay,
		const Rectangle &frame
	);

	void update(Milliseconds elapsedTime) override;

private:
	Animation animation; // holds source rectangles and display time of all frames of animation

	Milliseconds animation_time_elapsed;
	size_t animation_frame_index; // used for updating animations
};



// # ControllableSprite #
// - Sprite with a number of animations
// - Animations can be added to the sprite
// - Animations can be played upon request (once or in a loop)
class ControllableSprite : public Sprite {
public:
	ControllableSprite() = delete;

	ControllableSprite(
		SDL_Texture *spritesheet,
		const Vector2d &parentPosition,
		bool centered,
		bool overlay
	); // takes enitity spritesheet

	void addAnimation(const std::string &name, const std::vector< std::pair<Rectangle, Milliseconds> > &frames); // adds an animation to the entity
	void addAnimation(const std::string &name, const std::initializer_list< std::pair<Rectangle, Milliseconds> > &frames); // same as above
	void addAnimation(const std::string &name, const Rectangle &frame); // for single-frame animations

	void playAnimation(const std::string &name, bool once = false);
		// starts playing given animation IF it's not already playing
	void replayAnimation(const std::string &name, bool once = false);
		// starts playing given animation

	Milliseconds getDuration(const std::string &name); // returns full duration of an animation

	void update(Milliseconds elapsedTime) override;

private:
	std::unordered_map<std::string, Animation> animations; // holds all spites (animated or not) for the entity

	std::string animation_current;
	bool animation_once;
	Milliseconds animation_time_elapsed;
	size_t animation_frame_index; // used for updating animations	
};