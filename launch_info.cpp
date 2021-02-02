#include "launch_info.h"

#include "globalconsts.hpp" // rendering consts



// # LaunchInfo #
void LaunchInfo::setInfo_Window(int width, int height, Uint32 flag) {
	this->window_width = width;
	this->window_height = height;
	this->window_flag = flag;
	this->window_renderingScaleX = (float)width / rendering::RENDERING_WIDTH;
	this->window_renderingScaleY = (float)height / rendering::RENDERING_HEIGHT;
}