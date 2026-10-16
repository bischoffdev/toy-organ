# Toy Organ 🎹

A customizable 12-key toy organ audio plugin (VST3, AU, Standalone) built with **JUCE 8** and **CMake**, designed to work cross-platform on **macOS** and **Windows**.

---

## Concept

A playful, interactive 12-key sampler organ:
* **Classic Keyboard Layout:** 7 white keys (Ganztöne) with 5 elevated black keys (Halbtöne weiter oben, wie beim echten Klavier).
* **Two Visual States per Soundset:**
  * **Idle:** Displayed when the organ is waiting.
  * **Active:** Displayed when any key is pressed (via mouse click on the UI or via MIDI in your DAW).
* **Soundset System:** Each soundset lives in its own folder under `Soundsets/<Name>/` containing both the two mascot images (`idle.png` & `active.png`) and audio samples (`01_C.wav` .. `12_B.wav`, or a single sample).
* **Synthesizer Fallback:** If a soundset has no sample files yet, the built-in organ additive synth seamlessly plays the notes.

---

## Soundset Folder Structure

Place your soundsets in the `Soundsets/` directory (you can open it directly in the plugin via the **"📁 Ordner..."** button):

```text
Soundsets/
└── Huhn/
    ├── idle.png       # Bild im Ruhezustand
    ├── active.png     # Bild beim Spielen / Gackern!
    ├── 01_C.wav       # Audio-Sample für Note C
    ├── 02_CSharp.wav  # Audio-Sample für C# (Halbton)
    ├── ...
    └── 12_B.wav       # Audio-Sample für B (H)
```

> **Tipp:** Wenn du nur ein einzelnes Sample hast (z.B. `gaak.wav`), wird dieses automatisch über alle 12 Tasten chromatisch gepitcht!

---

## Automated CI/CD Pipeline

The repository includes a ready-to-run GitHub Actions workflow in [`.github/workflows/build.yml`](.github/workflows/build.yml).

Triggered manually via **Actions > Run workflow** or automatically when publishing a **Release**:
1. **macOS Runner:** Compiles universal binaries (`arm64` & `x86_64`) for:
   * **VST3** (`Toy Organ.vst3`)
   * **Audio Unit / AU** (`Toy Organ.component`)
   * **Standalone App** (`Toy Organ.app`)
2. **Windows Runner:** Compiles x64 binaries using Visual Studio 2022 for:
   * **VST3** (`Toy Organ.vst3`)
   * **Standalone Executable** (`Toy Organ.exe`)
3. **Artifact Upload:** Packaged ZIP files are uploaded directly to the GitHub Action summary for instant download and testing in your DAW.

---

## Pushing to GitHub

To publish this project to your GitHub account:

```bash
cd toy-organ

# Initialize Git repository
git init -b main
git add .
git commit -m "Add piano layout with elevated accidentals and soundset loader"
git push origin main
```

Once pushed, navigate to the **Actions** tab on your GitHub repository to watch the Mac and Windows builds run!

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

The compiled plugins will be located under `build/ToyOrgan_artefacts/Release/`.

---

## Roadmap

- [x] Cross-platform CMake & GitHub Actions CI/CD setup
- [x] 12-key visual layout and interactive mouse/MIDI triggering
- [x] Piano keyboard geometry with elevated semitones (Halbtöne weiter oben)
- [x] Audio synthesis engine & polyphonic sample player with auto-fallback
- [x] Sound Set Loader (supporting custom folders with samples & idle/active images)
- [x] Preset switching dropdown & quick folder reveal button

