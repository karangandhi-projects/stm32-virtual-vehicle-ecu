# Virtual Vehicle Model

The **Virtual Vehicle Model** provides a software-simulated representation of essential vehicle dynamics.  
It is updated periodically using a FreeRTOS task and feeds telemetry to the CAN interface.

This module is defined in:
- `vehicle.c`
- `vehicle.h`

---

## 1. VehicleState_t Structure

The model tracks a simplified set of vehicle parameters:

```c
typedef struct
{
    float speed_kph;          // Current speed of the vehicle
    float target_speed_kph;   // Target speed (set via CLI)
    float engine_rpm;         // Engine RPM
    float coolant_temp_c;     // Coolant temperature
} VehicleState_t;
```

These variables evolve over time based on inputs and simulated physics.

---

## 2. Update Logic (Vehicle_Update)

`Vehicle_Update()` applies basic but realistic behavior to simulate acceleration, RPM dynamics, and coolant temperature evolution.

### 2.1 Speed Dynamics
The vehicle accelerates or decelerates toward the target speed.

```
error = target_speed - current_speed
accel = proportional_gain * error
clamped to ±5 km/h per update
speed += accel * dt
```

This produces smooth speed transitions without advanced physics.

---

### 2.2 Engine RPM Mapping

RPM is mapped proportionally to speed:

```
rpm = idle_rpm + (speed * factor)
rpm is smoothed using exponential filtering
```

Example:
- 0 km/h → 800 RPM
- 60 km/h → ~3800 RPM (depending on constants)

---

### 2.3 Coolant Temperature Dynamics

Temperature rises with RPM and stabilizes around a safe range.

```
heat = engine_rpm / scaling_factor
cooling = constant cooling term
coolant_temp += (heat - cooling) * dt
clamped between 20°C and 110°C
```

This simulates:
- warm-up when driving,
- stabilization when idling.

---

## 3. Control Inputs

### 3.1 Target Speed (from CLI)
The user sets the vehicle’s desired speed:

```
veh speed 80
```

The model then gradually approaches **80 km/h**.

### 3.2 Forced Test State
Useful for testing or demos:

```
veh force
```

The system overrides values with predetermined constants.

---

## 4. Task Integration

The vehicle model is updated inside a FreeRTOS periodic task:

```c
void VehicleTask(void *argument)
{
    const TickType_t period = pdMS_TO_TICKS(100);
    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        Vehicle_Update(&g_vehicle, 0.1f);      // dt = 100ms
        CAN_IF_SendTelemetry(&g_vehicle);      // Send telemetry frame
        vTaskDelayUntil(&last, period);
    }
}
```

---

## 5. CAN Telemetry

Each update cycle:
- Speed, RPM, and coolant temp are encoded into an 8‑byte CAN frame
- Frame ID `0x100`
- Logged through loopback mode

See `CAN_PROTOCOL.md` for details.

---

## 6. Extending the Model

You can easily enhance the vehicle simulation with features like:

- Gearbox simulation and shift logic  
- Cruise control with PID loops  
- Wheel speed sensors  
- Throttle/brake variables  
- Fuel consumption model  
- Fault injections for diagnostics  
- Thermal runaway or overheating scenarios  

This makes the project scalable for interviews and portfolio demos.

