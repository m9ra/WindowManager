/*
Name:		WindowManagerLib.cpp
Created:	4/16/2016 12:50:26 PM
Author:	m9ra
Editor:	http://www.visualmicro.com
*/

#include "WindowManagerLib.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin


#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

//display handler
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 920
#define MINPRESSURE 20
#define MAXPRESSURE 1000

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);


#define FONT_WIDTH 6
#define FONT_HEIGHT 8

//manager needs to be initialized
bool _is_initialized = false;

//determine whether 
bool _window_needs_forced_redraw = true;

//begining of window list
Window* _first_window = NULL;

//window that is being displayed
Window* _displayed_window = NULL;

//control being pushed actualy
Control* _pushed_controll = NULL;

//initialize the manager
void WindowManager::initialize() {
	_is_initialized = true;

	tft.reset();

	uint16_t identifier = tft.readID();
	if (identifier == 0x9325 || identifier == 0x9327 || identifier == 0x9328 || identifier == 0x7575 || identifier == 0x9341 || identifier == 0x8357 || identifier == 0x0154) {
		tft.begin(identifier);
		tft.fillScreen(BLACK);
	}
	else {
		Serial.print(F("Unknown LCD driver chip: "));
		Serial.println(identifier, HEX);
	}
}

Window & WindowManager::createWindow()
{
	Window* new_window = new Window();
	new_window->_next_window = _first_window;
	_first_window = new_window;
	new_window->show();

	return *new_window;
}

void WindowManager::draw()
{
	if (!_is_initialized) {
		Serial.println(F("WindowManager isn't initialized"));
		return;
	}

	if (_displayed_window == NULL)
		//there is nothing to draw
		return;

	bool needs_forced_redraw = _window_needs_forced_redraw;
	_window_needs_forced_redraw = false;
	_displayed_window->draw(needs_forced_redraw);
}

void draw_border(int x, int y, int width, int height, int col1, int col2, int border_thickness=3) {
	for (int i = 0; i < border_thickness; ++i) {
		tft.drawFastHLine(x, y + i, width - border_thickness, col1);
		tft.drawFastHLine(x, y + height - 1 - i, width, col2);
		tft.drawFastVLine(x + i, y, height - border_thickness, col1);
		tft.drawFastVLine(x + width - 1 - i, y, height - border_thickness, col2);
	}
}

Control::Control(int x, int y) : X(x), Y(y)
{
	_next_control = NULL;
	_needs_redraw = true;
}

bool Control::_onDown(int x, int y)
{
	//by default we don't react on down events.
	return false;
}

void Control::_onUp()
{
	//by default we don't react on up events.
}

Button::Button(int x, int y, int width, int height, int text_size, const char* caption) :Control(x, y), Width(width), Height(height), TextSize(text_size), Caption(caption) {
	this->_is_down = false;
	this->_was_clicked = false;
}

bool Button::_onDown(int x, int y) {
	if (x < X || y < Y)
		return false;

	if (x > X + Width || y > Y + Height)
		return false;

	this->_is_down = true;
	this->_needs_redraw = true;
	return true;
}

void Button::_onUp() {
	this->_is_down = false;
	this->_needs_redraw = true;
	this->_was_clicked = true;
}

void Button::draw(bool complete_redraw)
{
	this->_needs_redraw = false;
	if (complete_redraw) {
		tft.fillRect(X, Y, Width, Height, PANEL);
		tft.setTextSize(this->TextSize);
		tft.setTextColor(BLUE);
		tft.setCursor(X + FONT_WIDTH, Y + FONT_HEIGHT);
		tft.print(this->Caption);
	}

	if (this->_is_down)
		draw_border(X, Y, Width, Height, PANEL_DARK, PANEL_LIGHT);
	else
		draw_border(X, Y, Width, Height, PANEL_LIGHT, PANEL_DARK);
}

bool Button::isClicked()
{
	if (this->_was_clicked) {
		//click can be registered only once
		this->_was_clicked = false;
		return true;
	}
	return false;
}

bool Button::isDown()
{
	return this->_is_down;
}



Checkbox::Checkbox(int x, int y, int width, int height, bool is_checked) :Control(x, y), Width(width), Height(height) {
	this->_was_fired = false;
	this->_is_checked = is_checked;
}

bool Checkbox::_onDown(int x, int y) {
	if (x < X || y < Y)
		return false;

	if (x > X + Width || y > Y + Height)
		return false;

	return true;
}

void Checkbox::_onUp() {
	this->_is_checked = !this->_is_checked;
	this->_was_fired = false;
	this->_needs_redraw = true;
}

void Checkbox::draw(bool complete_redraw)
{
	this->_needs_redraw = false;
	if (complete_redraw) {
		tft.fillRect(X + 3, Y + 3, Width - 6, Height - 6, PANEL);
		draw_border(X, Y, Width, Height, PANEL_LIGHT, PANEL_DARK);
	}

	int color = this->_is_checked ? BLUE : PANEL;
	tft.fillRect(X + CHECK_BORDER, Y + CHECK_BORDER, Width - 2 * CHECK_BORDER, Height - 2 * CHECK_BORDER, color);
}

bool Checkbox::isChecked()
{
	return this->_is_checked;
}

bool Checkbox::wasChecked()
{
	if (this->_was_fired || !this->_is_checked)
		//event was fired already
		return false;

	this->_was_fired = true;
	return true;
}

bool Checkbox::wasUnChecked()
{
	if (this->_was_fired || this->_is_checked)
		//event was fired already
		return false;

	this->_was_fired = true;
	return true;
}

Window::Window()
{
	this->_next_window = NULL;
	this->_first_control = NULL;
}

Button & Window::createButton(int x, int y, const char* caption)
{
	int caption_length = strlen(caption);
	int text_size = 5;
	int width = (caption_length)* text_size * FONT_WIDTH + FONT_WIDTH;
	int height = (text_size + 2) * FONT_HEIGHT;
	height = 50;
	Button* new_button = new Button(x, y, width, height, text_size, caption);

	//add button to the list
	new_button->_next_control = this->_first_control;
	this->_first_control = new_button;
	return *new_button;
}

Label & Window::createLabel(int x, int y, int width)
{
	int text_size = 5;
	int height = (text_size + 2) * FONT_HEIGHT;
	Label* new_label = new Label(x, y, width, height, text_size);

	//add label to the list
	new_label->_next_control = this->_first_control;
	this->_first_control = new_label;
	return *new_label;
}

Checkbox & Window::createCheckbox(int x, int y, bool is_checked)
{
	int text_size = 5;
	int height = (text_size + 2) * FONT_HEIGHT;
	int width = height;

	Checkbox* new_checkbox = new Checkbox(x, y, width, height, is_checked);

	//add label to the list
	new_checkbox->_next_control = this->_first_control;
	this->_first_control = new_checkbox;
	return *new_checkbox;
}


void Window::show()
{
	_displayed_window = this;
	_window_needs_forced_redraw = true;
}

void Window::draw(bool forced)
{
	//handle input
/*	TSPoint p = ts.getPoint();
	pinMode(XM, OUTPUT);
	pinMode(YP, OUTPUT);
	if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
		if (_pushed_controll == NULL) {
			//try to find clicked control
			p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
			p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

			Control* control = _first_control;
			while (control) {
				if (control->_onDown(p.x, p.y)) {
					//we have control handeling the event
					_pushed_controll = control;
					break;
				}
				control = control->_next_control;
			}
		}
	}
	else {
		if (_pushed_controll != NULL) {
			_pushed_controll->_onUp();
			_pushed_controll = NULL;
		}
	}*/


	//handle redraw
	if (forced) {
		//clear the screen
		tft.fillScreen(BLACK);
	}

	Control* control = this->_first_control;
	while (control)
	{
		bool needs_redraw = control->_needs_redraw | forced;
		if (needs_redraw)
			control->draw(forced);

		control = control->_next_control;
	}
}

Label::Label(int x, int y, int width, int height, int text_size) :Control(x, y), Width(width), Height(height), TextSize(text_size), Text(NULL),_new_text(NULL)
{
}

void Label::setText(const char * text)
{
	if (strcmp(this->_new_text, text) != 0) {
		delete this->_new_text;
		this->_new_text = (char*)malloc(strlen(text) + 1);
		strcpy((char*)this->_new_text, text);
		this->_needs_redraw = strcmp(this->Text, text);
	}
}

void Label::setText(String & text)
{
	this->setText(text.c_str());
}

void Label::setText(int value)
{
	this->setText(String(value).c_str());
}

void Label::setText(float value)
{
	this->setText(String(value).c_str());
}

void Label::draw(bool complete_redraw)
{
	this->_needs_redraw = false;

	const char* last_text = this->Text;
	int current_len = strlen(this->_new_text);
	this->Text = (char*)malloc(current_len + 1);
	strcpy((char*)this->Text, this->_new_text);

	if (complete_redraw) {
		tft.fillRect(X, Y, Width, Height, PANEL);
		draw_border(X, Y, Width, Height, PANEL_DARK, PANEL_DARK);
	}

	tft.setTextSize(this->TextSize);
	tft.setTextColor(RED, PANEL);
	tft.setCursor(X + FONT_WIDTH, Y + FONT_HEIGHT);
	tft.print(this->Text);

	int last_len = strlen(last_text);
	if (current_len < last_len) {
		tft.setTextColor(PANEL, PANEL);
		tft.print(last_text + current_len);
	}

	delete last_text;
}
