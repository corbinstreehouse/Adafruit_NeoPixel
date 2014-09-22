/*--------------------------------------------------------------------
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
 --------------------------------------------------------------------*/

#ifndef ADAFRUIT_NEOPIXEL_H
#define ADAFRUIT_NEOPIXEL_H

#include <Arduino.h>

// 'type' flags for LED pixels (third parameter to constructor):
#define NEO_GRB     0x01 // Wired for GRB data order
#define NEO_COLMASK 0x01
#define NEO_KHZ800  0x02 // 800 KHz datastream
#define NEO_SPDMASK 0x02
// Trinket flash space is tight, v1 NeoPixels aren't handled by default.
// Remove the ifndef/endif to add support -- but code will be bigger.
// Conversely, can comment out the #defines to save space on other MCUs.
#ifndef __AVR_ATtiny85__
// corbin: commented out because I don't need these; saves a bit of space
//#define NEO_RGB     0x00 // Wired for RGB data order
//#define NEO_KHZ400  0x00 // 400 KHz datastream
#endif

#include "pixeltypes.h"

typedef struct {
    uint8_t red, green, blue;
} rgb_color;

class Adafruit_NeoPixel {
private:
    CRGB *m_pixels;        // Holds LED color values (3 bytes each)
    bool m_ownPixels;
public:
    
    // Constructor: number of LEDs, pin number, LED type
    Adafruit_NeoPixel(uint32_t n, uint8_t p=6, uint8_t t=NEO_GRB + NEO_KHZ800, CRGB *pixels = NULL);
    ~Adafruit_NeoPixel();
    
    void begin(void);
    void show(void);
    void setPin(uint8_t p);
    inline uint8_t getPin() { return pin; }
    
    inline void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
        m_pixels[n] = CRGB(r, g, b);
    };
    
    inline void setPixelColor(uint16_t n, uint32_t c) {
        m_pixels[n] = c;
    };
    
    void setBrightness(uint8_t brightness) { m_brightness = brightness; };
    uint8_t getBrightness() { return m_brightness; };
    
    inline uint8_t *getPixels() const { return (uint8_t *)m_pixels; };
    uint32_t numPixels() const { return numLEDs; };
    
    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
    static rgb_color ConvertColorToRGBColor(uint32_t color) {
        rgb_color c;
        c.red = color >> 16;
        c.green = color >> 8;
        c.blue = color;
        return c;
    }
    uint32_t getPixelColor(uint16_t n) const {
        return m_pixels[n];
    }
    uint16_t getNumberOfBytes() { return _numberOfBytes; };
    
private:
    const uint32_t numLEDs;       // Number of RGB LEDs in strip
    const uint32_t _numberOfBytes;      // Size of 'pixels' buffer below; this is numLEDs*3, but storing it reduces the code size by about 8 bytes when compiled (due to the savings in *3 being done at other places)
#if defined(NEO_RGB) || defined(NEO_KHZ400)
    const uint8_t type;          // Pixel flags (400 vs 800 KHz, RGB vs GRB color)
#endif
    uint8_t pin;           // Output pin number
    uint8_t m_brightness;
    uint32_t endTime;       // Latch timing reference
#ifdef __AVR__
    const volatile uint8_t *port;         // Output PORT register
    uint8_t pinMask;       // Output PORT bitmask
#endif
    
};

#endif // ADAFRUIT_NEOPIXEL_H
