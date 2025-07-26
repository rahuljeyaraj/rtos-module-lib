#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "module_id.h"
#include "module_registry.h"

/**
 * @brief Abstract base class for modules.
 * @tparam ModuleTypeT Enum or integer uniquely identifying module type.
 *
 * Handles module registration and provides core interface.
 * Concrete classes must implement write() and read().
 */
// TODO: Consider making InstanceType a template parameter for stricter type safety.
//       WARNING: Templating on InstanceType may cause code bloat if used with many unique instance
//       types. It will also force a separate ModuleRegistry instance for each unique (ModuleTypeT,
//       InstanceTypeT) pair. For most use-cases, retain uint8_t as the instance type for simplicity
//       and minimal code size.
template <typename ModuleTypeT>
class BaseModule {
   public:
    /// Construct a Module and register it.
    BaseModule(const ModuleId<ModuleTypeT> id) : moduleId_{id} {
        bool reg_ok = ModuleRegistry<ModuleTypeT>::instance().registerModule(this);
        assert(reg_ok && "Module registry is full or duplicate module id!");
    }

    /// Unregister the Module on destruction.
    virtual ~BaseModule() {
        ModuleRegistry<ModuleTypeT>::instance().unregisterModule(moduleId_);
    }

    /// Start the module.
    virtual bool start() {
        return true;
    }

    /// Returns the module's type identifier.
    ModuleTypeT type() const {
        return moduleId_.type;
    }

    /// Returns the module's instance number.
    uint8_t instance() const {
        return moduleId_.instance;
    }

    /// Returns the module's name.
    constexpr const char *name() const noexcept {
        return moduleId_.name.data();
    }

    /// Returns the ModuleId for this module.
    const ModuleId<ModuleTypeT> moduleId() const {
        return moduleId_;
    }

    /// Write to the module.
    virtual bool write(const void *buf, size_t len, TickType_t ticksToWait = 0) {
        return false;
    }

    /// Read from the module.
    virtual bool read(void *buf, size_t maxLen, TickType_t ticksToWait = 0) {
        return false;
    }

   protected:
    /// Unique type+instance identifier for this module.
    const ModuleId<ModuleTypeT> moduleId_;
};
