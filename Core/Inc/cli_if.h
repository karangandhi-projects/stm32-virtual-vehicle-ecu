#ifndef CLI_IF_H
#define CLI_IF_H

#include "main.h"
#include "vehicle.h"

/*
 * Module: CLI Interface (cli_if)
 *
 * Role:
 *   - Owns a small line-based UART CLI.
 *   - Uses interrupt-driven RX into a ring buffer.
 *   - Parsed in CLI_IF_Task() at thread level.
 *
 * Example commands (current set):
 *   help          - show built-in help
 *   status        - show basic vehicle state
 *   veh status    - detailed vehicle state view
 *   veh speed X   - set target speed to X km/h
 *   veh cool-hot  - inject coolant overheat
 *   log on/off    - enable/disable CAN RX log printing
 */

/**
 * @brief Initialize the CLI interface.
 *
 * This:
 *   - Stores the UART handle used for CLI (typically &huart2).
 *   - Binds a VehicleState_t pointer for status/diagnostic commands.
 *   - Arms the first UART RX interrupt and prints a greeting/prompt.
 *
 * @param huart    UART handle used for CLI (e.g., &huart2).
 * @param vehicle  Pointer to global vehicle state to inspect/control.
 */
void CLI_IF_Init(UART_HandleTypeDef *huart, VehicleState_t *vehicle);

/**
 * @brief Poll the CLI, process any received characters/commands.
 *
 * Call this frequently from:
 *   - A dedicated RTOS task (recommended), or
 *   - A main() super-loop in a non-RTOS build.
 *
 * This function is non-blocking; it simply drains the ring buffer and
 * feeds characters into the line parser.
 */
void CLI_IF_Task(void);

#endif /* CLI_IF_H */

