#!/bin/bash
# Remove old plugins
rm -rf ~/Library/Audio/Plug-Ins/Components/AR3S.component
rm -rf ~/Library/Audio/Plug-Ins/Components/AR3S\ Satellite.component

# Clear AU cache
rm -rf ~/Library/Caches/AudioUnitCache
rm -rf ~/Library/Preferences/com.apple.audio.InfoHelper.plist

# Install new plugins
cp -R build/SimpleGain_artefacts/Release/AU/AR3S.component ~/Library/Audio/Plug-Ins/Components/
cp -R "build/AR3SSatellite_artefacts/Release/AU/AR3S Satellite.component" ~/Library/Audio/Plug-Ins/Components/

echo "Old plugins removed, cache cleared, new plugins installed!"
ls ~/Library/Audio/Plug-Ins/Components/ | grep AR3S
