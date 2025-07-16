#pragma once

#include "base_module.h"
#include "freertos/FreeRTOS.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include "rw_buffered_module.h"

/**
 * @brief Base for modules with their own FreeRTOS task.
 *
 * - Default loop: calls onIdleTick().
 * - Override points: onStart(), run(), onIdleTick().
 *
 * @tparam TypeT Enum or integer uniquely identifying module types.
 */
template <typename ModuleTypeT>
class TaskModule : public RWBufferedModule<ModuleTypeT>
{
public:
    TaskModule(const ModuleId<ModuleTypeT> &id,
               BaseChannel *inBuf = nullptr,
               BaseChannel *outBuf = nullptr,
               uint32_t stackSize = 4096,
               UBaseType_t priority = 1)
        : RWBufferedModule<ModuleTypeT>(id, inBuf, outBuf),
          stackSize_(stackSize),
          priority_(priority)
    {
        snprintf(taskName_, sizeof(taskName_), "mod_%d_%d",
                 static_cast<int>(type), static_cast<int>(instance));
    }

    /// Destroys the AsyncModule and releases resources.
    ~TaskModule() override
    {
        if (taskHandle_)
        {
            // WARNING: Forced task deletion! May cause use-after-free or resource leaks.
            // TODO: Replace with a safe shutdown signal.
            vTaskDelete(taskHandle_);
            taskHandle_ = nullptr;
        }
    }

    /// Spawns the module's FreeRTOS task.
    bool start() override
    {
        if (taskHandle_ != nullptr)
            return false; // Task already created/running.

        return xTaskCreate(&TaskModule::taskEntry,
                           taskName_, stackSize_, this,
                           priority_, &taskHandle_) == pdPASS;
    }

protected:
    /// Allows subclasses to customize the idle tick delay.
    virtual TickType_t idleTickDelay() const { return pdMS_TO_TICKS(10); }

    /// The default task loop (idle tick).
    virtual void run()
    {
        for (;;)
        {
            onIdleTick();
            vTaskDelay(idleTickDelay());
        }
    }

    /// Called every idle tick.
    virtual void onIdleTick() {}

private:
    /// Static entry point for the FreeRTOS task.
    static void taskEntry(void *param)
    {
        auto *self = static_cast<TaskModule *>(param);
        if (self->onStart())
            self->run();
        self->onEnd();
        self->taskHandle_ = nullptr;
        vTaskDelete(nullptr);
    }

    /// Optional hook for shutdown/cleanup logic (override in subclass)
    virtual void onEnd() { return; }

protected:
    char taskName_[32];
    uint32_t stackSize_;
    UBaseType_t priority_;
    TaskHandle_t taskHandle_ = nullptr;
};
