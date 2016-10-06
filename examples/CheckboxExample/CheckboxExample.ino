#include <TouchScreen.h>
#include <Adafruit_TFTLCD.h>
#include <Adafruit_GFX.h>

#include "WindowManagerLib.h"

Window& window = WindowManager::createWindow();
Checkbox& ledControl = window.createCheckbox(90, 100, false);
Label& ledStatus = window.createLabel(10, 10, 220);

// the setup function runs once when you press reset or power the board
void setup() {
	// initialize digital pin 13 as an output.
	pinMode(13, OUTPUT);

	WindowManager::initialize();
	window.show();
}

// the loop function runs over and over again forever
void loop() {
	if (ledControl.wasChecked()) {
		digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
		ledStatus.setText("LED ON");
	}

	if (ledControl.wasUnChecked()) {
		digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
		ledStatus.setText("LED OFF");
	}

	WindowManager::draw();
}
