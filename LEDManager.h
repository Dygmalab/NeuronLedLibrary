/* LEDManager - Managing the LED states and processes like led effects, layers, communication etc.
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

#include <vector>

#include "kbd_if.h"
#include "LEDEffect.h"

class LEDManager {

  public:

    result_t init( void );

    void led_effect_set( LEDEffect::led_effect_type_t effect_type );
    void led_effect_set_prio( LEDEffect::led_effect_type_t effect_type );
    void led_effect_reset_prio( void );
    void led_effect_refresh( void );

  private:

    typedef int16_t led_effect_id_t;
    typedef std::vector<LEDEffect*> LEDEffect_list_t;

    kbdif_t * p_kbdif = NULL;
    led_effect_id_t LEDEffect_id_regular = 0;

    LEDEffect * p_LEDEffect = nullptr;
    LEDEffect * p_LEDEffect_prio = nullptr;         /* This is a top priority effect which is forcefully being active until set to null */

    result_t kbdif_initialize( void );

    LEDEffect * led_effect_search_type( const LEDEffect_list_t & effect_list, LEDEffect::led_effect_type_t effect_type );
    LEDEffect * led_effect_search_type( LEDEffect::led_effect_type_t effect_type );

    void led_effect_set_type( LEDEffect::led_effect_type_t effect_type, bool prio );
    void led_effect_set_id( led_effect_id_t id );
    void led_effect_set_next( void );
    void led_effect_set_previous( void );
    void led_effect_activate( void );

  private:
    static const kbdif_handlers_t kbdif_handlers;
    static const LEDEffect_list_t LEDEffect_list_regular;
    static const LEDEffect_list_t LEDEffect_list_specific;

    static kbdapi_event_result_t kbdif_led_layer_change_event_cb( void * p_instance, kbdapi_led_layer_id_t layer_id );
    static kbdapi_event_result_t kbdif_led_effect_change_event_cb( void * p_instance, kbdapi_led_effect_action_t action );
};

extern class LEDManager LEDManager;
