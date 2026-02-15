#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "SatelliteProcessor.h"
#include "ThemeData.h"

class SatelliteEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit SatelliteEditor(SatelliteProcessor&);
    ~SatelliteEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateThemeFromMaster();
    void updateControlModeButton();
    void drawMeterBar(juce::Graphics& g, juce::Rectangle<float> bounds, 
                      float valueDb, const juce::String& label);
    void drawPhaseMeter(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGainKnob(juce::Graphics& g, juce::Rectangle<float> bounds, float sliderPos);
    
    SatelliteProcessor& processor;
    
    // Gain knob
    juce::Slider gainKnob;
    
    // Ceiling knob
    juce::Slider ceilingKnob;
    
    // Channel name
    juce::TextEditor channelNameEditor;
    
    // Source dropdown
    juce::ComboBox sourceBox;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> sourceAttach;
    
    // Meter values
    float smoothedRms = -60.0f;
    float smoothedPeak = -60.0f;
    float smoothedPhase = 0.0f;
    float smoothedGain = 1.0f;
    
    // Pre/Post meter toggle
    juce::TextButton meterModeButton { "POST" };
    bool showPreMeter = false;
    
    // Control mode toggle - local vs master control
    juce::TextButton controlModeButton { "LOCAL" };
    
    // Theme - synced from master
    ThemeColors theme;
    int lastThemeIndex = -1;
    int knobStyle = 0;  // Synced from master
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SatelliteEditor)
};
