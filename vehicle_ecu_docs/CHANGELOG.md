# Changelog – STM32 Virtual Vehicle ECU

This changelog tracks major features, fixes, and improvements across the project.  
All versions follow a simple **semantic-style versioning** scheme.

---

## v2.3.0 – Vehicle Model + CLI Integration (LATEST)

### Added
- Full **VehicleState_t** model (speed, RPM, coolant temperature)
- Implemented `Vehicle_Update()` physics loop
- Created **VehicleTask** for periodic updates (100 ms)
- Added CLI commands:
  - `veh status`
  - `veh speed <value>`
  - `veh force`
- Updated CAN telemetry encoding to use vehicle model values
- Expanded documentation:
  - `ARCHITECTURE.md`
  - `VEHICLE_MODEL.md`
  - `CLI_COMMANDS.md`
  - `CAN_PROTOCOL.md`

### Improved
- More realistic RPM + coolant temperature modeling
- Cleaned task structure and modularized CLI / CAN interface

---

## v2.2.0 – CAN Telemetry Encoding

### Added
- `CAN_IF_SendTelemetry()`  
  Encodes:
  - speed ×10 (uint16)
  - RPM (uint16)
  - coolant ×10 (int16)
- CAN framing with Standard ID `0x100`
- CAN loopback testing and frame decode verification

### Improved
- More stable CAN transmission with structured payload layout

---

## v2.1.0 – CAN RX Queue + ISR Integration

### Added
- `CAN_IF_Msg_t` abstraction struct
- Creation of CAN RX RTOS queue
- CAN ISR (`HAL_CAN_RxFifo0MsgPendingCallback`) pushes frames → queue
- CAN RX task processes queue entries
- CLI control for logging:
  - `log on`
  - `log off`

### Fixed
- Logging no longer blocks CLI interaction
- UART output separation for RX logging vs user commands

---

## v2.0.0 – Initial CAN + RTOS Foundation

### Added
- Base FreeRTOS tasks
- UART CLI input buffering + interrupt-driven RX
- Initial CAN1 peripheral setup
- CAN in **loopback mode** for testing without hardware
- Basic CLI echo and placeholders

### Fixed
- CAN initialization issues due to GPIO pull-up configuration
- Race conditions between CLI and logging during early testing

---

## v1.0.0 – Initial Project Setup

### Added
- STM32CubeIDE project creation
- LED blink + UART print sanity tests
- HAL initialization for GPIO + USART2
- Project folder structure foundation

