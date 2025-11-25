# Capability Architecture

## Overview
Crankshaft uses a capability-based security and extension model. Each feature an extension may request (location, network, filesystem, ui, event, bluetooth, wireless, audio, etc.) is represented by an abstract interface header (e.g. `LocationCapability.hpp`). Implementations were previously embedded directly inside `CapabilityManager.cpp`. This refactor extracts each implementation into its own pair of files following an Interface/Implementation pattern for clarity, maintainability and future evolution.

## Rationale for Interface / Implementation Split
- **Encapsulation**: Implementation details (state, timers, platform objects) are hidden from extensions, which only include interfaces. This reduces rebuild scope for dependent extensions.
- **Substitutability**: Backends can change (e.g. a mock, a Linux NetworkManager DBus backend, or a specialised Raspberry Pi backend) without changing consumer code or the manager logic. Factories simply return a different implementation.
- **Testability**: Interfaces can be easily mocked in unit tests. Implementations live in focused compilation units enabling isolated tests and profiling.
- **Compile Efficiency**: Changes inside an implementation CPP no longer force recompilation of all translation units including `CapabilityManager.cpp`.
- **Separation of Concerns**: `CapabilityManager.cpp` focuses on lifecycle, granting, revoking, and auditing – not concrete feature logic.
- **Consistency**: Wireless capability already followed this pattern; aligning all capabilities prevents style drift.

## File Layout
Each capability now has:
- `capabilities/<Name>Capability.hpp` – abstract interface (already existed).
- `capabilities/<Name>CapabilityImpl.hpp` – concrete class declaration + factory helper.
- `capabilities/<Name>CapabilityImpl.cpp` – method definitions (unless trivially inline).

Example (simplified):
```cpp
// LocationCapabilityImpl.hpp
class LocationCapabilityImpl : public LocationCapability { /* ... */ };
inline std::shared_ptr<LocationCapability> createLocationCapabilityInstance(const QString& id, CapabilityManager* mgr) { /* ... */ }
```

`CapabilityManager` includes the required `*Impl.hpp` headers and calls factory helpers inside its `createXCapability` methods.

## Migration Notes
No public APIs changed. Extensions requesting capabilities continue to do so via manifest permissions; only internal file organisation changed. Dynamic casts in the manager destructor still work because implementations are now in separate headers.

## Future Work
- Replace stub backends (Audio, Wireless) with real implementations.
- Introduce an optional plugin loader for platform-specific capability providers.
- Add per-capability audit verbosity levels & structured metrics.

## Contributing
When adding a new capability:
1. Create interface header `<NewCapability>.hpp` (pure virtual).
2. Create `<NewCapability>Impl.hpp/.cpp` and a factory helper `create<NewCapability>CapabilityInstance`.
3. Add a factory method plus case in `CapabilityManager::grantCapability`.
4. Update this document if any architectural patterns evolve.

Use British English in comments and documentation.
