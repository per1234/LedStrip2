// PIF_TOOL_CHAIN_OPTION: UPLOAD_OPTIONS := -c "raw,cr"
// PIF_TOOL_CHAIN_OPTION: EXTRA_LIBS := ArduinoLibs ArduinoTools

#include <Arduino.h>
#include "ledStrip/ledStrip.h"
#include "serialInput/serialInput.h"

#ifndef DEFAULT_BAUDRATE
	#define DEFAULT_BAUDRATE 115200
#endif

// onboard LED
#define LED_PIN 13

byte current = 0;
LedStrip strip[3];

void set(int l, byte r, byte g, byte b) {
	strip[current].setRGB(r, g, b, l);
	strip[current].update();
}

void black(int l) {
	set(l, 0, 0, 0);
}

void red(int l) {
	set(l, 100, 0, 0);
}

void green(int l) {
    set(l, 0, 100, 0);
}

void blue(int l) {
    set(l, 0, 0, 100);
}

void dump() {
	strip[0].dump(Serial);
	strip[1].dump(Serial);
	strip[2].dump(Serial);
}

void help() {
	Serial.print("s S : prepare next commands for strip S\n  current is ");
	Serial.println(current);
	Serial.println("k N : set led N off   (-1 = all)");
	Serial.println("r N : set led N red   (-1 = all)");
	Serial.println("g N : set led N green (-1 = all)");
	Serial.println("b N : set led N blue  (-1 = all)");
	Serial.println("d : dump strip data");
}

InputItem inputs[] = {
	{ 's', 'b', (void *)&current },

	{ 'k', 'I', (void *)black },
	{ 'r', 'I', (void *)red },
	{ 'g', 'I', (void *)green },
	{ 'b', 'I', (void *)blue },

	{ 'd', 'f', (void *)dump },

	{ 'h', 'f', (void *)help },
	{ '?', 'f', (void *)help }
};

void setup() {
	Serial.begin(DEFAULT_BAUDRATE);
	strip[0].begin(12, A0);
	strip[1].begin(20, 12);
	strip[2].begin(20, &(strip[1]));
	strip[0].off();
	strip[1].off();
	strip[2].off();

	registerInput(sizeof(inputs), inputs);
	help();
}

void loop() {
	handleInput();
}
