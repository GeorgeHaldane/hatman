#include "sprite.h"

#include "graphics.h" // access to rendering
#include "game.h" // access to timescale



// # Animation #
Animation::Animation(const std::vector< std::pair<Rectangle, Milliseconds> > &frames) :
	frames(frames)
{}
Animation::Animation(const std::initializer_list< std::pair<Rectangle, Milliseconds> > &frames) :
	frames(frames)
{}
Animation::Animation(const Rectangle &frame) :
	frames({ std::make_pair(frame, 0) })
{}

bool Animation::isSingleFrame() const {
	return (this->frames.size() == 1);
}



// # Sprite #
Sprite::Sprite(
	SDL_Texture *spritesheet,
	const Vector2d &parentPosition,
	bool centered,
	bool overlay
) :
	spritesheet(spritesheet),
	parent_position(parentPosition),
	centered(centered),
	overlay(overlay)
{}

void Sprite::update(Milliseconds elapsedTime) {} // does nothing

void Sprite::draw() const {
	SDL_Rect destRect = this->source_rect;

	// Center
	if (this->centered) {
		destRect.x = static_cast<int>(this->parent_position.x) - destRect.w / 2;
		destRect.y = static_cast<int>(this->parent_position.y) - destRect.h / 2;
	}
	else {
		destRect.x = static_cast<int>(this->parent_position.x);
		destRect.y = static_cast<int>(this->parent_position.y);
	}

	// Draw to correct backbuffer
	if (this->overlay) {
		Graphics::ACCESS->gui->textureToGUIEx(this->spritesheet, &this->source_rect, &destRect, 0.0, this->flip);
	}
	else {
		Graphics::ACCESS->camera->textureToCameraEx(this->spritesheet, &this->source_rect, &destRect, 0.0, this->flip);
	}
}



// # StaticSprite #
StaticSprite::StaticSprite(
	SDL_Texture *spritesheet,
	const Vector2d &parentPosition,
	bool centered,
	bool overlay
) :
	Sprite(spritesheet, parentPosition, centered, overlay)
{
	// Deduce texture size
	int textureWidth;
	int textureHeight;
	SDL_QueryTexture(this->spritesheet, NULL, NULL, &textureWidth, &textureHeight);

	this->source_rect = { 0, 0, textureWidth, textureHeight };
}

StaticSprite::StaticSprite(
	SDL_Texture *spritesheet,
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	SDL_Rect sourceRect // source rect on the spritesheet
) :
	Sprite(spritesheet, parentPosition, centered, overlay)
{
	this->source_rect = sourceRect;
}

void StaticSprite::update(Milliseconds elapsedTime) {}



// # AnimatedSprite #
AnimatedSprite::AnimatedSprite(
	SDL_Texture *spritesheet,
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	const std::vector< std::pair<Rectangle, Milliseconds> > &frames
) : 
	Sprite(spritesheet, parentPosition, centered, overlay),
	animation(frames)
{}

AnimatedSprite::AnimatedSprite(
	SDL_Texture *spritesheet,
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	std::vector< std::pair<Rectangle, Milliseconds> > &&frames
) :
	Sprite(spritesheet, parentPosition, centered, overlay),
	animation(frames)
{}

AnimatedSprite::AnimatedSprite(
	SDL_Texture *spritesheet,
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	const std::initializer_list<std::pair<Rectangle, Milliseconds>> &frames
) :
	Sprite(spritesheet, parentPosition, centered, overlay),
	animation(frames)
{}

AnimatedSprite::AnimatedSprite(
	SDL_Texture *spritesheet,
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	const Rectangle &frame
) :
	Sprite(spritesheet, parentPosition, centered, overlay),
	animation(frame)
{}

void AnimatedSprite::update(Milliseconds elapsedTime) {
	if (!animation.isSingleFrame()) {
		this->animation_time_elapsed += elapsedTime;

		const Milliseconds timeBeforeUpdate = animation.frames.at(this->animation_frame_index).second; // time current animation frame is supposed to be displayed

		if (this->animation_time_elapsed > timeBeforeUpdate) {
			this->animation_time_elapsed = 0;

			if (this->animation_frame_index < animation.frames.size() - 1) { // if current frame is not the last
				++this->animation_frame_index;
			}
			else { // if current frame is the last
				this->animation_frame_index = 0;
			}
		}
	}

	this->source_rect = this->animation.frames.at(this->animation_frame_index).first.toSDLRect();
}



// # ControllableSprite #
ControllableSprite::ControllableSprite(
	SDL_Texture *spritesheet,
	const Vector2d &parentPosition,
	bool centered,
	bool overlay
) :
	Sprite(spritesheet, parentPosition, centered, overlay),
	animation_time_elapsed(0),
	animation_frame_index(0),
	animation_once(false)
{}

void ControllableSprite::addAnimation(const std::string &name, const std::initializer_list< std::pair<Rectangle, Milliseconds> > &frames) {
	this->animations[name] = Animation(frames);
}
void ControllableSprite::addAnimation(const std::string &name, const std::vector< std::pair<Rectangle, Milliseconds> > &frames) {
	this->animations[name] = Animation(frames);
}
void ControllableSprite::addAnimation(const std::string &name, const Rectangle &frame) {
	this->animations[name] = Animation(frame);
}

void ControllableSprite::playAnimation(const std::string &name, bool once) {
	if (name != this->animation_current) {
		this->replayAnimation(name, once);
	}
}
void ControllableSprite::replayAnimation(const std::string &name, bool once) {
	if (this->animations.count(name) > 0) {
		this->animation_frame_index = 0;
		this->animation_once = once;
		this->animation_current = name;
	}
}

Milliseconds ControllableSprite::getDuration(const std::string &name) {
	Milliseconds total = 0.0;

	for (const auto& frame : this->animations.at(name).frames) {
		total += frame.second; /// Consider imbedding duration in animation itself
	}

	return total;
}

void ControllableSprite::update(Milliseconds elapsedTime) {
	const Animation &animation = this->animations.at(this->animation_current);

	if (!animation.isSingleFrame()) {
		this->animation_time_elapsed += elapsedTime;

		const Milliseconds timeBeforeUpdate = animation.frames.at(this->animation_frame_index).second; // time current animation frame is supposed to be displayed

		if (this->animation_time_elapsed > timeBeforeUpdate) {
			this->animation_time_elapsed = 0;

			if (this->animation_frame_index < animation.frames.size() - 1) { // if current frame is not the last
				++this->animation_frame_index;
			}
			else { // if current frame is the last
				if (!this->animation_once) { this->animation_frame_index = 0; }
				// If current animation is finished and repeatable set frame index to first frame
				// If animation is finished and it's non-repeatable, last frame is displayed
			}
		}
	}

	this->source_rect = this->animations.at(this->animation_current).frames.at(this->animation_frame_index).first.toSDLRect();
}