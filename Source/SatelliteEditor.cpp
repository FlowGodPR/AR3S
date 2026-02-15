#include "SatelliteEditor.h"
#include "SharedMemory.h"

SatelliteEditor::SatelliteEditor(SatelliteProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // Initialize theme and knob style from master immediately
    int masterTheme = processor.getMasterThemeIndex();
    theme = getTheme(static_cast<ThemeType>(masterTheme >= 0 ? masterTheme : 1));  // Default to ModernDark if invalid
    lastThemeIndex = masterTheme;
    
    int masterKnobStyle = processor.getMasterKnobStyle();
    knobStyle = masterKnobStyle >= 0 ? masterKnobStyle : 0;
    
    // Gain knob - custom drawn, transparent slider for interaction
    gainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainKnob.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    gainKnob.setRange(-24.0, 12.0, 0.1);
    gainKnob.setDoubleClickReturnValue(true, 0.0);
    gainKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    gainKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    // Auto-enable local override when user drags the knob
    gainKnob.onDragStart = [this] {
        processor.setLocalOverride(true);
        updateControlModeButton();
    };
    addAndMakeVisible(gainKnob);
    
    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getValueTreeState(), "gain", gainKnob);
    
    // Ceiling knob - for peak limiting
    ceilingKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ceilingKnob.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    ceilingKnob.setRange(-24.0f, 0.0f, 0.1f);
    ceilingKnob.setDoubleClickReturnValue(true, 0.0f);
    ceilingKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    ceilingKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    // Auto-enable local override when user drags the knob
    ceilingKnob.onDragStart = [this] {
        processor.setLocalOverride(true);
        updateControlModeButton();
    };
    addAndMakeVisible(ceilingKnob);
    
    ceilingAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getValueTreeState(), "ceiling", ceilingKnob);
    
    // Channel name - larger, more visible
    channelNameEditor.setText(processor.getChannelName(), juce::dontSendNotification);
    channelNameEditor.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    channelNameEditor.setJustification(juce::Justification::centred);
    channelNameEditor.onTextChange = [this] {
        processor.setChannelName(channelNameEditor.getText());
    };
    addAndMakeVisible(channelNameEditor);
    
    // Source dropdown - bigger
    sourceBox.addItemList({ "Lead Vox", "BG Vox", "Kick", "Snare", "HiHat", 
                           "Drums", "Bass", "E.Gtr", "A.Gtr", "Keys", 
                           "Synth", "Strings", "Other" }, 1);
    addAndMakeVisible(sourceBox);
    sourceAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.getValueTreeState(), "source", sourceBox);
    
    // Pre/Post meter toggle
    meterModeButton.setButtonText("POST");
    meterModeButton.onClick = [this] {
        showPreMeter = !showPreMeter;
        meterModeButton.setButtonText(showPreMeter ? "PRE" : "POST");
        repaint();
    };
    addAndMakeVisible(meterModeButton);
    
    // Control mode toggle - LOCAL allows free adjustment, SYNC follows master/AI
    controlModeButton.setButtonText(processor.isLocalOverride() ? "LOCAL" : "SYNC");
    controlModeButton.onClick = [this] {
        bool newOverride = !processor.isLocalOverride();
        processor.setLocalOverride(newOverride);
        updateControlModeButton();
    };
    addAndMakeVisible(controlModeButton);
    
    // MUCH LARGER SIZE - easier to see and use, now with ceiling knob
    setSize(600, 140);
    setResizable(false, false);
    startTimerHz(30);
}

SatelliteEditor::~SatelliteEditor()
{
    stopTimer();
}

void SatelliteEditor::updateThemeFromMaster()
{
    int masterTheme = processor.getMasterThemeIndex();
    int masterKnobStyle = processor.getMasterKnobStyle();

    // Always update knob style from master
    if (knobStyle != masterKnobStyle)
    {
        DBG("SatelliteEditor: Knob style updated from master: " << masterKnobStyle);
        knobStyle = masterKnobStyle;
        repaint();
    }

    // Always update theme from master
    if (lastThemeIndex != masterTheme && masterTheme >= 0)
    {
        DBG("SatelliteEditor: Theme updated from master: " << masterTheme);
        lastThemeIndex = masterTheme;
        theme = getTheme(static_cast<ThemeType>(masterTheme));

        // Update component colors
        channelNameEditor.setColour(juce::TextEditor::backgroundColourId, theme.bgPanel);
        channelNameEditor.setColour(juce::TextEditor::textColourId, theme.textBright);
        channelNameEditor.setColour(juce::TextEditor::outlineColourId, theme.accent.withAlpha(0.4f));

        sourceBox.setColour(juce::ComboBox::backgroundColourId, theme.bgPanel);
        sourceBox.setColour(juce::ComboBox::textColourId, theme.textBright);
        sourceBox.setColour(juce::ComboBox::outlineColourId, theme.accent.withAlpha(0.4f));
        sourceBox.setColour(juce::ComboBox::arrowColourId, theme.accent);

        meterModeButton.setColour(juce::TextButton::buttonColourId, theme.bgPanel);
        meterModeButton.setColour(juce::TextButton::textColourOffId, theme.textDim);

        updateControlModeButton();
        repaint();  // Force immediate repaint when theme changes
    }
    // Even if the theme index hasn't changed, always ensure the theme is set (robustness)
    else if (masterTheme >= 0)
    {
        theme = getTheme(static_cast<ThemeType>(masterTheme));
    }
}

void SatelliteEditor::updateControlModeButton()
{
    bool isLocal = processor.isLocalOverride();
    controlModeButton.setButtonText(isLocal ? "LOCAL" : "SYNC");
    
    if (isLocal)
    {
        // Local mode - accent color to show user has control
        controlModeButton.setColour(juce::TextButton::buttonColourId, theme.accent.withAlpha(0.3f));
        controlModeButton.setColour(juce::TextButton::textColourOffId, theme.accent);
    }
    else
    {
        // Sync mode - dim to show following master
        controlModeButton.setColour(juce::TextButton::buttonColourId, theme.bgPanel);
        controlModeButton.setColour(juce::TextButton::textColourOffId, theme.textDim);
    }
    repaint();
}

void SatelliteEditor::drawGainKnob(juce::Graphics& g, juce::Rectangle<float> bounds, float sliderPos)
{
    const float rotaryStartAngle = juce::MathConstants<float>::pi * 1.25f;
    const float rotaryEndAngle = juce::MathConstants<float>::pi * 2.75f;
    
    auto radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f - 8.0f;
    auto centre = bounds.getCentre();
    auto knobRadius = radius * 0.70f;  // Slightly smaller to make room for LED ring
    auto valueAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // === LED RING ARC (common to ALL styles) ===
    int numLeds = 21;
    float totalAngle = rotaryEndAngle - rotaryStartAngle;
    float ledAngle = totalAngle / numLeds;
    float ledR = radius * 0.92f;
    float dotR = juce::jmax(1.0f, radius * 0.06f);  // LED dot size scales with knob radius
    
    for (int i = 0; i < numLeds; ++i)
    {
        float angle = rotaryStartAngle + i * ledAngle + ledAngle * 0.5f;
        float lx = centre.x + std::sin(angle) * ledR;
        float ly = centre.y - std::cos(angle) * ledR;
        
        bool lit = (rotaryStartAngle + i * ledAngle) <= valueAngle;
        
        // Color coding: green -> yellow-green -> yellow -> red
        juce::Colour ledColor;
        float pos = (float)i / numLeds;
        if (pos < 0.5f)
            ledColor = juce::Colour::fromRGB(0, 200, 80);      // Green
        else if (pos < 0.7f)
            ledColor = juce::Colour::fromRGB(180, 220, 50);    // Yellow-green
        else if (pos < 0.85f)
            ledColor = juce::Colour::fromRGB(255, 200, 0);     // Yellow
        else
            ledColor = juce::Colour::fromRGB(255, 60, 50);     // Red
        
        if (lit)
        {
            // Lit LED: radial gradient + outer glow
            juce::Colour top = ledColor.brighter(0.3f);
            juce::Colour bot = ledColor.darker(0.45f);
            juce::ColourGradient grad(top, lx, ly - dotR * 0.6f, bot, lx, ly + dotR * 0.6f, false);
            g.setGradientFill(grad);
            g.fillEllipse(lx - dotR, ly - dotR, dotR * 2, dotR * 2);

            // soft outer glow
            g.setColour(ledColor.withAlpha(0.22f));
            g.fillEllipse(lx - dotR - 2.5f, ly - dotR - 2.5f, (dotR + 2.5f) * 2, (dotR + 2.5f) * 2);
        }
        else
        {
            // Unlit LED - darker surface with faint top highlight
            g.setColour(theme.bgPanel.darker(0.42f));
            g.fillEllipse(lx - dotR, ly - dotR, dotR * 2, dotR * 2);
            g.setColour(juce::Colours::white.withAlpha(0.03f));
            g.fillEllipse(lx - dotR, ly - dotR, dotR * 2, dotR * 2 * 0.45f);
        }
    }

    // === KNOB CENTER (style-specific) ===
    if (knobStyle == 0) // Modern Arc
    {
        juce::ColourGradient knobGrad(juce::Colour::fromRGB(55, 60, 70), centre.x, centre.y - knobRadius,
                                      juce::Colour::fromRGB(30, 35, 42), centre.x, centre.y + knobRadius, false);
        g.setGradientFill(knobGrad);
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2, knobRadius * 2);
        g.setColour(juce::Colour::fromRGB(80, 85, 95));
        g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2, knobRadius * 2, 1.5f);

        auto pointerLen = knobRadius * 0.6f;
        g.setColour(theme.accent);
        g.drawLine(centre.x, centre.y,
                   centre.x + std::sin(valueAngle) * pointerLen,
                   centre.y - std::cos(valueAngle) * pointerLen, 2.0f);
    }
    else if (knobStyle == 1) // Classic Pointer
    {
        juce::ColourGradient knobGrad(juce::Colour::fromRGB(90, 85, 80), centre.x, centre.y - knobRadius,
                                      juce::Colour::fromRGB(50, 48, 45), centre.x, centre.y + knobRadius, false);
        g.setGradientFill(knobGrad);
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2, knobRadius * 2);
        g.setColour(juce::Colour::fromRGB(40, 38, 35));
        g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2, knobRadius * 2, 2.0f);
        
        auto pointerLen = knobRadius * 0.7f;
        g.setColour(theme.textBright);
        g.drawLine(centre.x, centre.y,
                   centre.x + std::sin(valueAngle) * pointerLen,
                   centre.y - std::cos(valueAngle) * pointerLen, 3.0f);
        
        g.setColour(juce::Colour::fromRGB(60, 58, 55));
        g.fillEllipse(centre.x - 5, centre.y - 5, 10, 10);
    }
    else if (knobStyle == 2) // Minimal Dot
    {
        g.setColour(theme.bgPanel);
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2, knobRadius * 2);
        
        auto dotRadius = 4.0f;
        auto dotX = centre.x + std::sin(valueAngle) * (knobRadius * 0.6f);
        auto dotY = centre.y - std::cos(valueAngle) * (knobRadius * 0.6f);
        g.setColour(theme.accent);
        g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2, dotRadius * 2);
    }
    else if (knobStyle == 3) // Retro Dial
    {
        auto knobR = knobRadius * 0.9f;
        juce::ColourGradient bakelite(juce::Colour::fromRGB(60, 50, 45), centre.x - knobR, centre.y - knobR,
                                      juce::Colour::fromRGB(25, 20, 18), centre.x + knobR, centre.y + knobR, true);
        g.setGradientFill(bakelite);
        g.fillEllipse(centre.x - knobR, centre.y - knobR, knobR * 2, knobR * 2);
        g.setColour(juce::Colour::fromRGB(100, 90, 80));
        g.drawEllipse(centre.x - knobR, centre.y - knobR, knobR * 2, knobR * 2, 1.0f);
        
        g.setColour(juce::Colours::ivory);
        g.drawLine(centre.x, centre.y, 
                   centre.x + std::sin(valueAngle) * knobR * 0.7f,
                   centre.y - std::cos(valueAngle) * knobR * 0.7f, 2.5f);
    }
    else if (knobStyle == 4) // LED Ring (center is just dark knob)
    {
        auto knobR = knobRadius * 0.85f;
        g.setColour(juce::Colour::fromRGB(25, 28, 32));
        g.fillEllipse(centre.x - knobR, centre.y - knobR, knobR * 2, knobR * 2);
        g.setColour(juce::Colour::fromRGB(50, 55, 60));
        g.drawEllipse(centre.x - knobR, centre.y - knobR, knobR * 2, knobR * 2, 1.5f);
        
        g.setColour(theme.textBright);
        auto notchLen = knobR * 0.6f;
        g.drawLine(centre.x + std::sin(valueAngle) * knobR * 0.3f,
                   centre.y - std::cos(valueAngle) * knobR * 0.3f,
                   centre.x + std::sin(valueAngle) * notchLen,
                   centre.y - std::cos(valueAngle) * notchLen, 2.0f);
    }
    else // Flat Modern (style 5)
    {
        auto centerR = knobRadius * 0.6f;
        g.setColour(theme.bgDark);
        g.fillEllipse(centre.x - centerR, centre.y - centerR, centerR * 2, centerR * 2);
        
        g.setColour(theme.accent);
        g.drawLine(centre.x, centre.y,
                   centre.x + std::sin(valueAngle) * centerR * 0.8f,
                   centre.y - std::cos(valueAngle) * centerR * 0.8f, 3.0f);
    }
}

void SatelliteEditor::drawMeterBar(juce::Graphics& g, juce::Rectangle<float> bounds, 
                                    float valueDb, const juce::String& label)
{
    // Background
    g.setColour(theme.bgPanel.darker(0.5f));
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // LED meter - horizontal segments
    const int numLeds = 16;
    const float ledGap = 1.5f;
    const float ledWidth = (bounds.getWidth() - (numLeds - 1) * ledGap) / numLeds;
    
    float norm = juce::jlimit(0.0f, 1.0f, (valueDb + 60.0f) / 60.0f);
    int litLeds = static_cast<int>(norm * numLeds);
    
    for (int i = 0; i < numLeds; ++i)
    {
        float ledX = bounds.getX() + i * (ledWidth + ledGap);
        auto ledRect = juce::Rectangle<float>(ledX, bounds.getY() + 2, ledWidth, bounds.getHeight() - 4);
        
        // Color based on position
        juce::Colour ledColor;
        float pos = (float)i / numLeds;
        if (pos < 0.5f)
            ledColor = juce::Colour::fromRGB(0, 200, 80);      // Green
        else if (pos < 0.7f)
            ledColor = juce::Colour::fromRGB(180, 220, 50);    // Yellow-green
        else if (pos < 0.85f)
            ledColor = juce::Colour::fromRGB(255, 200, 0);     // Yellow
        else
            ledColor = juce::Colour::fromRGB(255, 60, 50);     // Red
        
        bool lit = (i < litLeds);
        
        if (lit)
        {
            // Glow
            g.setColour(ledColor.withAlpha(0.25f));
            g.fillRoundedRectangle(ledRect.expanded(0.5f), 1.0f);
            g.setColour(ledColor);
        }
        else
        {
            g.setColour(theme.bgPanel.darker(0.3f));
        }
        g.fillRoundedRectangle(ledRect, 1.0f);
    }
    
    // Label
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(theme.textDim);
    g.drawText(label, bounds.getX() - 35, bounds.getY(), 30, bounds.getHeight(), juce::Justification::right);
    
    // Value
    g.setColour(theme.textBright);
    juce::String valueStr = valueDb > -100 ? juce::String(valueDb, 1) : "-inf";
    g.drawText(valueStr, bounds.getRight() + 5, bounds.getY(), 45, bounds.getHeight(), juce::Justification::left);
}

void SatelliteEditor::drawPhaseMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Background
    g.setColour(theme.bgPanel.darker(0.3f));
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Phase indicator (0 = center, +1 = right, -1 = left)
    float centerX = bounds.getCentreX();
    float norm = (smoothedPhase + 1.0f) / 2.0f;  // 0-1 range
    float indicatorX = bounds.getX() + bounds.getWidth() * norm;
    
    // Fill from center to position
    juce::Colour phaseColor = smoothedPhase > 0.5f ? theme.meterGreen 
                            : (smoothedPhase > -0.3f ? theme.meterYellow : theme.meterRed);
    
    auto fillRect = juce::Rectangle<float>(
        std::min(centerX, indicatorX), bounds.getY(),
        std::abs(indicatorX - centerX), bounds.getHeight()
    );
    g.setColour(phaseColor.withAlpha(0.7f));
    g.fillRoundedRectangle(fillRect, 3.0f);
    
    // Center line
    g.setColour(theme.textDim.withAlpha(0.5f));
    g.drawVerticalLine(static_cast<int>(centerX), bounds.getY(), bounds.getBottom());
    
    // Label
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(theme.textDim);
    g.drawText("PHASE", bounds.getX() - 45, bounds.getY(), 40, bounds.getHeight(), juce::Justification::right);
    
    // Value
    g.setColour(phaseColor);
    g.drawText(juce::String(smoothedPhase, 2), bounds.getRight() + 5, bounds.getY(), 45, bounds.getHeight(), juce::Justification::left);
}

void SatelliteEditor::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();
    
    // Background
    g.fillAll(theme.bgDark);
    
    // Main panel border
    g.setColour(theme.accent.withAlpha(0.4f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(2), 6.0f, 2.0f);
    
    // === TOP BAR ===
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.setColour(theme.accent);
    g.drawText("AR3S SATELLITE", 12, 6, 150, 20, juce::Justification::left);
    
    // Connection status
    bool connected = processor.isConnected();
    g.setColour(connected ? theme.meterGreen : theme.meterYellow);
    g.fillEllipse(w - 24, 10, 12, 12);
    
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(theme.textDim);
    g.drawText(connected ? "LINKED" : "STANDALONE", w - 100, 8, 70, 16, juce::Justification::right);
    
    // Master control indicator
    if (processor.isControlledByMaster())
    {
        g.setColour(theme.accent);
        g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
        g.drawText("MASTER CTRL", 160, 8, 80, 16, juce::Justification::left);
    }
    
    // === GAIN KNOB AREA ===
    // Label above knob
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    g.drawText("GAIN", 10, 28, 90, 14, juce::Justification::centred);
    
    float sliderPos = static_cast<float>((gainKnob.getValue() - gainKnob.getMinimum()) / 
                                          (gainKnob.getMaximum() - gainKnob.getMinimum()));
    drawGainKnob(g, juce::Rectangle<float>(10, 42, 90, 80), sliderPos);
    
    // Gain value display
    float gainDbVal = static_cast<float>(gainKnob.getValue());
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    juce::String gainStr = (gainDbVal >= 0 ? "+" : "") + juce::String(gainDbVal, 1) + " dB";
    g.drawText(gainStr, 10, 122, 90, 18, juce::Justification::centred);
    
    // === CEILING KNOB AREA ===
    // Label above knob
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    g.drawText("CEILING", 105, 28, 90, 14, juce::Justification::centred);
    
    float ceilingSliderPos = static_cast<float>((ceilingKnob.getValue() - ceilingKnob.getMinimum()) / 
                                                 (ceilingKnob.getMaximum() - ceilingKnob.getMinimum()));
    drawGainKnob(g, juce::Rectangle<float>(105, 42, 90, 80), ceilingSliderPos);
    
    // Ceiling value display
    float ceilingDbVal = static_cast<float>(ceilingKnob.getValue());
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    juce::String ceilingStr = juce::String(ceilingDbVal, 1) + " dB";
    g.drawText(ceilingStr, 105, 122, 90, 18, juce::Justification::centred);
    
    // === METERS ===
    const float meterX = 460;  // Moved further right to avoid controls
    const float meterW = w - meterX - 10;  // Adjusted width
    const float meterH = 14;
    
    drawMeterBar(g, juce::Rectangle<float>(meterX, 60, meterW, meterH), smoothedRms, "RMS");
    drawMeterBar(g, juce::Rectangle<float>(meterX, 80, meterW, meterH), smoothedPeak, "PEAK");
    drawPhaseMeter(g, juce::Rectangle<float>(meterX, 100, meterW, meterH));
}

void SatelliteEditor::resized()
{
    // Channel name - moved further right to avoid ceiling knob
    channelNameEditor.setBounds(200, 42, 140, 22);
    
    // Source dropdown - moved further right
    sourceBox.setBounds(342, 42, 110, 22);
    
    // Gain knob - left side (adjusted for label)
    gainKnob.setBounds(10, 42, 90, 80);
    
    // Ceiling knob - right next to gain knob
    ceilingKnob.setBounds(105, 42, 90, 80);
    
    // Pre/Post toggle - moved further right
    meterModeButton.setBounds(560, 42, 50, 22);
    
    // Control mode toggle (LOCAL/SYNC) - moved further right
    controlModeButton.setBounds(615, 42, 60, 22);
}

void SatelliteEditor::timerCallback()
{
    // Sync theme from master
    updateThemeFromMaster();
    
    // Get meter values
    float rawRms = showPreMeter ? processor.getPreRmsDb() : processor.getPostRmsDb();
    float rawPeak = showPreMeter ? processor.getPrePeakDb() : processor.getPostPeakDb();
    float rawPhase = showPreMeter ? processor.getPrePhaseCorrelation() : processor.getPostPhaseCorrelation();
    
    // Smooth values for readable display - 0.97 makes numeric values much more readable
    const float smooth = 0.97f;
    smoothedRms = smooth * smoothedRms + (1.0f - smooth) * rawRms;
    smoothedPeak = smooth * smoothedPeak + (1.0f - smooth) * rawPeak;
    smoothedPhase = smooth * smoothedPhase + (1.0f - smooth) * rawPhase;
    smoothedGain = 0.95f * smoothedGain + 0.05f * processor.getCurrentGain();
    
    repaint();
}
