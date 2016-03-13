/**
 * Simplified version of Adafruit_NeoPixel lib
 * constraints :
 * - only for WS2812 @ 800KHz with GRB LEDs
 * - only one strip is handled
 * advantages :
 * - can set color by r, g, b / Color struct / H / H, L
 * todo :
 * - use some defines to handle other strips kinds (just have to change order of
 *   fields in color struct)
 */

#ifndef LEDSTRIP_STRIP_H
#define LEDSTRIP_STRIP_H

#include <Arduino.h>

typedef struct _color {
	// fields in the order needed to send raw data
	byte g;
	byte r;
	byte b;
} Color;

class LedStrip {
public:
	// first segment of a strip chain
	void begin(int len, byte pin);
	// other segment in parent chain
	void begin(int len, LedStrip *parent);

	// set color for one entry, or all
	void setRGB(byte r, byte g, byte b, int position = -1);
	void setColor(Color c, int position = -1);
	void setH(byte h, int position = -1);
	void setHL(byte h, byte l, int position = -1);

	// get color for given position
	Color getColor(int position);
	byte getH(int position);

	// shortcut to set all to black
	void off();

	// really send data to full strip chain
	void update();

	void dump(Stream &s);

protected:
	// current color of each led
	Color *strip = NULL;

	// first LedStrip object this one is chained to
	LedStrip *parent = NULL;
	// position of first led in full strip
	int offset;

	// following field are set only in parent segment

	// pin this chain is connected to
	byte pin;
	// number of leds for this segment
	int len;

	// all segment together
	Color *fullStrip = NULL;
	// total number of led in chain
	int fullLen;

private:
	// TODO : set them locally in update method ?
	const volatile byte *port;
	byte pinMask;
};

#endif
