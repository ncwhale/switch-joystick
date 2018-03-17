#ifndef SERIAL_HH
#define SERIAL_HH

#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD 3 // in 16Mhz, 3 for 250k bps 

const unsigned char buffer_size = 255;

typedef struct {
	unsigned char control;
	unsigned char data[7];
} Serial_Control_Type;

// extern Serial_Control_Type control_map_buffer[buffer_size];

// Init serial port.
void Serial_Init();

// Serial process in main loop.
void Serial_Task();

void Reset_Joystick();
void Report_Count(unsigned char count);

#endif //SERIAL_HH
