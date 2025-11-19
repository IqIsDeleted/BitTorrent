#include "bitfield.h"

void set_bit(uint8_t* bits, uint64_t index) {
  bits[index / 8] |= (1 << (index % 8));
}

int get_bit(const uint8_t* bits, uint64_t index) {
  return (bits[index / 8] & (1 << (index % 8))) != 0;
}

void clear_bit(uint8_t* bits, uint64_t index) {
  bits[index / 8] &= ~(1 << (index % 8));
}