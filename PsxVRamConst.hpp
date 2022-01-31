#ifndef PSXVRAMCONST_HPP
#define PSXVRAMCONST_HPP

#include "BitsHelper.hpp"

struct PsxVRamConst
{
    static constexpr uint8_t const PIXEL_DEPTH = 1;
    static constexpr uint8_t const PIXEL_SIZE = 1 << PIXEL_DEPTH;
    static constexpr uint8_t const WIDTH_DEPTH = 0xb;
    static constexpr uint16_t const WIDTH = 1 << WIDTH_DEPTH;
    static constexpr uint16_t const PIXELS_PER_LINE = WIDTH / PIXEL_SIZE;
    static constexpr uint8_t const HEIGHT_DEPTH = 0x9;
    static constexpr uint16_t const HEIGHT = 1 << HEIGHT_DEPTH;
    static constexpr uint32_t const SIZE = WIDTH * HEIGHT;
    static constexpr uint8_t const TEXTURE_PAGE_X_SHIFT = 6;
    static constexpr uint8_t const TEXTURE_PAGE_Y_SHIFT = 8;
    static constexpr uint16_t const TEXTURE_PAGE_SIZE = 256;
    static constexpr uint16_t const TEXTURE_16BPP_PAGE_WIDTH =
            TEXTURE_PAGE_SIZE;
    static constexpr uint16_t const TEXTURE_8BPP_PAGE_WIDTH =
            TEXTURE_PAGE_SIZE >> 1;
    static constexpr uint16_t const TEXTURE_4BPP_PAGE_WIDTH =
            TEXTURE_PAGE_SIZE >> 2;
    static constexpr uint16_t const TEXTURE_PAGE_HEIGHT =
            TEXTURE_PAGE_SIZE;
    static constexpr uint8_t const CLUT_X_SHIFT = 4;
    static constexpr uint8_t const CLUT_Y_SHIFT = 0;
    static constexpr uint16_t const CLUT_8_BPP_WIDTH = valuesInBits(8);
    static constexpr uint16_t const CLUT_4_BPP_WIDTH = valuesInBits(4);
    static constexpr uint16_t const CLUT_HEIGHT = 1;

    PsxVRamConst() = delete;
};

#endif // PSXVRAMCONST_HPP
