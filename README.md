# Neural Gain Stager (VST3 + AU)

JUCE-based gain staging plugin with auto gain, vocal rider, and optional Ollama-powered suggestions. Builds VST3 and AU formats for macOS and Windows.

## Requirements

- CMake 3.22+
- C++17 compiler
- Local JUCE source checkout
- Ollama running locally (optional, for AI suggestions)

## Configure

Set `JUCE_DIR` to your local JUCE directory when configuring the project.

### macOS (Xcode)

1. Configure:
   - `cmake -S . -B build -G Xcode -DJUCE_DIR=/path/to/JUCE`
2. Build:
   - `cmake --build build`

### Windows (Visual Studio)

1. Configure:
   - `cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_DIR=C:\path\to\JUCE`
2. Build:
   - `cmake --build build --config Release`

## Notes

- The plugin exposes auto gain, a target dB control, a vocal rider, and a noise suppressor.
- Ollama is used via HTTP at http://127.0.0.1:11434. Use "Refresh Models" to list installed models.
- Output formats: VST3 and AU.
- For Pro Tools compatibility later, you can add AAX support once you have the AAX SDK and licensing.
