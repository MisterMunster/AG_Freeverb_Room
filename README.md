# Antigrav Reverb

**Antigrav Reverb** is a true stereo algorithmic reverb plugin built with the JUCE framework and C++20. It utilizes a Feedback Delay Network (FDN) architecture to create lush, modulate-able spaces ranging from small rooms to massive ethereal halls.

## Project Structure

The project is structured as a standard CMake-based JUCE audio plugin:

- **Source/**: Core C++ source code.
  - **DSP/**: Digital Signal Processing modules (DelayLine, AllpassFilter, LFO, EarlyReflections, LateReverb).
  - **UI/**: Plugin editor and LookAndFeel customization.
  - **PluginProcessor**: The main audio processing entry point.
  - **PluginEditor**: The GUI entry point.
- **Tests/**: Unit tests for DSP components using `juce::UnitTest`.
- **CMakeLists.txt**: Build configuration, including automatic fetching of the JUCE library.

## Dependencies

- **JUCE Framework**: Used for VST3/Standalone wrappers, audio device management, and GUI.
  - *Version*: Fetched automatically from `master` branch via CMake `FetchContent`.
- **C++ Standard**: C++20.

## Build Instructions

This project uses **CMake**. You do not need to manually download JUCE; it will be downloaded during the configure step.

### Prerequisites
- CMake 3.22+
- A C++20 compatible compiler (MSVC, Clang, or GCC)
- Visual Studio 2022 (on Windows) or Ninja/Make.

### Building
1. **Configure**:
   ```bash
   cmake -B build
   ```
2. **Build**:
   ```bash
   cmake --build build --config Debug
   ```
   *For Release builds, change `Debug` to `Release`.*

### Artifacts
After a successful build, artifacts are located in:
- **VST3**: `build/AntigravReverb_artefacts/Debug/VST3/`
- **Standalone**: `build/AntigravReverb_artefacts/Debug/Standalone/`
- **Unit Tests**: `build/AntigravReverb_Tests_artefacts/Debug/`

## Development Notes

### Architecture
The reverb engine consists of two main stages:
1. **Early Reflections**: Multi-tap delay line simulation for spatial cues.
2. **Late Reverb**: An 8-channel Feedback Delay Network (FDN) with:
   - Prime number delay lengths to avoid resonance buildup.
   - Householder mixing matrix for maximum echo density.
   - Modulated delay lines (LFO) to add chorus/shimmer and prevent metallic ringing.
   - High-cut and Low-cut filters in the feedback loop for damping control.

### Running Tests
To ensure DSP correctness (filters, delay lines, math):
run the test runner executable:
```bash
./build/AntigravReverb_Tests_artefacts/Debug/AntigravReverb_Tests.exe
```

### Git Workflow
- **Branching**: Feature branches recommended (e.g., `feature/new-filter`).
- **Build Artifacts**: The `build/` directory is ignored.
