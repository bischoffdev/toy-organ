# Toy Organ 🎹

A customizable 12-key toy organ audio plugin (VST3, AU, Standalone) built with **JUCE 8** and **CMake**, designed to work cross-platform on **macOS** and **Windows**.

---

## Concept

A playful, interactive 12-key sampler organ:
* **12 Keys** (covering one full chromatic octave, C4 to B4).
* **Two Visual States per Key:**
  * **Idle:** Displayed when the key is waiting.
  * **Active:** Displayed when the key is pressed (either via mouse click on the UI or via MIDI in your DAW).
* **Sound Set Customization:** Ready to support loading custom sample folders (e.g. *Chicken Organ* ("gaak!"), *Bimmbammbömm Organ*, or classic retro organs).

---

## Automated CI/CD Pipeline

The repository includes a ready-to-run GitHub Actions workflow in [`.github/workflows/build.yml`](.github/workflows/build.yml).

Every time code is pushed or a pull request is made:
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
git commit -m "Initial commit: 12-key Toy Organ plugin with cross-platform CI/CD"

# Create private repository on GitHub under bischoffdev using GitHub CLI
gh repo create bischoffdev/toy-organ --private --source=. --remote=origin --push
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
- [x] Audio synthesis engine
- [ ] Sound Set Loader (loading custom folders containing 12 `.wav`/`.ogg` audio files)
- [ ] PNG image loading for Idle and Active character states above each key
- [ ] Preset switching dropdown
