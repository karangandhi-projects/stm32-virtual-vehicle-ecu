# Module Overview

- `vehicle`  : Virtual vehicle model (speed, RPM, coolant temperature).
- `can_if`   : CAN telemetry interface, filters + ISR → RTOS queue.
- `cli_if`   : UART command-line interface, interrupt-driven RX.
- `main`     : FreeRTOS task creation and global orchestration.

Each module uses brief @file headers and Doxygen-style comments on the
main public APIs to make the generated documentation easy to browse.
