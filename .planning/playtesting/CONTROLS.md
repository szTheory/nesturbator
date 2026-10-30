# Xbox controller testing cheat sheet

Read from the installed RetroArch SDL2 configuration and Steam Virtual Gamepad
profile on 2026-10-30. These are configured bindings, not a claim that every
combination has been physically tested. **View/Back** is the small button left
of Xbox; **Menu/Start** is the small button to its right. **L3/R3** means pressing
the left/right stick down like a button.

## While a game is running in RetroArch

Hold View first, then press the second button.

| Action | Xbox combination |
|---|---|
| Reset/restart the game | **View + L3** |
| Quit the game and return to ES-DE | **View + Menu/Start** |
| Open RetroArch's Quick Menu | **View + R3** |
| Take a screenshot | **View + B** |
| Save state in the selected slot | **View + RB** |
| Load state from the selected slot | **View + LB** |
| Next / previous state slot | **View + D-pad right / left** |
| Pause / resume emulation | **View + A** |

View alone is NES Select; Menu alone is NES Start. The installed mapping uses
Xbox **B for NES A**, and Xbox **A for NES B**. In-game instructions naming NES
A/B therefore differ from Xbox lettering.

These are RetroArch bindings. ES-DE manages the collection and launches the
host; it does not implement an NES reset. In ES-DE's own menus, use the visible
button legend rather than assuming in-game hotkeys apply there.

## Save slots and automatic resume

Choose an unused numbered slot before making a problem-scene checkpoint.
Loading a snapshot can also restore battery RAM and later affect saved daily
progress. Existing selected-game progress is privately backed up; preserve it
before experimenting. Rewind and fast-forward affect the test context and are
not needed for the current visual probes.

The installed configuration enables automatic save and automatic load. A live
Mario 3 log explicitly recorded a successful automatic load of a Nesturbator
state. That is actual same-core load evidence, separate from the earlier Zelda
battery-save reuse. A controlled manual save/exit/relaunch test and compatibility
with other cores or future formats remain separate claims.

Reset restarts the running core; it is not a clean power-on experiment and does
not disable automatic resume on the next launch. The private replay recipes
offer a fresh-content-load path when boot behavior matters, without changing
daily-play settings.

## Controller-port correction and simple diagnostics

The single-controller launcher now leaves player two unassigned. Previously,
player one and inherited player two both used the Xbox controller's index.
A relaunched Mega Man 3 configuration was read back with distinct P1/unassigned
P2 choices. Two-player play needs an explicit separate controller mapping.
The owner still reports crowded/damage slowdown after the correction; the port
fix does not explain it away, and the visual seams remain open.

Keep the FPS counter visible if useful, note audio crackle/speed changes and
record the exact scene, enemy load or action. No profiler/debugger needs to be
open for casual play. Sprite flicker, game-side lag, host frame pacing and
persistent graphics corruption are different observations.

The owner reports 59.96–59.98 with no correlation between those small changes
and the action slowdown. The actual core advertises 60.0988138974 Hz, matching
the usual NTSC NES rate; the host is configured for a 60 Hz display with VSync
and audio synchronization. Display-counter fluctuations alone do not establish
an accuracy issue. Stable near-full output while action slows is consistent
with game-side lag, but not proof of hardware-correct emulation timing.
See [NES timing](https://www.nesdev.org/wiki/Cycle_reference_chart) and
[frontend rate control](https://docs.libretro.com/development/cores/dynamic-rate-control/).
A later Shatterhand drifting-line report led to a backed-up per-core
presentation override: regular VSync on, Adaptive VSync off for the next launch.
A fresh Shatterhand launch was read back with regular VSync on and adaptive
VSync off. The owner still reports falling/left-right disturbances and slowdown;
the original exact drifting-line comparison remains inconclusive. This setting
is not credited as a fix. Relaunching alone applies it; reset is unnecessary.

## Screenshots and clips

View+B uses the configured RetroArch screenshots directory. The agent can also
capture the current display over SSH without pressing buttons. The private
capture helper checks game/core identity and records actual sample times; its
short display clips have no audio, may include overlays, and are not performance
measurements. Capture work can itself perturb timing.

The private headless replay helper instead exports exact native frame captures,
PCM audio and a short MP4 from a deterministic input recipe. See
[automation and regression direction](AUTOMATION.md) for the verified scope and
the distinction between native checkpoints and RetroArch disk containers.
Raw captures and save bytes stay outside Git.

[EmuDeck's hotkey reference](https://emudeck.github.io/controls-and-hotkeys/steamos/hotkeys/)
and [RetroArch's input guide](https://docs.libretro.com/guides/input-and-controls/)
provide background; the live installed configuration is authoritative here.

The later host log showed a failed load of numbered slot 0; that file did not
exist. A separate Shatterhand autosave exists and was privately backed up.
Choose a saved slot (or the Auto entry in the host's state controls) rather than
assuming numbered slot 0 contains the previous automatic resume point. Its
exact saved scene was not independently inspected. No further reset/relaunch is
required just to continue playing with the verified current settings.
