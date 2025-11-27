/**
 * @mainpage STM32 Virtual Vehicle ECU (FreeRTOS + CAN + CLI)
 *
 * @section intro_sec Introduction
 *
 * This firmware implements a small, virtual automotive ECU on an STM32F446RE
 * Nucleo board. It demonstrates:
 *  - A virtual vehicle model (speed, RPM, coolant temperature)
 *  - CAN telemetry using bxCAN (loopback mode for single-board tests)
 *  - A UART-based command-line interface (CLI)
 *  - FreeRTOS-based scheduling and ISR → RTOS message passing
 *
 * The goal is to mimic realistic embedded/automotive project structure,
 * suitable for technical interviews and portfolio demonstrations.
 *
 * @section arch_sec Architecture
 *
 * High-level blocks:
 *  - Vehicle model (`vehicle.c` / `vehicle.h`)
 *  - CAN interface (`can_if.c` / `can_if.h`)
 *  - CLI interface (`cli_if.c` / `cli_if.h`)
 *  - Top-level orchestration and RTOS tasks (`main.c`)
 *
 * @section tasks_sec RTOS Tasks
 *  - VehicleTask: updates the model and sends CAN telemetry
 *  - CanRxTask: receives and processes CAN messages via an RTOS queue
 *  - CliTask: polls and parses UART input, executes commands
 *
 * @section usage_sec How to Build
 *
 *  1. Open the project in STM32CubeIDE (F446RE Nucleo target).
 *  2. Build and flash the firmware.
 *  3. Open a UART terminal at 115200 8N1.
 *  4. Use `help` in the CLI to discover available commands.
 *
 * @section doxy_sec Doxygen Notes
 *
 * This documentation was generated using a minimal C-focused Doxygen
 * configuration. Source-level comments in `vehicle.c`, `can_if.c`,
 * `cli_if.c` and `main.c` are written to be Doxygen-friendly.
 */
