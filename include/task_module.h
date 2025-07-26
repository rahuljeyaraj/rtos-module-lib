#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "base_module.h"
#include "freertos/FreeRTOS.h"
#include "rw_buffered_module.h"

/**
 * @brief Base for modules with their own FreeRTOS task.
 *
 * - Default loop: calls onIdleTick().
 * - Override points: onTaskStart(), run(), onIdleTick().
 *
 * @tparam TypeT Enum or integer uniquely identifying module types.
 */
template <typename ModuleTypeT>
class TaskModule : public RWBufferedModule<ModuleTypeT> {
   public:
    TaskModule(const ModuleId<ModuleTypeT> id, BaseChannel *inBuf = nullptr,
               BaseChannel *outBuf = nullptr, uint32_t stackSize = 4096, UBaseType_t priority = 1)
        : RWBufferedModule<ModuleTypeT>(id, inBuf, outBuf),
          stackSize_(stackSize),
          priority_(priority) {
        snprintf(taskName_, sizeof(taskName_), "module_%d_%d_task", static_cast<int>(id.type),
                 static_cast<int>(id.instance));
    }

    /// Destroys the AsyncModule and releases resources.
    ~TaskModule() override {
        if (taskHandle_) {
            // WARNING: Forced task deletion! May cause use-after-free or resource leaks.
            // TODO: Replace with a safe shutdown signal.
            vTaskDelete(taskHandle_);
            taskHandle_ = nullptr;
        }
    }

    /// Spawns the module's FreeRTOS task.
    bool start() override {
        if (taskHandle_ != nullptr) return false;  // Task already created/running.

        return xTaskCreate(&TaskModule::taskEntry, taskName_, stackSize_, this, priority_,
                           &taskHandle_) == pdPASS;
    }

   protected:
    char taskName_[32];
    virtual bool onTaskStart() {  // override for setup, return false to skip run
        return true;
    }
    virtual void run() = 0;      // pure virtual: main loop (child must implement)
    virtual void onTaskEnd() {}  // override for cleanup if needed

   private:
    /// Static entry point for the FreeRTOS task.
    static void taskEntry(void *param) {
        auto *self = static_cast<TaskModule *>(param);
        if (self->onTaskStart()) {
            self->run();
        }
        self->onTaskEnd();
        self->taskHandle_ = nullptr;
        vTaskDelete(nullptr);
    }

    uint32_t stackSize_;
    UBaseType_t priority_;
    TaskHandle_t taskHandle_ = nullptr;
};
