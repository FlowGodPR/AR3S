# AR3S by ARES - AI-Powered Gain Staging Plugin

## Project Overview
AR3S is a professional audio plugin for automatic gain staging, vocal riding, and metering, powered by AI. Built with JUCE and CMake, it features a modern, responsive UI, advanced metering, and multi-provider AI integration for intelligent audio processing and mentoring.

## Features
- Modern tabbed UI with 12 themes
- Smart Auto-Set gain based on source/genre
- Vocal Rider, Noise Gate, Phase Correlation, RMS/Peak/Crest/FFT/ VU metering
- AI chat and suggestion modes (Ollama, OpenAI, Anthropic, OpenRouter, MiniMax)
- Equipment/genre selection, custom gear input
- Full localization (10 languages)

## Build Instructions (macOS)
1. Install JUCE (see below) and CMake.
2. Open Terminal and navigate to the project root:
   ```bash
   cd "/Users/untitled folder/plugins/build"
   cmake --build . --target SimpleGain_AU --config Release
   ```
3. The AU plugin will be built at:
   `build/SimpleGain_artefacts/Release/AU/AR3S.component`

## Install Instructions
1. Copy the built `.component` files to:
   `~/Library/Audio/Plug-Ins/Components/`
2. Clear AU plugin caches:
   ```bash
   rm -rf ~/Library/Caches/AudioUnitCache* ~/Library/Caches/com.apple.audiounits*
   ```
3. Restart your DAW/host.

## Usage
- Load "AR3S" as an Audio Unit plugin in your DAW.
- Use the Main, Settings, and Chat tabs for all features.
- Configure AI provider and API keys in Settings.
- Select source type, genre, and equipment for best results.

## Goals
- Professional, modern, and responsive UI
- Effortless gain staging and vocal riding
- Accurate metering and analysis
- AI-powered mentoring and suggestions
- Easy extensibility and localization

## JUCE Path
- JUCE SDK required: `/Users/flowgodpr/Downloads/JUCE`

## Settings Location
- `~/Library/Application Support/ARES/settings.xml`

## Support
- For issues, contact the original developer or refer to the code comments.

---
