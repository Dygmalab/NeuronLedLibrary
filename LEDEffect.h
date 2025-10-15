/* LEDEffectIf - Interface for Dygma LED Effects.
 * Copyright (C) 2025  DygmaLabs, S. L.
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

class LEDEffect
{
  public:

    typedef enum
    {
        LED_EFFECT_TYPE_DEFAULT = 0,

        LED_EFFECT_TYPE_SOLID_RED = 1,
        LED_EFFECT_TYPE_SOLID_GREEN,
        LED_EFFECT_TYPE_SOLID_BLUE,
        LED_EFFECT_TYPE_SOLID_WHITE,
        LED_EFFECT_TYPE_SOLID_BLACK,
        LED_EFFECT_TYPE_RAINBOW,
        LED_EFFECT_TYPE_RAINBOW_WAVE,
        LED_EFFECT_TYPE_STALKER,
        LED_EFFECT_TYPE_BREATHE,

        LED_EFFECT_TYPE_BATTERY_LEVEL,
        LED_EFFECT_TYPE_BLUETOOTH_PAIRING,
    } led_effect_type_t;

    /* Constructor */
    explicit LEDEffect( led_effect_type_t type ) : led_effect_type( type ){}

    virtual void activate() {}

    led_effect_type_t type_get( void );

  private:

    led_effect_type_t led_effect_type;

};
