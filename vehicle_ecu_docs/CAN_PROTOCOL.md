# CAN Telemetry Protocol

This document describes the custom **CAN telemetry frame** used by the STM32 Virtual Vehicle ECU.  
The project uses **bxCAN** in **loopback mode**, so no external transceiver or second node is required.

---

## 1. CAN Bus Configuration

| Parameter | Value |
|----------|-------|
| Peripheral | CAN1 (bxCAN) |
| Mode | Loopback Mode |
| Bitrate | 500 kbps (example) |
| Frame Type | Standard ID (11-bit) |
| Interrupts | FIFO0 RX interrupt enabled |

Bit timing is chosen for reliability and simplicity rather than strict automotive tuning.

---

## 2. Telemetry Frame Format

### **Message ID**
```
0x100
```

### **Payload Layout (8 bytes)**

| Byte Index | Signal          | Type   | Scaling | Description |
|-----------|----------------|--------|---------|-------------|
| 0-1       | Speed (km/h)   | uint16 | ×10     | Vehicle speed |
| 2-3       | RPM            | uint16 | ×1      | Engine RPM |
| 4-5       | Coolant Temp   | int16  | ×10     | °C |
| 6-7       | Reserved       | —      | —       | Future use |

> Multi‑byte values are **little‑endian**.

---

## 3. Encoding Example

Given:

- Speed: 45.2 km/h  
- RPM: 1560  
- Coolant: 72.4 °C  

Encoding:

```
speed_encoded   = (uint16) (45.2 × 10) = 452   → 0xC4 0x01
rpm_encoded     = (uint16) 1560        = 1560  → 0x18 0x06
temp_encoded    = (int16) (72.4 × 10)  = 724   → 0xD4 0x02
```

Final CAN frame:

```
ID: 0x100
DLC: 8
DATA: C4 01  18 06  D4 02  00 00
```

---

## 4. Decoding Example

```
uint16_t spd = (data[1] << 8) | data[0];
uint16_t rpm = (data[3] << 8) | data[2];
int16_t  tmp = (data[5] << 8) | data[4];

float speed_kph      = spd / 10.0f;
float engine_rpm     = rpm;
float coolant_temp_c = tmp / 10.0f;
```

---

## 5. Logging Format (UART)

When logging is enabled via CLI (`log on`), example output:

```
CAN RX: ID=0x100 DLC=8
DATA:  C4 01  18 06  D4 02  00 00
```

This lets you inspect frames live over UART.

---

## 6. CAN Receive Flow

1. CAN frame received into FIFO0  
2. HAL ISR triggers `HAL_CAN_RxFifo0MsgPendingCallback()`  
3. ISR copies frame → `CAN_IF_Msg_t`  
4. Frame pushed into RTOS queue  
5. `CanRxTask` pops message  
6. `CAN_IF_ProcessRxMsg()` logs or processes frame  

This structure mimics real automotive ECUs where:

- ISR is **minimal**
- Heavy work is done in a **task context**

---

## 7. Extending This Protocol

Ideas for future additions:

- Add wheel speed frames (`0x101`)
- Add engine load or throttle position
- Add UDS/OBD‑II request/response harness
- Add DTC (diagnostic trouble code) frames
- Add checksum or counter fields
- Move layout to a `.dbc` file for CAN tools

---

## 8. Doxygen Tag

Add to `can_if.h`:

```c
/**
 * @defgroup CAN_IF CAN Interface Layer
 * @brief Handles CAN encoding, decoding, and message routing.
 */
```

This allows Doxygen to group CAN documentation automatically.

