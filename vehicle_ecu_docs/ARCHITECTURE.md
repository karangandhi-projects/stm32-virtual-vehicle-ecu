# STM32 Virtual Vehicle ECU – Architecture

This document describes the high-level architecture of the **STM32 Virtual Vehicle ECU** firmware running on the **NUCLEO-F446RE**.  
The project demonstrates a small, automotive-style ECU using:

- FreeRTOS for task scheduling
- A virtual vehicle model (speed, RPM, coolant temperature)
- CAN (bxCAN) telemetry in loopback mode
- UART-based CLI for diagnostics
- Modular interfaces for CAN and CLI

---

## 1. High-Level Overview

The system is structured in three main layers:

- **Application Layer**
  - `vehicle.c` / `vehicle.h` – vehicle state and update logic
  - CLI commands to inspect & control the vehicle state

- **Service / Interface Layer**
  - `can_if.c` / `can_if.h` – CAN telemetry, RX queue, logging
  - `cli_if.c` / `cli_if.h` – UART CLI, command parsing

- **Platform / HAL Layer**
  - STM32Cube HAL (CAN, UART, GPIO, RCC)
  - FreeRTOS (CMSIS-RTOS v2 API)

---

## 2. Task-Level Architecture

FreeRTOS is used to split responsibilities into separate tasks:

### 2.1 Vehicle Task

- **Source**: implemented in `main.c` (or a dedicated vehicle task function)
- **Period**: typically every 100 ms
- **Responsibilities**:
  - Update the `VehicleState_t` structure based on simple physics
  - Apply target speed / overrides
  - Call `CAN_IF_SendTelemetry()` to push a CAN frame with the current state

### 2.2 CAN RX Task

- **Source**: `CanRxTask` in `main.c`
- **Trigger**: Blocks on an RTOS message queue created in `can_if.c`
- **Responsibilities**:
  - Receive `CAN_IF_Msg_t` messages from the queue
  - Call `CAN_IF_ProcessRxMsg()` to decode/log frames
  - In the current design, logging to UART is optional and can be toggled

### 2.3 CLI Task

- **Source**: `CliTask` in `main.c` and `cli_if.c`
- **Input**: Single characters received by UART interrupt → queued/buffered
- **Responsibilities**:
  - Collect characters into a line buffer
  - Parse commands such as `veh status`, `veh speed 60`, `log on`, etc.
  - Interact with:
    - `VehicleState_t` (read/write)
    - `CAN_IF_SetLogging()`

---

## 3. Data Flow

### 3.1 Vehicle → CAN

1. Vehicle task updates `VehicleState_t` (speed, rpm, coolant).
2. Vehicle task calls `CAN_IF_SendTelemetry(&g_vehicle);`
3. `CAN_IF_SendTelemetry()` encodes:
   - speed_kph × 10 → uint16
   - engine_rpm → uint16
   - coolant_temp_c × 10 → int16
4. Message is sent via `HAL_CAN_AddTxMessage()` and transmitted in **loopback mode**.

### 3.2 CAN → RTOS Queue → CAN RX Task

1. A frame is received by bxCAN in FIFO0.
2. HAL calls `HAL_CAN_RxFifo0MsgPendingCallback()`.
3. `can_if.c` reads the frame into a `CAN_IF_Msg_t`:
   - `id`, `dlc`, `data[8]`
4. The message is posted to an `osMessageQueueId_t` queue.
5. `CanRxTask` blocks on that queue and receives the message.
6. `CanRxTask` calls `CAN_IF_ProcessRxMsg()` to log or process it.

---

## 4. Module Dependencies

- `vehicle.c` / `vehicle.h`
  - Defines `VehicleState_t`
  - No direct dependency on HAL

- `can_if.c` / `can_if.h`
  - Depends on:
    - `main.h` for CAN handle (`extern CAN_HandleTypeDef hcan1;`)
    - `cmsis_os2.h` for RTOS types
    - `vehicle.h` for `VehicleState_t`

- `cli_if.c` / `cli_if.h`
  - Depends on:
    - `main.h` for UART handle (`extern UART_HandleTypeDef huart2;`)
    - `vehicle.h` for accessing `VehicleState_t`
    - `can_if.h` for logging control

- `main.c`
  - Owns:
    - Global `VehicleState_t g_vehicle`
    - Task creation
    - System initialization and peripheral setup

---

## 5. Build & Integration

- Project created and built with **STM32CubeIDE** for STM32F446RE
- Uses:
  - HAL drivers for CAN & UART
  - FreeRTOS CMSIS-RTOS v2 wrapper
- CAN is configured in **loopback mode** so no external transceiver is required.

This architecture is intentionally similar to small automotive ECUs (body, powertrain, or HIL nodes) and is suitable for showcasing:

- RTOS task design
- Interrupt-driven queues
- CAN telemetry encoding
- CLI diagnostics paths
