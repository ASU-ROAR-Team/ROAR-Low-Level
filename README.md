# ROAR-Low-Level
Low Level Repository for the ASU ROAR Mars Rover system.

## Overview

This repository contains the low-level software components for the ASU ROAR (Robotics Operations and Automation Research) Mars Rover system. The software is designed to run on various microcontrollers and embedded systems throughout the rover.

## Components

### Rover Gateway
The [Rover Gateway](./rover_gateway) is the communication hub for the rover, implemented on an STM32F072xB microcontroller. It handles communication between various rover subsystems and the central command system.

### Motor Gateway
The [Motor Gateway](./motor_gateway) controls the rover's motor systems, including drive motors and actuators. It interfaces with RoboClaw motor controllers and uses an Adafruit BNO055 IMU for motion sensing.

## Repository Structure

```
ROAR-Low-Level/
├── motor_gateway/      # STM32-based motor control gateway
├── rover_gateway/      # STM32-based communication gateway
└── ...                 # Other low-level components
```

## Getting Started

1. Clone the repository:
   ```bash
   git clone <repository-url>
   ```

2. Navigate to the component you want to work with:
   ```bash
   cd rover_gateway  # or motor_gateway
   ```

3. Follow the component-specific README for build and deployment instructions.

## Prerequisites

- Appropriate development environment for each component
- ARM GCC toolchain for STM32 components
- CMake 3.15 or higher (for Rover Gateway)
- ST-LINK programmer for flashing STM32 devices

## Contributing

Please read CONTRIBUTING.md for details on our code of conduct and the process for submitting pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contact

For questions or support, please contact the ROAR team at Arizona State University.
