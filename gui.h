#pragma once

#include <SDL.h> // 'SDL_Texture' type
#include <memory> // 'unique_ptr' type
#include <unordered_map> // related type
#include <set> // related type

#include "timer.h" // 'Milliseconds' type
#include "geometry_utils.h" // geometry types
#include "collection.hpp" // 'Collection' class
#include "player.h" // 'Forms' enum



// # Color #
// - Represents an RGB color with transparency
struct RGBColor {
	RGBColor(); // default color is non-transparent (255, 255, 255) white
	RGBColor(Uint8 r, Uint8 g, Uint8 b, Uint8 alpha = 255);

	Uint8 r, g, b;
	Uint8 alpha;

	RGBColor operator*(double coef);
	RGBColor operator+(const RGBColor &other);

	RGBColor transparent() const; // returns fully transparent version of a color
};

namespace colors {
	const RGBColor WHITE(255, 255, 255);
	const RGBColor BLACK(0, 0, 0);

	const RGBColor IVORY(255, 243, 214);
}



// # Font #
// - Represents a monospace font, used by 'Text' objects and GUI
// - Can be used to draw single lines and symbols of any color, without creating the 'Text' object
class Font {
public:
	Font() = delete;
	Font(SDL_Texture* texture, const Vector2 &size, const Vector2 &gap);

	Vector2 draw_symbol(const Vector2 &position, char symbol, bool overlay = true) const;
		// returns position of character end (top-right corner)
	Vector2 draw_line(const Vector2 &position, const std::string &line, bool overlay = true) const;

	// Color
	void color_set(const RGBColor &color);
	void color_reset();
		// Color modification follows this formula:
		// COLOR.r = COLOR.r * (MODIFIER.r / 255);
		// COLOR.g = COLOR.g * (MODIFIER.g / 255);
		// COLOR.b = COLOR.b * (MODIFIER.b / 255);
		// COLOR.alpha = COLOR.alpha * (MODIFIER.alpha / 255);
		// For that reason (255, 255, 255, 255) white is a 'neutral' color
	
	// Getters
	Vector2 get_size() const;
	Vector2 get_gap() const;
	Vector2 get_monospace() const; // monospace == font_size + font_gap

private:
	SDL_Texture* font_texture;

	Vector2 font_size;
	Vector2 font_gap;
};



// # Text #
// - Text as an independent object
// - Used to enable smooth (aka 'animated') text display
// - Supports any monospace fonts, colors and animation delays
class Text {
public:
	Text() = delete;
	Text(const std::string &text, const Rectangle &bounds, Font* font);

	void update(Milliseconds elapsedTime);
	void draw() const;

	// Getters
	bool is_finished() const; // true if text display is finished / is instant
	const Font& get_font() const;

	// Setters
	void set_properties(const RGBColor &color, bool overlay, bool centered, int delay); // for all properites at the same time

	void set_color(const RGBColor &color); // here purely for interface convenience
	void set_color(Uint8 r, Uint8 g, Uint8 b, Uint8 alpha = 255);
	void set_overlay(bool value); // here purely for interface convenience
	void set_centered(bool value); // here purely for interface convenience
	void set_delay(int delay);

	RGBColor color = RGBColor(); // defaults as white (aka no color modifier)

	bool overlay = true; // if false, text is rendered to camera

	bool centered = false;
private:
	std::string content; // necessary to have elements in correct order
	std::set<std::string::const_iterator> line_breaks; // unordered_set can't properly hash iterators

	Rectangle bounds; // rectangle that contains text

	Font* font;

	int delay = 0; // delay between dispaying of each symbol (leave at 0 for instant dispay)
	Milliseconds time_elapsed = 0; // records elapsed time since last advance of 'display_end'

	std::string::const_iterator finish; // holds iterator to the last displayable symbol

	void draw_line(const Vector2 &position, const std::string &line) const;

	void setup_line_breaks();
};



// # GUI_FPSCounter #
// - Counts FPS and displays it
class GUI_FPSCounter {
public:
	GUI_FPSCounter();

	void update(Milliseconds elapsedTime);
	void draw() const; // non-const as it can set 'timeElapsed' and 'framesElapsed' to 0

private:
	Vector2 position = Vector2(2, 352); // position is de-facto a constant

	Collection<Text>::handle text_handle;

	Milliseconds time_elapsed; // DOES NOT ACCOUNT FOR TIMESCALE ( unique property)

	int frames_elapsed;
	int currentFPS;

	Milliseconds UPDATE_RATE = 1000; // time between FPS counter updates in ms
};



// # GUI_Healthbar #
class GUI_Healthbar {
public:
	GUI_Healthbar();

	void update(Milliseconds elapsedTime);
	void draw() const;

private:
	Vector2 position = Vector2(2, 255); // position is de-facto a constant

	SDL_Texture* texture_border;
	SDL_Texture* texture_fill;

	double percentage = 1.0;
};


// # GUI_CDbar #
class GUI_CDbar {
public:
	GUI_CDbar();

	void update(Milliseconds elapsedTime);
	void draw() const;

private:
	Vector2 position = Vector2(25, 340); // position is de-facto a constant

	SDL_Texture* texture_border;
	SDL_Texture* texture_fill;

	double percentage = 1.0;
};



// # GUI_FormPortrait #
class GUI_Portrait {
public:
	GUI_Portrait() = delete;
	GUI_Portrait(Forms playerForm);

	void update(Milliseconds elapsedTime); // empty
	void draw() const;

private:
	Vector2 position = Vector2(61, 336); // X is for CENTER, Y if for BOTTOM of the image!
	Vector2 size; // equal to the texture size

	SDL_Texture* texture;
};



// # FormSelection #
// - Slows time during selection
class FormSelection {
public:
	FormSelection();

	~FormSelection(); // sets timescale back to normal, changes player form

	void update(Milliseconds elapsedTime);
	void draw() const;

	Side selected;

	double original_timescale; // game timescale before selection popup was called
};



// # GUI_Fade #
// - Used to apply 'fading' effect to the whole screen
// - Allows any color, but beware of high 'alpha' value
class GUI_Fade {
public:
	GUI_Fade() = delete;
	GUI_Fade(const RGBColor &color);

	virtual void update(Milliseconds elapsedTime); // does nothing
	void draw() const;

protected:
	SDL_Texture* texture;

	RGBColor color;
};



// # SmoothFade #
// - Unlike its simplier parent, smoothly goes from one color to another in a given time
class GUI_SmoothFade : public GUI_Fade {
public:
	GUI_SmoothFade() = delete;
	GUI_SmoothFade(const RGBColor &colorStart, const RGBColor &colorEnd, Milliseconds duration);

	void update(Milliseconds elapsedTime); // used for smooth color change

private:
	Milliseconds duration;
	Milliseconds time_elapsed;
	
	RGBColor color_start;
	RGBColor color_end;
};



// # Gui #
// - Used for rendering all overlays
// - Serves as an interface for creation and managing of all GUI elements and modules
class Gui {
public:
	Gui();

	~Gui(); // frees 'backbuffer' texture

	void update(Milliseconds elapsedTime);
	void draw() const;

	// Text
	// pass (overlay == false) to draw as an object rather than as overlay
	Collection<Text>::handle make_text(const std::string &text, const Rectangle &field); // makes text
	Collection<Text>::handle make_line(const std::string &line, const Vector2 &position); // makes single-line text
	Collection<Text>::handle make_line_centered(const std::string &line, const Vector2 &position);

	std::unordered_map<std::string, std::unique_ptr<Font>> fonts;
	Collection<Text> texts;

	// # Gui::InventoryGUI #
	class InventoryGUI {
	public:
		InventoryGUI(); // sets up textures

		void update(Milliseconds elapsedTime);
		void draw() const;

		void show();
		void hide();
		bool toggle(); // switches visibility to oppposite, returns visibility bool

		enum class Tab {
			ITEMS
		};

		Tab next_tab(); // swithces current tab to next, returns new current tab


		Tab currentTab;

		Vector2 position; // position of top-left corner

	private:
		bool visible;

		void draw_tab_current() const;
		Vector2 tab_size = Vector2(293, 192);

		void draw_tab_items() const;
		SDL_Texture* tab_items_texture;


		//void draw_tab_quests() const;
		//void draw_tab_stats() const;
		//void draw_tab_cards() const;
		//void draw_tab_journal() const;
	};

	// FPSCounter
	void FPSCounter_on();
	void FPSCounter_off();

	// Healthbar
	void Healthbar_on();
	void Healthbar_off();

	// CDbar
	void CDbar_on();
	void CDbar_off();

	// Portrait
	void Portrait_on(Forms playerForm);
	void Portrait_off();

	// FormSelection
	void FormSelection_on();
	void FormSelection_off();

	// Fade
	void Fade_on(const RGBColor &color); // makes a static fade effect
	void Fade_on(const RGBColor &colorStart, const RGBColor &colorEnd, Milliseconds duration); // makes a smooth fade effect
	void Fade_off();


	// General
	void textureToGUI(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect);
	void textureToGUIEx(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect, double angle, SDL_RendererFlip flip);
		// same as above but allows rotation and flips

	void GUIToRenderer();
	void GUIClear();

	// GUI elements that need to remember internal state while changing visibility
	InventoryGUI inventoryGUI;

private:
	SDL_Texture* backbuffer; // requires destruction!

	// GUI elements that do NOT need to remember internal state while changing visibility
	std::unique_ptr<GUI_FPSCounter> FPS_counter;
	std::unique_ptr<GUI_Healthbar> healthbar;
	std::unique_ptr<GUI_CDbar> cdbar;
	std::unique_ptr<GUI_Portrait> portrait;
	std::unique_ptr<FormSelection> form_selection;
	std::unique_ptr<GUI_Fade> fade;
};