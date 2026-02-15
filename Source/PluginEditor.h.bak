#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class SimpleGainAudioProcessorEditor : public juce::AudioProcessorEditor
                                      , private juce::Timer
                                      , private juce::Slider::Listener
{
public:
    explicit SimpleGainAudioProcessorEditor(SimpleGainAudioProcessor&);
    ~SimpleGainAudioProcessorEditor() override;


    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // Internal helpers for drag/resize
    void handleMouseDragAtPoint(juce::Point<int> pos);
    void startComponentDragAt(juce::Component* component, juce::Point<int> pos, juce::ModifierKeys mods = juce::ModifierKeys::noModifiers);

private:
    void timerCallback() override;
    void sliderValueChanged(juce::Slider* slider) override;
    void drawMeter(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawPhaseMeter(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawSatellitePanel(juce::Graphics& g, juce::Rectangle<float> bounds);
    void showMainView();
    void showSettingsView();
    void showChatView();
    void sendChatMessage();
    void applyTheme();
    void updateLabels();  // Update all UI labels (e.g., when language changes)

    static constexpr int baseWidth = 1200;
    static constexpr int baseHeight = 800;

    // Theme colors (customizable)
    struct Theme {
        juce::Colour bgDark       { 4, 6, 10 };
        juce::Colour bgPanel      { 12, 16, 22 };
        juce::Colour accent       { 255, 60, 80 };
        juce::Colour accentAlt    { 255, 120, 80 };
        juce::Colour textBright   { 240, 245, 255 };
        juce::Colour textDim      { 100, 110, 130 };
        juce::Colour meterGreen   { 40, 220, 90 };
        juce::Colour meterYellow  { 255, 200, 40 };
        juce::Colour meterRed     { 255, 45, 60 };
    };
    Theme theme;
    int currentThemeIndex = 0;
    int currentKnobStyle = 0;

    class AresLookAndFeel;

    // Base class for movable components with lock support
    class MovableComponent : public juce::Component
    {
    public:
        MovableComponent(SimpleGainAudioProcessorEditor& editor) : owner(editor)
        {
            setInterceptsMouseClicks(true, true);
        }

        bool isLocked() const { return locked; }
        void setLocked(bool v) { locked = v; repaint(); }
        void toggleLocked() { locked = !locked; repaint(); owner.layoutDirty = true; owner.saveLayoutToFile(); owner.layoutLoaded = true; }

        // Draw a small padlock overlay in the top-right corner
        void drawLockOverlay(juce::Graphics& g)
        {
            auto r = getLocalBounds();
            juce::Rectangle<int> lockRect(r.getRight() - 18, r.getY() + 6, 14, 14);
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.fillRect(lockRect);
            g.setColour(juce::Colours::white);
            if (locked)
            {
                // simple locked glyph
                g.fillRect(lockRect.withTrimmedTop(6).reduced(3));
                g.drawEllipse(lockRect.getX() + 3, lockRect.getY(), 8, 6, 1.0f);
            }
            else
            {
                g.drawRect(lockRect.reduced(3));
                g.drawEllipse(lockRect.getX() + 3, lockRect.getY(), 8, 6, 1.0f);
            }
        }

        bool lockHandleContains(juce::Point<int> pos) const
        {
            auto r = getLocalBounds();
            juce::Rectangle<int> lockRect(r.getRight() - 18, r.getY() + 6, 14, 14);
            return lockRect.contains(pos);
        }

    protected:
        SimpleGainAudioProcessorEditor& owner;

    private:
        bool locked = false;
    };

    // Custom meter components that can be moved around
    class VUMeterComponent : public MovableComponent
    {
    public:
        VUMeterComponent(SimpleGainAudioProcessorEditor& editor) : MovableComponent(editor) {}
        void paint(juce::Graphics& g) override {
            juce::Rectangle<int> r = getLocalBounds();
            g.reduceClipRegion(r);
            owner.drawMeter(g, r.toFloat());
            drawLockOverlay(g);
        }
        void mouseDown(const juce::MouseEvent& e) override { if (lockHandleContains(e.getPosition())) { toggleLocked(); return; } if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(this, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override {
            auto r = getLocalBounds();
            juce::Rectangle<int> resizeHandle(r.getRight() - 16, r.getBottom() - 16, 16, 16);
            if (resizeHandle.contains(e.getPosition())) setMouseCursor(juce::MouseCursor::StandardCursorType::DraggingHandCursor);
            else setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
        }
        void mouseExit(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); }
    };

    class PhaseMeterComponent : public MovableComponent
    {
    public:
        PhaseMeterComponent(SimpleGainAudioProcessorEditor& editor) : MovableComponent(editor) {}
        void paint(juce::Graphics& g) override {
            juce::Rectangle<int> r = getLocalBounds();
            g.reduceClipRegion(r);
            owner.drawPhaseMeter(g, r.toFloat());
            drawLockOverlay(g);
        }
        void mouseDown(const juce::MouseEvent& e) override { if (lockHandleContains(e.getPosition())) { toggleLocked(); return; } if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(this, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override {
            auto r = getLocalBounds();
            juce::Rectangle<int> resizeHandle(r.getRight() - 16, r.getBottom() - 16, 16, 16);
            if (resizeHandle.contains(e.getPosition())) setMouseCursor(juce::MouseCursor::StandardCursorType::DraggingHandCursor);
            else setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
        }
        void mouseExit(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); }
    };

    class GainReductionMeterComponent : public MovableComponent
    {
    public:
        GainReductionMeterComponent(SimpleGainAudioProcessorEditor& editor) : MovableComponent(editor) {}
        void paint(juce::Graphics& g) override {
            juce::Rectangle<int> r = getLocalBounds();
            g.reduceClipRegion(r);
            owner.drawGainReductionMeter(g, r.toFloat());
            drawLockOverlay(g);
        }
        void mouseDown(const juce::MouseEvent& e) override { if (lockHandleContains(e.getPosition())) { toggleLocked(); return; } if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(this, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override {
            auto r = getLocalBounds();
            juce::Rectangle<int> resizeHandle(r.getRight() - 16, r.getBottom() - 16, 16, 16);
            if (resizeHandle.contains(e.getPosition())) setMouseCursor(juce::MouseCursor::StandardCursorType::DraggingHandCursor);
            else setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
        }
        void mouseExit(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); }
    };

    class IOComparisonMeterComponent : public MovableComponent
    {
    public:
        IOComparisonMeterComponent(SimpleGainAudioProcessorEditor& editor) : MovableComponent(editor) {}
        void paint(juce::Graphics& g) override {
            juce::Rectangle<int> r = getLocalBounds();
            g.reduceClipRegion(r);
            owner.drawIOComparisonMeter(g, r.toFloat());
            drawLockOverlay(g);
        }
        void mouseDown(const juce::MouseEvent& e) override { if (lockHandleContains(e.getPosition())) { toggleLocked(); return; } if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(this, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override {
            auto r = getLocalBounds();
            juce::Rectangle<int> resizeHandle(r.getRight() - 16, r.getBottom() - 16, 16, 16);
            if (resizeHandle.contains(e.getPosition())) setMouseCursor(juce::MouseCursor::StandardCursorType::DraggingHandCursor);
            else setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
        }
        void mouseExit(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); }
    };

    class FrequencyBalanceComponent : public MovableComponent
    {
    public:
        FrequencyBalanceComponent(SimpleGainAudioProcessorEditor& editor) : MovableComponent(editor) {}
        void paint(juce::Graphics& g) override {
            juce::Rectangle<int> r = getLocalBounds();
            g.reduceClipRegion(r);
            owner.drawFrequencyBalance(g, r.toFloat());
            drawLockOverlay(g);
        }
        void mouseDown(const juce::MouseEvent& e) override { if (lockHandleContains(e.getPosition())) { toggleLocked(); return; } if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(this, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override {
            auto r = getLocalBounds();
            juce::Rectangle<int> resizeHandle(r.getRight() - 16, r.getBottom() - 16, 16, 16);
            if (resizeHandle.contains(e.getPosition())) setMouseCursor(juce::MouseCursor::StandardCursorType::DraggingHandCursor);
            else setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
        }
        void mouseExit(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); }
    };

    class AdvancedMetersComponent : public MovableComponent
    {
    public:
        AdvancedMetersComponent(SimpleGainAudioProcessorEditor& editor) : MovableComponent(editor) {}
        void paint(juce::Graphics& g) override {
            juce::Rectangle<int> r = getLocalBounds();
            g.reduceClipRegion(r);
            owner.drawAdvancedMeters(g, r.toFloat());
            drawLockOverlay(g);
        }
        void mouseDown(const juce::MouseEvent& e) override { if (lockHandleContains(e.getPosition())) { toggleLocked(); return; } if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(this, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override {
            auto r = getLocalBounds();
            juce::Rectangle<int> resizeHandle(r.getRight() - 16, r.getBottom() - 16, 16, 16);
            if (resizeHandle.contains(e.getPosition())) setMouseCursor(juce::MouseCursor::StandardCursorType::DraggingHandCursor);
            else setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
        }
        void mouseExit(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); }
    };

    class QuickIssuesComponent : public MovableComponent
    {
    public:
        QuickIssuesComponent(SimpleGainAudioProcessorEditor& editor) : MovableComponent(editor) {}
        void paint(juce::Graphics& g) override {
            juce::Rectangle<int> r = getLocalBounds();
            g.reduceClipRegion(r);
            owner.drawQuickIssues(g, r.toFloat());
            drawLockOverlay(g);
        }
        void mouseDown(const juce::MouseEvent& e) override { if (lockHandleContains(e.getPosition())) { toggleLocked(); return; } if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(this, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override {
            auto r = getLocalBounds();
            juce::Rectangle<int> resizeHandle(r.getRight() - 16, r.getBottom() - 16, 16, 16);
            if (resizeHandle.contains(e.getPosition())) setMouseCursor(juce::MouseCursor::StandardCursorType::DraggingHandCursor);
            else setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
        }
        void mouseExit(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); }
    };

    class SatellitePanelComponent : public MovableComponent
    {
    public:
        SatellitePanelComponent(SimpleGainAudioProcessorEditor& editor) : MovableComponent(editor) {}
        void paint(juce::Graphics& g) override { 
            if (!owner.standaloneMode)
                owner.drawSatellitePanel(g, getLocalBounds().toFloat()); 
            drawLockOverlay(g);
        }
        void mouseDown(const juce::MouseEvent& e) override { if (lockHandleContains(e.getPosition())) { toggleLocked(); return; } if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(this, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { if (isLocked()) return; auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override {
            auto r = getLocalBounds();
            juce::Rectangle<int> resizeHandle(r.getRight() - 12, r.getBottom() - 12, 12, 12);
            if (resizeHandle.contains(e.getPosition())) setMouseCursor(juce::MouseCursor::StandardCursorType::DraggingHandCursor);
            else setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
        }
        void mouseExit(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); }
    };


    // Mouse forwarder attached to interactive child components so they can be moved/resized
    struct ChildMouseForwarder : public juce::MouseListener
    {
        SimpleGainAudioProcessorEditor& owner;
        ChildMouseForwarder(SimpleGainAudioProcessorEditor& o) : owner(o) {}
        void mouseDown(const juce::MouseEvent& e) override { auto ev = e.getEventRelativeTo(&owner); owner.startComponentDrag(e.eventComponent, ev); }
        void mouseDrag(const juce::MouseEvent& e) override { auto ev = e.getEventRelativeTo(&owner); owner.mouseDrag(ev); }
        void mouseUp(const juce::MouseEvent& e) override { auto ev = e.getEventRelativeTo(&owner); owner.mouseUp(ev); }
        void mouseMove(const juce::MouseEvent& e) override { /* nothing */ }
    };

    std::unique_ptr<ChildMouseForwarder> childForwarder;

private:
    SimpleGainAudioProcessor& processor;
    juce::Rectangle<float> meterBounds, phaseMeterBounds, satellitePanelBounds;
    juce::Rectangle<float> crestMeterBounds, grMeterBounds, ioMeterBounds;

    juce::ComponentBoundsConstrainer resizeConstrainer;
    std::unique_ptr<juce::ResizableCornerComponent> resizeCorner;

    // Drag and drop state
    juce::Component* draggedComponent = nullptr;
    juce::Point<int> dragStartPos;
    juce::Rectangle<int> componentStartBounds;
    
    // Resize state
    juce::Component* resizedComponent = nullptr;
    juce::Rectangle<int> resizeStartBounds;
    bool isResizing = false;

    // Candidate state for delayed drag/resize (allow click-and-drag without modifiers)
    juce::Component* candidateComponent = nullptr;              // potential component that may become dragged/resized
    juce::Point<int> candidateStartPos;                         // initial mouse pos when candidate started
    juce::Rectangle<int> candidateStartBounds;                  // initial bounds of candidate
    bool candidateIsResize = false;                             // whether candidate intent is resize
    const int dragThreshold = 6;                                // pixels of movement before starting a drag/resize



    // Movable meter components
    VUMeterComponent vuMeter;
    PhaseMeterComponent phaseMeter;
    GainReductionMeterComponent grMeter;
    IOComparisonMeterComponent ioMeter;
    FrequencyBalanceComponent freqBalance;
    AdvancedMetersComponent advancedMeters;
    QuickIssuesComponent quickIssues;
    SatellitePanelComponent satellitePanel;

    // Navigation
    juce::TextButton mainTabBtn, settingsTabBtn, chatTabBtn;
    // Edit & Auto-fit toggles
    juce::ToggleButton editModeToggle {"Edit"};
    juce::ToggleButton autoFitToggle {"Auto-fit"};
    int currentTab = 0;

    // Edit / layout modes
    bool editMode = false;       // when true, components can be moved/resized freely
    bool autoFitMode = true;     // when true, plugin uses automatic layout (unless editMode is enabled)

    // Animator used to smooth final nudges after drag is complete
    juce::ComponentAnimator animator;

    // Auto-layout helper and overlap resolver
    void applyAutoLayout();
    void resolveAllOverlaps(bool animate = false);

    // Main view components
    juce::Slider gainSlider;
    juce::Label gainLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment;

    juce::Slider targetDbSlider;
    juce::Label targetDbLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> targetDbAttachment;

    juce::Slider ceilingSlider;
    juce::Label ceilingLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttachment;

    juce::ToggleButton autoToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoToggleAttachment;

    juce::Slider riderAmountSlider;
    juce::Label riderAmountLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> riderAmountAttachment;

    juce::ToggleButton riderToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> riderToggleAttachment;
    
    // Standalone mode - allows source selection on each instance
    juce::ToggleButton standaloneModeToggle;
    bool standaloneMode = false;

    // Context
    juce::ComboBox genreBox, sourceBox, situationBox;
    juce::Label genreLabel, sourceLabel, situationLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> genreAttachment, sourceAttachment, situationAttachment;

    // AI controls
    juce::TextButton autoSetButton, aiSuggestButton, applyAiButton;
    juce::Label analysisLabel, aiStatusLabel;
    juce::TextEditor aiNotes;

    // Settings view components
    juce::ComboBox aiProviderBox, modelBox, themeBox, knobStyleBox;
    juce::Label aiProviderLabel, modelLabel, apiKeyLabel, themeLabel, knobStyleLabel;
    juce::Label accentColorLabel, bgColorLabel;
    juce::TextEditor apiKeyEditor;
    juce::TextButton refreshModelsButton, saveSettingsBtn, saveLayoutBtn, loadLayoutBtn;
    juce::Slider accentHueSlider, bgBrightnessSlider;
    int lastModelsVersion = -1;
    
    // Equipment settings for AI context
    juce::ComboBox micBox, preampBox, interfaceBox;
    juce::Label micLabel, preampLabel, interfaceLabel;
    juce::TextEditor customMicEditor, customPreampEditor, customInterfaceEditor;
    bool showCustomMicEditor = false, showCustomPreampEditor = false, showCustomInterfaceEditor = false;
    
    // Language settings
    juce::ComboBox languageBox;
    juce::Label languageLabel;

    // Chat view components
    juce::TextEditor chatHistory;
    juce::TextEditor chatInput;
    juce::TextButton sendChatBtn;
    juce::Label chatLabel;
    int lastChatVersion = -1;

    // Meter state
    float smoothedRmsDb = -60.0f;
    float smoothedPeakDb = -60.0f;
    float aiSuggestedDb = -18.0f;
    float presetSuggestedDb = -18.0f;
    float peakHoldDb = -60.0f;
    int peakHoldCounter = 0;
    float phaseCorrelation = 1.0f;  // -1 to +1 phase correlation
    float crestFactorDb = 0.0f;  // Peak-to-RMS ratio
    float gainReductionDb = 0.0f;  // How much auto/rider is reducing
    float inputRmsDb = -60.0f;  // Input level
    float outputRmsDb = -60.0f;  // Output level
    
    // Advanced metering state
    float stereoWidth = 0.0f;
    float shortTermLufs = -60.0f;
    float integratedLufs = -60.0f;
    float truePeakDb = -60.0f;
    int clipCount = 0;
    float headroom = 60.0f;
    float lowEnergy = 0.33f;
    float midEnergy = 0.33f;
    float highEnergy = 0.33f;
    bool monoCompatible = true;
    float lufsTarget = -14.0f;   // Current LUFS target
    bool lufsEnabled = false;    // Whether LUFS mode is active
    
    // Platform target presets
    juce::ComboBox platformTargetBox;
    juce::Label platformTargetLabel;
    
    // Advanced analysis bounds
    juce::Rectangle<float> advancedMeterBounds;
    juce::Rectangle<float> freqBalanceBounds;
    juce::Rectangle<float> quickIssuesBounds;
    void drawAdvancedMeters(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawFrequencyBalance(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawQuickIssues(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    // Satellite panel mouse handling for movable component
    void handleSatellitePanelMouse(const juce::MouseEvent& e, juce::Rectangle<float> panelBounds);
    void startComponentDrag(juce::Component* component, const juce::MouseEvent& e);

    // Pre/Post meter toggle
    juce::TextButton meterModeButton { "POST" };
    bool showPreMeter = false;  // false = post (after processing), true = pre (before)
    
    // VU meter IN/OUT toggle
    juce::TextButton vuModeButton { "OUT" };
    bool vuShowInput = false;  // false = output, true = input
    
    // Satellite control components
    int selectedSatellite = -1;  // -1 = none selected
    void handleSatelliteClick(int index);
    void showSatelliteContextMenu(int satIndex, juce::Point<int> screenPos);
    void setSatelliteGain(int satIndex, float gainDb);
    void setSatelliteTarget(int satIndex, float targetDb);
    void setSatelliteCeiling(int satIndex, float ceilingDb);
    void setSatelliteAuto(int satIndex, bool enabled);
    void setSatelliteRider(int satIndex, float amount);
    
    void drawCrestMeter(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGainReductionMeter(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawIOComparisonMeter(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Layout persistence
    void saveLayoutToFile();
    void loadLayoutFromFile();
    juce::File getLayoutFile() const;

    bool layoutLoaded = false; // whether a saved layout has been loaded
    bool layoutDirty = false;  // whether layout changed and should be saved on mouseUp

    // Last window bounds used for proportional resizing when user resizes plugin window
    juce::Rectangle<int> lastWindowBounds;
    std::unique_ptr<AresLookAndFeel> aresLaf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleGainAudioProcessorEditor)
};
