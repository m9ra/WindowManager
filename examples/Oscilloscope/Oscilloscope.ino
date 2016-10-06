#include <TouchScreen.h>
#include <Adafruit_TFTLCD.h>
#include <Adafruit_GFX.h>

#include "WindowManagerLib.h"

int channel = 0;
Window& window = WindowManager::createWindow();

Button& minusButton = window.createButton(50, 170, "-");
Button& plusButton = window.createButton(160, 170, "+");


Label& voltageLabel = window.createLabel(20, 0, 200);
Label& timeLabel = window.createLabel(20, 70, 200);
Label& channelLabel = window.createLabel(100, 165, 40);

#define ADC_PRESCALER 7
#define bit(position) 1<<position
#define BUFFER_SIZE 256

uint8_t const INPUT_CHANNEL = A1;

int active_buffer = 0;
int active_buffer_index;
uint16_t buffer0[BUFFER_SIZE];
uint16_t buffer1[BUFFER_SIZE];
uint8_t const ADC_REF = (1 << REFS0);  // Vcc Reference.

volatile uint16_t* to_send_buffer = NULL;
volatile uint16_t* accept_buffer = buffer0;

volatile uint16_t current_sample = 0;
volatile uint16_t sample_counter = 0;
volatile uint16_t stall_counter = 0;


// ADC done interrupt.
ISR(ADC_vect) {
	// Read ADC data.
	current_sample = ADCL | (ADCH << 8);
	++sample_counter;

	if (active_buffer_index >= BUFFER_SIZE) {
		if (to_send_buffer != NULL) {
			//we are out of buffer
			++stall_counter;
			return;
		}

		active_buffer_index = 0;
		if (active_buffer) {
			active_buffer = 0;
			accept_buffer = buffer0;
			to_send_buffer = buffer1;
		}
		else {
			active_buffer = 1;
			accept_buffer = buffer1;
			to_send_buffer = buffer0;
		}
	}

	accept_buffer[active_buffer_index++] = current_sample;
}

void adcStart(){ 
	cli();

	ADMUX = (INPUT_CHANNEL & 7) | ADC_REF;
	ADCSRB = 0;

	ADCSRA = bit(ADEN) | bit(ADIE) | bit(ADSC) | ADC_PRESCALER;
	ADCSRA |= B01000000;


	TCCR1A = 0;// set entire TCCR1A register to 0
	TCCR1B = 0;// same for TCCR1B
	TCNT1 = 0;//initialize counter value to 0

	//OCR1A = 15624;// 1hz				
	OCR1A = 16;
	TCCR1B |= (1 << WGM12);  // turn on CTC mode
	TCCR1B |= (1 << CS12) | (1 << CS10); // Set CS10 and CS12 bits for 1024 prescaler
	TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

	sei();
}

ISR(TIMER1_COMPA_vect) {
	ADCSRA |= (1 << ADSC);
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(256000);
	Serial.println("START");
	delay(1000);
	WindowManager::initialize();
	window.show();

	adcStart();
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (to_send_buffer != NULL)
	{
		Serial.write((char*)to_send_buffer, 2 * BUFFER_SIZE);
		to_send_buffer = NULL;
	}
	else {
		return;
	}


	if (plusButton.isClicked()) {
		channel = channel + 1;
		if (channel > 5)
			channel = 5;
	}

	if (minusButton.isClicked()) {
		channel = channel - 1;
		if (channel < 0)
			channel = 0;
	}

	float voltage = current_sample * 5 / 1024.0;
	voltageLabel.setText(String(voltage) + "V");
	timeLabel.setText(String(stall_counter).c_str());

	WindowManager::draw();
}
