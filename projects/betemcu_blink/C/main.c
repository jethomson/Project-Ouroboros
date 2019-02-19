#include <avr/io.h> // allows access to AVR hardware registers
#include <util/delay.h> // includes delay functions

int main (void)
{
    DDRC |= _BV(0); // green LED pin to output
    PORTC |= _BV(0); // green LED off, HIGH turns LED off, 00000001
    DDRC |= _BV(1); // red LED pin to output
    PORTC |= _BV(1); // red LED off

    while(1) {

        PORTC &= ~_BV(0); // green LED on, LOW turns LED on, ~(00000001) = 11111110

        _delay_ms(1000);

        PORTC &= ~_BV(1); // red LED on

        _delay_ms(1000);

        PORTC |= _BV(0); // green LED off

        _delay_ms(1000);

        PORTC |= _BV(1); // red LED off

        _delay_ms(1000);
    };
}
