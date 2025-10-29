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

#include "Kaleidoscope-FocusSerial.h"
#include "Kaleidoscope-EEPROM-Settings.h"

result_t LEDPalette::init( void )
{
    palette_memory_pos = ::EEPROMSettings.requestSlice( palette_color_cnt * sizeof(cRGB));

    return RESULT_OK;
}

/***********************************/
/*        Command processing       */
/***********************************/
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
