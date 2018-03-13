#include "Serial.h"

enum {
	INIT = 0,
	W_CONTROL,
	W_NUMBER
} Serial_State;

Serial_Control_Type control_map_buffer[buffer_size];

// Only 1 byte for tx, only empty cache reported.
unsigned char tx_byte = 0;

// Register ISR for RX
ISR(USART1_RX_vect) {
	// TODO: write stage code.
	if(UCSR1A & (1 << UDRE1)) {
		unsigned char d = UDR1;
		UDR1 = d;
	} else {
		tx_byte = UDR1;
	}
}

ISR(USART1_TX_vect) {
	if(tx_byte != 0) {
		UDR1 = tx_byte;
		tx_byte = 0;
	}
}

void Serial_Init() {
	// Calcute registers, and enable interrupt.

	/* Set baud rate */
    UBRR1H = (unsigned char)(BAUD>>8);
    UBRR1L = (unsigned char)BAUD;

    /* Enable receiver and transmitter */
    UCSR1B = (1<<TXCIE1)|(1<<RXCIE1)|(1<<TXEN1)|(1<<RXEN1);

    /* Set frame format: 8data, 1 stop bit */
    UCSR1C = (3<<UCSZ10);
}
