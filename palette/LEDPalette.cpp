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

#include "Config_manager.h"
#include "LEDManager.h"
#include "LEDPalette.h"

#include "Kaleidoscope-FocusSerial.h"
#include "Kaleidoscope-EEPROM-Settings.h"

#define PALETTE_COLOR_GET( color_id )   ( ( void * )( (uint32_t)p_color_palette + ( color_id * color_size ) ))

result_t LEDPalette::init( void )
{
    palette_memory_pos = ::EEPROMSettings.requestSlice( color_palette_size );

    return RESULT_OK;
}

/***********************************/
/*        Palette processing       */
/***********************************/

void LEDPalette::update_palette_piece( Packet &packet, uint8_t color_id, uint8_t color_cnt )
{
    uint8_t * p_color;

    packet.header.command = Communications_protocol::PALETTE_COLORS;
    packet.header.size = color_size * color_cnt;

    /* Set the first color id */
    packet.data[0] = color_id;

    /* Fill the color data */
    p_color = &packet.data[1];

    while( color_cnt != 0 )
    {
        memory_color_load( color_id, p_color );

        color_id++;
        color_cnt--;
        p_color += color_size;
    }

    Communications.sendPacket( packet );
}

void LEDPalette::update_palette( Packet &packet )
{
    ASSERT_DYGMA( color_size <= 4, "color_size larger than 4 has not been considered so far" );

    update_palette_piece( packet, 0, 6 );
    update_palette_piece( packet, 6, 6 );
    update_palette_piece( packet, 12, 4 );
}

/***********************************/
/*        Memory processing        */
/***********************************/

void LEDPalette::memory_color_save( uint8_t color_id, uint8_t * p_color )
{
    uint8_t i;
    const void * p_color_config = PALETTE_COLOR_GET( color_id );

    /* NOTE: We keep the XOR to assure backward compatibility with the old memory processing */
    for( i = 0; i < color_size; i++ )
    {
        p_color[i] ^= 0xFF;
    }

    cfgmem_color_save( p_color_config, p_color );
}

void LEDPalette::memory_color_load( uint8_t color_id, uint8_t * p_color )
{
    uint8_t i;
    const void * p_color_config = PALETTE_COLOR_GET( color_id );

    memcpy( p_color, p_color_config, color_size );

    /* NOTE: We keep the XOR to assure backward compatibility with the old memory processing */
    for( i = 0; i < color_size; i++ )
    {
        p_color[i] ^= 0xFF;
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

/****************************************************/
/*                   Config Memory                  */
/****************************************************/

void LEDPalette::cfgmem_color_save( const void * p_color_config, void * p_color )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( p_color_config, p_color, color_size );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}
