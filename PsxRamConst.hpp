#ifndef PSXRAMCONST_HPP
#define PSXRAMCONST_HPP

#include "PsxRamAddress.hpp"

#include <QByteArray>
#include <cstdint>

struct PsxRamConst
{
    static constexpr uint32_t const SIZE = 0x200000;
    static constexpr PsxRamAddress::Raw const KUSEG_ADDRESS = 0x0;
    static constexpr PsxRamAddress::Raw const KSEG0_ADDRESS = 0x80000000;
    static constexpr PsxRamAddress::Raw const KSEG1_ADDRESS = 0xa0000000;
    static constexpr PsxRamAddress::Mask const ADDRESS_MASK = 0x00ffffff;
    static constexpr PsxRamAddress::Mask const SEGMENT_MASK = 0xff000000;
    static QByteArray const PSX_RAM_PATTERN;

    PsxRamConst() = delete;

    static constexpr PsxRamAddress::Raw toKSeg0RamAddress(
            PsxRamAddress::Raw rawAddress)
    { return toNoSegRamAddress(rawAddress) | PsxRamConst::KSEG0_ADDRESS; }
    static constexpr PsxRamAddress::Raw toKSeg1RamAddress(
            PsxRamAddress::Raw rawAddress)
    { return toNoSegRamAddress(rawAddress) | PsxRamConst::KSEG1_ADDRESS; }
    static constexpr PsxRamAddress::Raw toNoSegRamAddress(
                                       PsxRamAddress::Raw rawAddress)
    { return rawAddress & ADDRESS_MASK; }
    static constexpr PsxRamAddress::Raw toSegPsxRamAddress(
            PsxRamAddress::Raw rawAddress)
    { return rawAddress & SEGMENT_MASK; }
    static constexpr bool isInPsxRamAddress(PsxRamAddress::Raw rawAddress)
    {
        auto segment = toSegPsxRamAddress(rawAddress);
        if (
                segment != KUSEG_ADDRESS &&
                segment != KSEG0_ADDRESS &&
                segment != KSEG1_ADDRESS)
        { return false; }
        auto address = toNoSegRamAddress(rawAddress);
        return address < SIZE;
    }
};

#endif // PSXRAMCONST_HPP
