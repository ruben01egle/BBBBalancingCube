# lib/

This folder is for custom, reusable components (local libraries).

Each subfolder in `lib/` should contain a self-contained module, optionally with its own `src/` and `include/` structure. 
These can be reused across multiple projects or contribute to separation of concerns in your codebase.

## Guidelines

- Each library must be in its own subfolder.
- Structure each library like a mini-project:
    - `lib/<name>/src/...`
    - `lib/<name>/include/...`
    or for smaller libs:
    - `lib/<name>/...`

---

## Example structure
```
lib/
├── Hardware/
│   └── CMotor.hpp
│   └── CMotor.cpp
│   └── CSensor.cpp
│   └── CSensor.cpp
├── Framework/
│   └── CSemaphore.hpp
│   └── CSemaphore.cpp
│   └── CSocket.cpp
│   └── CSocket.cpp
```

---

## Provided framework
 
The provided framework manages hardware access, network communication and is the base for the multi-thread environment.

### Framework

The general framework provides a pre-configured multi-threading environment, including POSIX thread management, mutexes, semaphores, and data classes to store and exchange information within and between threads.

### Hardware

The **Hardware** module encapsulates all low-level access to the physical components of the system.  
It provides a clean abstraction layer between the control logic and the underlying hardware drivers.  
All hardware interaction is managed through the central interface class **`CHardware`**.

The `CHardware` class serves as the **host and entry point** to all hardware functionality on the BeagleBone Black.  
It aggregates and coordinates access to sensors, motor control, ADC readings, and GPIO-based brake handling.

#### Responsibilities

- Acts as a unified interface for all hardware-related operations.  
- Combines multiple hardware driver classes (`CADCMMap`, `CMPU9250`, `CMotor`, `CGPIO`).  
- Provides high-level functions to interact with sensors and actuators in a safe and consistent way.

#### Function Overview

| Function | Description |
|----------|-------------|
| `fetchValues` | Reads the current ADC value and IMU sensor data from both sensors. |
| `enableMotor` | Activates the motor driver, allowing torque commands to be applied. |
| `disableMotor` | Deactivates the motor driver, ensuring the motor is safely turned off. |
| `setTorque` | Sends a torque command to the motor driver to control motor output for balancing the cube. |

#### Internal Components

- **`CADCMMap`** — Low-level ADC access for analog sensor readings.  
- **`CMPU9250`** — IMU sensor interfaces (two instances for redundant measurement).  
- **`CMotor`** — Motor driver control (enable, disable, torque commands).  
- **`CGPIO`** — Digital I/O for actuator control.  

---

### socket

The **Socket** module provides a simple TCP server interface for communication with the python gui to visualize and record the experiment data. It abstracts the low-level socket handling and ensures reliable message transmission and reception.

The `CServer` class implements a blocking TCP server on the BeagleBone Black.  
It manages client connections, sending, and receiving structured messages (`CContent`), handling disconnects transparently.

#### Responsibilities

- Provides a unified interface for TCP communication.  
- Manages client connections and socket lifecycle.  
- Ensures blocking transmission and reception of messages.  
- Abstracts low-level socket details (`socket()`, `bind()`, `listen()`, `accept()`, `send()`, `recv()`).

#### Function Overview

| Function | Description |
|----------|-------------|
| `init` | Initializes the server socket and prepares it for client connections. |
| `waitForClient` | Blocks until a client connects to the server. |
| `transmitMessage` | Sends a structured message (`CContent`) to the connected client. Returns `true` on success, `false` if the client disconnected. |

#### Internal Components

- **`mSocketFD`** — File descriptor for the server socket.  
- **`mConnectedSocketFD`** — File descriptor for the active client connection.  
- **`mClientAddr` / `mClientLen`** — Stores the client's address information for connection handling.  
- **Port `40000`** — The default listening port for the server.  