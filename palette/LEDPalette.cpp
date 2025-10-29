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

#include "LEDManager.h"
#include "LEDPalette.h"
#include "EEPROM.h"

#include "Kaleidoscope-FocusSerial.h"
#include "Kaleidoscope-EEPROM-Settings.h"

#define PALETTE_COLOR_GET( color_id )   ( ( void * )( (uint32_t)p_color_palette + ( color_id * color_size ) ))

result_t LEDPalette::init( void )
{
    palette_memory_pos = ::EEPROMSettings.requestSlice( palette_color_cnt * color_size );

    return RESULT_OK;
}

/***********************************/
/*        Palette processing       */
/***********************************/

void LEDPalette::update_palette( Packet &packet )
{
    uint8_t * p_packet;

    packet.header.command = Communications_protocol::PALETTE_COLORS;
    packet.header.size = color_size * 6;

    ASSERT_DYGMA( color_size <= 4, "color_size larger than 4 has not been considered so far" );

    memory_color_palette_load();

    packet.data[0] = 0;
    p_packet = (uint8_t *)PALETTE_COLOR_GET( packet.data[0] );
    memcpy( &packet.data[1], p_packet, packet.header.size );
    Communications.sendPacket( packet );

    packet.data[0] = 6;
    p_packet = (uint8_t *)PALETTE_COLOR_GET( packet.data[0] );
    memcpy( &packet.data[1], p_packet, packet.header.size );
    Communications.sendPacket( packet );

    packet.header.size = color_size * 4;
    packet.data[0] = 12;
    p_packet = (uint8_t *)PALETTE_COLOR_GET( packet.data[0] );
    memcpy( &packet.data[1], p_packet, packet.header.size );
    Communications.sendPacket( packet );
}

/***********************************/
/*        Memory processing        */
/***********************************/

inline uint16_t LEDPalette::memory_color_pos( uint8_t color_id )
{
    return ( palette_memory_pos + ( color_id * color_size ) );
}

void LEDPalette::memory_color_save( uint8_t color_id, uint8_t * p_color )
{
    uint8_t i;

    /* NOTE: We keep the XOR to assure backward compatibility with the old memory processing */
    for( i = 0; i < color_size; i++ )
    {
        p_color[i] ^= 0xFF;
    }

    EEPROM.put( memory_color_pos( color_id ), p_color, color_size );
}

void LEDPalette::memory_color_load( uint8_t color_id, uint8_t * p_color )
{
    uint8_t i;

    EEPROM.get( memory_color_pos( color_id ), p_color, color_size );

    /* NOTE: We keep the XOR to assure backward compatibility with the old memory processing */
    for( i = 0; i < color_size; i++ )
    {
        p_color[i] ^= 0xFF;
    }
}

void LEDPalette::memory_color_palette_load( void )
{
    uint8_t color_id;
    uint8_t * p_color;

    for ( color_id = 0; color_id < palette_color_cnt; color_id++ )
    {
        p_color = (uint8_t *)PALETTE_COLOR_GET( color_id );
        memory_color_load( color_id, p_color );
    }
}

/***********************************/
/*        Command processing       */
/***********************************/

void LEDPalette::command_report_color( uint8_t color_id )
{
    uint8_t i;
    uint8_t * p_color = (uint8_t *)PALETTE_COLOR_GET( color_id );

    ASSERT_DYGMA( color_id <= palette_color_cnt, "color_id exceeds the number of palette colors" );

    memory_color_load( color_id, p_color );

    for( i = 0; i < color_size; i++ )
    {
        ::Focus.send( p_color[i] );
    }
}

void LEDPalette::command_parse_color( uint8_t color_id )
{
    uint8_t i;
    uint8_t * p_color = (uint8_t *)PALETTE_COLOR_GET( color_id );

    for( i = 0; i < color_size; i++ )
    {
        ::Focus.read( p_color[i] );
    }

    memory_color_save( color_id, p_color );
}

kbdapi_event_result_t LEDPalette::command_process( const char * p_command )
{
    uint8_t color_id;
    const char * cmd = "palette";

    if ( ::Focus.handleHelp( p_command, cmd ) )
        return KBDAPI_EVENT_RESULT_IGNORED;

    if ( strcmp( p_command, cmd ) != 0 )
        return KBDAPI_EVENT_RESULT_IGNORED;

    if ( ::Focus.isEOL( ) )
    {
        for ( color_id = 0; color_id < palette_color_cnt; color_id++ )
        {
            command_report_color( color_id );
        }
        return KBDAPI_EVENT_RESULT_CONSUMED;
    }

    color_id = 0;
    while ( color_id < palette_color_cnt && !::Focus.isEOL( ) )
    {
        command_parse_color( color_id );
        color_id++;
    }

    kaleidoscope::Runtime.storage( ).commit( );

    Packet packet { };
    packet.header.device = UNKNOWN;
    update_palette( packet );

    LEDManager.led_effect_refresh();
    return KBDAPI_EVENT_RESULT_CONSUMED;
}
