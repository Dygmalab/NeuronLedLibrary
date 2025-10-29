/* LEDPaletteRGBW - Managing the Palette of RGBW LED colors
 * Copyright (C) 2025 DygmaLabs, S. L.
 *
 * The MIT License (MIT)
 * Copyright © 2025 <copyright holders>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in the
 * Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "LEDPalette.h"

class LEDPaletteRGBW : public LEDPalette
{
  public:
    /* Constructor */
    LEDPaletteRGBW():LEDPalette() {}

  private:
    typedef struct
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t white;
    } PACK color_t;

    typedef struct
    {
        color_t colors[palette_color_cnt];
    } PACK palette_raw_t;

  private:

    /* Palette processing  */
    void palette_raw_get( palette_raw_t &palette_raw );
    void update_palette( Packet &packet ) final;

    /* Command processing */
    void command_report_color( uint8_t color_id ) final;
    void command_parse_color( uint8_t color_id ) final;

    /* Memory processing */
    inline uint16_t memory_color_pos( uint8_t color_id );
    void memory_color_save( uint8_t color_id, color_t color );
    color_t memory_color_load( uint8_t color_id );
};
