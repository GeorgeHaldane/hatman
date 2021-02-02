#include "camera.h"

#include "graphics.h" // access to rendering
#include "globalconsts.hpp" // rendering consts



// # Camera #
Camera::Camera(const Vector2d &position) :
	position(position),
	zoom(1),
	angle(0)
{
	this->standard_FOV = Vector2(rendering::RENDERING_WIDTH, rendering::RENDERING_HEIGHT);
	this->backbuffer_size = (this->standard_FOV + Vector2(this->MARGIN, this->MARGIN)) * 2;

	this->backbuffer = SDL_CreateTexture(
		Graphics::ACCESS->getRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
		this->backbuffer_size.x, this->backbuffer_size.y
	);

	SDL_SetTextureBlendMode(this->backbuffer, SDL_BLENDMODE_BLEND); // necessary for proper blending of of transparent parts
}
Camera::~Camera() {
	SDL_DestroyTexture(this->backbuffer);
}

Rectangle Camera::getFOV_Rect() const {
	return Rectangle(this->position.toVector2(), this->getFOV_size(), true);
}
Vector2 Camera::getFOV_size() const {
	return (this->standard_FOV * this->zoom);
}

void Camera::textureToCamera(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect) {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // target backbuffer for rendering

	const Vector2 cameraCornerPos = this->position.toVector2() - (this->backbuffer_size / 2 - Vector2(this->MARGIN, this->MARGIN));
	// position of top-left corner of the camera with standard zoom

	SDL_Rect backbufferDestRect = {
		destRect->x + this->MARGIN - cameraCornerPos.x, destRect->y + this->MARGIN - cameraCornerPos.y,
		destRect->w, destRect->h
	};

	SDL_RenderCopy(Graphics::ACCESS->getRenderer(), texture, sourceRect, &backbufferDestRect);
}
void Camera::textureToCameraEx(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect, double angle, SDL_RendererFlip flip) {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // target backbuffer for rendering

	const Vector2 cameraCornerPos = this->position.toVector2() - (this->backbuffer_size / 2 - Vector2(this->MARGIN, this->MARGIN));
	// position of top-left corner of the camera with standard zoom

	SDL_Rect backbufferDestRect = {
		destRect->x + this->MARGIN - cameraCornerPos.x, destRect->y + this->MARGIN - cameraCornerPos.y,
		destRect->w, destRect->h
	};

	SDL_RenderCopyEx(Graphics::ACCESS->getRenderer(), texture, sourceRect, &backbufferDestRect, angle, NULL, flip);
}

void Camera::cameraToRenderer() {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), NULL); // give target back to the renderer

	const Vector2 sourceRectCenter = this->backbuffer_size / 2;
	const Vector2 sourceRectDimensions = this->standard_FOV * zoom + Vector2(this->MARGIN, this->MARGIN) * 2 * zoom;

	SDL_Rect sourceRect = Rectangle(sourceRectCenter, sourceRectDimensions, true).toSDLRect();
	SDL_Rect destRect = {
		-this->MARGIN, -this->MARGIN,
		rendering::RENDERING_WIDTH + 2 * this->MARGIN, rendering::RENDERING_HEIGHT + 2 * this->MARGIN
	}; // account for different backbuffer including marign 

	Graphics::ACCESS->copyTextureToRendererEx(this->backbuffer, &sourceRect, &destRect, this->angle);
}
void Camera::cameraClear() {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // target backbuffer for rendering

	SDL_RenderClear(Graphics::ACCESS->getRenderer());
}