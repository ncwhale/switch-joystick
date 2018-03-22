#include "Joystick.h"
#include "Serial.h"


// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) {
	// We need to disable watchdog if enabled by bootloader/fuses.
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// We need to disable clock division before initializing the USB hardware.
	clock_prescale_set(clock_div_1);
	// We can then initialize our hardware and peripherals, including the USB stack.
	// Serial_Init();

	// The USB stack should be initialized last.
	USB_Init();
	
	// Reset joystick report data.
	ResetJoystick(&Next_Report_Data);
}

// Main entry point.
int main(void) {
	GlobalInterruptDisable();
	// We'll start by performing hardware and peripheral setup.
	// board::init();
	SetupHardware();
	// We'll then enable global interrupts for our use.
	GlobalInterruptEnable();

	// Let USB run for a while.
	for(unsigned int i = 160000; i > 0; --i) {
		HID_Task();
		USB_USBTask();
	}
	// Setup Serial & Timer right now.
	Serial_Init();
	// Once that's done, we'll enter an infinite loop.
	for (;;)
	{
		// We need to run our task to process and deliver data for our IN and OUT endpoints.
		HID_Task();
		// We also need to run the main USB management task.
		USB_USBTask();
		// And process serial tasks.
		Serial_Task();
	}
}
