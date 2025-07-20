# rtos-module-lib

C++ toolkit for building lightweight, message-driven modules on FreeRTOS—usable from both ESP-IDF and Arduino-ESP32.

* **Zero runtime cost** – everything is templates and `inline`.
* **Module registry** – look up modules by _type_ + _instance_ ID.
* **Plug-and-play channels** – `MessageBuffer`, `Queue`, or your own `BaseChannel`.
* **Task modules** – spin an RTOS task with one line.
* **No heap surprises** – ETL containers + explicit FreeRTOS calls only.

## Features at a glance

| Class                  | What it does                                             |
|------------------------|----------------------------------------------------------|
| `BaseModule`           | Core ID + registry hooks                                 |
| `ModuleRegistry`       | O(1) lookup for modules (thread-safe)                    |
| `RWBufferedModule`     | Drop-in Rx/Tx buffering via any `BaseChannel`            |
| `TaskModule`           | Adds its own FreeRTOS task + idle tick loop              |
| `MBufChannel`          | `xMessageBuffer` wrapper with optional mutexes           |
| `QueueChannel`         | `xQueue` wrapper                                         |
| `ModuleId`             | Tiny POD: type + instance + human name                   |

## Install (PlatformIO)

```ini
lib_deps =
  rahuljeyaraj/rtos-module-lib@^1.0.0
