#include "Serial.h"
#include "Joystick.h"

enum {
	INIT = 0,
	W_CONTROL,
	W_NUMBER
};

Serial_Control_Type control_map_buffer[buffer_size];

// Only 1 byte for tx, only empty cache reported.
unsigned char tx_byte = 0;
unsigned char rx_byte = 0;
char data_count = 0;
char data_expect = -1;

// Register ISR for RX
ISR(USART1_RX_vect) {
	// Do control/data split here.
	rx_byte = UDR1;
	if (rx_byte & 0x80) {
		// It's a control byte.
		// Write control byte to buffer.
		// Reset data_count everytime when received control byte.
		data_count = 0;
		switch (rx_byte & 0x70) {
		case 0x00:
			// Button	1	0	0	0
		case 0x20:
			// Left Stick	1	0	1	0
		case 0x30:
			// Right Stick	1	0	1	1
			data_expect = 2;
			break;
		case 0x50:
			// Delay/Sync	1	1	0	1
			data_expect = rx_byte & 0x03;
			break;
		case 0x60:
			// Update All	1	1	1	0
			data_expect = 7;
		default:
			// HAT	1	0	0	1
			// Reset	1	1	0	0
			// Reset All	1	1	1	1
			data_expect = 0;
			break;
			break;
		}
	} else {
		// It's a data byte.
		if (data_count < data_expect) {
			// Fill this data buffer, add count
			++data_count;
		}
	}

	if(data_count == data_expect) {
		// Full filled, move next.
		data_expect = -1;
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

void Serial_Task() {
	// Do timing & modify joystick data here.
	// Parse CMDs here.
}

void Report_Count(unsigned char count) {
	if(UCSR1A & (1 << UDRE1)) {
		UDR1 = count;
	} else {
		tx_byte = count;
	}
}
