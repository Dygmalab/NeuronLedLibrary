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

#include "Config_manager.h"
#include "LEDPaletteRGBW.h"

result_t LEDPaletteRGBW::init( void )
{
    result_t result = RESULT_ERR;
    const palette_config_t * p_palette_config;

    /* Load the LED palette config */
    result = ConfigManager.config_item_request( ConfigManager::CFG_ITEM_TYPE_LEDS_PALETTE, (const void **)&p_palette_config );
    EXIT_IF_ERR( result, "ConfigManager.config_item_request failed" );

    p_color_palette = &p_palette_config->palette;

    result = LEDPalette::init( sizeof( color_t ) );
    EXIT_IF_ERR( result, "LEDPalette::init failed" );

_EXIT:
    return result;
}
