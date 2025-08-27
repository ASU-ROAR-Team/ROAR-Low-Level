# ROAR Motor Gateway

STM32-based motor control gateway for the ASU ROAR Mars Rover system.

## Overview

The Motor Gateway is responsible for controlling the rover's motor systems, including drive motors, actuator motors, and other motorized components. This embedded software runs on an STM32 microcontroller and interfaces with motor drivers to provide precise control of the rover's movement.

## Hardware

- **Microcontroller**: STM32F072xB series
- **Motor Drivers**: RoboClaw motor controllers
- **Sensors**: Adafruit BNO055 IMU for orientation and motion sensing
- **Communication**: UART/Serial interface with RoboClaw controllers

## Project Structure

```
motor_gateway/
├── Core/               # Core system files (CMSIS, HAL, startup)
├── Drivers/            # Hardware abstraction layer and device drivers
├── Adafruit_BNO055/    # IMU sensor library
├── roboclaw/           # RoboClaw motor controller library
├── serial/             # Serial communication library
├── Debug/              # Debug build output
└── *.ld               # Linker script files
```

## Features

- PWM motor speed control
- Position feedback from encoders
- IMU integration for motion tracking
- RoboClaw serial communication protocol
- Emergency stop functionality
- Motor calibration routines
- Real-time motor control loop

## Prerequisites

- STM32CubeIDE or other ARM development environment
- ARM GCC toolchain
- ST-LINK/V2 programmer/debugger

## Build Instructions

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd ROAR-Low-Level/motor_gateway
   ```

2. Open the project in STM32CubeIDE
3. Build the project using the IDE's build system

## Flashing the Firmware

To flash the firmware to the STM32 microcontroller:

```bash
# Using ST-LINK utility
ST-LINK_CLI.exe -c SWD -P motor_gateway.bin 0x08000000 -V after_programming -HardRst

# Using OpenOCD
openocd -f interface/stlink-v2.cfg -f target/stm32f0x.cfg -c "program Debug/motor_gateway.hex verify reset exit"
```

## Development Setup

1. Install STM32CubeIDE
2. Clone this repository
3. Import the project into STM32CubeIDE
4. Install any required drivers for RoboClaw controllers

## Testing

Motor functionality can be tested using:
1. Manual command input via serial terminal
2. Encoder feedback verification
3. IMU data verification
4. Current monitoring during operation

## Safety

When working with motor systems:
- Always disconnect power before making connections
- Ensure all safety protocols are followed
- Test motors in a controlled environment first
- Verify emergency stop functionality before operation

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contact

For questions or support, please contact the ROAR team at Arizona State University.