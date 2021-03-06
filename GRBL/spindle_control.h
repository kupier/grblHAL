/*
  spindle_control.h - spindle control methods
  Part of Grbl

  Copyright (c) 2017-2019 Terje Io
  Copyright (c) 2012-2015 Sungeun K. Jeon
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef spindle_control_h
#define spindle_control_h

typedef union {
    uint8_t value;
    uint8_t mask;
    struct {
        uint8_t on           :1,
                ccw          :1,
                pwm          :1, // NOTE: only used for PWM inversion setting
                reserved3    :1,
                reserved4    :1,
                reserved5    :1,
                at_speed     :1,
                synchronized :1;
    };
} spindle_state_t;

typedef struct {
    float rpm;
    float start;
    float end;
} pwm_piece_t;

// Precalculated values that may be set/used by HAL driver to speed up RPM to PWM conversions if variable spindle is supported
typedef struct {
    uint_fast16_t period;
    uint_fast16_t off_value;
    uint_fast16_t min_value;
    uint_fast16_t max_value;
    float pwm_gradient;
    uint_fast16_t n_pieces;
    pwm_piece_t piece[SPINDLE_NPWM_PIECES];
} spindle_pwm_t;

// Used when HAL driver supports spindle synchronization
typedef struct {
    volatile uint32_t index_count;
    volatile uint32_t pulse_count;
    float rpm;
    float rpm_low_limit;
    float rpm_high_limit;
    float angular_position; // Number of revolutions since last reset
    float rpm_programmed;
    spindle_state_t state_programmed;
} spindle_data_t;

typedef enum {
    SpindleData_Counters,
    SpindleData_RPM,
    SpindleData_AngularPosition
} spindle_data_request_t;

void spindle_set_override (uint_fast8_t speed_override);

// Called by g-code parser when setting spindle state and requires a buffer sync.
// Immediately sets spindle running state with direction and spindle RPM, if enabled.
// Called by spindle_sync() after sync and parking motion/spindle stop override during restore.

// Called by g-code parser when setting spindle state and requires a buffer sync.
bool spindle_sync (spindle_state_t state, float rpm);

// Sets spindle running state with direction, enable, and spindle RPM.
bool spindle_set_state (spindle_state_t state, float rpm);

// Spindle speed calculation and limit handling
float spindle_set_rpm (float rpm, uint8_t speed_override);

// Restore spindle running state with direction, enable, spindle RPM and appropriate delay.
bool spindle_restore (spindle_state_t state, float rpm);

//
// The following functions are not called by the core, may be called by driver code.
//

// Precompute PWM values for faster conversion.
bool spindle_precompute_pwm_values (spindle_pwm_t *pwm_data, uint32_t clock_hz);

// Spindle speed to PWM conversion.
uint_fast16_t spindle_compute_pwm_value (spindle_pwm_t *pwm_data, float rpm, bool pid_limit);

#endif
