/**
 * @file    cli_if.c
 * @brief   Minimal UART CLI implementation for Mini ECU.
 *
 * Responsibilities:
 *   - Interrupt-driven RX into a ring buffer.
 *   - Simple line editor + parser.
 *   - Commands for vehicle state and CAN logging.
 */

#include "cli_if.h"
#include "can_if.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>   /* atof */
#include "vehicle.h"  /* VehicleState_t */

extern VehicleState_t g_vehicle;   /* defined in main.c */

/* Static references to UART and vehicle state */
static UART_HandleTypeDef *s_cliUart = NULL;
static VehicleState_t     *s_vehicle = NULL;

/* RX byte buffer for interrupt-driven receive */
static uint8_t s_rxByte;

/* Ring buffer for received characters (from ISR to main context) */
static volatile uint8_t  s_cliBuf[64];
static volatile uint8_t  s_cliHead = 0;
static volatile uint8_t  s_cliTail = 0;

/* --------------------------------------------------------------------------
 * Local helpers
 * -------------------------------------------------------------------------- */

static void cli_uart_print(const char *s)
{
    if (s_cliUart == NULL) return;
    HAL_UART_Transmit(s_cliUart, (uint8_t *)s, strlen(s), HAL_MAX_DELAY);
}

static void cli_push(uint8_t c)
{
    uint8_t next = (uint8_t)((s_cliHead + 1U) % sizeof(s_cliBuf));
    if (next != s_cliTail)
    {
        s_cliBuf[s_cliHead] = c;
        s_cliHead = next;
    }
    /* If full, silently drop */
}

/* Local line-based parser */
static void cli_handle_char(uint8_t c)
{
    static char line[32];
    static uint8_t idx = 0;

    if (c == '\r' || c == '\n')
    {
        if (idx == 0)
        {
            cli_uart_print("\r\n> ");
            return;
        }

        line[idx] = '\0';
        idx = 0;

        /* --- Command decoding --- */

        if ((strcmp(line, "h") == 0) || (strcmp(line, "help") == 0))
        {
            cli_uart_print("\r\nCommands:\r\n");
            cli_uart_print("  help          - show this help\r\n");
            cli_uart_print("  status        - show basic vehicle state\r\n");
            cli_uart_print("  veh status    - show detailed vehicle state\r\n");
            cli_uart_print("  veh speed X   - set target speed to X km/h\r\n");
            cli_uart_print("  veh cool-hot  - inject coolant overheat\r\n");
            cli_uart_print("  log on        - enable CAN RX logging\r\n");
            cli_uart_print("  log off       - disable CAN RX logging\r\n> ");
        }
        else if (strcmp(line, "status") == 0)
        {
            if (s_vehicle)
            {
                char buf[128];
                snprintf(buf, sizeof(buf),
                         "\r\nSpeed:   %.1f km/h\r\n"
                         "RPM:     %u\r\n"
                         "Coolant: %.1f C\r\n> ",
                         s_vehicle->speed_kph,
                         s_vehicle->engine_rpm,
                         s_vehicle->coolant_temp_c);
                cli_uart_print(buf);
            }
            else
            {
                cli_uart_print("\r\n[ERR] No vehicle bound to CLI\r\n> ");
            }
        }
        else if (strcmp(line, "log on") == 0)
        {
            CAN_IF_SetLogging(1);
            cli_uart_print("\r\nCAN logging ENABLED\r\n> ");
        }
        else if (strcmp(line, "log off") == 0)
        {
            CAN_IF_SetLogging(0);
            cli_uart_print("\r\nCAN logging DISABLED\r\n> ");
        }
        else if (strcmp(line, "veh status") == 0)
        {
            char buf[128];
            snprintf(buf, sizeof(buf),
                     "\r\nVehicle:\r\n"
                     "  Speed   : %.1f km/h\r\n"
                     "  RPM     : %u\r\n"
                     "  Coolant : %.1f C\r\n> ",
                     g_vehicle.speed_kph,
                     g_vehicle.engine_rpm,
                     g_vehicle.coolant_temp_c);
            cli_uart_print(buf);
        }
        else if (strncmp(line, "veh speed ", 10) == 0)
        {
            float v = atof(&line[10]);  /* very simple parsing; assumes valid input */
            Vehicle_SetTargetSpeed(&g_vehicle, v);
            cli_uart_print("\r\nOK: speed updated\r\n> ");
        }
        else if (strcmp(line, "veh cool-hot") == 0)
        {
            /* Quick “overheat” demo */
            Vehicle_Force(&g_vehicle,
                          g_vehicle.speed_kph,
                          g_vehicle.engine_rpm,
                          115.0f);
            cli_uart_print("\r\nInjected: coolant overheat\r\n> ");
        }
        else
        {
            cli_uart_print("\r\nUnknown command. Try 'help'.\r\n> ");
        }
    }
    else
    {
        if (idx < (sizeof(line) - 1U))
        {
            line[idx++] = (char)c;

            /* Echo */
            if (s_cliUart)
            {
                HAL_UART_Transmit(s_cliUart, &c, 1, HAL_MAX_DELAY);
            }
        }
        /* else: line overflow, extra chars ignored */
    }
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void CLI_IF_Init(UART_HandleTypeDef *huart, VehicleState_t *vehicle)
{
    s_cliUart  = huart;
    s_vehicle  = vehicle;
    s_cliHead  = 0;
    s_cliTail  = 0;

    if (s_cliUart)
    {
        /* Start RX interrupt */
        HAL_UART_Receive_IT(s_cliUart, &s_rxByte, 1);
        cli_uart_print("\r\nCLI ready. Type 'help' and press Enter.\r\n> ");
    }
}

void CLI_IF_Task(void)
{
    /* Drain ring buffer and feed the line parser */
    while (s_cliTail != s_cliHead)
    {
        uint8_t c = s_cliBuf[s_cliTail];
        s_cliTail = (uint8_t)((s_cliTail + 1U) % sizeof(s_cliBuf));
        cli_handle_char(c);
    }
}

/* --------------------------------------------------------------------------
 * HAL Weak callback override – lives in this module now
 * -------------------------------------------------------------------------- */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == s_cliUart)
    {
        cli_push(s_rxByte);
        HAL_UART_Receive_IT(s_cliUart, &s_rxByte, 1);
    }
}
