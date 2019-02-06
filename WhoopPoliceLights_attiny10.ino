/**
 * Whoop Police Lights by Andrey Voroshkov (BShep)
 * Inspired by Artem Shubin
*/

#include <avr/io.h>

#define BIGDELAY    70
#define SMALLDELAY  30

#define BRIGHTNESS_RISE_SPEED_FACTOR 2
#define BRIGHTNESS_FALL_SPEED_FACTOR 4

#define RED   (1 << PB1)
#define BLUE  (1 << PB0)

void delay (int millis) {
    for (volatile unsigned int i = 34*millis; i>0; i--);
}

void blink(int pins, int count) {
    for(int i = 0; i < count; i++) {
        PORTB = pins;       // write output to specified pins
        delay(BIGDELAY);
        PORTB = 0;          // clear all port pins (set to zero)
        delay(SMALLDELAY);
    }
}

void startPWM() {
    TCCR0A = 1<<COM0A1 | 1<<COM0B1 | 1<<WGM00;  // Toggle OC0A and OC0B, Fast PWM 8bit mode TOP=0xFF
    TCCR0B = 1<<WGM02 | 1<<CS00;    // Fast PWM 8 bit; no prescaler
}

void stopPWM() {
    TCCR0A = 1<<WGM00;  // disconnect OC0A and OC0B, allow normal port operation, Fast PWM 8bit mode
    TCCR0B &= ~(3<<CS00); // detach OC0A and OC0B to allow using PB0 and PB1
}

void flashLightIteration(int * val, uint8_t * phase, uint8_t riseSpeed, uint8_t fallSpeed, uint8_t phase3Delay) {
    if (*phase == 0) {
        *val = *val + *val/riseSpeed + 1;
        if (*val > 254) {
            *val = 255;
            *phase = 1;
        }
    } else if (*phase == 1) {
        *val = *val - *val/fallSpeed;
        if (*val <= fallSpeed ) {
            *phase = 2;
            *val = phase3Delay; // this is a light-off delay
        }
    } else {
        *val = *val - 1;
        if (*val == 1) {
            *phase = 0;
        }
    }
}

void blueFlashLightIteration() {
    static int val = 0;
    static uint8_t phase = 0;
    flashLightIteration(&val, &phase, BRIGHTNESS_RISE_SPEED_FACTOR, BRIGHTNESS_FALL_SPEED_FACTOR, 5);
    if (phase == 2) {
        OCR0A = 0;
    } else {
        OCR0A = val;
    }
    TCNT0 = 255; // OCR0x is only set when counter value reaches the top
}

void redFlashLightIteration() {
    static int val = 200;
    static uint8_t phase = 0;
    flashLightIteration(&val, &phase, BRIGHTNESS_RISE_SPEED_FACTOR, BRIGHTNESS_FALL_SPEED_FACTOR, 6); // phase3Delay=6 gives a bit bigger delay than 5 for blue, thus producing a slight disbalance to make it more real
    if (phase == 2) {
        OCR0B = 0;
    } else {
        OCR0B = val;
    }
    TCNT0 = 255; // OCR0x is only set when counter value reaches the top
}

#define SPEED_BLINK_COUNT 7

int main (void) {
    DDRB = 0b0011;         // Pins 0, 1 - output;

    while (1) {
        // ---------- effect #1 -----------
        for(uint8_t i = 0; i < 6; i++) {
            blink(RED, SPEED_BLINK_COUNT);
            delay(SMALLDELAY);

            blink(BLUE, SPEED_BLINK_COUNT);
            delay(SMALLDELAY);
        }
        delay(100);

        // ---------- effect #2 -----------
        for(uint8_t i = 0; i < 6; i++) {
            PORTB = RED;
            delay((SMALLDELAY+BIGDELAY) * SPEED_BLINK_COUNT/4);
            PORTB = 0;
            delay((SMALLDELAY+BIGDELAY) * SPEED_BLINK_COUNT/2);

            PORTB = BLUE;
            delay((SMALLDELAY+BIGDELAY) * SPEED_BLINK_COUNT/4);
            PORTB = 0;
            delay((SMALLDELAY+BIGDELAY) * SPEED_BLINK_COUNT/2);
        }
        delay(100);

        // ---------- effect #3 -----------
        startPWM();
        for(int i = 0; i < 500; i++) {
            blueFlashLightIteration();
            redFlashLightIteration();
            delay(20);
        }
        stopPWM();
        delay(100);

        // ---------- effect #4 -----------
        for (uint8_t j = 0; j < 7; j++) {
            for(uint8_t i = 0; i < 5; i++) {
                uint8_t counter = j * 5 + i;
                uint8_t isRed = counter % 2;
                PORTB = isRed ? RED : BLUE;
                delay(BIGDELAY);
            }
            delay((SMALLDELAY + BIGDELAY) * 4);
        }
        delay(100);
    }
}