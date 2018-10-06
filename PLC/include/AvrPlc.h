#ifndef _INCLUDE_AVR_PLC_H
#define _INCLUDE_AVR_PLC_H

#include <cstdint>

// The PLC instructions are coded as 1 or 2 Byte
// 76543210
// |||-----> Argument 0..30, 31 means one mor Byte allows 256 more
// |||
// Instruction:
// 000: Read Input X
// 001: Read Ouput X
// 010: Read Flag X
// 011: Read Monoflop X
// 100: Operation, see below
// 101: Write Output X
// 110: Write Flag X
// 111: Write Monoflop X
// Operation Argument defines the Operation:
// 0: Not
// 1: And
// 2: Or
// 3-31: reserved
//
// Examples:
// 00     Read Input 0
// 1E     Read Input 30 (1E)
// 81     And
// 1F 10  Read Input 47 = (1F + 10)
// 80     Not
// 82     Or
// A5     Write Output 5

namespace avrplc
{
  constexpr const uint8_t INSTRUCTION_MASK = 0xe0;
  constexpr const uint8_t ARGUMENT_MASK = 0x1f;
  constexpr const uint8_t ARGUMENT_EXTENDED = 0x1f;
  constexpr const unsigned ARGUMENT_MAXIMUM = ARGUMENT_EXTENDED + 255;

  constexpr const uint8_t READ_INPUT = 0;
  constexpr const uint8_t READ_OUTPUT = 0x20;
  constexpr const uint8_t READ_FLAG= 0x40;
  constexpr const uint8_t READ_MONOFLOP = 0x60;
  constexpr const uint8_t OPERATION = 0x80;
  constexpr const uint8_t WRITE_OUTPUT = 0xa0;
  constexpr const uint8_t WRITE_FLAG = 0xc0;
  constexpr const uint8_t WRITE_MONOFLOP = 0xe0;

  constexpr const uint8_t NOT = 0;
  constexpr const uint8_t AND = 1;
  constexpr const uint8_t OR = 2;
}

#endif // _INCLUDE_AVR_PLC_H

