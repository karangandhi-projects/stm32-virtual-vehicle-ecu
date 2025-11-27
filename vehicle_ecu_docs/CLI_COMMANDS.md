# UART CLI – Command Reference

The firmware provides a lightweight **command-line interface (CLI)** over UART for debugging and controlling the virtual vehicle model.

- **UART**: USART2  
- **Baudrate**: 115200  
- **Format**: 8-N-1  
- **Terminal**: PuTTY / CoolTerm / TeraTerm recommended  

---

## 1. Usage

The CLI receives characters through UART interrupts, buffers them, and processes completed lines.  
Every command is followed by **ENTER**.

Example session:

```
help
veh status
veh speed 60
log on
```

---

## 2. Command List

### **help**
Displays available commands.

Example:
```
Commands:
  help
  veh status
  veh speed <value>
  veh force
  log on/off
  clear
```

---

### **veh status**
Prints the current virtual vehicle state.

Example:
```
Vehicle Status:
  Speed   : 42.5 km/h
  RPM     : 1900
  Coolant : 68.3 C
```

---

### **veh speed <value>**
Sets the **target vehicle speed** (km/h), which the vehicle model will gradually approach.

Usage:
```
veh speed 80
```

Response:
```
OK (target speed set to 80 km/h)
```

---

### **veh force**
Applies a fixed, forced vehicle state useful for testing or fault injection.

Example:
```
veh force
OK (forced test values applied)
```

---

### **log on**
Enables CAN RX UART logging.

```
log on
CAN RX logging enabled
```

---

### **log off**
Disables CAN RX UART logging.

```
log off
CAN RX logging disabled
```

---

### **clear**
Clears the terminal using ANSI escape sequences:

```
clear
```

Behavior:
- Clears screen
- Moves cursor to the top-left

---

## 3. Behind the Scenes

The CLI backend (`cli_if.c`) handles:
- UART RX interrupt → pushes characters into a queue  
- Line assembly  
- Command parsing  
- Calling into:
  - `Vehicle_Update()`
  - `CAN_IF_SetLogging()`
  - `Vehicle_SetTargetSpeed()`

The CLI is designed to be **non-blocking** and **RTOS-safe**.

---

## 4. Adding New Commands

To extend the CLI:
1. Open `cli_if.c`
2. Locate `cli_handle_line()`
3. Add a new handler:

```c
else if (strncmp(line, "veh rpm ", 8) == 0)
{
    float rpm = atof(&line[8]);
    Vehicle_SetRPM(&g_vehicle, rpm);
}
```

4. Update the `help` text.

---

## 5. Doxygen Tag

Add inside `cli_if.h`:

```c
/**
 * @defgroup CLI_IF Command Line Interface
 * @brief UART-based diagnostic interface for the ECU.
 */
```

