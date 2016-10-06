/*
Name:		WindowManagerLib.h
Created:	4/16/2016 12:50:26 PM
Author:	m9ra
Editor:	http://www.visualmicro.com
*/

#ifndef _WindowManagerLib_h
#define _WindowManagerLib_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif


#include "WString.h"


#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define PANEL 0xA534 
#define PANEL_LIGHT 0xDEDB 
#define PANEL_DARK 0x4A49
#define CHECK_BORDER 9

class Window; //forward declaration

class Control {
	friend Window;
public:
	// Coordinates of the control
	const int X, Y;

	// Initializes new control
	Control(int x, int y);

protected:
	//determine whether control has to be updated
	bool _needs_redraw;

	// Forces drawing of the control
	virtual void draw(bool complete_redraw) = 0;

private:
	Control* _next_control;

	// Handler called when down event is registered.
	// If true is returned, event propagation ends.
	virtual bool _onDown(int x, int y);

	// Handler called when up event is registered 
	// Is called only on control accepting down event.
	virtual void _onUp();

};

class Label : public Control {
public:
	//Width and height of the button
	const int Width, Height, TextSize;

	//Caption of the button
	const char *Text;

	// Initializes new button
	Label(int x, int y, int width, int height, int text_size);

	void setText(const char* text);

	void setText(String& text);

	void setText(int value);

	void setText(float value);

private:
	// Inherited via Control
	virtual void draw(bool complete_redraw) override;

	const char* _new_text;
};

class Button : public Control {
public:
	//Width and height of the button
	const int Width, Height, TextSize;

	//Caption of the button
	const char *Caption;

	// Initializes new button
	Button(int x, int y, int width, int height, int text_size, const char *caption);

	// Determine whether button is clicked
	// (Click consists of push and release)
	bool isClicked();

	// Determine whether button is pushed
	bool isDown();


private:
	// Determine whether button was clicked.
	bool _was_clicked;

	// Determine whether button is down.
	bool _is_down;

	// Inherited via Control
	virtual void draw(bool complete_redraw) override;

	// Event handling override
	virtual bool _onDown(int x, int y) override;

	// Event handling override
	virtual void _onUp() override;
};


class Checkbox : public Control {
public:
	//Width and height of the checkbox
	const int Width, Height;

	// Initializes new checkbox
	Checkbox(int x, int y, int width, int height, bool is_checked);

	// Determine whether checkbox is checked or not	
	bool isChecked();

	// Is fired once, after checkbox is checked
	bool wasChecked();

	// Is fired once, after checkbox is unchecked
	bool wasUnChecked();


private:
	// Determine whether checkbox change event was fired.
	bool _was_fired;

	// Determine whether checkbox is checked.
	bool _is_checked;

	// Inherited via Control.
	virtual void draw(bool complete_redraw) override;

	// Event handling override.
	virtual bool _onDown(int x, int y) override;

	// Event handling override.
	virtual void _onUp() override;
};

class WindowManager {
public:
	// Window manager has to be initialized before using.
	static void initialize();

	// Creates new window.
	static Window& createWindow();

	// Draw active window. (has to be called iteratively)
	static void draw();
};

class Window {
	friend WindowManager;

public:
	// Creates a new window.
	Window();

	// Creates new button within this window.
	Button& createButton(int x, int y, const char *caption);

	// Creates new label within this window.
	Label & createLabel(int x, int y, int width);

	// Creates new checkbox within this window.
	Checkbox & createCheckbox(int x, int y, bool is_checked=false);

	// Show this window. (it can be hidden by showing another window)
	void show();

private:
	// Next window in the linked list.
	Window* _next_window;

	// First control in the list.
	Control* _first_control;

	// Draw this window. If forced is true, all controls are drawed.
	void draw(bool forced);
};

#endif

