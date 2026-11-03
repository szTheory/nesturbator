# Nesturbator emulation and persistence

This glossary keeps player, host, and core discussions aligned on region and
progress terms used across the library, runner, and libretro adapter.

## Region and output

**Region profile**:
A console timing and output model for a game. NTSC, PAL, and Dendy are distinct
profiles with their own expectations.
_Avoid_: TV region, frame-rate setting

**Multi-region game**:
Content intended to adapt to more than one timing profile. The host's selected
or default profile determines which region the core runs.
_Avoid_: automatic region detection

**Emulation behavior revision**:
An identity for changes that can alter emulated video or audio behavior. It is
separate from a library release version, API compatibility, and state format.
_Avoid_: core version

## Progress and continuation

**Battery media**:
Persistent cartridge memory that stores game progress across play sessions.
It is game progress data, not a snapshot of the live machine.
_Avoid_: save state, machine state

**Machine state**:
A snapshot of the emulated machine at a point in play, used to resume that
session. Its compatibility is separate from battery media compatibility.
_Avoid_: battery save, save RAM

**State format**:
The compatibility identity for a machine-state representation. It is separate
from the library release version, API compatibility, and emulation behavior
revision.
_Avoid_: library version, behavior revision

## Library identity

**Library release version**:
The version identifying a packaged native library release. By itself, it does
not establish API compatibility, emulation behavior, or machine-state
compatibility.
_Avoid_: ABI version, behavior revision

**API compatibility**:
Whether a host and a library agree on the public calling and data contract.
Matching API compatibility does not establish matching emulation behavior or
machine-state format.
_Avoid_: behavior compatibility

