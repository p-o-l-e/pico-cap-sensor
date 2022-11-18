#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"

#define SENSORS 1  // Count
#define PIN_OUT 16 // via 1-22 MOhm 
#define PIN_IN  17 

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef struct
{
    uint8_t in;
    uint8_t out;

} io;

static io       _sensor[SENSORS];
static uint64_t _trigger_level[SENSORS];
static uint8_t  _current[SENSORS];
static uint8_t  _prior[SENSORS];

void _sensors_init()
{
    for(int i = 0; i < SENSORS; i++)
    {
        gpio_init(_sensor[i].in);
        gpio_init(_sensor[i].out);
        gpio_set_dir(_sensor[i].in, GPIO_IN);
        gpio_set_dir(_sensor[i].out, GPIO_OUT);
        _trigger_level[i] = 0;
        _current[i] = 0;
        _prior[i] = 0;
        gpio_put(_sensor[i].out, 0);
        gpio_pull_down(_sensor[i].out);

        gpio_set_input_hysteresis_enabled(_sensor[i].in, false);
        gpio_set_slew_rate(_sensor[i].out, GPIO_SLEW_RATE_FAST);
    } 
}

uint64_t _get_cap(uint8_t id)
{
    uint64_t start = time_us_64();
    gpio_put(_sensor[id].out, 1);
    gpio_pull_up(_sensor[id].in);
    while(gpio_get(_sensor[id].in) == 0) {  };
    gpio_put(_sensor[id].out, 0);
    gpio_pull_down(_sensor[id].in);
    return time_us_64() - start;
}


void _calibrate_sensor(uint8_t id, uint8_t sensivity)
{
    for(int i = 0; i < 50; i++)
    {
        _trigger_level[id] = max(_get_cap(id)/sensivity, _trigger_level[id]);
    }
    sleep_ms(100);
}



bool sense(uint8_t id, uint8_t lag)
{
    _prior[id] = _current[id];
    _get_cap(id) > _trigger_level[id] ? _current[id] = 1 : _current[id] = 0;
    bool state;
    if (_current[id] && _prior[id]) // Switch only, if two consecutive same levels detected
    {
        state = 1;
    }
    else if ((!_current[id]) && (!_prior[id]))
    {
        state = 0;
    }
    sleep_ms(lag);
    return state;
}
