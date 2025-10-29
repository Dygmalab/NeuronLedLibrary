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

#include "LEDPaletteRGBW.h"

#include "Kaleidoscope-FocusSerial.h"
//#include "Kaleidoscope-EEPROM-Settings.h"

/***********************************/
/*        Palette processing       */
/***********************************/

void LEDPaletteRGBW::palette_raw_get( palette_raw_t &palette_raw )
{
    uint8_t color_id;

    for ( color_id = 0; color_id < palette_color_cnt; color_id++ )
    {
        palette_raw.colors[ color_id ] = memory_color_load( color_id );
    }
}

void LEDPaletteRGBW::update_palette( Packet &packet )
{
    packet.header.command = Communications_protocol::PALETTE_COLORS;
    packet.header.size = sizeof( color_t ) * 6;
    palette_raw_t palette_raw;

    palette_raw_get( palette_raw );

    packet.data[0] = 0;
    memcpy( &packet.data[1], &palette_raw.colors[packet.data[0]], packet.header.size );
    Communications.sendPacket( packet );

    packet.data[0] = 6;
    memcpy( &packet.data[1], &palette_raw.colors[packet.data[0]], packet.header.size );
    Communications.sendPacket( packet );

    packet.header.size = sizeof( color_t ) * 4;
    packet.data[0] = 12;
    memcpy( &packet.data[1], &palette_raw.colors[packet.data[0]], packet.header.size );
    Communications.sendPacket( packet );
}

/***********************************/
/*        Memory processing        */
/***********************************/

inline uint16_t LEDPaletteRGBW::memory_color_pos( uint8_t color_id )
{
    return ( palette_memory_pos + ( color_id * sizeof( color_t ) ) );
}

void LEDPaletteRGBW::memory_color_save( uint8_t color_id, color_t color )
{
    /* NOTE: We keep the XOR to assure backward compatibility with the old memory processing */
    color.red   ^= 0xff;
    color.green ^= 0xff;
    color.blue  ^= 0xff;
    color.white ^= 0xff;

    kaleidoscope::Runtime.storage( ).put( memory_color_pos( color_id ), color );
}

LEDPaletteRGBW::color_t LEDPaletteRGBW::memory_color_load( uint8_t color_id )
{
    color_t color;

    kaleidoscope::Runtime.storage( ).get( memory_color_pos( color_id ), color );

    /* NOTE: We keep the XOR to assure backward compatibility with the old memory processing */
    color.red   ^= 0xff;
    color.green ^= 0xff;
    color.blue  ^= 0xff;
    color.white ^= 0xff;

    return color;
}

/***********************************/
/*        Command processing       */
/***********************************/

void LEDPaletteRGBW::command_report_color( uint8_t color_id )
{
    color_t color;

    ASSERT_DYGMA( color_id <= palette_color_cnt, "color_id exceeds the number of palette colors" );

    color = memory_color_load( color_id );
    ::Focus.send( color.red, color.green, color.blue, color.white );
}

void LEDPaletteRGBW::command_parse_color( uint8_t color_id )
{
    color_t color;

    ::Focus.read( color.red );
    ::Focus.read( color.green );
    ::Focus.read( color.blue );
    ::Focus.read( color.white );

    memory_color_save( color_id, color );
}
