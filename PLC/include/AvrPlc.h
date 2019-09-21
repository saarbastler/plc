#ifndef _INCLUDE_AVR_PLC_H
#define _INCLUDE_AVR_PLC_H

#include <stdint.h>

// The PLC instructions are coded as 1 or 2 Byte
// 76543210
// ||||----> Argument 0..14, 15 means one more Byte allows 256 more
// ||||
// Instruction:
// 0000: Read Output X
// 0001: Read Flag X
// 0010: Read Monoflop X
// 0011: Read Input X
// 0100: Write Output X
// 0101: Write Flag X
// 0110: Write Monoflop X
// 0111: Reserved: 70..7F
// 1000: Set Output X
// 1001: Set Flag X
// 1010: Set Monoflop X
// 1011: Reserved: B0..BF
// 1100: Reset Output X
// 1101: Reset Flag X
// 1110: Reset Monoflop X
// 1111: Operation, see below
// Write means Target= Value, 
// Set means set Target, if Value==1
// Reset means reset Target, if Value==1
// Operation Argument defines the Operation:
// 0: Not
// 1: And
// 2: Or
// 3-15: reserved
//
// Examples:
// 30     Read Input 0
// 3E     Read Input 14 (0E)
// F1     And
// 3F 10  Read Input 31 = (0F + 10)
// F0     Not
// F2     Or
// 45     Write Output 5

namespace avrplc
{
  constexpr const uint8_t INSTRUCTION_MASK = 0xf0;
  constexpr const uint8_t ARGUMENT_MASK = 0x0f;
  constexpr const uint8_t ARGUMENT_EXTENDED = 0x0f;
  constexpr const unsigned ARGUMENT_MAXIMUM = ARGUMENT_EXTENDED + 255;

  constexpr const uint8_t READ_OUTPUT = 0x00;
  constexpr const uint8_t READ_FLAG= 0x10;
  constexpr const uint8_t READ_MONOFLOP = 0x20;
  constexpr const uint8_t READ_INPUT = 0x30;
  constexpr const uint8_t WRITE_OUTPUT = 0x40;
  constexpr const uint8_t WRITE_FLAG = 0x50;
  constexpr const uint8_t WRITE_MONOFLOP = 0x60;
  constexpr const uint8_t RESERVED1 = 0x70;
  constexpr const uint8_t SET_OUTPUT = 0x80;
  constexpr const uint8_t SET_FLAG = 0x90;
  constexpr const uint8_t SET_MONOFLOP = 0xA0;
  constexpr const uint8_t RESERVED2 = 0xB0;
  constexpr const uint8_t RESET_OUTPUT = 0xC0;
  constexpr const uint8_t RESET_FLAG = 0xD0;
  constexpr const uint8_t RESET_MONOFLOP = 0xE0;
  constexpr const uint8_t OPERATION = 0xF0;

  constexpr const uint8_t NOT = 0;
  constexpr const uint8_t AND = 1;
  constexpr const uint8_t OR = 2;

  constexpr const uint8_t NOP = ARGUMENT_MASK;
}

#endif // _INCLUDE_AVR_PLC_H

