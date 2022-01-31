#include "BufferedPsxRam.hpp"
#include <algorithm>
#include <cstring>

BufferedPsxRam::BufferedPsxRam()
{ ram_.fill(0); }

void BufferedPsxRam::fill(char const* ram)
{ std::memcpy(ram_.data(), ram, ram_.size()); }

uint8_t BufferedPsxRam::readByte(PsxRamAddress address) const
{ return *readAsPointer<uint8_t>(address); }

int8_t BufferedPsxRam::readSBbyte(PsxRamAddress address) const
{ return *readAsPointer<int8_t>(address); }

uint16_t BufferedPsxRam::   readWord(PsxRamAddress address) const
{ return *readAsPointer<uint16_t>(address); }

int16_t BufferedPsxRam::readSWord(PsxRamAddress address) const
{ return *readAsPointer<int16_t>(address); }

uint32_t BufferedPsxRam::readDWord(PsxRamAddress address) const
{ return *readAsPointer<uint32_t>(address); }

int32_t BufferedPsxRam::readSDWord(PsxRamAddress address) const
{ return *readAsPointer<int32_t>(address); }

PsxRamAddress BufferedPsxRam::readAddress(PsxRamAddress address) const
{ return *readAsPointer<PsxRamAddress::Raw>(address); }

void BufferedPsxRam::readRegion(
        PsxRamAddress::Region const& region,
        uint8_t* buffer) const
{
    PsxRamAddress inPsxRamAddress = PsxRamConst::toNoSegRamAddress(region.address);
    if (!isInPsxRamRegion(inPsxRamAddress, region.size))
    { throwReadOutOfBoundsError(region.address, region.size); }
    std::memcpy(buffer, inBufferPointer(inPsxRamAddress), region.size);
}

bool BufferedPsxRam::isInPsxRamRegion(
        PsxRamAddress address,
        uint32_t size) const
{
    if (size > 0)
    { address.addOffset(size - 1); }
    return PsxRamConst::isInPsxRamAddress(address);
}

uint8_t* BufferedPsxRam::inBufferPointer(PsxRamAddress address)
{ return ram_.begin() + address.raw(); }

uint8_t const* BufferedPsxRam::inBufferPointer(PsxRamAddress address) const
{ return ram_.cbegin() + address.raw(); }

void BufferedPsxRam::throwReadOutOfBoundsError(
        PsxRamAddress address,
        uint32_t size) const
{ throwOutOfBoundsError("Read", address, size); }

void BufferedPsxRam::throwWriteOutOfBoundsError(
        PsxRamAddress address,
        uint32_t size) const
{ throwOutOfBoundsError("Write", address, size); }

void BufferedPsxRam::throwOutOfBoundsError(
        char const* typeString,
        PsxRamAddress address,
        uint32_t size) const
{
    throw QString("%1 out of bounds error (address: 0x%2, size: 0x%3).")
            .arg(typeString)
            .arg(address.raw(), 0, 16)
            .arg(size, 0, 16);
}
