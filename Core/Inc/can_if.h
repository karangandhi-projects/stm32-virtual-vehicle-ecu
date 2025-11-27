#ifndef CAN_IF_H
#define CAN_IF_H

#include "main.h"
#include "cmsis_os2.h"
#include "vehicle.h"
#include <stdint.h>

/*
 * Module: CAN Interface (can_if)
 *
 * Role:
 *   - Wraps low-level HAL CAN access behind a small, testable API.
 *   - Owns the RX message queue used by the CanRxTask.
 *   - Encodes/decodes a simple telemetry frame from VehicleState_t.
 *
 * Version history (module-level):
 *   v2.0 - Initial CAN loopback + basic send helper.
 *   v2.1 - Added RX queue, ISR → RTOS hand-off, logging control.
 *   v2.2 - Integrated with VehicleState_t telemetry encoding.
 */

/* --------------------------------------------------------------------------
 * CAN interface types
 * -------------------------------------------------------------------------- */

/**
 * @brief Simple CAN message representation for the interface layer.
 *
 * This type is used by:
 *   - HAL CAN RX ISR (producer)
 *   - CanRxTask (consumer)
 */
typedef struct
{
    uint32_t id;          /**< Standard CAN ID (11-bit) */
    uint8_t  dlc;         /**< Data Length Code (0–8)   */
    uint8_t  data[8];     /**< Data bytes               */
} CAN_IF_Msg_t;

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialize CAN application layer: filters, start, notifications, RX queue.
 *
 * Steps:
 *   - Configure a simple "accept all" filter into FIFO0.
 *   - Start the CAN peripheral (assumes low-level init done in MX_CAN1_Init()).
 *   - Enable RX and error notifications.
 *   - Create the RX message queue used by CanRxTask.
 *
 * @retval HAL_OK on success, error status otherwise.
 */
HAL_StatusTypeDef CAN_IF_Init(void);

/**
 * @brief Send a telemetry CAN frame with the current vehicle state.
 *
 * Encodes:
 *   - speed_kph * 10 (uint16_t)
 *   - engine_rpm     (uint16_t)
 *   - coolant_temp_c * 10 (int16_t)
 *
 * @param vs Pointer to vehicle state.
 * @retval HAL_OK if frame is queued successfully, error status otherwise.
 */
HAL_StatusTypeDef CAN_IF_SendTelemetry(const VehicleState_t *vs);

/**
 * @brief Enable/disable CAN RX logging over UART.
 *
 * When enabled, CAN_IF_ProcessRxMsg() will print frames to the CLI UART.
 *
 * @param enable 0 to disable, non-zero to enable.
 */
void CAN_IF_SetLogging(uint8_t enable);

/**
 * @brief Get the handle to the CAN RX message queue.
 *
 * CanRxTask blocks on this queue and receives CAN_IF_Msg_t messages.
 *
 * @return osMessageQueueId_t of the RX queue, or NULL if init failed.
 */
osMessageQueueId_t CAN_IF_GetRxQueueHandle(void);

/**
 * @brief Process a received CAN message (decode/log/etc).
 *
 * Called from CanRxTask at thread level, not from ISR.
 * Current implementation just logs the frame when logging is enabled.
 *
 * @param msg Pointer to a valid CAN_IF_Msg_t.
 */
void CAN_IF_ProcessRxMsg(const CAN_IF_Msg_t *msg);

#endif /* CAN_IF_H */
