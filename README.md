# STM32 Virtual Vehicle ECU

A fully modular, RTOS‑based embedded firmware project for the **STM32F446RE (NUCLEO‑F446RE)**.  
This project simulates a simplified automotive Electronic Control Unit (ECU) with:

- ✔ FreeRTOS (CMSIS‑RTOS2)
- ✔ Virtual Vehicle Model (speed, RPM, coolant temp)
- ✔ CAN Telemetry (bxCAN loopback)
- ✔ UART Command Line Interface (CLI)
- ✔ Modular interface design (`vehicle`, `can_if`, `cli_if`)
- ✔ Doxygen‑ready documentation

---

## 🚗 Project Overview

The goal of this project is to mimic the structure of a **real automotive ECU** in a simplified form.  
It includes:

### **1. Virtual Vehicle Model**
- Simulated speed, RPM, coolant temperature
- Update loop driven by a periodic FreeRTOS task
- Adjustable target speed (via CLI)
- Fault injection function (`veh force`)

### **2. CAN Telemetry (Loopback Mode)**
- Encodes vehicle state into an 8‑byte CAN frame
- Uses HAL CAN API with interrupt‑based RX
- RX messages pushed into an RTOS queue
- CAN frames logged via CLI (`log on`)

### **3. UART CLI**
Commands include:
- `help`
- `veh status`
- `veh speed <value>`
- `veh force`
- `log on/off`
- `clear`

---

## 📁 Folder Structure

```
Core/
 ├── Inc/
 │    ├── main.h
 │    ├── vehicle.h
 │    ├── can_if.h
 │    ├── cli_if.h
 │
 └── Src/
      ├── main.c
      ├── vehicle.c
      ├── can_if.c
      ├── cli_if.c

Docs/
 ├── ARCHITECTURE.md
 ├── CLI_COMMANDS.md
 ├── VEHICLE_MODEL.md
 ├── CAN_PROTOCOL.md
 └── CHANGELOG.md
```

---

## 🛠 Requirements

- **STM32CubeIDE**
- NUCLEO‑F446RE development board
- USB cable
- UART terminal (115200 8‑N‑1)
- *(Optional)* CAN analyzer (not required because loopback mode is used)

---

## ▶️ How to Run

1. Clone the repository:
   ```
   git clone https://github.com/karangandhi-projects/stm32-virtual-vehicle-ecu.git
   ```

2. Open the project using **STM32CubeIDE**.

3. Build + flash the firmware.

4. Open PuTTY / TeraTerm at **115200 8‑N‑1**.

5. You should see:
   ```
   CLI Ready
   ```

6. Try commands:
   ```
   help
   veh status
   veh speed 60
   log on
   ```

---

## 🧪 Example CAN Frame (Loopback)

```
CAN RX: ID=0x100 DLC=6
DATA:  A0 00  1B 07  58 02  00 00
```

Decoded:
- Speed     → 16.0 km/h  
- RPM       → 1819  
- Coolant   → 60.0 °C

---

## 📄 Doxygen Support

All header + source files include:
- `@brief`
- `@param`
- `@return`
- Module descriptions

Use:
```
doxygen Doxyfile
```

---

## 🧩 Future Extensions

You can expand this ECU with:

- PID cruise control
- Multiple CAN frames (wheel speeds, throttle, gear)
- UDS diagnostics (0x7DF request/response)
- CAN FD upgrade (on supported MCUs)
- Simulated faults + diagnostic trouble codes
- Telemetry streaming over BLE/WiFi

---

## 👨‍💻 Author

**Karan Gandhi**  
Embedded Systems Engineer  
GitHub: https://github.com/karangandhi-projects

---

## 📌 Version

See `Docs/CHANGELOG.md` for full history.
