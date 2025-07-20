/*
 * Blinker – minimal demo for rtos-module-lib (PlatformIO version)
 * Creates a TaskModule that toggles the on-board LED every 500 ms.
 */

#include <Arduino.h>
#include "task_module.h"

enum class MyType : uint8_t
{
    Blinker
};

class Blinker : public TaskModule<MyType>
{
public:
    Blinker() : TaskModule({MyType::Blinker, 0, "Blinker"}) {}

protected:
    bool onStart() override
    {
        pinMode(LED_BUILTIN, OUTPUT);
        return true; // task starts if true
    }

    void run() override
    {
        for (;;)
        {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
};

Blinker blinky; // auto-registers with ModuleRegistry

void setup() { blinky.start(); }
void loop() {} // everything handled in the task
