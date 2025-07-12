#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include "module_id.h"
#include "module_registry.h"

/**
 * @brief Abstract base class for modules.
 * @tparam ModuleTypeT Enum or integer uniquely identifying module type.
 *
 * Handles module registration and provides core interface.
 * Concrete classes must implement write() and read().
 */
template <typename ModuleTypeT>
class BaseModule
{
public:
    /// Construct a Module and register it.
    BaseModule(ModuleTypeT type, uint8_t instance) : moduleId_{type, instance}
    {
        bool reg_ok = ModuleRegistry<ModuleTypeT>::instance().registerModule(this);
        assert(reg_ok && "Module registry is full or duplicate module id!");
    }

    /// Unregister the Module on destruction.
    virtual ~BaseModule()
    {
        ModuleRegistry<ModuleTypeT>::instance().unregisterModule(moduleId_);
    }

    /// Start the module (default: calls onStart()).
    virtual bool start()
    {
        return onStart();
    }

    /// Returns the module's type identifier.
    ModuleTypeT type() const { return moduleId_.type; }

    /// Returns the module's instance number.
    uint8_t instance() const { return moduleId_.instance; }

    /// Returns the ModuleId for this module.
    const ModuleId<ModuleTypeT> &moduleId() const { return moduleId_; }

    /// Write to the module.
    virtual bool write(const void *buf, size_t len, TickType_t ticksToWait = 0) { return false; }

    /// Read from the module.
    virtual bool read(void *buf, size_t maxLen, TickType_t ticksToWait = 0) { return false; }

protected:
    /// Optional hook for startup logic (override in subclass)
    virtual bool onStart() { return true; }

    const ModuleId<ModuleTypeT> moduleId_;
};
