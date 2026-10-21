# Laxersaz Organ 🎹

A customizable 12-key toy organ audio plugin (VST3, AU, LV2, Standalone) built with **JUCE 8** and **CMake**, designed to work cross-platform on **macOS**, **Windows**, and **Linux**.

---

## Concept

A playful, interactive 13-key sampler organ:
* **Toy Organ Keyboard Layout:** Full octave with 8 whole-tone keys and 5 elevated semitone keys (as wide as the white keys for a playful toy aesthetic). All keys share the soundset's custom background color with crisp `#ff7f00` borders and `#0d1926` ink typography.
* **Full Octave 13-Key Range:** Exclusively handles the 13 chromatic keys from C4 to C5 (MIDI notes 60 to 72).
* **Mascot Character ON Each Key:** Each key features the mascot character right on the key (like a classic toy duck organ). When a key is played, that key's character instantly springs into its active singing state.
* **Soundset System:** Each soundset lives in its own folder under `Soundsets/<Name>/` containing the key background color (`color.txt`), mascot images (`idle.png` & `active.png`), and audio samples (`01.wav` .. `13.wav`, or a single sample).
* **Synthesizer Fallback:** If a soundset has no sample files yet, the built-in organ additive synth seamlessly plays the notes.

---

## Soundset Folder Structure

Place your soundsets in the `Soundsets/` directory (you can open it directly in the plugin via the **"📁 Ordner..."** button):

```text
Soundsets/
└── Huhn/
    ├── color.txt      # Hintergrundfarbe aller Tasten (z.B. #FFEAA7)
    ├── idle.png       # Bild im Ruhezustand
    ├── active.png     # Bild beim Spielen / Gackern!
    ├── 01.wav         # Audio-Sample für Note C
    ├── 02.wav         # Audio-Sample für C# (Halbton)
    ├── ...
    └── 13.wav         # Audio-Sample für hohes C (optional)
```

> **Tipp:** Wenn du nur ein einzelnes Sample hast (z.B. `gaak.wav`), wird dieses automatisch über alle 12 Tasten chromatisch gepitcht!

---

## Automated CI/CD Pipeline

The repository includes a ready-to-run GitHub Actions workflow in [`.github/workflows/build.yml`](.github/workflows/build.yml).

Triggered manually via **Actions > Run workflow** or automatically when publishing a **Release**:
1. **macOS Runner:** Compiles universal binaries (`arm64` & `x86_64`) for:
   * **VST3** (`Laxersaz Organ.vst3`)
   * **Audio Unit / AU** (`Laxersaz Organ.component`)
   * **LV2** (`Laxersaz Organ.lv2`)
   * **Standalone App** (`Laxersaz Organ.app`)
2. **Windows Runner:** Compiles x64 binaries using Visual Studio 2022 for:
   * **VST3** (`Laxersaz Organ.vst3`)
   * **LV2** (`Laxersaz Organ.lv2`)
   * **Standalone Executable** (`Laxersaz Organ.exe`)
3. **Linux Runner (Ubuntu 22.04):** Compiles x64 binaries for:
   * **VST3** (`Laxersaz Organ.vst3`)
   * **LV2** (`Laxersaz Organ.lv2`)
   * **Standalone Executable** (`Laxersaz Organ`)
4. **Artifact Upload:** Packaged ZIP files are uploaded directly to the GitHub Action summary for instant download and testing in your DAW. Each archive contains `install.sh` (Mac/Linux) or `install.bat` (Windows).

---

## Installation & Updating

Each release ZIP includes a one-click installer script:

* **macOS & Linux (`./install.sh`):**
  Installs or updates plugins into user directories (`~/Library/Audio/Plug-Ins/...` on macOS, `~/.vst3` & `~/.lv2` on Linux) and copies factory soundsets to `~/Music/LaxersazOrgan/Soundsets/`.
* **Windows (`install.bat`):**
  Double-click to install or update VST3, LV2, and factory soundsets into system & user directories.
* **Local CMake Developers:**
  Run `cmake --build build --target install_user` to compile and install in one step.

### How Reinstalling & Updating Works
* **Plugin Binaries:** Overwrites the previous plugin version cleanly with the new binary.
* **Factory Soundsets:** Updates factory soundset folders (`Huhn`, `Bananabot`, etc.) in-place with new assets.
* **Preserving Custom Presets:** Any custom soundset folders you added are **never deleted or overwritten**.
* **macOS Audio Unit Cache:** Automatically resets `AudioComponentRegistrar` so Logic Pro and other DAWs recognize the update immediately without a system restart.
* **Uninstallation:** Run `./install.sh --uninstall` (or `install.bat --uninstall`) to remove the plugin files while leaving your soundsets intact.

---

## Pushing to GitHub

To publish this project to your GitHub account:

```bash
cd /Users/benjaminbischoff/Development/toy-organ

# Initialize Git repository
git init -b main
git add .
git commit -m "Add piano layout with elevated accidentals, Linux support, and soundset loader"
git push origin main
```

Once pushed, navigate to the **Actions** tab on your GitHub repository to watch the Mac, Windows, and Linux builds run!

---

## Building Locally (Optional)

### macOS
Requirements: Xcode Command Line Tools (`xcode-select --install`) and CMake (`brew install cmake`).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

### Windows
Requirements: Visual Studio 2022 Community (with "Desktop development with C++") and CMake.

```cmd
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### Linux (Ubuntu / Debian)
Requirements: Build essentials, CMake, and development libraries for ALSA, FreeType, and X11:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git \
    libasound2-dev libjack-jackd2-dev libfreetype6-dev libfontconfig1-dev \
    libx11-dev libxcomposite-dev libxcursor-dev libxinerama-dev libxrandr-dev libgl1-mesa-dev

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
```

The compiled plugins will be located under `build/ToyOrgan_artefacts/Release/`.

---

## Roadmap

- [x] Cross-platform CMake & GitHub Actions CI/CD setup (macOS, Windows, Linux)
- [x] 13-key visual layout and interactive mouse/MIDI triggering
- [x] Piano keyboard geometry with elevated semitones (Halbtöne weiter oben)
- [x] Audio synthesis engine & polyphonic sample player with auto-fallback
- [x] Sound Set Loader (supporting custom folders with samples & idle/active images)
- [x] Preset switching dropdown & quick folder reveal button
- [x] Full DAW project state saving & restoration
- [x] Master volume parameter (DAW automatable with smooth ramping)
- [x] Musical typing (computer keyboard playing with QWERTY, QWERTZ, and AZERTY layout support)
- [x] Resizable UI with aspect ratio preservation and size persistence
- [x] MIDI Pitch Bend & Modulation Wheel (wobbly toy organ vibrato)

