#pragma once

#include <SDL.h> // 'SDL_Texture' type

#include "geometry_utils.h" // geometry types



// # Camera #
// - Handles all calculation of relative positions and etc
// - Contains a backbuffer for rendering
// - Supports tilt (up to ~25 degrees)
// - Supports zoom (from 0 to 2, where < 1 is a zoom-in, > 1 is a zoom-out)
class Camera {
public:
	Camera(const Vector2d &position = Vector2d());

	~Camera(); // frees 'backbuffer' texture

	Rectangle getFOV_Rect() const; // returns rectangle with current Field Of View
	Vector2 getFOV_size() const; // returns size of a current Field Of View

	void textureToCamera(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect);
		// copies sourceRect from given texture to destinationRect on renderer
	void textureToCameraEx(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect, double angle, SDL_RendererFlip flip);
		// same as above but allows rotation and flips

	void cameraToRenderer();
	void cameraClear();

	Vector2d position;
	double zoom; // max zoom-out is 2
	double angle;

private:
	Vector2 standard_FOV;

	SDL_Texture* backbuffer; // requires destruction!
		// a bit bigger than view field to account for small rotation/transitions during screen shake
	Vector2 backbuffer_size; // backbuffer size (not including margin)
	int MARGIN = 200; // said margin
};