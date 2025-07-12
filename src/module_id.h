#pragma once

/**
 * @brief Unique identifier for a module (type + instance).
 * @tparam ModuleTypeT  Enum or integer identifying the module type.
 */
template <typename ModuleTypeT>
struct ModuleId
{
    ModuleTypeT type;
    uint8_t instance;

    // Enables value comparison in registries
    constexpr bool operator==(const ModuleId &other) const
    {
        return type == other.type && instance == other.instance;
    }

    // Enables fast lookup in etl::flat_map (binary search)
    constexpr bool operator<(const ModuleId &other) const
    {
        return (type < other.type) ||
               (type == other.type && instance < other.instance);
    }
};