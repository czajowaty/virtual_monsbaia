#ifndef BUFFEREDPSXRAM_HPP
#define BUFFEREDPSXRAM_HPP

#include "PsxRamAddress.hpp"
#include "PsxRamConst.hpp"
#include <QByteArray>
#include <QString>
#include <array>

class BufferedPsxRam
{
    using PsxRamBuffer = std::array<uint8_t, PsxRamConst::SIZE>;

public:
    BufferedPsxRam();

    void fill(char const* ram);
    uint8_t readByte(PsxRamAddress address) const;
    int8_t readSBbyte(PsxRamAddress address) const;
    uint16_t readWord(PsxRamAddress address) const;
    int16_t readSWord(PsxRamAddress address) const;
    uint32_t readDWord(PsxRamAddress address) const;
    int32_t readSDWord(PsxRamAddress address) const;
    template <typename T>
    T const* readAsPointer(PsxRamAddress address) const
    {
        constexpr uint32_t const size = sizeof(T);
        PsxRamAddress inPsxRamAddress = PsxRamConst::toNoSegRamAddress(address);
        if (!isInPsxRamRegion(inPsxRamAddress, size))
        { throwReadOutOfBoundsError(address, size); }
        return reinterpret_cast<T const*>(inBufferPointer(inPsxRamAddress));
    }
    PsxRamAddress readAddress(PsxRamAddress address) const;
    void readRegion(PsxRamAddress::Region const& region, uint8_t* buffer) const;

private:
    constexpr std::size_t size() const
    { return ram_.size(); }
    bool isInPsxRamRegion(PsxRamAddress address, uint32_t size) const;
    uint8_t* inBufferPointer(PsxRamAddress address);
    uint8_t const* inBufferPointer(PsxRamAddress address) const;
    void throwReadOutOfBoundsError(PsxRamAddress address, uint32_t size) const;
    void throwWriteOutOfBoundsError(PsxRamAddress address, uint32_t size) const;
    void throwOutOfBoundsError(
            char const* typeString,
            PsxRamAddress address,
            uint32_t size) const;

    PsxRamBuffer ram_;
};

#endif // BUFFEREDPSXRAM_HPP
