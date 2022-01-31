#ifndef BITSHELPER_HPP
#define BITSHELPER_HPP

#include <QtGlobal>
#include <cstdint>

inline constexpr uint32_t valuesInBits(uint8_t bits)
{ return 1 << bits; }

inline constexpr int rightShifted(qreal v, uint8_t bitShift)
{ return static_cast<int>(v) >> bitShift; }

inline constexpr int alignedToBitShift(qreal v, uint8_t bitShift)
{ return rightShifted(v, bitShift) << bitShift; };

#endif // BITSHELPER_HPP
