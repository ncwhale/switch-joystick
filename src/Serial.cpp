#include "Serial.h"
#include "FixedRingBuffer.h"
#include "Joystick.h"

FixedRingBuffer<Serial_Control_Type> control_map_buffer;
Serial_Control_Type *write_control = &control_map_buffer.tail();
Serial_Control_Type *read_control = &control_map_buffer.head();
bool hard_reset = false;
uint32_t delay_count = 0;
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
    // Write control byte to buffer.333333
    // Reset data_count everytime when received control byte.
    write_control->control = rx_byte;
    data_count = 0;
    switch (rx_byte & 0x70) {
    case 0x00:
    // Button 1 0 0 0
    case 0x20:
    // Left Stick 1 0 1 0
    case 0x30:
      // Right Stick 1 0 1 1
      data_expect = 2;
      break;
    case 0x50:
      // Delay/Sync 1 1 0 1
      data_expect = rx_byte & 0x03;
      break;
    case 0x60:
      // Update All 1 1 1 0
      data_expect = 7;
      break;
    case 0x70:
      // Reset All 1 1 1 1
      hard_reset = (rx_byte == (unsigned char)'\xFF');
      data_expect = -1;
      break;
    default:
      // HAT 1 0 0 1
      // Reset 1 1 0 0
      data_expect = 0;
      break;
    }
  } else {
    // It's a data byte.
    if (data_count < data_expect) {
      // Fill this data buffer, add count
      write_control->data[data_count] = rx_byte;
      ++data_count;
    }
  }

  if (data_count == data_expect) {
    // Full filled, move next.
    write_control = control_map_buffer.write();
    Report_Count(control_map_buffer.spare());
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
  OCR1AH = (unsigned char)(CTC_MATCH_OVERFLOW >> 8);
  OCR1AL = (unsigned char)CTC_MATCH_OVERFLOW;

  // Enable the compare match interrupt
  TIMSK1 |= (1 << OCIE1A);

  // Reset joystick when boot.
  // Reset_Joystick();
  // SET Port D1 to output 0(as GND)
  DDRD |= (1 << DDD1);
  PORTD &= ~(1 << PORTD1);
}

void Reset_Joystick() { ResetJoystick(&Next_Report_Data); }

void Serial_Task() {
  // if hard_reset flag is set, reset all state at once.
  if (hard_reset) {
    control_map_buffer.clear();
    Report_Count(control_map_buffer.spare());
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

  // bool need_report_buffer = control_map_buffer.isAvailable();
  // Do timing & modify joystick data here.
  if (control_map_buffer.isAvailable()) {
    uint16_t button;
    uint32_t delay_count_calc;

    read_control = control_map_buffer.read();

    if ((read_control->control & '\x70') == '\x60') {
      // Update All 1 1 1 0 LX LY RX RY
      // Button First 0 CA HO RC LC + - ZR
      // Button Second 0 ZL R L X A B Y
      button = read_control->data[1];
      button <<= 7;
      button |= read_control->data[0];
      Next_Report_Data.Button = button;
      // HAT 0 0 0 0 x x x x
      Next_Report_Data.HAT = read_control->data[2];
      // Left Stick X
      if (read_control->control & 0x08) {
        Next_Report_Data.LX = read_control->data[3] | 0x80;
      } else {
        Next_Report_Data.LX = read_control->data[3];
      }
      // Left Stick Y
      if (read_control->control & 0x04) {
        Next_Report_Data.LY = read_control->data[4] | 0x80;
      } else {
        Next_Report_Data.LY = read_control->data[4];
      }
      // Right Stick X
      if (read_control->control & 0x02) {
        Next_Report_Data.RX = read_control->data[5] | 0x80;
      } else {
        Next_Report_Data.RX = read_control->data[5];
      }
      // Right Stick Y
      if (read_control->control & 0x01) {
        Next_Report_Data.RY = read_control->data[6] | 0x80;
      } else {
        Next_Report_Data.RY = read_control->data[6];
      }

      // continue;
    } else {

      delay_count_calc = 0;

      switch (read_control->control & '\x7F') {
      case '\x00':
        // Button 1 0 0 0 0 0 0 0
        // for 14 button 1-press/0-release
        button = read_control->data[1];
        button <<= 7;
        button |= read_control->data[0];
        Next_Report_Data.Button = button;
        break;
      case '\x10':
      case '\x11':
      case '\x12':
      case '\x13':
      case '\x14':
      case '\x15':
      case '\x16':
      case '\x17':
      case '\x18':
        // HAT 1 0 0 1 x x x x
        Next_Report_Data.HAT = read_control->control & 0x0F;
        break;
      case '\x20':
        // Left Stick 1 0 1 0 0 x 0 y
        Next_Report_Data.LX = read_control->data[0];
        Next_Report_Data.LY = read_control->data[1];
        break;
      case '\x21':
        // Left Stick 1 0 1 0 0 x 0 y
        Next_Report_Data.LX = read_control->data[0];
        Next_Report_Data.LY = read_control->data[1] | 0x80;
        break;
      case '\x24':
        // Left Stick 1 0 1 0 0 x 0 y
        Next_Report_Data.LX = read_control->data[0] | 0x80;
        Next_Report_Data.LY = read_control->data[1];
        break;
      case '\x25':
        // Left Stick 1 0 1 0 0 x 0 y
        Next_Report_Data.LX = read_control->data[0] | 0x80;
        Next_Report_Data.LY = read_control->data[1] | 0x80;
        break;
      case '\x30':
        // Right Stick 1 0 1 0 0 x 0 y
        Next_Report_Data.RX = read_control->data[0];
        Next_Report_Data.RY = read_control->data[1];
        break;
      case '\x31':
        // Right Stick 1 0 1 0 0 x 0 y
        Next_Report_Data.RX = read_control->data[0];
        Next_Report_Data.RY = read_control->data[1] | 0x80;
        break;
      case '\x34':
        // Right Stick 1 0 1 0 0 x 0 y
        Next_Report_Data.RX = read_control->data[0] | 0x80;
        Next_Report_Data.RY = read_control->data[1];
        break;
      case '\x35':
        // Right Stick 1 0 1 0 0 x 0 y
        Next_Report_Data.RX = read_control->data[0] | 0x80;
        Next_Report_Data.RY = read_control->data[1] | 0x80;
        break;
      case '\x40':
        // Reset 1 1 0 0 0 0 0 0
        Reset_Joystick();
        break;
      case '\x50':
        // Delay/Sync 1 1 0 1 0 0 x x
        wait_for_report = true;
        sync_report_count = Joystick_Report_Count + SYNC_REPORT_COUNT;
        break;
      case '\x53':
        delay_count_calc = ((int)read_control->data[2]) << 14;
      case '\x52':
        delay_count_calc |= ((int)read_control->data[1]) << 7;
      case '\x51':
        delay_count_calc |= read_control->data[0];
        ATOMIC_BLOCK(ATOMIC_FORCEON) { delay_count = delay_count_calc; }
        break;
      }
    }
    // break when delay_count is set.
    // if((delay_count_calc > 0 ) || wait_for_report) break;
    Report_Count(control_map_buffer.spare());
  }

  // if (need_report_buffer)Report_Count(control_map_buffer.read_offset);
}
