# Stub Implementations for Missing Capabilities (audio, contacts, phone)

Date: 2025-11-23

Summary

- Implemented minimal capability support for `audio`, `contacts`, and `phone` to remove grant-time warnings and provide forward-compatible tokens.

Details

- Added a minimal `AudioCapability` implementation inside `CapabilityManager.cpp`:
  - Provides a valid capability token with stubbed playback/volume APIs.
  - Logs usage via the audit log; returns benign defaults where appropriate.
- Introduced a lightweight token capability for simple permissions:
  - `TokenCapabilityImpl` used for `contacts` and `phone`.
  - Grants a valid, revocable capability without exposing additional APIs.
- Wired factories and grant paths in `CapabilityManager`:
  - `audio` → `createAudioCapability()`
  - `contacts`/`phone` → `createTokenCapability()`
  - Ensured proper invalidation on revoke/destruction.

Impact

- Bluetooth and Media Player extensions now receive their requested capabilities without `Unknown capability type` warnings.
- Behaviour is conservative (no real audio or PIM/telephony access) but establishes the contract for future full implementations.

Files Changed

- `src/core/capabilities/CapabilityManager.hpp`
- `src/core/capabilities/CapabilityManager.cpp`

Notes

- These are safe, stubbed implementations for development. We can later replace `AudioCapabilityImpl` with a PipeWire/ALSA-backed implementation and split PIM/telephony into richer interfaces as needed.
