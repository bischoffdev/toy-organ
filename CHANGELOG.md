# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Missing Sample Placeholder Graphic**: Dedicated visual placeholder rendered on any key lacking an audio sample (featuring an inner dashed drop-slot border, circular music emblem with question mark, and "NO SAMPLE" label).
- **Custom Soundset Placeholder Support**: Soundsets can supply an optional custom `placeholder.png` (or `missing.png` / `empty.png`) to override the default empty key graphic.
- **Muted Badges on Unassigned Keys**: Key badges for notes without audio samples adopt a subtle, muted visual style to visually distinguish populated vs. unpopulated keys at a glance.

### Removed
- **Synthetic Fallback Tone Generator**: Removed the additive oscillator so keys without audio samples produce silence instead of artificial synth tones.

## [0.1.0] - 2026-10-26

### Added
- **13-Key Mascot Keyboard Layout**: Piano keyboard geometry featuring 8 natural keys and 5 elevated accidental keys (covering C4 to C5).
- **Audio Synthesis Engine**: Built-in 8-voice polyphonic additive synthesizer (fundamental + 2nd and 3rd harmonics) serving as an immediate zero-configuration fallback.
- **Polyphonic Sampler Engine**: 8-voice sample playback supporting per-note WAV/AIFF/FLAC/OGG audio files (`01.wav`–`13.wav`) or automatic chromatic pitching from a single sample file.
- **Mascot Animation Engine**: Dual-state visual presentation (`idle` vs. `active`) supporting both static image files (`idle.png`, `active.png`) and sequential multi-frame animation directories (`idle/` and `active/`).
- **Dynamic Theming**: Soundset background coloring driven by an optional `color.txt` (hex or RGB format) in each soundset directory.
- **Factory Soundsets**:
  - **Huhn**: Animated chicken mascot with idle movement, singing active frame sequences, custom audio, and themed background.
  - **Bananabot**: Robot mascot with unique voice samples and custom styling.
- **Computer Keyboard Musical Typing**: Play notes directly from your computer keyboard with support for **QWERTY**, **QWERTZ**, and **AZERTY** layouts.
- **Master Volume Control**: Linear slider with numeric percentage readout in the header bar.
- **MIDI Control**: Support for MIDI note input (60–72), pitch bend (+/- 2 semitones), and modulation wheel vibrato (CC 1).
- **State Persistence**: XML-based state serialization saving and restoring selected soundset, master volume, keyboard layout, and custom window bounds across DAW sessions.
- **Resizable UI**: Scalable window dimensions maintaining a fixed 880x420 aspect ratio.
- **Multi-Format Plugin & App**: Standalone Application, VST3, AU (Audio Unit), and LV2 plugins.
- **Automated CI/CD**: GitHub Actions workflow compiling universal binaries for macOS (`arm64` & `x86_64`), Windows (x64), and Linux with automated release packaging and asset upload.
- **Cross-Platform Installers**: Double-clickable and command-line installers (`install.command`, `install.sh`, `install.bat`) that safely update binaries and factory soundsets without overwriting user creations.
- **Headless Screenshot Utility**: CMake target `render_screenshot` for generating high-resolution UI captures.

### Changed
- Re-architected top header toolbar into a balanced 3-zone layout with vector icons and anti-collision sizing.
- Key labels now display clean musical note names (`C`, `C#`, `D`, etc.) without computer keyboard key clutter.
- Soundset scanner (`scanForSoundsets()`) preserves the active soundset selection across rescans rather than resetting to the first item.
- Automatic soundset discovery on window focus (`focusGained`), eliminating the need to manually reload after adding soundsets to disk.

### Removed
- Removed manual reload button (`↻`) from the header bar in favor of automatic focus-based rescanning.
