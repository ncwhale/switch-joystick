#ifndef SERIAL_HH
#define SERIAL_HH

#include <avr/io.h>
#include <avr/interrupt.h>
#define BAUD 123

// Init serial port.
void Serial_Init();

/// @cond notdocumented
// Forward declare interrupt service routines to allow them as friends.
extern "C" {
	// void ADC_vect(void) __attribute__ ((signal));
	// void ANALOG_COMP_vect(void) __attribute__ ((signal));
	// void INT0_vect(void) __attribute__ ((signal));
	// void INT1_vect(void) __attribute__ ((signal));
	// void INT2_vect(void) __attribute__ ((signal));
	// void INT3_vect(void) __attribute__ ((signal));
	// void INT6_vect(void) __attribute__ ((signal));
	// void PCINT0_vect(void) __attribute__ ((signal));
	// void SPI_STC_vect(void) __attribute__ ((signal));
	// void TIMER0_COMPA_vect(void) __attribute__ ((signal));
	// void TIMER0_COMPB_vect(void) __attribute__ ((signal));
	// void TIMER0_OVF_vect(void) __attribute__ ((signal));
	// void TIMER1_CAPT_vect(void)  __attribute__ ((signal));
	// void TIMER1_COMPA_vect(void) __attribute__ ((signal));
	// void TIMER1_COMPB_vect(void) __attribute__ ((signal));
	// void TIMER1_COMPC_vect(void) __attribute__ ((signal));
	// void TIMER1_OVF_vect(void) __attribute__ ((signal));
	// void TIMER3_CAPT_vect(void)  __attribute__ ((signal));
	// void TIMER3_COMPA_vect(void) __attribute__ ((signal));
	// void TIMER3_COMPB_vect(void) __attribute__ ((signal));
	// void TIMER3_COMPC_vect(void) __attribute__ ((signal));
	// void TIMER3_OVF_vect(void) __attribute__ ((signal));
	// void TIMER4_COMPA_vect(void) __attribute__ ((signal));
	// void TIMER4_COMPB_vect(void) __attribute__ ((signal));
	// void TIMER4_COMPD_vect(void) __attribute__ ((signal));
	// void TIMER4_OVF_vect(void) __attribute__ ((signal));
	// void TWI_vect(void) __attribute__ ((signal));
	// void WDT_vect(void) __attribute__ ((signal));
	void USART1_RX_vect(void) __attribute__ ((signal));
	void USART1_TX_vect(void) __attribute__ ((signal));
	void USART1_UDRE_vect(void) __attribute__ ((signal));
	// void EE_READY_vect(void) __attribute__ ((signal));
}
/// @endcond

#endif //SERIAL_HH
