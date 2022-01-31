#ifndef MEMORYADDRESS_HPP
#define MEMORYADDRESS_HPP

#include <cstdint>

template <typename TRaw, typename TOffset>
class MemoryAddress
{
public:
    using Raw = TRaw;
    using Offset = TOffset;
    using Mask = Raw;
    struct Region
    {
        MemoryAddress address;
        uint32_t size;

        MemoryAddress end() const
        { return size > 0 ? MemoryAddress(address + size - 1) : address; }
        bool isInRegion(MemoryAddress checkedAddress) const
        { return checkedAddress >= address && checkedAddress <= end(); }
        bool doesContain(Region const& checkedRegion) const
        {
            return checkedRegion.address >= address &&
                    checkedRegion.end() <= end();
        }
        bool doesContain(MemoryAddress checkedAddress) const
        { return checkedAddress >= address && checkedAddress <= end(); }
        bool doesIntersect(Region const& checkedRegion) const
        {
            if (doesContain(checkedRegion.address))
            { return true; }
            if (doesContain(checkedRegion.end()))
            { return true; }
            return false;
        }
        bool isAdjacent(Region const& checkedRegion) const
        {
            return (address == checkedRegion.end() + 1u) ||
                    (end() + 1u == checkedRegion.address);
        }
    };

    static constexpr Raw const ZERO{0};
    static constexpr Raw const MAX{static_cast<Raw>(-1)};

    MemoryAddress() : MemoryAddress(ZERO)
    {}
    MemoryAddress(Raw rawAddress) : rawAddress_{rawAddress}
    {}
    MemoryAddress(void* pointer)
        : MemoryAddress(reinterpret_cast<Raw>(pointer))
    {}

    bool isNull() const
    { return rawAddress_ == ZERO; }
    inline void addOffset(Offset offset)
    { rawAddress_ += offset; }
    inline void addMask(Mask mask)
    { rawAddress_ |= mask; }
    inline void clearMask(Mask mask)
    { rawAddress_ &= mask; }
    inline bool operator==(MemoryAddress other) const
    { return raw() == other.raw(); }
    inline bool operator!=(MemoryAddress other) const
    { return !(*this == other); }
    inline bool operator>=(MemoryAddress other) const
    { return raw() >= other.raw(); }
    inline bool operator>(MemoryAddress other) const
    { return raw() > other.raw(); }
    inline bool operator<=(MemoryAddress other) const
    { return raw() <= other.raw(); }
    inline bool operator<(MemoryAddress other) const
    { return raw() < other.raw(); }
    inline MemoryAddress operator+(Offset offset) const
    { return copy() += offset; }
    inline MemoryAddress& operator+=(Offset offset)
    {
        addOffset(offset);
        return *this;
    }
    inline MemoryAddress operator|(Mask mask)
    { return copy() |= mask; }
    inline MemoryAddress& operator|=(Mask mask)
    {
        addMask(mask);
        return *this;
    }
    inline MemoryAddress operator&(Mask mask)
    { return copy() &= mask; }
    inline MemoryAddress& operator&=(Mask mask)
    {
        clearMask(mask);
        return *this;
    }
    inline MemoryAddress operator<<(uint8_t shift)
    { return copy() <<= shift; }
    inline MemoryAddress& operator<<=(uint8_t shift)
    {
        rawAddress_ <<= shift;
        return *this;
    }
    inline MemoryAddress operator>>(uint8_t shift)
    { return copy() >>= shift; }
    inline MemoryAddress& operator>>=(uint8_t shift)
    {
        rawAddress_ >>= shift;
        return *this;
    }
    inline Raw raw() const
    { return rawAddress_; }
    inline void* asPointer() const
    { return reinterpret_cast<void*>(raw()); }
    inline operator Raw() const
    { return raw(); }
    inline operator void*() const
    { return asPointer(); }

private:
    inline MemoryAddress copy() const
    { return MemoryAddress(raw()); }

    Raw rawAddress_;
};

#endif // MEMORYADDRESS_HPP
