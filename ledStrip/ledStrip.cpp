#include "ledStrip.h"
#include <stdlib.h>

static void HtoRGB(byte h, byte &r, byte &g, byte &b);
static void HLtoRGB(byte h, byte l, byte &r, byte &g, byte &b);
static byte RGBtoH(Color c);

void LedStrip::dump(Stream &s) {
	s.print("this      ");s.println((long)this, HEX);
	s.print("parent    ");s.println((long)parent, HEX);
	s.print("len       ");s.println(len);
	s.print("fullLen   ");s.println(fullLen);
	s.print("fullstrip ");s.println((long)fullStrip, HEX);
	s.print("strip     ");s.println((long)(parent->fullStrip + offset), HEX);
	s.print("offset    ");s.println(offset);
}

void LedStrip::begin(int len, byte pin) {
	port = portOutputRegister(digitalPinToPort(pin));
	pinMask = digitalPinToBitMask(pin);

	this->fullLen = this->len = len;
	int rawLen = len * 3;

	fullStrip = (Color *)malloc(rawLen);
	memset(fullStrip, 0, rawLen);

	// first strip of chain is its own beginning
	parent = this;

	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

void LedStrip::begin(int len, LedStrip *parent) {
	this->len = len;
	this->parent = parent;
	// this strip start after other ones in chain
	offset = parent->fullLen;
	// thus grow chain
	parent->fullLen += len;
	// and realloc it
	int rawLen = parent->fullLen * 3;
	parent->fullStrip = (Color *)realloc(parent->fullStrip, rawLen);
	memset(parent->fullStrip, 0, rawLen);
}

void LedStrip::setRGB(byte r, byte g, byte b, int position) {
	Color *strip = parent->fullStrip + offset;
	if (position == -1) {
		for (int i = 0; i < len; i++) {
			strip[i].r = r;
			strip[i].g = g;
			strip[i].b = b;
		}
	} else {
		strip[position].r = r;
		strip[position].g = g;
		strip[position].b = b;
	}
}

void LedStrip::setColor(Color c, int position) {
	setRGB(c.r, c.g, c.b, position);
}

void LedStrip::setH(byte h, int position) {
	byte r, g, b;
	HtoRGB(h, r, g, b);
	setRGB(r, g, b, position);
}
void LedStrip::setHL(byte h, byte l, int position) {
	byte r, g, b;
	HLtoRGB(h, l, r, g, b);
	setRGB(r, g, b, position);
}

Color LedStrip::getColor(int position) {
	Color *strip = parent->fullStrip + offset;
	return strip[position];
}
byte LedStrip::getH(int position) {
	Color *strip = parent->fullStrip + offset;
	return RGBtoH(strip[position]);
}

void LedStrip::off() {
	memset(parent->fullStrip + offset, 0, 3 * len);
	update();
}

// TODO : compare to http://fr.wikipedia.org/wiki/Cercle_chromatique

static void HLtoRGB(byte h, byte l, byte &r, byte &g, byte &b) {
	int hh = h, ll = l;
	if (h == 0) {
		r = 0; g = 0; b = 0;
	} else if (h <= 85) {
		r = (hh*3 * ll) >> 8;
		g = ((255 - hh*3) * ll) >> 8;
		b = 0;
	} else if (h <= 170) {
		r = ((255 - (hh-85)*3) * ll) >> 8;
		g = 0;
		b = ((hh-85)*3 * ll) >> 8;
	} else {
		r = 0;
		g = ((hh-170)*3 * ll) >> 8;
		b = ((255 - (hh-170)*3) * ll) >> 8;
	}
}

static void HtoRGB(byte h, byte &r, byte &g, byte &b) {
	if(h == 0) {
		r = 0; g = 0; b = 0;
	} else if (h <= 85) {
		r = h*3; g = 255 - h*3; b = 0;
	} else if (h <= 170) {
		h -= 85;
		r = 255 - h*3; g = 0; b = h*3;
	} else {
		h -= 170;
		r = 0; g = h*3; b = 255 - h*3;
	}
}

static byte RGBtoH(Color c) {
	if (c.r == 0) {
		if (c.g == 0) {
			if (c.b == 0) {
				return 0;
			} else {
				return 170;
			}
		} else {
			return c.g / 3 + 170;
		}
	} else if (c.g == 0) {
		return c.b / 3 + 85;
	} else {
		return c.r / 3;
	}
}

static long endTime = 0;

// FROM MBLOCK FIRMWARE
// see https://github.com/Makeblock-official/Makeblock-Firmware/blob/master/firmware/MeRGBLed.cpp#L107
/*
  This routine writes an array of bytes with RGB values to the Dataout pin
  using the fast 800kHz clockless WS2811/2812 protocol.
*/

// Timing in ns
#define w_zeropulse   350
#define w_onepulse    900
#define w_totalperiod 1250

// Fixed cycles used by the inner loop
#define w_fixedlow    3
#define w_fixedhigh   6
#define w_fixedtotal  10   

// Insert NOPs to match the timing, if possible
#define w_zerocycles    (((F_CPU/1000)*w_zeropulse          )/1000000)
#define w_onecycles     (((F_CPU/1000)*w_onepulse    +500000)/1000000)
#define w_totalcycles   (((F_CPU/1000)*w_totalperiod +500000)/1000000)

// w1 - nops between rising edge and falling edge - low
#define w1 (w_zerocycles-w_fixedlow)
// w2   nops between fe low and fe high
#define w2 (w_onecycles-w_fixedhigh-w1)
// w3   nops to complete loop
#define w3 (w_totalcycles-w_fixedtotal-w1-w2)

#if w1>0
  #define w1_nops w1
#else
  #define w1_nops  0
#endif

// The only critical timing parameter is the minimum pulse length of the "0"
// Warn or throw error if this timing can not be met with current F_CPU settings.
#define w_lowtime ((w1_nops+w_fixedlow)*1000000)/(F_CPU/1000)
#if w_lowtime>550
   #error "Light_ws2812: Sorry, the clock speed is too low. Did you set F_CPU correctly?"
#elif w_lowtime>450
   #warning "Light_ws2812: The timing is critical and may only work on WS2812B, not on WS2812(S)."
   #warning "Please consider a higher clockspeed, if possible"
#endif   
#if w2>0
#define w2_nops w2
#else
#define w2_nops  0
#endif

#if w3>0
#define w3_nops w3
#else
#define w3_nops  0
#endif

#define w_nop1  "nop      \n\t"
#define w_nop2  "rjmp .+0 \n\t"
#define w_nop4  w_nop2 w_nop2
#define w_nop8  w_nop4 w_nop4
#define w_nop16 w_nop8 w_nop8

void LedStrip::update() {
	// assert there's more than 50 µ since last call
	while((micros() - endTime) < 50L);

	byte curbyte, ctr, maskhi, masklo;
	byte oldSREG = SREG;
	cli();  //Disables all interrupts

	masklo = *parent->port & ~parent->pinMask;
	maskhi = *parent->port | parent->pinMask;

	int datlen = parent->fullLen * 3;
	byte *data = (byte *)(parent->fullStrip);

	Serial.println(datlen);
	Serial.println((long)data, HEX);

	while (datlen--) {
		curbyte=*data++;

		asm volatile(
			"       ldi   %0,8  \n\t"
			"loop%=:            \n\t"
			"       st    X,%3 \n\t"    //  '1' [02] '0' [02] - re
			#if (w1_nops&1)
			w_nop1
			#endif
			#if (w1_nops&2)
			w_nop2
			#endif
			#if (w1_nops&4)
			w_nop4
			#endif
			#if (w1_nops&8)
			w_nop8
			#endif
			#if (w1_nops&16)
			w_nop16
			#endif
			"       sbrs  %1,7  \n\t"    //  '1' [04] '0' [03]
			"       st    X,%4 \n\t"     //  '1' [--] '0' [05] - fe-low
			"       lsl   %1    \n\t"    //  '1' [05] '0' [06]
			#if (w2_nops&1)
			  w_nop1
			#endif
			#if (w2_nops&2)
			  w_nop2
			#endif
			#if (w2_nops&4)
			  w_nop4
			#endif
			#if (w2_nops&8)
			  w_nop8
			#endif
			#if (w2_nops&16)
			  w_nop16
			#endif
			"       brcc skipone%= \n\t"    //  '1' [+1] '0' [+2] -
			"       st   X,%4      \n\t"    //  '1' [+3] '0' [--] - fe-high
			"skipone%=:               "     //  '1' [+3] '0' [+2] -

			#if (w3_nops&1)
			w_nop1
			#endif
			#if (w3_nops&2)
			w_nop2
			#endif
			#if (w3_nops&4)
			w_nop4
			#endif
			#if (w3_nops&8)
			w_nop8
			#endif
			#if (w3_nops&16)
			w_nop16
			#endif

			"       dec   %0    \n\t"    //  '1' [+4] '0' [+3]
			"       brne  loop%=\n\t"    //  '1' [+5] '0' [+4]
			:	"=&d" (ctr)
		//    :	"r" (curbyte), "I" (_SFR_IO_ADDR(ws2812_PORTREG)), "r" (maskhi), "r" (masklo)
			:	"r" (curbyte), "x" (parent->port), "r" (maskhi), "r" (masklo)
		);
	}

	SREG = oldSREG;

	endTime = micros();
}

