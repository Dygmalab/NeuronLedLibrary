/* LEDPalette - Managing the LED colors in the form of a set of pre-defined color values
 *              assigned to a color id - Palette
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

#include "LEDTypes.h"
#include "kbd_core.h"
#include "Communications.h"

class LEDPalette
{
  public:
    /* Constructor */
    explicit LEDPalette( void ) {}

    virtual result_t init( void );

    /* Command processing (external configuration) */
    kbdapi_event_result_t command_process( const char * p_command );

    /* Updating the palette in the system  */
    void update_palette( Packet &packet );

  protected:

    static constexpr uint8_t palette_color_cnt = 16;      /* The number of colors in the palette. Currently fixed to 16 due to the 4-bit color id nature */

    void * p_color_palette = nullptr;
    uint16_t color_palette_size;
    uint8_t color_size = 0;

  private:

    uint16_t palette_memory_pos = 0;

    /* Command processing */
    void command_report_color( uint8_t color_id );
    void command_parse_color( uint8_t color_id );

    /* Memory processing */
    inline uint16_t memory_color_pos( uint8_t color_id );
    void memory_color_save( uint8_t color_id, uint8_t * p_color );
    void memory_color_load( uint8_t color_id, uint8_t * p_color );
    void memory_color_palette_load( void );
};
