#ifndef VEHICLE_H
#define VEHICLE_H

#include <stdint.h>

/*
 * Module: Vehicle model (vehicle)
 *
 * Role:
 *   - Encapsulates a tiny "virtual vehicle" state.
 *   - Provides a simple update step and helper APIs for CLI control.
 *
 * Version history (module-level):
 *   v2.2 - Initial model: speed, RPM, coolant temperature.
 */

/**
 * @brief Simple virtual vehicle state.
 */
typedef struct
{
    float    speed_kph;        /**< Vehicle speed in km/h        */
    uint16_t engine_rpm;       /**< Engine speed in RPM          */
    float    coolant_temp_c;   /**< Coolant temperature in °C    */
} VehicleState_t;

/**
 * @brief Initialize the vehicle state to sane defaults.
 *
 * Defaults:
 *   - speed_kph      = 0.0f
 *   - engine_rpm     = 800 (idle)
 *   - coolant_temp_c = 30.0 °C ("cold" engine)
 */
void Vehicle_Init(VehicleState_t *vs);

/**
 * @brief Integrate / update the vehicle model.
 *
 * @param vs      Pointer to vehicle state.
 * @param dt_s    Time step in seconds (e.g. 0.1f).
 *
 * The model is intentionally very simple – just enough to make the
 * telemetry feel "alive" when plotted or inspected over time.
 */
void Vehicle_Update(VehicleState_t *vs, float dt_s);

/**
 * @brief Apply a “driver command” to the model (e.g. target speed).
 *
 * @param vs                 Pointer to vehicle state.
 * @param target_speed_kph   New target speed in km/h (clamped to [0, 200]).
 */
void Vehicle_SetTargetSpeed(VehicleState_t *vs, float target_speed_kph);

/**
 * @brief Force state values directly (used from CLI for testing/fault injection).
 *
 * @param vs         Pointer to vehicle state.
 * @param speed_kph  New speed in km/h.
 * @param rpm        New engine speed in RPM.
 * @param temp_c     New coolant temperature in °C.
 */
void Vehicle_Force(VehicleState_t *vs,
                   float speed_kph,
                   uint16_t rpm,
                   float temp_c);

#endif /* VEHICLE_H */
