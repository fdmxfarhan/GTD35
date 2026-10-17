# GTD35 Arduino Library

An Arduino library for controlling **GTD35 motor drivers over RS485** using a UART interface. Designed for microcontrollers such as the STM32F103, it provides a simple interface for changing motor IDs and controlling four motors simultaneously.

## Features

- Initialize UART communication with the GTD35 motor drivers.
- Assign motor IDs using a single function call.
- Control four motors simultaneously with signed speed commands.
- Communicate through an RS485-to-UART converter.
- Use a simple API with minimal configuration.

## Installation

### Method 1: Install manually

1. Download or clone this repository.
2. Copy the library folder, named `GTD35`, into your Arduino libraries directory.

   Typical locations:

   **Windows**
   ```text
   Documents/Arduino/libraries/GTD35/
   ```

   **Linux**
   ```text
   ~/Arduino/libraries/GTD35/
   ```

3. Ensure the folder structure is as follows:

   ```text
   GTD35/
   ├── src/
   │   ├── GTD35.h
   │   └── GTD35.cpp
   ├── examples/
   │   ├── SetMotorID/
   │   │   └── SetMotorID.ino
   │   └── FourMotorSpeed/
   │       └── FourMotorSpeed.ino
   ├── library.properties
   └── README.md
   ```

4. Restart Arduino IDE.
5. Open **File → Examples → GTD35** to access the example sketches.

### Method 2: Install from a ZIP archive

1. Download the library repository as a ZIP file.
2. Open Arduino IDE.
3. Select **Sketch → Include Library → Add .ZIP Library...**.
4. Select the downloaded ZIP file.
5. Restart Arduino IDE if necessary.

## Hardware Requirements

- A GTD35 motor driver for each motor.
- A microcontroller with a hardware UART, such as the STM32F103.
- An RS485-to-UART converter compatible with the motor driver's electrical interface.
- A suitable power supply for the motors and drivers.

### Connection overview

Connect the microcontroller UART to the TTL side of the RS485 converter, and connect the RS485 differential bus to the motor driver network.

| Microcontroller / converter | Connection |
|---|---|
| MCU TX | Converter UART RX |
| MCU RX | Converter UART TX |
| Converter A | RS485 A |
| Converter B | RS485 B |
| GND | Common reference, as required by the hardware |

Use the converter manufacturer's pinout to identify A/B and TX/RX correctly. Some converters require a GPIO to control their transmit-enable and receive-enable pins; others handle direction automatically.

**Important:** Use compatible logic voltage levels and follow the GTD35 hardware documentation for power, grounding, and RS485 wiring.

## Quick Start

Include the library and create a driver instance using the UART connected to your RS485 converter.

```cpp
#include <GTD35.h>

GTD35 driver(Serial1);

void setup() {
  driver.begin(115200);
}

void loop() {
  driver.motor(100, 100, 100, 100);
  delay(1000);

  driver.motor(0, 0, 0, 0);
  delay(1000);
}
```

This example initializes the motor UART, commands all four motors to speed `100`, and then sends a stop command.

The UART baud rate must match the configuration of your GTD35 drivers.

## API Reference

### `begin()`

Initializes the UART communication.

```cpp
driver.begin(115200);
```

For a converter that requires manual direction control:

```cpp
driver.begin(115200, RS485_DE_PIN, HIGH);
```

Parameters:

- `baud`: UART baud rate. Defaults to `115200`.
- `directionPin`: GPIO used to control RS485 transmit/receive direction. Defaults to `-1`, meaning automatic direction control.
- `txLevel`: GPIO level that enables transmission. Defaults to `HIGH`.

`Serial1` is the motor communication interface in the examples. You do not need `Serial.begin()` unless you also want to use the computer-facing Serial Monitor for debugging.

### `motor(v0, v1, v2, v3)`

Sets the speed references of all four motors in a single command.

```cpp
driver.motor(v0, v1, v2, v3);
```

The motor order is:

| Argument | Motor |
|---|---|
| `v0` | L1 |
| `v1` | L2 |
| `v2` | R2 |
| `v3` | R1 |

Example:

```cpp
driver.motor(100, 100, -100, -100);
```

Each speed is transmitted as a signed 16-bit value. Positive and negative values represent opposite directions according to the motor driver's configuration.

The library uses the following fixed settings:

- Motor IDs: `0, 1, 2, 3`
- Angle field: `4000`
- Motor enable control bits: `0x0F`
- Encoder reset: disabled

These settings are fixed internally to keep the public interface simple.

**Note:** The supported speed range and the physical direction associated with each sign depend on the GTD35 configuration and motor wiring.

### `setID(newID)`

Changes the ID of a connected motor using a single argument.

```cpp
driver.setID(3);
```

The function sends ID-change commands for previous IDs `0` through `254`, skipping the requested new ID.

**Connect only one motor to the RS485 bus when using this function.**

Before using an ID sweep, confirm the valid ID range and command behavior in the GTD35 documentation. The function sends commands without verifying acknowledgements, so it does not guarantee that the motor accepted the new ID.

### `setMotorID(previousID, newID)`

Changes one specific motor ID directly.

```cpp
driver.setMotorID(1, 3);
```

This sends a command to change the motor currently using ID `1` to ID `3`.

Use this method when the current motor ID is known.

## Example: Four-Motor Speed Control

```cpp
#include <GTD35.h>

GTD35 driver(Serial1);

void setup() {
  driver.begin(115200);
  delay(200);
}

void loop() {
  // Move using the same speed reference for all four motors.
  driver.motor(100, 100, 100, 100);
  delay(1000);

  // Reverse all four motor speed references.
  driver.motor(-100, -100, -100, -100);
  delay(1000);

  // Stop the motors.
  driver.motor(0, 0, 0, 0);
  delay(1000);
}
```

## Communication Protocol

The library implements the serial packet format used in the supplied GTD35 protocol example.

### Packet header

Each command starts with:

```text
0xAA 0xAA
```

### Motor ID assignment

The ID-setting packet follows this format:

| Byte | Description |
|---|---|
| 0 | Header: `0xAA` |
| 1 | Header: `0xAA` |
| 2 | Command length/type field: `0x00` |
| 3 | Previous motor ID |
| 4 | Command: `0x11` |
| 5 | New motor ID |
| 6 | XOR checksum |

### Four-motor motion command

The motion packet contains the header, motor count, four motor IDs, five data bytes per motor, and a final XOR checksum.

Each motor's five-byte data section contains:

1. Control byte.
2. Speed most significant byte.
3. Speed least significant byte.
4. Angle most significant byte.
5. Angle least significant byte.

The checksum is calculated by XOR-ing the bytes beginning at byte index `2`, excluding the header and the checksum byte itself.

## Troubleshooting

**The motors do not respond**

- Check the UART baud rate.
- Verify UART TX/RX wiring.
- Check the RS485 A/B connections.
- Confirm that the motor IDs match the configured IDs.
- Ensure the drivers have appropriate power and a common reference where required.

**The motors respond incorrectly**

- Verify the motor order: L1, L2, R2, R1.
- Test with small speed values first.
- Confirm the sign convention and motor direction configuration.
- Verify that the angle field `4000` matches the required GTD35 protocol.

**The ID does not change**

- Connect only one motor during ID assignment.
- Verify the motor's current ID.
- Confirm that the new ID is valid.
- Remember that the library does not currently check for acknowledgements.

## License

Add a `LICENSE` file to the repository to specify the terms under which this library may be used, modified, and distributed.