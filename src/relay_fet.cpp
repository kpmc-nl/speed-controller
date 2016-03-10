/**
 * Firmware for an RC speed controller with a circuit based on some FETs and two relays
 */

#include <avr/io.h>
#include <controller/Controller.h>
#include <avr/interrupt.h>
#include <output/PWMOut.h>
#include <util/delay.h>
#include "clock/Clock.h"

//
static PWMOut fet(&OCR0A, PB0, COM0A0, COM0A1);
static RCInput rcInput(&PORTB, &PINB, &DDRB, PB2, PCINT2);

static DOut relay(&PORTB, &DDRB, PB1);
static DOut led0(&PORTB, &DDRB, PB3);
static DOut led1(&PORTB, &DDRB, PB4);

static Controller controller(&rcInput, &led0, &led1);


int main(void) {
    /* run at 8MHz */
    CLKPR = 0x80; // set system clock to 8mhz with no prescale
    CLKPR = 0x00; // these 2 CLKPR instructions have to be run together in order to set clock to 8 Mhz


    Clock::init();


    _delay_ms(500); // wait for RCInput to parse first signal

    controller.init();

    sei(); // enable interrupts globally

    /* if everything is initialized, run the actual ESC logic */
    while (1) {

        controller.update();

        uint64_t pulse = controller.getPulse();

        if (pulse > LOW_THRESH && pulse < HIGH_THRESH) {
            fet.write(0);
//            led1.high();
            continue;
        }
//        led1.low();


        if (pulse >= HIGH_THRESH) {
            relay.low();
            fet.write((pulse - HIGH_THRESH) * 255 / controller.getHighDiff());
            continue;
        }

        if (pulse <= LOW_THRESH) {
            relay.high();
            fet.write((LOW_THRESH - pulse) * 255 / controller.getLowDiff());
            continue;
        }

    }
}

