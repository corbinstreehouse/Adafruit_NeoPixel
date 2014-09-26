/*-------------------------------------------------------------------------
  Arduino library to control a wide variety of WS2811- and WS2812-based RGB
  LED devices such as Adafruit FLORA RGB Smart Pixels and NeoPixel strips.
  Currently handles 400 and 800 KHz bitstreams on 8, 12 and 16 MHz ATmega
  MCUs, with LEDs wired for RGB or GRB color order.  8 MHz MCUs provide
  output on PORTB and PORTD, while 16 MHz chips can handle most output pins
  (possible exception with upper PORT registers on the Arduino Mega).

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries,
  contributions by PJRC and other members of the open source community.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  -------------------------------------------------------------------------
  This file is part of the Adafruit NeoPixel library.

  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

#include "Adafruit_NeoPixel.h"
#include "controller.h"

// Macro to convert from nano-seconds to clocks and clocks to nano-seconds
// #define NS(_NS) (_NS / (1000 / (F_CPU / 1000000L)))
#if 1 || (F_CPU < 96000000)
#define NS(_NS) ( (_NS * (F_CPU / 1000000L))) / 1000
#define CLKS_TO_MICROS(_CLKS) ((_CLKS)) / (F_CPU / 1000000L)
#else
#define NS(_NS) ( (_NS * (F_CPU / 2000000L))) / 1000
#define CLKS_TO_MICROS(_CLKS) ((long)(_CLKS)) / (F_CPU / 2000000L)
#endif

Adafruit_NeoPixel::Adafruit_NeoPixel(uint32_t n, uint8_t p, uint8_t t, CRGB *pixels) : m_brightness(255), numLEDs(n), _numberOfBytes(n * sizeof(CRGB)), pin(p)
#if defined(NEO_RGB) || defined(NEO_KHZ400)
  ,type(t)
#endif
#ifdef __AVR__
  ,port(portOutputRegister(digitalPinToPort(p))),
   pinMask(digitalPinToBitMask(p))
#endif
{
    m_pixels = pixels;
    if (m_pixels == NULL) {
        m_ownPixels = true;
        m_pixels = (CRGB *)malloc(_numberOfBytes);
        memset(m_pixels, 0, _numberOfBytes);
    }
}

Adafruit_NeoPixel::~Adafruit_NeoPixel() {
    if (m_ownPixels) {
        free(m_pixels);
    }
}

void Adafruit_NeoPixel::begin(void) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

__attribute__ ((always_inline)) inline static void writeBits(register uint32_t &cyc, volatile uint8_t *set, volatile uint8_t *clr, register uint8_t &pix, uint32_t cycles, uint32_t t0h, uint32_t t1h)  {

    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        while(ARM_DWT_CYCCNT - cyc < cycles);
        cyc  = ARM_DWT_CYCCNT;
        *set = 1;
        if(pix & mask) {
            while(ARM_DWT_CYCCNT - cyc < t1h);
        } else {
            while(ARM_DWT_CYCCNT - cyc < t0h);
        }
        *clr = 1;
    }

}

void Adafruit_NeoPixel::show(void) {

//  if(!pixels) return; // corbin, constructor should always allocate, unless _numberOfBytes was 0..

  // Data latch = 50+ microsecond pause in the output stream.  Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed.  This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
  while((micros() - endTime) < 50L);
  // endTime is a private member (rather than global var) so that mutliple
  // instances on different pins can be quickly issued in succession (each
  // instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with any pin,
  // SBI/CBI instructions are eschewed in favor of full PORT writes via the
  // OUT or ST instructions.  It relies on two facts: that peripheral
  // functions (such as PWM) take precedence on output pins, so our PORT-
  // wide writes won't interfere, and that interrupts are globally disabled
  // while data is being issued to the LEDs, so no other code will be
  // accessing the PORT.  The code takes an initial 'snapshot' of the PORT
  // state, computes 'pin high' and 'pin low' values, and writes these back
  // to the PORT register as needed.

//    uint32_t start = micros();

    noInterrupts(); // Need 100% focus on instruction timing

    // from micros()
//	uint32_t count, current, istatus;
//    current = SYST_CVR;
//	count = systick_millis_count;
//	istatus = SCB_ICSR;	// bit 26 indicates if systick exception pending
//
//    if ((istatus & SCB_ICSR_PENDSTSET) && current > 50) count++;
//	current = ((F_CPU / 1000) - 1) - current;
//	uint32_t microsPassed = current / (F_CPU / 1000000);
//
//    static int microsLeftOver = 0;
//    
//    long microsTaken;
//    int32_t increase;

#ifdef __AVR__
..snipped

#elif defined(__arm__)

#if defined(__MK20DX128__) || defined(__MK20DX256__) // Teensy 3.0 & 3.1
#define CYCLES_800_T0H  (F_CPU / 2500000) // corbin: .35us on, .8us off
#define CYCLES_800_T1H  (F_CPU / 1250000) // 0.7us on, .6us off ??
#define CYCLES_800      (F_CPU /  800000)
#define CYCLES_400_T0H  (F_CPU / 2000000)
#define CYCLES_400_T1H  (F_CPU /  833333)
#define CYCLES_400      (F_CPU /  400000)

    
    // TODO: test dithering??
    CRGB scale = CRGB(m_brightness, m_brightness, m_brightness); // corbin, fix m_brightness to not mess w/stuff
    // corbin...figure out how to do the template based on the type
    PixelController<GRB> pixelController(m_pixels, numLEDs, scale, DISABLE_DITHER);

    pixelController.preStepFirstByteDithering();
    
    volatile uint8_t *set = portSetRegister(pin);
    volatile uint8_t *clr = portClearRegister(pin);
    
    // Get access to the clock
    ARM_DEMCR    |= ARM_DEMCR_TRCENA; // what is this set for??
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
    ARM_DWT_CYCCNT = 0;
    
#ifdef NEO_KHZ400
    if((type & NEO_SPDMASK) == NEO_KHZ800) { // 800 KHz bitstream
#endif
        uint32_t cyc = ARM_DWT_CYCCNT + CYCLES_800;
        
        register uint8_t b = pixelController.loadAndScale0();
        while (pixelController.has(1)) {
            pixelController.stepDithering();
            
            // Write first byte, read next byte
            writeBits(cyc, set, clr, b, CYCLES_800, CYCLES_800_T0H, CYCLES_800_T1H);
            b = pixelController.loadAndScale1();
            
            // Write second byte, read 3rd byte
            writeBits(cyc, set, clr, b, CYCLES_800, CYCLES_800_T0H, CYCLES_800_T1H);
            b = pixelController.loadAndScale2();
            
            // Write third byte, read 1st byte of next pixel
            writeBits(cyc, set, clr, b, CYCLES_800, CYCLES_800_T0H, CYCLES_800_T1H);
            b = pixelController.advanceAndLoadAndScale0();
        };
        
        while (ARM_DWT_CYCCNT - cyc < CYCLES_800);
        
		long clocks = ARM_DWT_CYCCNT;

        // adjust the counter.. (copied from fast LED)
        // this fixes millis to be right!!!
        
		long microsTaken = CLKS_TO_MICROS(clocks);
        
//        microsTaken += microsLeftOver;
//        microsTaken += microsPassed; // cuz I'm going to reset it
        
        uint32_t increase = floor((float)microsTaken / 1000.0); // hacking in less time... (like 90%) makes it late...
//        microsLeftOver = microsTaken - 1000*increase;
        
        // TOOO LARGE an incease!! the timing is happening early, which means the value is being incremented too much..
        
//        /*uint32_t*/ increase = floor((float)microsTaken / 1000.0);
//        if ((istatus & SCB_ICSR_PENDSTSET) && current > 50) increase--;
        
        systick_millis_count += increase;

        SYST_CVR = 0; // reset now...this helps with timing...
        
#ifdef NEO_KHZ400
    } else { // 400 kHz bitstream
        
        // corbin, re-write...
#error corbin re-write for 400mhz
        
    uint32_t cyc = ARM_DWT_CYCCNT + CYCLES_400;
    while(p < end) {
      pix = *p++;
      for(mask = 0x80; mask; mask >>= 1) {
        while(ARM_DWT_CYCCNT - cyc < CYCLES_400);
        cyc  = ARM_DWT_CYCCNT;
        *set = 1;
        if(pix & mask) {
          while(ARM_DWT_CYCCNT - cyc < CYCLES_400_T1H);
        } else {
          while(ARM_DWT_CYCCNT - cyc < CYCLES_400_T0H);
        }
        *clr = 1;
      }
    }
    while(ARM_DWT_CYCCNT - cyc < CYCLES_400);
  }
#endif

#else // Arduino Due

snip

#endif // end Arduino Due

#endif // end Architecture select
//    int endcvr = SYST_CVR;

//    current = SYST_CVR;
//	count = systick_millis_count;
//	istatus = SCB_ICSR;	// bit 26 indicates if systick exception pending
//	if ((istatus & SCB_ICSR_PENDSTSET) && current > 50) {
//        systick_millis_count--; //count++; // what is causing micros() to sometimes be 1000 more??
//    }
//	current = ((F_CPU / 1000) - 1) - current;
//	int end = count * 1000 + current / (F_CPU / 1000000);
    
//    if (end-start > 1000) {
//       // systick_millis_count--; // hack??
//    }
    interrupts();
    int end = micros();

//    Serial.print("start:");
//    Serial.print(current);
//    Serial.print(" start:");
//    Serial.print(endcvr);
    
    
//    Serial.print(" actual micros:");
//    Serial.print(end - start);
//    
//
//    Serial.print(" microsTaken:");
//    Serial.print(microsTaken);
//    Serial.print(" inc:");
//    Serial.println(increase);
  
    
    endTime = micros(); // Save EOD time for latch on next call
}

// Set the output pin number
void Adafruit_NeoPixel::setPin(uint8_t p) {
  pinMode(pin, INPUT);
  pin = p;
  pinMode(p, OUTPUT);
  digitalWrite(p, LOW);
#ifdef __AVR__
  port    = portOutputRegister(digitalPinToPort(p));
  pinMask = digitalPinToBitMask(p);
#endif
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t Adafruit_NeoPixel::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

