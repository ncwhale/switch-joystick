#ifndef SERIAL_HH
#define SERIAL_HH

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#define BAUD 3 // in 16Mhz, 3 for 250k bps
#define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 8)
#define SYNC_REPORT_COUNT 1

typedef struct {
	char control;
	char data[7];
} Serial_Control_Type;

// Init serial port.
void Serial_Init();

// Serial process in main loop.
void Serial_Task();

void Reset_Joystick();
void Report_Count(unsigned char count);

#endif //SERIAL_HH
