/*
Name:		WindowExample.ino
Created:	4/16/2016 12:50:26 PM
Author:	m9ra
Editor:	http://www.visualmicro.com
*/


#include <TouchScreen.h>
#include <Adafruit_TFTLCD.h>
#include <Adafruit_GFX.h>

#include "WindowManagerLib.h"

int value = 0;
Window& window1 = WindowManager::createWindow();
Window& window2 = WindowManager::createWindow();

Button& plusButton = window1.createButton(0, 70, "+");
Button& minusButton = window1.createButton(70, 70, "-");
Button& goToWindow2Button = window1.createButton(0, 150, "Win2");
Label& valueLabel = window1.createLabel(20, 0, 100);

Button& resetButton = window2.createButton(0, 100, "Reset");


// the setup function runs once when you press reset or power the board
void setup() {
	WindowManager::initialize();
	window1.show();
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (goToWindow2Button.isClicked()) {
		window2.show();
	}

	if (resetButton.isClicked()) {
		window1.show();
		value = 0;
	}

	if (plusButton.isDown()) {
		value = value + 1;
	}

	if (minusButton.isClicked()) {
		value = value - 1;
	}

	valueLabel.setText(value);

	WindowManager::draw();
}
