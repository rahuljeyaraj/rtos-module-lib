#pragma once

#include <cstddef>

#include "etl/flat_map.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "module_id.h"

#ifndef MAX_MODULES
#define MAX_MODULES 8
#endif

// Forward-declare your module base
template <typename ModuleTypeT>
class BaseModule;

namespace rtos_module {
// Simple LockGuard for FreeRTOS mutexes (always non-optional)
struct LockGuard {
    SemaphoreHandle_t mtx;
    LockGuard(SemaphoreHandle_t m) : mtx(m) {
        xSemaphoreTake(mtx, portMAX_DELAY);
    }
    ~LockGuard() {
        xSemaphoreGive(mtx);
    }
};
}  // namespace rtos_module

template <typename ModuleTypeT>
class ModuleRegistry {
   public:
    using Key = ModuleId<ModuleTypeT>;
    using Value = BaseModule<ModuleTypeT> *;

    static ModuleRegistry &instance() {
        static ModuleRegistry inst;
        return inst;
    }

    bool registerModule(Value mod) {
        rtos_module::LockGuard lock(mtx_);
        auto res = map_.insert(etl::make_pair(mod->moduleId(), mod));
        return res.second;  // false if duplicate or full
    }

    bool unregisterModule(const Key &id) {
        rtos_module::LockGuard lock(mtx_);
        return map_.erase(id) != 0;
    }

    Value find(const Key &id) {
        rtos_module::LockGuard lock(mtx_);
        auto it = map_.find(id);
        return (it != map_.end()) ? it->second : nullptr;
    }

    size_t count() const {
        // ETL containers are not thread-safe for concurrent readers/writers,
        // so we lock here as well for safety.
        rtos_module::LockGuard lock(mtx_);
        return map_.size();
    }

   private:
    ModuleRegistry() {
        mtx_ = xSemaphoreCreateMutex();
    }
    ~ModuleRegistry() {
        vSemaphoreDelete(mtx_);
    }

    etl::flat_map<Key, Value, MAX_MODULES> map_;
    mutable SemaphoreHandle_t mtx_;
};

template <typename ModuleTypeT>
inline BaseModule<ModuleTypeT> *lookup(const ModuleId<ModuleTypeT> &id) {
    return ModuleRegistry<ModuleTypeT>::instance().find(id);
}
