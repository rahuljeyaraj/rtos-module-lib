# RtosModuleLib

**RtosModuleLib** is a header-only C++ library providing modular, reusable components  
for any RTOS-based embedded system. Designed to be lightweight and flexible, it  
enables clean, maintainable architecture similar to hardware design modules.

## Features

- Header-only for easy integration  
- Designed for any RTOS (not limited to FreeRTOS)  
- Message and task module abstractions  
- Modular, self-contained components inspired by Verilog modules  

## Installation

Add the following to your `platformio.ini`:

```ini
[env:your_env]
platform = <your_platform>
framework = freertos
lib_deps =
  rahuljeyaraj/RtosModuleLib
