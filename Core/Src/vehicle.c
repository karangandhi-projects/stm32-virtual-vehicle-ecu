/**
 * @file    vehicle.c
 * @brief   Tiny "virtual vehicle" model used by Mini ECU.
 */

#include "vehicle.h"
#include <stddef.h>

static float clamp_f(float v, float min, float max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

void Vehicle_Init(VehicleState_t *vs)
{
    if (vs == NULL) return;

    vs->speed_kph      = 0.0f;
    vs->engine_rpm     = 800;    /* idle */
    vs->coolant_temp_c = 30.0f;  /* “cold” engine */
}

void Vehicle_Update(VehicleState_t *vs, float dt_s)
{
    if (vs == NULL) return;
    if (dt_s <= 0.0f) return;

    /* Super simple “physics” just so things move a bit */

    /* Let speed slowly decay if > 0 (friction) */
    if (vs->speed_kph > 0.1f)
    {
        vs->speed_kph -= 1.0f * dt_s;   /* 1 km/h per second */
        if (vs->speed_kph < 0.0f)
        {
            vs->speed_kph = 0.0f;
        }
    }

    /* RPM loosely tied to speed (fake gear)
       idle at 800, add ~50 RPM per km/h
    */
    float target_rpm = 800.0f + vs->speed_kph * 50.0f;
    float rpm_f      = (float)vs->engine_rpm;

    /* Simple first-order lag towards target */
    rpm_f += (target_rpm - rpm_f) * 0.5f * dt_s;
    vs->engine_rpm = (uint16_t)clamp_f(rpm_f, 600.0f, 6000.0f);

    /* Coolant temp: warm up slowly toward 90°C, cool slightly when stopped */
    if (vs->speed_kph > 1.0f || vs->engine_rpm > 1500)
    {
        vs->coolant_temp_c += 2.0f * dt_s;   /* warm up */
    }
    else
    {
        vs->coolant_temp_c -= 0.2f * dt_s;   /* cool a bit */
    }
    vs->coolant_temp_c = clamp_f(vs->coolant_temp_c, 20.0f, 110.0f);
}

void Vehicle_SetTargetSpeed(VehicleState_t *vs, float target_speed_kph)
{
    if (vs == NULL) return;

    /* For now just snap to target; later we can add acceleration limits. */
    vs->speed_kph = clamp_f(target_speed_kph, 0.0f, 200.0f);
}

void Vehicle_Force(VehicleState_t *vs,
                   float speed_kph,
                   uint16_t rpm,
                   float temp_c)
{
    if (vs == NULL) return;

    vs->speed_kph      = clamp_f(speed_kph, 0.0f, 300.0f);
    vs->engine_rpm     = (uint16_t)clamp_f((float)rpm, 0.0f, 8000.0f);
    vs->coolant_temp_c = clamp_f(temp_c,  -40.0f, 140.0f);
}
