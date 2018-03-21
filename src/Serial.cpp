#include "Serial.h"
#include "FixedRingBuffer.h"
#include "Joystick.h"

FixedRingBuffer<Serial_Control_Type> control_map_buffer;
Serial_Control_Type &write_control = control_map_buffer.tail();
Serial_Control_Type &read_control = control_map_buffer.head();
bool hard_reset = false;
int delay_count = 0;
bool wait_for_report = false;
unsigned char sync_report_count;

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
    write_control.control = rx_byte;
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
      break;
    case 0xF0:
      // Reset All	1	1	1	1
      hard_reset = rx_byte == 0xFF;
      data_expect = -1;
      break;
    default:
      // HAT	1	0	0	1
      // Reset	1	1	0	0
      data_expect = 0;
      break;
    }
  } else {
    // It's a data byte.
    if (data_count < data_expect) {
      // Fill this data buffer, add count
      write_control.data[data_count] = rx_byte;
      ++data_count;
    }
  }

  if (data_count == data_expect) {
    // Full filled, move next.
    write_control = control_map_buffer.write();
    Report_Count(control_map_buffer.free());
    data_expect = -1;
  }
}

void Report_Count(unsigned char count) {
  if (UCSR1A & (1 << UDRE1)) {
    UDR1 = count;
  } else {
    tx_byte = count;
  }
}

ISR(USART1_TX_vect) {
  if (tx_byte != 0) {
    UDR1 = tx_byte;
    tx_byte = 0;
  }
}

ISR(TIMER1_COMPA_vect) {
  if (delay_count > 0)
    --delay_count;
}

void Serial_Init() {
  // Calcute registers, and enable interrupt.

  /* Set baud rate */
  UBRR1H = (unsigned char)(BAUD >> 8);
  UBRR1L = (unsigned char)BAUD;

  /* Enable receiver and transmitter */
  UCSR1B = (1 << TXCIE1) | (1 << RXCIE1) | (1 << TXEN1) | (1 << RXEN1);

  /* Set frame format: 8data, 1 stop bit */
  UCSR1C = (3 << UCSZ10);

  // Setup Timer1 for 1ms countdown.
  // CTC mode, Clock/8
  TCCR1B |= (1 << WGM12) | (1 << CS11);

  // Load the high byte, then the low byte
  // into the output compare
  OCR1AH = (CTC_MATCH_OVERFLOW >> 8);
  OCR1AL = CTC_MATCH_OVERFLOW;

  // Enable the compare match interrupt
  TIMSK1 |= (1 << OCIE1A);
}

void Reset_Joystick() { ResetJoystick(&Next_Report_Data); }

void Serial_Task() {
  // if hard_reset flag is set, reset all state at once.
  if (hard_reset) {
    control_map_buffer.clear();
    Report_Count(control_map_buffer.free());
    hard_reset = false;
    wait_for_report = false;
    delay_count = 0;
    Reset_Joystick();
    return;
  }

  if (wait_for_report) {
    if (sync_report_count != Joystick_Report_Count)
      return;
    wait_for_report = false;
  }

  if (delay_count > 0) {
    // wait for timer clear this flag.
    return;
  }
  // Do timing & modify joystick data here.
  while (control_map_buffer.isAvailable()) {
    read_control = control_map_buffer.read();
    uint16_t button;
    int delay_count_calc = 0;

    if((read_control.control & 0xF0) == 0xE0) {
      // Update All	1	1	1	0	LX	LY	RX	RY	
      // Button First	0	CA	HO	RC	LC	+	-	ZR
      // Button Second	0	ZL	R	L	X	A	B	Y
      button = read_control.data[1];
      button <<= 7;
      button |= read_control.data[0];
      Next_Report_Data.Button = button;
      // HAT	0	0	0	0	x	x	x	x
      Next_Report_Data.HAT = read_control.data[2];
      // Left Stick X	0	x	x	x	x	x	x	x
      if (read_control.control & 0x08) {
        Next_Report_Data.LX = read_control.data[3] | 0x80;
      } else {
        Next_Report_Data.LX = read_control.data[3];        
      }
      // Left Stick Y	0	x	x	x	x	x	x	x
      if (read_control.control & 0x04) {
        Next_Report_Data.LY = read_control.data[4] | 0x80;
      } else {
        Next_Report_Data.LY = read_control.data[4];        
      }
      // Right Stick X	0	x	x	x	x	x	x	x
      if (read_control.control & 0x02) {
        Next_Report_Data.RX = read_control.data[5] | 0x80;
      } else {
        Next_Report_Data.RX = read_control.data[5];        
      }
      // Right Stick Y	0	x	x	x	x	x	x	x
      if (read_control.control & 0x01) {
        Next_Report_Data.RY = read_control.data[6] | 0x80;
      } else {
        Next_Report_Data.RY = read_control.data[6];        
      }

      continue;
    }
    
    switch (read_control.control) {
    case 0x80:
      // Button	1	0	0	0	0	0	0	0	2 Number for 14 button 1-press/0-release
      button = read_control.data[1];
      button <<= 7;
      button |= read_control.data[0];
      Next_Report_Data.Button = button;
      break;
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
    case 0x98:
      // HAT	1	0	0	1	x	x	x	x	HAT direction in low 4 bits(0~8)
      Next_Report_Data.HAT = read_control.control & 0x0F;
      break;
    case 0xA0:
      // Left Stick	1	0	1	0	0	x	0	y	x/y high bits. Follow 2 numbers.
      Next_Report_Data.LX = read_control.data[0];
      Next_Report_Data.LY = read_control.data[1];
      break;
    case 0xA1:
      // Left Stick	1	0	1	0	0	x	0	y	x/y high bits. Follow 2 numbers.
      Next_Report_Data.LX = read_control.data[0];
      Next_Report_Data.LY = read_control.data[1] | 0x80;
      break;
    case 0xA4:
      // Left Stick	1	0	1	0	0	x	0	y	x/y high bits. Follow 2 numbers.
      Next_Report_Data.LX = read_control.data[0] | 0x80;
      Next_Report_Data.LY = read_control.data[1];
      break;
    case 0xA5:
      // Left Stick	1	0	1	0	0	x	0	y	x/y high bits. Follow 2 numbers.
      Next_Report_Data.LX = read_control.data[0] | 0x80;
      Next_Report_Data.LY = read_control.data[1] | 0x80;
      break;
    case 0xB0:
      // Right Stick	1	0	1	0	0	x	0	y	x/y high bits. Follow 2 numbers.
      Next_Report_Data.RX = read_control.data[0];
      Next_Report_Data.RY = read_control.data[1];
      break;
    case 0xB1:
      // Right Stick	1	0	1	0	0	x	0	y	x/y high bits. Follow 2 numbers.
      Next_Report_Data.RX = read_control.data[0];
      Next_Report_Data.RY = read_control.data[1] | 0x80;
      break;
    case 0xB4:
      // Right Stick	1	0	1	0	0	x	0	y	x/y high bits. Follow 2 numbers.
      Next_Report_Data.RX = read_control.data[0] | 0x80;
      Next_Report_Data.RY = read_control.data[1];
      break;
    case 0xB5:
      // Right Stick	1	0	1	0	0	x	0	y	x/y high bits. Follow 2 numbers.
      Next_Report_Data.RX = read_control.data[0] | 0x80;
      Next_Report_Data.RY = read_control.data[1] | 0x80;
      break;
    case 0xC0:
      // Reset	1	1	0	0	0	0	0	0	Release all button&Hat/Sticks in center
      Reset_Joystick();
      break;
    case 0xD0:
      // Delay/Sync	1	1	0	1	0	0	x	x	Follow 0~3 Numbers. (ms)(Max ~34min)
      wait_for_report = true;
      sync_report_count = Joystick_Report_Count + SYNC_REPORT_COUNT;
      break;
    case 0xD3:
      delay_count_calc = ((int) read_control.data[2]) << 14;    
    case 0xD2:
      delay_count_calc |= ((int) read_control.data[1]) << 7;
    case 0xD1:
      // Delay/Sync	1	1	0	1	0	0	x	x	Follow 0~3 Numbers. (ms)(Max ~34min)
      delay_count_calc |= read_control.data[0];
      ATOMIC_BLOCK(ATOMIC_FORCEON) {
        delay_count = delay_count_calc;
      }
      break;
      // Update All	1	1	1	0	LX	LY	RX	RY	
      // Reset All	1	1	1	1	1	1	1	1	Reset Joystick & Clear Buffer.(Hard reset)
    }
    // break when delay_count is set.
    if(delay_count_calc > 0 || wait_for_report) break;
  }

  Report_Count(control_map_buffer.free());  
}
