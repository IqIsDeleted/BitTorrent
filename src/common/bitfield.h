/**
 * @file bitfield.h
 * @brief Bitfield utility functions for manipulating bits in a byte array.
 *
 * This module provides functions to set, get, and clear individual bits
 * in a bitfield represented as a uint8_t array. Useful for tracking
 * states like piece availability.
 */

#ifndef BITFIELD_H_
#define BITFIELD_H_

#include <stdint.h>

/**
 * @brief Sets a bit at the specified index in the bitfield.
 *
 * @param bits Pointer to the bitfield array (uint8_t*).
 * @param index The index of the bit to set (0-based, up to the size of the
 * array).
 */
void set_bit(uint8_t* bits, uint64_t index);

/**
 * @brief Gets the value of a bit at the specified index in the bitfield.
 *
 * @param bits Pointer to the bitfield array (uint8_t*).
 * @param index The index of the bit to get (0-based).
 * @return 1 if the bit is set, 0 if not.
 */
int get_bit(const uint8_t* bits, uint64_t index);

/**
 * @brief Clears a bit at the specified index in the bitfield.
 *
 * @param bits Pointer to the bitfield array (uint8_t*).
 * @param index The index of the bit to clear (0-based).
 */
void clear_bit(uint8_t* bits, uint64_t index);

#endif  // BITFIELD_H_