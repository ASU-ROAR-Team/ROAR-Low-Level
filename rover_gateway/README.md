# Rover Gateway - STM32 CAN-to-UART Bridge

## Overview

The Rover Gateway is an embedded systems project implemented on the STM32F072C8T6 microcontroller that acts as a bidirectional bridge between CAN bus and UART communication protocols. This device enables communication between CAN-based systems and UART-based systems, making it ideal for robotics applications where different subsystems use different communication protocols.

## Hardware Platform

- **Microcontroller**: STM32F072C8T6 (ARM Cortex-M0, 64KB Flash, 16KB RAM)
- **Communication Interfaces**:
  - CAN bus (via PA11/CAN_RX, PA12/CAN_TX)
  - UART (via PA2/USART2_TX, PA3/USART2_RX)
- **Status Indicator**: LED on PB15 for visual feedback
- **Clock Source**: External 16MHz crystal (PF0/PF1)

## Project Structure

```
rover_gateway/
├── Core/
│   ├── Inc/           # Header files
│   └── Src/           # Source files
├── Drivers/           # STM32 HAL drivers
├── cmake/             # CMake build configuration
├── .ioc               # STM32CubeMX configuration file
├── CMakeLists.txt     # CMake build script
└── README.md          # This file
```

## Features

1. **Bidirectional Communication**:
   - Translates CAN messages to UART frames
   - Converts UART frames back to CAN messages

2. **Frame Protocol**:
   - Custom UART framing with start/end markers
   - Checksum validation for data integrity
   - Fixed-length message format for reliable parsing

3. **Real-time Processing**:
   - Interrupt-driven UART reception
   - CAN message reception via FIFO interrupts
   - Non-blocking message processing

4. **Error Handling**:
   - Checksum validation for received UART frames
   - CAN bus error detection and reporting
   - Robust state machine for UART parsing

## Communication Protocol

### UART Frame Format

All UART messages follow this structure:

```
[START][IDH][IDL][DLC][DATA0]...[DATA7][CHECKSUM][END]
```

- **START**: 0xAA (start marker)
- **IDH**: High byte of CAN message ID
- **IDL**: Low byte of CAN message ID
- **DLC**: Data length code (0-8 bytes)
- **DATA**: 0-8 bytes of payload data
- **CHECKSUM**: XOR of IDH, IDL, DLC, and all data bytes
- **END**: 0x55 (end marker)

### CAN Message Handling

- **CAN Baud Rate**: 500 kbps
- **Message IDs**: Standard 11-bit identifiers
- **Data Length**: 0-8 bytes per message

## Implementation Details

### Main Components

1. **CAN Interface**:
   - Configured for 500 kbps communication
   - Uses interrupt-based reception
   - Implements transmit mailbox system

2. **UART Interface**:
   - Configured for 115200 baud
   - Uses interrupt-driven reception with buffering
   - Implements state machine for frame parsing

3. **Gateway Functions**:
   - `gateway_send_can_over_uart()`: Converts CAN messages to UART frames
   - `gateway_process_uart_byte()`: Parses UART bytes and reconstructs CAN messages

### State Machine

The UART reception uses a state machine with the following states:
1. `UART_STATE_WAITING_FOR_START`: Waiting for start byte (0xAA)
2. `UART_STATE_READING_ID_HIGH`: Reading high byte of message ID
3. `UART_STATE_READING_ID_LOW`: Reading low byte of message ID
4. `UART_STATE_READING_DLC`: Reading data length code
5. `UART_STATE_READING_DATA`: Reading payload data
6. `UART_STATE_READING_CHECKSUM`: Reading and validating checksum
7. `UART_STATE_READING_END`: Reading end byte (0x55) and sending CAN message

### LED Indication

The LED on PB15 toggles to indicate:
- Successful CAN-to-UART message forwarding
- Successful UART-to-CAN message forwarding

## Build System

This project uses CMake for building:

```bash
# Configure the project
cmake -B build

# Build the project
cmake --build build
```

## Development Environment

- **IDE**: STM32CubeIDE or any CMake-compatible IDE
- **Toolchain**: GCC ARM Embedded
- **Debugger**: ST-LINK/V2 or compatible
- **Framework**: STM32 HAL Library

## Flashing the Device

Use STM32CubeProgrammer or OpenOCD to flash the compiled binary to the microcontroller:

```bash
# Using OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f0x.cfg -c "program build/rover_gateway.elf verify reset exit"
```

## Usage

1. Connect the device to a CAN bus network
2. Connect the UART interface to a host computer or another microcontroller
3. Power the device (3.3V or 5V tolerant)
4. Monitor the LED on PB15 for communication activity

## Extending the Project

To add new features or modify existing functionality:

1. Add new source files to `Core/Src/` and corresponding headers to `Core/Inc/`
2. Update `CMakeLists.txt` to include new source files
3. Rebuild the project using CMake

## Troubleshooting

Common issues and solutions:

1. **No communication**: Check wiring and power supply
2. **LED not blinking**: Verify CAN bus activity or UART traffic
3. **Checksum errors**: Check UART signal integrity and baud rate settings
4. **CAN errors**: Verify termination resistors and bus wiring

## License

This project is provided as-is without warranty. See individual files for specific licensing information.