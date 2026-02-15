#include "PluginEditor.h"
void SimpleGainAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(15);
    auto headerArea = area.removeFromTop(55);

    // Tab buttons (moved up by removing extra spacing)
    auto tabRow = headerArea.removeFromTop(32);
    int tabW = tabRow.getWidth() / 4; // make room for toggles
    mainTabBtn.setBounds(tabRow.removeFromLeft(tabW).reduced(2));
    settingsTabBtn.setBounds(tabRow.removeFromLeft(tabW).reduced(2));
    chatTabBtn.setBounds(tabRow.removeFromLeft(tabW).reduced(2));

    // Place edit & auto-fit toggles in header
    autoFitToggle.setBounds(tabRow.removeFromLeft(tabW).reduced(2));
    editModeToggle.setBounds(headerArea.removeFromTop(22).withTrimmedTop(6).withTrimmedLeft(6).withSizeKeepingCentre(60, 18));

    // If Auto-fit mode is enabled and we're not in edit mode, use automatic layout and return
    if (currentTab == 0 && autoFitMode && !editMode)
    {
        applyAutoLayout();
        return;
    }

    area.removeFromTop(10);
    resizeCorner->setBounds(getWidth() - 18, getHeight() - 18, 18, 18);
    
    // Responsive small-width handling for Main tab: stacked, proportional layout
    if (currentTab == 0)
    {
        int w = area.getWidth();
        if (w < 900)
        {
            // Super-compact single-column mode for very narrow windows
            if (w < 820)
            {
                auto col = area.reduced(6);

                // Smaller knob area and tighter spacing
                int knobH = juce::jmax(56, col.getHeight() / 9);
                auto gArea = col.removeFromTop(knobH).reduced(3); gainLabel.setBounds(gArea.removeFromTop(14)); gainSlider.setBounds(gArea.reduced(2));
                col.removeFromTop(6);
                auto tArea = col.removeFromTop(knobH).reduced(3); targetDbLabel.setBounds(tArea.removeFromTop(14)); targetDbSlider.setBounds(tArea.reduced(2));
                col.removeFromTop(6);
                auto cArea = col.removeFromTop(knobH).reduced(3); ceilingLabel.setBounds(cArea.removeFromTop(14)); ceilingSlider.setBounds(cArea.reduced(2));

                col.removeFromTop(8);

                // Primary meter (VU) gets most of the remaining visual space; hide heavy analysis components
                vuMeter.setBounds(col.removeFromTop((int)std::round(col.getHeight() * 0.55f)).reduced(3));

                // Minimal meters shown
                grMeter.setBounds(col.removeFromTop(48).reduced(3));
                satellitePanel.setBounds(col.reduced(3));

                // Hide expensive/large visuals to keep UI legible
                ioMeter.setVisible(false);
                freqBalance.setVisible(false);
                advancedMeters.setVisible(false);
                quickIssues.setVisible(false);

                // Ensure resize handle position
                resizeCorner->setBounds(getWidth() - 18, getHeight() - 18, 18, 18);

                // Diagnostics to capture what the UI looks like in the wild
                DBG("[Layout] super-compact mode: window=" << getWidth() << "x" << getHeight());
                DBG("gainSlider=" << gainSlider.getBounds().toString()
                    << " targetDb=" << targetDbSlider.getBounds().toString()
                    << " ceiling=" << ceilingSlider.getBounds().toString());
                DBG("vuMeter=" << vuMeter.getBounds().toString() << " grMeter=" << grMeter.getBounds().toString());
                DBG("satPanel=" << satellitePanel.getBounds().toString());

                return;
            }

            // Default compact proportional layout for medium narrow widths
            int topHeight = juce::jmin(160, area.getHeight() / 4);
            auto top = area.removeFromTop(topHeight).reduced(6);
            int each = juce::jmax(36, top.getHeight() / 3);
            auto gArea = top.removeFromTop(each); gainLabel.setBounds(gArea.removeFromTop(16)); gainSlider.setBounds(gArea);
            auto tArea = top.removeFromTop(each); targetDbLabel.setBounds(tArea.removeFromTop(16)); targetDbSlider.setBounds(tArea);
            auto cArea = top.removeFromTop(each); ceilingLabel.setBounds(cArea.removeFromTop(16)); ceilingSlider.setBounds(cArea);

            int remaining = area.getHeight();
            int ioH = (int)std::round(remaining * 0.28f);
            int freqH = (int)std::round(remaining * 0.28f);
            int vuH = (int)std::round(remaining * 0.30f);
            int advH = juce::jmax(48, remaining - ioH - freqH - vuH);

            ioMeter.setVisible(true);
            freqBalance.setVisible(true);
            advancedMeters.setVisible(true);
            quickIssues.setVisible(true);

            ioMeter.setBounds(area.removeFromTop(ioH).reduced(3));
            freqBalance.setBounds(area.removeFromTop(freqH).reduced(3));
            vuMeter.setBounds(area.removeFromTop(vuH).reduced(3));
            advancedMeters.setBounds(area.removeFromTop(juce::jmin(advH, 110)).reduced(3));
            quickIssues.setBounds(area.reduced(3));

            // Ensure resize corner sits correctly
            resizeCorner->setBounds(getWidth() - 18, getHeight() - 18, 18, 18);

            // Diagnostic logging to help debug narrow layouts
            DBG("[Layout] small layout: window=" << getWidth() << "x" << getHeight());
            DBG("gainSlider=" << gainSlider.getBounds().toString()
                << " targetDb=" << targetDbSlider.getBounds().toString()
                << " ceiling=" << ceilingSlider.getBounds().toString());
            DBG("ioMeter=" << ioMeter.getBounds().toString() << " freqBalance=" << freqBalance.getBounds().toString());
            DBG("vuMeter=" << vuMeter.getBounds().toString() << " advanced=" << advancedMeters.getBounds().toString());
            DBG("satPanel=" << satellitePanel.getBounds().toString());

            return;
        }
    }

    if (!layoutLoaded)
    {
        if (currentTab == 0) // Main
        {
            // Pro layout: Controls | VU + Analysis | Main Meter | Context/AI
            int totalWidth = area.getWidth();
            int controlsWidth = juce::jlimit(180, 260, totalWidth / 6);    // scale controls with window
            int vuAnalysisWidth = juce::jlimit(220, 360, totalWidth / 6);  // give VU more room but cap it
            int meterWidth = juce::jmin(360, totalWidth / 3);       // tightened cap on main meter to avoid dominance
            int contextWidth = totalWidth - controlsWidth - vuAnalysisWidth - meterWidth;
            if (contextWidth < 200) { // fall back to balanced widths if space is tight
                meterWidth = juce::jmin(meterWidth, totalWidth / 4);
                contextWidth = totalWidth - controlsWidth - vuAnalysisWidth - meterWidth;
            }
            
            auto controlsCol = area.removeFromLeft(controlsWidth).reduced(5);
            auto vuCol = area.removeFromLeft(vuAnalysisWidth).reduced(5);
            auto meterCol = area.removeFromLeft(meterWidth).reduced(5);
            auto rightCol = area.reduced(5);

            // === CONTROLS COLUMN (Left) ===
            // Main Knobs stacked vertically - use proportional heights so small widths don't push content offscreen
            int knobArea = juce::jmax(72, controlsCol.getHeight() / 5);
            auto gainArea = controlsCol.removeFromTop(knobArea).reduced(3);
            gainLabel.setBounds(gainArea.removeFromTop(16));
            gainSlider.setBounds(gainArea);
            
            controlsCol.removeFromTop(6);
            auto targetArea = controlsCol.removeFromTop(knobArea).reduced(3);
            targetDbLabel.setBounds(targetArea.removeFromTop(16));
            targetDbSlider.setBounds(targetArea);
            
            controlsCol.removeFromTop(6);
            auto ceilingArea = controlsCol.removeFromTop(knobArea).reduced(3);  // proportional knob size
            ceilingLabel.setBounds(ceilingArea.removeFromTop(16));
            ceilingSlider.setBounds(ceilingArea);
            
            // Toggles
            controlsCol.removeFromTop(8);
            autoToggle.setBounds(controlsCol.removeFromTop(24));
            controlsCol.removeFromTop(4);
            riderToggle.setBounds(controlsCol.removeFromTop(24));
            
            // Rider amount 
            controlsCol.removeFromTop(6);
            riderAmountLabel.setBounds(controlsCol.removeFromTop(14));
            riderAmountSlider.setBounds(controlsCol.removeFromTop(22));
            
            // Phase correlation (compact)
            controlsCol.removeFromTop(10);
            phaseMeter.setBounds(controlsCol.removeFromTop(45).reduced(3));
            
            // Gain reduction meter
            controlsCol.removeFromTop(6);
            grMeter.setBounds(controlsCol.removeFromTop(45).reduced(3));
            
            // Hide crest meter - info is in advanced panel
            crestMeterBounds = juce::Rectangle<float>(0, 0, 0, 0);

            // === VU + ANALYSIS COLUMN ===
            // VU Meters (IN/OUT) - Prominent size
            ioMeter.setBounds(vuCol.removeFromTop(220).reduced(3));
            
            vuCol.removeFromTop(6);
            
            // Frequency Balance - LARGE prominent spectrum analyzer
            freqBalance.setBounds(vuCol.removeFromTop(200).reduced(3));
            
            vuCol.removeFromTop(6);
            
            // Advanced Analysis Panel (consolidated metrics)
            advancedMeters.setBounds(vuCol.removeFromTop(60).reduced(3));
            
            // Quick Issues below
            vuCol.removeFromTop(4);
            quickIssues.setBounds(vuCol.reduced(3));

            // === CENTER METER COLUMN ===
            auto meterModeArea = meterCol.removeFromTop(28);
            int btnW = 60;
            meterModeButton.setBounds(meterModeArea.removeFromLeft(btnW).withTrimmedTop(2));
            vuMeter.setBounds(meterCol);

            // === RIGHT COLUMN - Context and AI ===
            auto contextArea = rightCol.removeFromTop(120);
            auto row1 = contextArea.removeFromTop(42);
            auto genreArea = row1.removeFromLeft(row1.getWidth() / 2).reduced(3);
            genreLabel.setBounds(genreArea.removeFromTop(16));
            genreBox.setBounds(genreArea);
            auto sourceArea = row1.reduced(3);
            sourceLabel.setBounds(sourceArea.removeFromTop(16));
            sourceBox.setBounds(sourceArea);

            contextArea.removeFromTop(4);
            auto row2 = contextArea.removeFromTop(42);
            auto sitArea = row2.removeFromLeft(row2.getWidth() / 2).reduced(3);
            situationLabel.setBounds(sitArea.removeFromTop(16));
            situationBox.setBounds(sitArea);
            
            // Standalone mode toggle (right side of row2)
            auto standAloneArea = row2.reduced(3);
            standaloneModeToggle.setBounds(standAloneArea.removeFromTop(26).withTrimmedTop(10));

            contextArea.removeFromTop(6);
            auto btnRow = contextArea.removeFromTop(32);
            int bw = btnRow.getWidth() / 3;
            autoSetButton.setBounds(btnRow.removeFromLeft(bw).reduced(2));
            aiSuggestButton.setBounds(btnRow.removeFromLeft(bw).reduced(2));
            applyAiButton.setBounds(btnRow.reduced(2));

            rightCol.removeFromTop(10);
            
            // Platform target 
            auto platformRow = rightCol.removeFromTop(26);
            platformTargetLabel.setBounds(platformRow.removeFromLeft(55).reduced(2, 0));
            platformTargetBox.setBounds(platformRow.reduced(2, 0));
            
            rightCol.removeFromTop(6);
            analysisLabel.setBounds(rightCol.removeFromTop(18).reduced(3, 0));
            aiStatusLabel.setBounds(rightCol.removeFromTop(18).reduced(3, 0));
            rightCol.removeFromTop(4);
            
            // AI notes area
            auto aiNotesArea = rightCol.removeFromTop(rightCol.getHeight() / 2 - 5);
            aiNotes.setBounds(aiNotesArea.reduced(3));
            
            rightCol.removeFromTop(8);
            satellitePanel.setBounds(rightCol.reduced(3));
        }
    }
    else if (currentTab == 1) // Settings
    {
        auto content = area.reduced(20);
        // Compact stacked layout for narrow windows
        if (content.getWidth() < 700)
        {
            auto stack = content.reduced(10);
            themeLabel.setBounds(stack.removeFromTop(18)); themeBox.setBounds(stack.removeFromTop(36));
            knobStyleLabel.setBounds(stack.removeFromTop(18)); knobStyleBox.setBounds(stack.removeFromTop(36));
            modelLabel.setBounds(stack.removeFromTop(18)); modelBox.setBounds(stack.removeFromTop(36));
            aiProviderLabel.setBounds(stack.removeFromTop(18)); aiProviderBox.setBounds(stack.removeFromTop(36));
            apiKeyLabel.setBounds(stack.removeFromTop(18)); apiKeyEditor.setBounds(stack.removeFromTop(36));

            micLabel.setBounds(stack.removeFromTop(18)); micBox.setBounds(stack.removeFromTop(28));
            preampLabel.setBounds(stack.removeFromTop(18)); preampBox.setBounds(stack.removeFromTop(28));
            interfaceLabel.setBounds(stack.removeFromTop(18)); interfaceBox.setBounds(stack.removeFromTop(28));
            languageLabel.setBounds(stack.removeFromTop(18)); languageBox.setBounds(stack.removeFromTop(28));

            saveSettingsBtn.setBounds(stack.removeFromTop(36));
            saveLayoutBtn.setBounds(stack.removeFromTop(36));
            loadLayoutBtn.setBounds(stack.removeFromTop(36));
            return;
        }

        auto left = content.removeFromLeft(content.getWidth() / 2).reduced(10);
        auto right = content.reduced(10);

        // Left: AI setup
        aiProviderLabel.setBounds(left.removeFromTop(20));
        aiProviderBox.setBounds(left.removeFromTop(30));
        left.removeFromTop(10);
        modelLabel.setBounds(left.removeFromTop(20));
        modelBox.setBounds(left.removeFromTop(30));
        left.removeFromTop(8);
        refreshModelsButton.setBounds(left.removeFromTop(32));
        left.removeFromTop(15);
        apiKeyLabel.setBounds(left.removeFromTop(20));
        apiKeyEditor.setBounds(left.removeFromTop(30));
        
        // Equipment section below API key (for AI context)
        left.removeFromTop(20);
        micLabel.setBounds(left.removeFromTop(16));
        micBox.setBounds(left.removeFromTop(26));
        if (showCustomMicEditor)
        {
            customMicEditor.setVisible(true);
            customMicEditor.setBounds(left.removeFromTop(26));
        }
        else
        {
            customMicEditor.setVisible(false);
        }
        left.removeFromTop(6);
        preampLabel.setBounds(left.removeFromTop(16));
        preampBox.setBounds(left.removeFromTop(26));
        if (showCustomPreampEditor)
        {
            customPreampEditor.setVisible(true);
            customPreampEditor.setBounds(left.removeFromTop(26));
        }
        else
        {
            customPreampEditor.setVisible(false);
        }
        left.removeFromTop(6);
        interfaceLabel.setBounds(left.removeFromTop(16));
        interfaceBox.setBounds(left.removeFromTop(26));
        if (showCustomInterfaceEditor)
        {
            customInterfaceEditor.setVisible(true);
            customInterfaceEditor.setBounds(left.removeFromTop(26));
        }
        else
        {
            customInterfaceEditor.setVisible(false);
        }

        // Right: Appearance
        themeLabel.setBounds(right.removeFromTop(20));
        themeBox.setBounds(right.removeFromTop(30));
        right.removeFromTop(10);
        knobStyleLabel.setBounds(right.removeFromTop(20));
        knobStyleBox.setBounds(right.removeFromTop(30));
        right.removeFromTop(15);
        accentColorLabel.setBounds(right.removeFromTop(20));
        accentHueSlider.setBounds(right.removeFromTop(30));
        right.removeFromTop(10);
        bgColorLabel.setBounds(right.removeFromTop(20));
        bgBrightnessSlider.setBounds(right.removeFromTop(30));
        right.removeFromTop(15);
        languageLabel.setBounds(right.removeFromTop(20));
        languageBox.setBounds(right.removeFromTop(30));
        right.removeFromTop(20);
        saveSettingsBtn.setBounds(right.removeFromTop(35));
        right.removeFromTop(10);
        saveLayoutBtn.setBounds(right.removeFromTop(35));
        right.removeFromTop(5);
        loadLayoutBtn.setBounds(right.removeFromTop(35));
    }
    else if (currentTab == 2) // Chat
    {
        auto content = area.reduced(15);

        // Compact chat layout for narrow widths
        if (content.getWidth() < 480)
        {
            chatLabel.setBounds(content.removeFromTop(22));
            chatHistory.setBounds(content.removeFromTop(content.getHeight() - 48));
            auto inputRow = content.removeFromTop(40);
            sendChatBtn.setBounds(inputRow.removeFromRight(80).reduced(2));
            chatInput.setBounds(inputRow.reduced(2));
            return;
        }

        chatLabel.setBounds(content.removeFromTop(22));
        content.removeFromTop(6);
        auto inputRow = content.removeFromBottom(40);
        sendChatBtn.setBounds(inputRow.removeFromRight(90).reduced(2));
        chatInput.setBounds(inputRow.reduced(2));
        content.removeFromBottom(8);
        // Full chat history - use all available space for rich conversation
        chatHistory.setBounds(content);
    }

    // If a saved layout was loaded, ensure components fit in the current window
    if (layoutLoaded)
    {
        auto windowBounds = getLocalBounds().reduced(10);

        juce::Component* components[] = {
            &gainSlider, &targetDbSlider, &ceilingSlider,
            &autoToggle, &riderToggle, &riderAmountSlider,
            &genreBox, &sourceBox, &situationBox,
            &autoSetButton, &aiSuggestButton, &applyAiButton,
            &analysisLabel, &aiStatusLabel, &aiNotes,
            &vuMeter, &phaseMeter, &grMeter, &ioMeter,
            &freqBalance, &advancedMeters, &quickIssues, &satellitePanel,
            &meterModeButton
        };

        // If the window size changed since last time we can do a proportional layout scaling
        if (lastWindowBounds.getWidth() > 0 && (windowBounds.getWidth() != lastWindowBounds.getWidth() || windowBounds.getHeight() != lastWindowBounds.getHeight()))
        {
            float sx = (float)windowBounds.getWidth() / (float)lastWindowBounds.getWidth();
            float sy = (float)windowBounds.getHeight() / (float)lastWindowBounds.getHeight();

            for (auto* comp : components)
            {
                if (!comp || !comp->isVisible()) continue;
                auto b = comp->getBounds();

                int nx = windowBounds.getX() + (int)std::round((b.getX() - lastWindowBounds.getX()) * sx);
                int ny = windowBounds.getY() + (int)std::round((b.getY() - lastWindowBounds.getY()) * sy);
                int nw = juce::jmax(50, (int)std::round(b.getWidth() * sx));
                int nh = juce::jmax(20, (int)std::round(b.getHeight() * sy));

                juce::Rectangle<int> nb(nx, ny, nw, nh);
                if (nb.getRight() > windowBounds.getRight()) nb.setX(windowBounds.getRight() - nb.getWidth());
                if (nb.getBottom() > windowBounds.getBottom()) nb.setY(windowBounds.getBottom() - nb.getHeight());
                if (nb.getX() < windowBounds.getX()) nb.setX(windowBounds.getX());
                if (nb.getY() < windowBounds.getY() + 60) nb.setY(windowBounds.getY() + 60);
                comp->setBounds(nb);
            }
        }
        else
        {
            for (auto* comp : components)
            {
                if (!comp || !comp->isVisible()) continue;
                auto b = comp->getBounds();
                if (b.getRight() > windowBounds.getRight())
                    b.setX(windowBounds.getRight() - b.getWidth());
                if (b.getBottom() > windowBounds.getBottom())
                    b.setY(windowBounds.getBottom() - b.getHeight());
                if (b.getX() < windowBounds.getX()) b.setX(windowBounds.getX());
                if (b.getY() < windowBounds.getY() + 60) b.setY(windowBounds.getY() + 60);
                comp->setBounds(b);
            }
        }

        lastWindowBounds = windowBounds;
    }

    // Update cached float bounds for hit-testing and drawing
    meterBounds = vuMeter.getBounds().toFloat();
    phaseMeterBounds = phaseMeter.getBounds().toFloat();
    satellitePanelBounds = satellitePanel.getBounds().toFloat();
    crestMeterBounds = juce::Rectangle<float>(0, 0, 0, 0);
    grMeterBounds = grMeter.getBounds().toFloat();
    ioMeterBounds = ioMeter.getBounds().toFloat();
    advancedMeterBounds = advancedMeters.getBounds().toFloat();
    freqBalanceBounds = freqBalance.getBounds().toFloat();
    quickIssuesBounds = quickIssues.getBounds().toFloat();

    // Ensure resize handle sits on top and within bounds
    resizeCorner->toFront(true);

    // Initialize lastWindowBounds if not set yet
    if (lastWindowBounds.getWidth() == 0)
        lastWindowBounds = getLocalBounds().reduced(10);
}
#include "PluginEditor.h"
#include "Localization.h"
#include "ThemeData.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace
{
    float dbToNormalized(float db, float minDb = -60.0f, float maxDb = 6.0f)
    {
        return juce::jlimit(0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
    }
}

class SimpleGainAudioProcessorEditor::AresLookAndFeel : public juce::LookAndFeel_V4
{
public:
    Theme* themePtr = nullptr;
    int knobStyle = 0;

    AresLookAndFeel() { updateColours(); }

    void updateColours()
    {
        if (!themePtr) return;
        auto& t = *themePtr;

        setColour(juce::Slider::textBoxTextColourId, t.textBright);
        setColour(juce::Slider::textBoxOutlineColourId, t.accent.withAlpha(0.4f));
        setColour(juce::Slider::textBoxBackgroundColourId, t.bgDark);
        setColour(juce::Slider::thumbColourId, t.accent);
        setColour(juce::Slider::rotarySliderFillColourId, t.accent);
        setColour(juce::Slider::rotarySliderOutlineColourId, t.bgPanel);
        setColour(juce::Slider::trackColourId, t.accent);

        setColour(juce::ComboBox::backgroundColourId, t.bgPanel);
        setColour(juce::ComboBox::textColourId, t.textBright);
        setColour(juce::ComboBox::outlineColourId, t.accent.withAlpha(0.5f));
        setColour(juce::ComboBox::arrowColourId, t.accent);

        setColour(juce::TextButton::buttonColourId, t.bgPanel);
        setColour(juce::TextButton::textColourOffId, t.textBright);
        setColour(juce::TextButton::textColourOnId, t.accent);

        setColour(juce::TextEditor::backgroundColourId, t.bgDark);
        setColour(juce::TextEditor::textColourId, t.textBright);
        setColour(juce::TextEditor::outlineColourId, t.accent.withAlpha(0.3f));
        setColour(juce::TextEditor::highlightColourId, t.accent.withAlpha(0.3f));

        setColour(juce::PopupMenu::backgroundColourId, t.bgPanel);
        setColour(juce::PopupMenu::textColourId, t.textBright);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, t.accent.withAlpha(0.3f));
        setColour(juce::PopupMenu::highlightedTextColourId, t.textBright);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        if (!themePtr) return;
        auto& t = *themePtr;

        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height));
        auto radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f - 6.0f;
        auto centre = bounds.getCentre();
        auto knobRadius = radius * 0.70f;
        
        auto valueAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // === OUTER RING with realistic depth ===
        g.setColour(juce::Colour::fromRGB(20, 22, 26));
        g.fillEllipse(centre.x - radius - 3, centre.y - radius - 3, (radius + 3) * 2, (radius + 3) * 2);
        g.setColour(juce::Colour::fromRGB(35, 38, 42));
        g.fillEllipse(centre.x - radius - 1, centre.y - radius - 1, (radius + 1) * 2, (radius + 1) * 2);
        
        // === LED RING - Matches Satellite style ===
        int numLeds = 21;
        float totalAngle = rotaryEndAngle - rotaryStartAngle;
        float ledAngle = totalAngle / numLeds;
        float ledR = radius * 0.92f;
        float dotR = juce::jmax(1.0f, radius * 0.06f);  // scale LED dot size with knob radius
        
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
                ledColor = juce::Colour::fromRGB(255, 60, 50);    // Red
            
            // LED housing
            g.setColour(juce::Colour::fromRGB(18, 20, 24));
            g.fillEllipse(lx - dotR - 1, ly - dotR - 1, (dotR + 1) * 2, (dotR + 1) * 2);
            
            if (lit)
            {
                // Glow effect
                g.setColour(ledColor.withAlpha(0.3f));
                g.fillEllipse(lx - dotR - 2, ly - dotR - 2, (dotR + 2) * 2, (dotR + 2) * 2);
                g.setColour(ledColor);
            }
            else
            {
                g.setColour(t.bgPanel.darker(0.4f));
            }
            g.fillEllipse(lx - dotR, ly - dotR, dotR * 2, dotR * 2);
        }

        // === KNOB CENTER - Style-dependent ===
        // Metallic base gradient
        juce::ColourGradient knobGrad(juce::Colour::fromRGB(75, 80, 90), centre.x, centre.y - knobRadius,
                                      juce::Colour::fromRGB(30, 35, 42), centre.x, centre.y + knobRadius, false);
        g.setGradientFill(knobGrad);
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2, knobRadius * 2);
        
        // Outer ring bevel
        g.setColour(juce::Colour::fromRGB(100, 105, 115));
        g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2, knobRadius * 2, 1.5f);
        
        // Inner shadow ring
        g.setColour(juce::Colour::fromRGB(20, 23, 28));
        g.drawEllipse(centre.x - knobRadius + 2, centre.y - knobRadius + 2, 
                      (knobRadius - 2) * 2, (knobRadius - 2) * 2, 1.0f);
        
        // === REALISTIC SHINE - Subtle and professional ===
        {
            // Top-left ambient light reflection
            juce::ColourGradient ambientGrad(
                juce::Colours::white.withAlpha(0.18f), centre.x - knobRadius * 0.4f, centre.y - knobRadius * 0.5f,
                juce::Colours::transparentWhite, centre.x, centre.y + knobRadius * 0.3f, true);
            g.setGradientFill(ambientGrad);
            g.fillEllipse(centre.x - knobRadius * 0.95f, centre.y - knobRadius * 0.95f, 
                          knobRadius * 1.9f, knobRadius * 1.9f);
            
            // Small specular highlight
            g.setColour(juce::Colours::white.withAlpha(0.22f));
            g.fillEllipse(centre.x - knobRadius * 0.35f, centre.y - knobRadius * 0.55f, 
                          knobRadius * 0.5f, knobRadius * 0.25f);
        }

        // === POINTER LINE with glow ===
        auto pLen = knobRadius * 0.65f;
        g.setColour(t.accent.withAlpha(0.35f));
        g.drawLine(centre.x + std::sin(valueAngle) * knobRadius * 0.25f,
                   centre.y - std::cos(valueAngle) * knobRadius * 0.25f,
                   centre.x + std::sin(valueAngle) * pLen,
                   centre.y - std::cos(valueAngle) * pLen, 4.5f);
        g.setColour(t.accent);
        g.drawLine(centre.x + std::sin(valueAngle) * knobRadius * 0.25f,
                   centre.y - std::cos(valueAngle) * knobRadius * 0.25f,
                   centre.x + std::sin(valueAngle) * pLen,
                   centre.y - std::cos(valueAngle) * pLen, 2.0f);
        
        // Dot at end of pointer
        float ptrDotR = 3.0f;
        float dotX = centre.x + std::sin(valueAngle) * pLen;
        float dotY = centre.y - std::cos(valueAngle) * pLen;
        g.setColour(t.accent);
        g.fillEllipse(dotX - ptrDotR, dotY - ptrDotR, ptrDotR * 2, ptrDotR * 2);
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.fillEllipse(dotX - ptrDotR * 0.4f, dotY - ptrDotR * 0.5f, ptrDotR * 0.7f, ptrDotR * 0.5f);
        
        // === CENTER CAP with metallic look ===
        float capR = knobRadius * 0.22f;
        juce::ColourGradient capGrad(juce::Colour::fromRGB(85, 90, 100), centre.x, centre.y - capR,
                                     juce::Colour::fromRGB(35, 40, 48), centre.x, centre.y + capR, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(centre.x - capR, centre.y - capR, capR * 2, capR * 2);
        g.setColour(juce::Colour::fromRGB(110, 115, 125));
        g.drawEllipse(centre.x - capR, centre.y - capR, capR * 2, capR * 2, 1.0f);
        // Cap highlight
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.fillEllipse(centre.x - capR * 0.4f, centre.y - capR * 0.6f, capR * 0.7f, capR * 0.4f);
    }
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos, 
                          juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (!themePtr) return;
        auto& t = *themePtr;
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height));
        
        bool isHorizontal = (style == juce::Slider::LinearHorizontal || 
                             style == juce::Slider::LinearBar ||
                             style == juce::Slider::LinearBarVertical == false);
        
        if (isHorizontal)
        {
            // === HORIZONTAL FADER STYLE ===
            float trackH = 6.0f;
            float trackY = bounds.getCentreY() - trackH * 0.5f;
            auto trackBounds = juce::Rectangle<float>(bounds.getX() + 4, trackY, bounds.getWidth() - 8, trackH);
            
            // Track background with 3D inset effect
            g.setColour(juce::Colour::fromRGB(15, 15, 18));
            g.fillRoundedRectangle(trackBounds.expanded(1), 3.0f);
            
            juce::ColourGradient trackGrad(juce::Colour::fromRGB(25, 28, 32), 0, trackBounds.getY(),
                                           juce::Colour::fromRGB(18, 20, 24), 0, trackBounds.getBottom(), false);
            g.setGradientFill(trackGrad);
            g.fillRoundedRectangle(trackBounds, 2.5f);
            
            // Track lit portion (left of fader) with theme accent
            float faderX = sliderPos;
            auto litPortion = juce::Rectangle<float>(trackBounds.getX(), trackBounds.getY(), 
                                                     faderX - trackBounds.getX(), trackH);
            juce::ColourGradient litGrad(t.accent.darker(0.2f), 0, litPortion.getY(),
                                         t.accent.brighter(0.1f), 0, litPortion.getBottom(), false);
            g.setGradientFill(litGrad);
            g.fillRoundedRectangle(litPortion, 2.0f);
            
            // Fader cap (thumb) - horizontal orientation
            float faderW = 12.0f;
            float faderH = 20.0f;
            auto faderBounds = juce::Rectangle<float>(faderX - faderW * 0.5f, bounds.getCentreY() - faderH * 0.5f, faderW, faderH);
            
            // Fader body with metallic gradient
            juce::ColourGradient faderGrad(juce::Colour::fromRGB(90, 95, 105), faderBounds.getX(), faderBounds.getY(),
                                           juce::Colour::fromRGB(45, 50, 58), faderBounds.getRight(), faderBounds.getY(), false);
            g.setGradientFill(faderGrad);
            g.fillRoundedRectangle(faderBounds, 3.0f);
            
            // Fader edge highlight
            g.setColour(juce::Colour::fromRGB(110, 115, 125));
            g.drawRoundedRectangle(faderBounds, 3.0f, 1.0f);
            
            // Center grip line with accent color (vertical for horizontal slider)
            g.setColour(t.accent);
            g.fillRect(faderBounds.getCentreX() - 1, faderBounds.getCentreY() - 5, 2.0f, 10.0f);
            
            // Top shine on fader
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.fillRoundedRectangle(faderBounds.getX() + 2, faderBounds.getY() + 1, faderBounds.getWidth() - 4, 3.0f, 1.5f);
        }
        else
        {
            // === VERTICAL FADER STYLE ===
            float trackW = 8.0f;
            float trackX = bounds.getCentreX() - trackW * 0.5f;
            auto trackBounds = juce::Rectangle<float>(trackX, bounds.getY() + 4, trackW, bounds.getHeight() - 8);
            
            // Track background with 3D inset effect
            g.setColour(juce::Colour::fromRGB(15, 15, 18));
            g.fillRoundedRectangle(trackBounds.expanded(1), 3.0f);
            
            juce::ColourGradient trackGrad(juce::Colour::fromRGB(25, 28, 32), trackBounds.getX(), 0,
                                           juce::Colour::fromRGB(18, 20, 24), trackBounds.getRight(), 0, false);
            g.setGradientFill(trackGrad);
            g.fillRoundedRectangle(trackBounds, 2.5f);
            
            // Track lit portion (below fader) with theme accent
            float faderY = sliderPos;
            auto litPortion = juce::Rectangle<float>(trackBounds.getX(), faderY, trackW, trackBounds.getBottom() - faderY);
            juce::ColourGradient litGrad(t.accent.darker(0.2f), litPortion.getX(), 0,
                                         t.accent.brighter(0.1f), litPortion.getRight(), 0, false);
            g.setGradientFill(litGrad);
            g.fillRoundedRectangle(litPortion, 2.0f);
            
            // Fader cap (thumb)
            float faderW = 26.0f;
            float faderH = 14.0f;
            auto faderBounds = juce::Rectangle<float>(bounds.getCentreX() - faderW * 0.5f, faderY - faderH * 0.5f, faderW, faderH);
            
            // Fader body with metallic gradient
            juce::ColourGradient faderGrad(juce::Colour::fromRGB(90, 95, 105), faderBounds.getX(), faderBounds.getY(),
                                           juce::Colour::fromRGB(45, 50, 58), faderBounds.getX(), faderBounds.getBottom(), false);
            g.setGradientFill(faderGrad);
            g.fillRoundedRectangle(faderBounds, 3.0f);
            
            // Fader edge highlight
            g.setColour(juce::Colour::fromRGB(110, 115, 125));
            g.drawRoundedRectangle(faderBounds, 3.0f, 1.0f);
            
            // Center grip line with accent color
            g.setColour(t.accent);
            g.fillRect(faderBounds.getCentreX() - 6, faderBounds.getCentreY() - 1, 12.0f, 2.0f);
            
            // Top shine on fader
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.fillRoundedRectangle(faderBounds.getX() + 2, faderBounds.getY() + 1, faderBounds.getWidth() - 4, 3.0f, 1.5f);
        }
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour&,
                              bool isOver, bool isDown) override
    {
        if (!themePtr) return;
        auto& t = *themePtr;
        auto r = btn.getLocalBounds().toFloat().reduced(1.0f);
        auto base = isDown ? t.accent.withAlpha(0.3f) : (isOver ? t.bgPanel.brighter(0.15f) : t.bgPanel);
        if (isOver) { g.setColour(t.accent.withAlpha(0.12f)); g.fillRoundedRectangle(r.expanded(2), 8.0f); }
        g.setColour(base);
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(t.accent.withAlpha(isOver ? 0.7f : 0.35f));
        g.drawRoundedRectangle(r, 6.0f, 1.2f);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool highlighted, bool) override
    {
        if (!themePtr) return;
        auto& t = *themePtr;
        auto bounds = btn.getLocalBounds().toFloat();
        auto toggleSize = 20.0f;
        auto toggleBounds = bounds.removeFromLeft(toggleSize + 6).withSizeKeepingCentre(toggleSize, toggleSize);

        g.setColour(t.bgDark);
        g.fillRoundedRectangle(toggleBounds, 4.0f);
        bool on = btn.getToggleState();
        if (on) { g.setColour(t.meterGreen.withAlpha(0.25f)); g.fillRoundedRectangle(toggleBounds.expanded(2), 6.0f); }
        g.setColour(on ? t.meterGreen : t.meterRed.withAlpha(0.6f));
        g.fillRoundedRectangle(toggleBounds.reduced(4), 2.0f);
        g.setColour(t.accent.withAlpha(0.4f));
        g.drawRoundedRectangle(toggleBounds, 4.0f, 1.0f);
        g.setColour(t.textBright);
        g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
        g.drawText(btn.getButtonText(), bounds.reduced(4, 0), juce::Justification::centredLeft, true);
    }

    void drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&) override
    {
        if (!themePtr) return;
        auto& t = *themePtr;
        auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(w), static_cast<float>(h));
        g.setColour(t.bgPanel);
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour(t.accent.withAlpha(0.35f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 5.0f, 1.0f);
        auto arr = bounds.removeFromRight(static_cast<float>(h)).reduced(7.0f);
        juce::Path arrow;
        arrow.addTriangle(arr.getX(), arr.getCentreY() - 2, arr.getRight(), arr.getCentreY() - 2, arr.getCentreX(), arr.getCentreY() + 4);
        g.setColour(t.accent);
        g.fillPath(arrow);
    }
};

SimpleGainAudioProcessorEditor::SimpleGainAudioProcessorEditor(SimpleGainAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      gainAttachment(processor.getValueTreeState(), "gain", gainSlider),
      vuMeter(*this), phaseMeter(*this), grMeter(*this), ioMeter(*this),
      freqBalance(*this), advancedMeters(*this), quickIssues(*this), satellitePanel(*this)
{
    // Enable mouse event interception so we can handle drag/resize globally
    setInterceptsMouseClicks(true, true);

    // Global proxy removed to avoid duplicate event forwarding — child components forward events themselves.
    aresLaf = std::make_unique<AresLookAndFeel>();
    aresLaf->themePtr = &theme;
    aresLaf->updateColours();
    setLookAndFeel(aresLaf.get());

    auto& apvts = processor.getValueTreeState();
    // Remove references to the old IN/OUT control (vuModeButton) so it is no longer part of forwardables or layout

    // Navigation tabs - ensure they're always clickable above all content
    mainTabBtn.setButtonText("Main");
    settingsTabBtn.setButtonText("Settings");
    chatTabBtn.setButtonText("Assistant");
    mainTabBtn.onClick = [this] { currentTab = 0; showMainView(); resized(); repaint(); };
    settingsTabBtn.onClick = [this] { currentTab = 1; showSettingsView(); resized(); repaint(); };
    chatTabBtn.onClick = [this] { currentTab = 2; showChatView(); resized(); repaint(); };
    mainTabBtn.setAlwaysOnTop(true);
    settingsTabBtn.setAlwaysOnTop(true);
    chatTabBtn.setAlwaysOnTop(true);
    addAndMakeVisible(mainTabBtn);
    addAndMakeVisible(settingsTabBtn);
    addAndMakeVisible(chatTabBtn);

    // Header toggles: Edit Mode and Auto-fit
    editModeToggle.setClickingTogglesState(true);
    editModeToggle.setToggleState(editMode, juce::dontSendNotification);
    editModeToggle.onClick = [this] {
        editMode = editModeToggle.getToggleState();
        // When entering/exiting edit mode, re-layout as necessary
        if (!editMode && autoFitMode)
            applyAutoLayout();
        resized();
        saveLayoutToFile();
    };
    addAndMakeVisible(editModeToggle);

    autoFitToggle.setClickingTogglesState(true);
    autoFitToggle.setToggleState(autoFitMode, juce::dontSendNotification);
    autoFitToggle.onClick = [this] {
        autoFitMode = autoFitToggle.getToggleState();
        if (autoFitMode && !editMode)
            applyAutoLayout();
        resized();
        saveLayoutToFile();
    };
    addAndMakeVisible(autoFitToggle);

    // Keep header controls on top
    editModeToggle.setAlwaysOnTop(true);
    autoFitToggle.setAlwaysOnTop(true);

    // === MAIN VIEW ===
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 22);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setNumDecimalPlacesToDisplay(1);
    gainLabel.setText("GAIN", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    
    // Add a listener to sync gain changes to all satellites
    gainSlider.addListener(this);

    targetDbSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    targetDbSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 22);
    targetDbSlider.setTextValueSuffix(" dB");
    targetDbSlider.setNumDecimalPlacesToDisplay(1);
    targetDbLabel.setText("TARGET", juce::dontSendNotification);
    targetDbLabel.setJustificationType(juce::Justification::centred);
    targetDbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "auto_target_db", targetDbSlider);
    
    // Add a listener to sync target changes to all satellites
    targetDbSlider.addListener(this);

    ceilingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ceilingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 22);
    ceilingSlider.setTextValueSuffix(" dB");
    ceilingSlider.setNumDecimalPlacesToDisplay(1);
    ceilingLabel.setText("MAX PEAK", juce::dontSendNotification);
    ceilingLabel.setJustificationType(juce::Justification::centred);
    ceilingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "ceiling", ceilingSlider);
    
    // Add a listener to sync ceiling changes to satellites
    ceilingSlider.addListener(this);

    autoToggle.setButtonText("Auto Gain");
    autoToggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "auto_enabled", autoToggle);

    riderToggle.setButtonText("Vocal Rider");
    riderToggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "rider_enabled", riderToggle);
    
    // Standalone mode toggle - allows source dropdown on each instance
    standaloneModeToggle.setButtonText("Standalone Mode");
    standaloneModeToggle.onClick = [this] {
        standaloneMode = standaloneModeToggle.getToggleState();
        if (currentTab == 0)
            showMainView();
        resized();
    };
    addAndMakeVisible(standaloneModeToggle);

    riderAmountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    riderAmountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 22);
    riderAmountSlider.setTextValueSuffix("%");
    riderAmountSlider.setNumDecimalPlacesToDisplay(0);
    riderAmountLabel.setText("Rider", juce::dontSendNotification);
    riderAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "rider_amount", riderAmountSlider);

    genreLabel.setText("Genre", juce::dontSendNotification);
    sourceLabel.setText("Source", juce::dontSendNotification);
    situationLabel.setText("Stage", juce::dontSendNotification);

    genreBox.addItemList({ "Pop", "Rock", "Hip-Hop", "R&B", "Trap", "Reggaeton", "EDM", "Jazz", "Classical", "Podcast", "Lo-Fi", "Metal", "Country", "Other" }, 1);
    sourceBox.addItemList({ "Lead Vocal", "Background Vocal", "Kick", "Snare", "Hi-Hat", "Full Drums", "Bass", "Electric Guitar", "Acoustic Guitar", "Keys/Piano", "Synth", "Strings", "Mix Bus", "Master" }, 1);
    situationBox.addItemList({ "Tracking", "Editing", "Mixing", "Mastering" }, 1);

    genreAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "genre", genreBox);
    sourceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "source", sourceBox);
    situationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "situation", situationBox);
    
    // Platform loudness targets - LUFS-based
    platformTargetLabel.setText("LUFS Target", juce::dontSendNotification);
    platformTargetBox.addItem("Off", 1);
    platformTargetBox.addItem("Spotify (-14 LUFS)", 2);
    platformTargetBox.addItem("Apple Music (-16 LUFS)", 3);
    platformTargetBox.addItem("YouTube (-14 LUFS)", 4);
    platformTargetBox.addItem("Amazon (-14 LUFS)", 5);
    platformTargetBox.addItem("Tidal (-14 LUFS)", 6);
    platformTargetBox.addItem("CD Master (-9 LUFS)", 7);
    platformTargetBox.addItem("Broadcast (-24 LUFS)", 8);
    platformTargetBox.addItem("Podcast (-16 LUFS)", 9);
    platformTargetBox.addItem("Club/DJ (-6 LUFS)", 10);
    platformTargetBox.setSelectedId(1);
    platformTargetBox.onChange = [this] {
        int selection = platformTargetBox.getSelectedId();
        
        if (selection == 1)  // Off
        {
            // Disable LUFS mode
            if (auto* p = processor.getValueTreeState().getParameter("lufs_enabled"))
                p->setValueNotifyingHost(0.0f);
            return;
        }
        
        // Enable LUFS mode and set target
        float targetLufs = -14.0f;
        switch (selection) {
            case 2: targetLufs = -14.0f; break;  // Spotify
            case 3: targetLufs = -16.0f; break;  // Apple
            case 4: targetLufs = -14.0f; break;  // YouTube
            case 5: targetLufs = -14.0f; break;  // Amazon
            case 6: targetLufs = -14.0f; break;  // Tidal
            case 7: targetLufs = -9.0f; break;   // CD
            case 8: targetLufs = -24.0f; break;  // Broadcast
            case 9: targetLufs = -16.0f; break;  // Podcast
            case 10: targetLufs = -6.0f; break;  // Club
            default: return;
        }
        
        // Enable LUFS mode
        if (auto* p = processor.getValueTreeState().getParameter("lufs_enabled"))
            p->setValueNotifyingHost(1.0f);
        
        // Set LUFS target
        if (auto* p = processor.getValueTreeState().getParameter("lufs_target"))
            p->setValueNotifyingHost(p->convertTo0to1(targetLufs));
    };
    addAndMakeVisible(platformTargetBox);
    addAndMakeVisible(platformTargetLabel);

    autoSetButton.setButtonText("Auto-Set");
    autoSetButton.onClick = [this] { processor.autoSetGainFromAnalysis(); };

    aiSuggestButton.setButtonText("Ask AI");
    aiSuggestButton.onClick = [this] {
        processor.requestAiSuggestion(genreBox.getText(), sourceBox.getText(), situationBox.getText());
    };

    applyAiButton.setButtonText("Apply AI");
    applyAiButton.onClick = [this] { processor.applyAiRecommendation(); };

    analysisLabel.setText("Waiting for audio...", juce::dontSendNotification);
    aiStatusLabel.setText("Status: Ready", juce::dontSendNotification);
    aiNotes.setReadOnly(true);
    aiNotes.setMultiLine(true);
    aiNotes.setScrollbarsShown(true);
    aiNotes.setText("AI recommendations appear here...");

    // === SETTINGS VIEW ===
    aiProviderLabel.setText("AI Provider", juce::dontSendNotification);
    aiProviderBox.addItemList({ "Ollama (Local)", "OpenAI", "Anthropic", "OpenRouter", "MiniMax" }, 1);
    
    // Load saved provider
    auto savedProvider = static_cast<int>(processor.getAiProvider()) + 1;
    aiProviderBox.setSelectedId(savedProvider, juce::dontSendNotification);
    
    aiProviderBox.onChange = [this] {
        int providerIndex = aiProviderBox.getSelectedId() - 1;
        processor.setAiProvider(static_cast<SimpleGainAudioProcessor::AiProvider>(providerIndex));
        
        bool needsKey = aiProviderBox.getSelectedId() > 1;
        apiKeyEditor.setVisible(needsKey);
        apiKeyLabel.setVisible(needsKey);
        
        // Refresh models for the new provider
        processor.refreshModelList();
    };

    modelLabel.setText("Model", juce::dontSendNotification);
    modelBox.setTextWhenNothingSelected("Select model");
    modelBox.addItemList(processor.getAvailableModels(), 1);
    
    // Select saved model if available
    auto savedModel = processor.getSelectedModel();
    auto models = processor.getAvailableModels();
    int modelIdx = models.indexOf(savedModel);
    if (modelIdx >= 0)
        modelBox.setSelectedId(modelIdx + 1, juce::dontSendNotification);
    else if (modelBox.getNumItems() > 0)
        modelBox.setSelectedId(1, juce::dontSendNotification);
    
    modelBox.onChange = [this] {
        if (modelBox.getSelectedId() > 0)
            processor.setSelectedModel(modelBox.getText());
    };

    apiKeyLabel.setText("API Key", juce::dontSendNotification);
    apiKeyEditor.setPasswordCharacter('*');
    apiKeyEditor.setTextToShowWhenEmpty("Enter API key...", juce::Colours::grey);
    
    // Load saved API key
    auto savedKey = processor.getApiKey();
    if (savedKey.isNotEmpty())
        apiKeyEditor.setText(savedKey, juce::dontSendNotification);
    
    // Show/hide API key based on provider
    bool needsKey = savedProvider > 1;
    apiKeyEditor.setVisible(needsKey);
    apiKeyLabel.setVisible(needsKey);
    
    apiKeyEditor.onTextChange = [this] {
        processor.setApiKey(apiKeyEditor.getText());
        // Refresh models when API key changes
        processor.refreshModelList();
    };

    refreshModelsButton.setButtonText("Refresh Models");
    refreshModelsButton.onClick = [this] { processor.refreshModelList(); };

    themeLabel.setText("Theme", juce::dontSendNotification);
    themeBox.addItemList(getThemeNames(), 1);
    themeBox.setSelectedId(5, juce::dontSendNotification);  // SleekBlack default (darkest)
    currentThemeIndex = 4;  // 0-based index (SleekBlack)
    themeBox.onChange = [this] { 
        currentThemeIndex = themeBox.getSelectedId() - 1; 
        applyTheme(); 
        // Sync theme to shared memory for satellites
        processor.setThemeIndex(currentThemeIndex);
    };

    knobStyleLabel.setText("Knob Style", juce::dontSendNotification);
    knobStyleBox.addItemList({ "Modern Arc", "Classic Pointer", "Minimal Dot", "Retro Dial", "LED Ring", "Flat Modern" }, 1);
    knobStyleBox.setSelectedId(1, juce::dontSendNotification);
    knobStyleBox.onChange = [this] { 
        currentKnobStyle = knobStyleBox.getSelectedId() - 1; 
        aresLaf->knobStyle = currentKnobStyle; 
        processor.setKnobStyle(currentKnobStyle);  // Sync to satellites
        repaint(); 
    };

    accentColorLabel.setText("Accent Hue", juce::dontSendNotification);
    accentHueSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    accentHueSlider.setRange(0, 360, 1);
    accentHueSlider.setValue(0);
    accentHueSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    accentHueSlider.onValueChange = [this] { applyTheme(); };

    bgColorLabel.setText("BG Brightness", juce::dontSendNotification);
    bgBrightnessSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bgBrightnessSlider.setRange(0.0, 0.15, 0.01);
    bgBrightnessSlider.setValue(0.05);
    bgBrightnessSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    bgBrightnessSlider.onValueChange = [this] { applyTheme(); };

    saveSettingsBtn.setButtonText("Save Settings");
    saveSettingsBtn.onClick = [this] { 
        processor.saveSettings(); 
        // Visual feedback
        saveSettingsBtn.setButtonText("Saved!");
        juce::Timer::callAfterDelay(1500, [this]() {
            if (saveSettingsBtn.isShowing())
                saveSettingsBtn.setButtonText("Save Settings");
        });
    };

    saveLayoutBtn.setButtonText("Save Layout");
    saveLayoutBtn.onClick = [this] { 
        saveLayoutToFile();
        // Visual feedback
        saveLayoutBtn.setButtonText("Layout Saved!");
        juce::Timer::callAfterDelay(1500, [this]() {
            if (saveLayoutBtn.isShowing())
                saveLayoutBtn.setButtonText("Save Layout");
        });
    };

    loadLayoutBtn.setButtonText("Load Layout");
    loadLayoutBtn.onClick = [this] { 
        loadLayoutFromFile();
        // Visual feedback
        loadLayoutBtn.setButtonText("Layout Loaded!");
        juce::Timer::callAfterDelay(1500, [this]() {
            if (loadLayoutBtn.isShowing())
                loadLayoutBtn.setButtonText("Load Layout");
        });
    };

    // === CHAT VIEW ===
    chatLabel.setText("ARES Assistant - Your AI Mixing Engineer", juce::dontSendNotification);
    chatHistory.setReadOnly(true);
    chatHistory.setMultiLine(true);
    chatHistory.setScrollbarsShown(true);
    chatHistory.setText("Welcome to ARES Assistant!\n\nI'm your AI mixing engineer. Ask me about:\n\n- Plugin recommendations for your tracks\n- EQ, compression, reverb settings\n- Signal chain and processing order\n- Mixing techniques for any instrument\n- Mastering chain suggestions\n- Genre-specific production tips\n- Your current levels and gear\n\nType your question below...\n\n");
    chatInput.setMultiLine(false);
    chatInput.setTextToShowWhenEmpty("Type your question here...", juce::Colours::grey);
    chatInput.onReturnKey = [this] { sendChatMessage(); };
    sendChatBtn.setButtonText("Send");
    sendChatBtn.onClick = [this] { sendChatMessage(); };

    // Add all main view components
    addAndMakeVisible(gainSlider); addAndMakeVisible(gainLabel);
    addAndMakeVisible(targetDbSlider); addAndMakeVisible(targetDbLabel);
    addAndMakeVisible(ceilingSlider); addAndMakeVisible(ceilingLabel);
    addAndMakeVisible(autoToggle); addAndMakeVisible(riderToggle);
    addAndMakeVisible(riderAmountSlider); addAndMakeVisible(riderAmountLabel);
    addAndMakeVisible(genreBox); addAndMakeVisible(genreLabel);
    addAndMakeVisible(sourceBox); addAndMakeVisible(sourceLabel);
    addAndMakeVisible(situationBox); addAndMakeVisible(situationLabel);
    addAndMakeVisible(autoSetButton);
    addAndMakeVisible(aiSuggestButton); addAndMakeVisible(applyAiButton);
    addAndMakeVisible(analysisLabel); addAndMakeVisible(aiStatusLabel);
    addAndMakeVisible(aiNotes);
    
    // Add movable meter components
    addAndMakeVisible(vuMeter);
    // Give components friendly names for debugging/logging
    vuMeter.setName("vuMeter");
    phaseMeter.setName("phaseMeter");
    grMeter.setName("grMeter");
    ioMeter.setName("ioMeter");
    freqBalance.setName("freqBalance");
    advancedMeters.setName("advancedMeters");
    quickIssues.setName("quickIssues");
    satellitePanel.setName("satellitePanel");

    // Name main controls
    gainSlider.setName("gainSlider");
    targetDbSlider.setName("targetDbSlider");
    ceilingSlider.setName("ceilingSlider");
    meterModeButton.setName("meterModeButton");
    vuModeButton.setName("vuModeButton");
    vuModeButton.setVisible(false); // removed control — keep hidden
    addAndMakeVisible(phaseMeter);
    addAndMakeVisible(grMeter);
    addAndMakeVisible(ioMeter);
    addAndMakeVisible(freqBalance);
    addAndMakeVisible(advancedMeters);
    addAndMakeVisible(quickIssues);
    addAndMakeVisible(satellitePanel);
    
    // Pre/Post meter toggle button
    meterModeButton.setButtonText("POST");
    meterModeButton.onClick = [this] {
        showPreMeter = !showPreMeter;
        meterModeButton.setButtonText(showPreMeter ? "PRE" : "POST");
        repaint();
    };
    addAndMakeVisible(meterModeButton);
    
    // VU IN/OUT control removed — use single VU display only.

    // Add settings components
    addChildComponent(aiProviderBox); addChildComponent(aiProviderLabel);
    addChildComponent(modelBox); addChildComponent(modelLabel);
    addChildComponent(apiKeyEditor); addChildComponent(apiKeyLabel);
    addChildComponent(refreshModelsButton); addChildComponent(themeBox);
    addChildComponent(themeLabel); addChildComponent(knobStyleBox);
    addChildComponent(knobStyleLabel); addChildComponent(accentColorLabel);
    addChildComponent(accentHueSlider); addChildComponent(bgColorLabel);
    addChildComponent(bgBrightnessSlider); addChildComponent(saveSettingsBtn);
    addChildComponent(saveLayoutBtn); addChildComponent(loadLayoutBtn);
    
    // Equipment settings for AI context
    micLabel.setText("Microphone", juce::dontSendNotification);
    micBox.addItem("Not Specified", 1);
    // Shure
    micBox.addItem("Shure SM7B", 2);
    micBox.addItem("Shure SM58", 3);
    micBox.addItem("Shure SM57", 4);
    micBox.addItem("Shure SM27", 5);
    micBox.addItem("Shure KSM32", 6);
    micBox.addItem("Shure KSM44A", 7);
    micBox.addItem("Shure KSM8", 8);
    micBox.addItem("Shure Beta 58A", 9);
    micBox.addItem("Shure Beta 87A", 10);
    micBox.addItem("Shure MV7", 11);
    micBox.addItem("Shure MV7+", 12);
    micBox.addItem("Shure SM7dB", 13);
    // Neumann
    micBox.addItem("Neumann U87", 14);
    micBox.addItem("Neumann U47", 15);
    micBox.addItem("Neumann TLM 103", 16);
    micBox.addItem("Neumann TLM 102", 17);
    micBox.addItem("Neumann TLM 49", 18);
    micBox.addItem("Neumann TLM 67", 19);
    micBox.addItem("Neumann M149", 20);
    micBox.addItem("Neumann U67", 21);
    // AKG
    micBox.addItem("AKG C414 XLII", 22);
    micBox.addItem("AKG C414 XLS", 23);
    micBox.addItem("AKG C12", 24);
    micBox.addItem("AKG C214", 25);
    micBox.addItem("AKG C3000", 26);
    micBox.addItem("AKG C451", 27);
    micBox.addItem("AKG P220", 28);
    // Electro-Voice
    micBox.addItem("EV RE20", 29);
    micBox.addItem("EV RE320", 30);
    micBox.addItem("EV RE27N/D", 31);
    // Sennheiser
    micBox.addItem("Sennheiser MD421", 32);
    micBox.addItem("Sennheiser MD441", 33);
    micBox.addItem("Sennheiser MKH 416", 34);
    micBox.addItem("Sennheiser e835", 35);
    micBox.addItem("Sennheiser e945", 36);
    // Rode
    micBox.addItem("Rode NT1 5th Gen", 37);
    micBox.addItem("Rode NT1-A", 38);
    micBox.addItem("Rode NT2-A", 39);
    micBox.addItem("Rode PodMic", 40);
    micBox.addItem("Rode PodMic USB", 41);
    micBox.addItem("Rode Procaster", 42);
    micBox.addItem("Rode NTK", 43);
    micBox.addItem("Rode K2", 44);
    micBox.addItem("Rode NT5", 45);
    micBox.addItem("Rode VideoMic Pro", 46);
    // Audio-Technica
    micBox.addItem("Audio-Technica AT2020", 47);
    micBox.addItem("Audio-Technica AT2035", 48);
    micBox.addItem("Audio-Technica AT4040", 49);
    micBox.addItem("Audio-Technica AT4050", 50);
    micBox.addItem("Audio-Technica AT4053b", 51);
    micBox.addItem("Audio-Technica ATR2100x", 52);
    micBox.addItem("Audio-Technica AT2020USB+", 53);
    // Warm Audio
    micBox.addItem("Warm Audio WA-47", 54);
    micBox.addItem("Warm Audio WA-47jr", 55);
    micBox.addItem("Warm Audio WA-87 R2", 56);
    micBox.addItem("Warm Audio WA-14", 57);
    micBox.addItem("Warm Audio WA-251", 58);
    micBox.addItem("Warm Audio WA-8000", 59);
    // Aston
    micBox.addItem("Aston Origin", 60);
    micBox.addItem("Aston Spirit", 61);
    micBox.addItem("Aston Stealth", 62);
    // Telefunken
    micBox.addItem("Telefunken U47", 63);
    micBox.addItem("Telefunken CU-29", 64);
    micBox.addItem("Telefunken M80", 65);
    // Other condensers
    micBox.addItem("MXL 990", 66);
    micBox.addItem("MXL V67G", 67);
    micBox.addItem("Blue Yeti", 68);
    micBox.addItem("Blue Yeti X", 69);
    micBox.addItem("Blue Spark", 70);
    micBox.addItem("Blue Baby Bottle", 71);
    micBox.addItem("Manley Reference", 72);
    micBox.addItem("Slate ML-1", 73);
    micBox.addItem("Lewitt LCT 440 Pure", 74);
    micBox.addItem("Lewitt LCT 540 Subzero", 75);
    micBox.addItem("sE Electronics X1", 76);
    micBox.addItem("sE Electronics 2200a", 77);
    micBox.addItem("Mojave MA-200", 78);
    // Ribbon mics
    micBox.addItem("Royer R-121", 79);
    micBox.addItem("Royer R-122", 80);
    micBox.addItem("AEA R84", 81);
    micBox.addItem("AEA R88", 82);
    micBox.addItem("Coles 4038", 83);
    micBox.addItem("Beyerdynamic M160", 84);
    // USB/Podcast
    micBox.addItem("Elgato Wave:3", 85);
    micBox.addItem("HyperX QuadCast", 86);
    micBox.addItem("Samson Q2U", 87);
    micBox.addItem("Samson C01", 88);
    // Generic categories
    micBox.addItem("Other Condenser", 89);
    micBox.addItem("Other Dynamic", 90);
    micBox.addItem("Other Ribbon", 91);
    micBox.addItem("Other USB Mic", 92);
    // Custom option - must be last
    micBox.addItem("Custom...", 100);
    micBox.setSelectedId(1);
    
    // Custom mic text editor
    customMicEditor.setMultiLine(false);
    customMicEditor.setTextToShowWhenEmpty("Enter custom mic name...", theme.textDim);
    customMicEditor.onReturnKey = [this] { 
        processor.setSelectedMic(customMicEditor.getText());
    };
    customMicEditor.onFocusLost = [this] {
        if (customMicEditor.getText().isNotEmpty())
            processor.setSelectedMic(customMicEditor.getText());
    };
    addChildComponent(customMicEditor);
    
    addChildComponent(micBox); addChildComponent(micLabel);
    
    preampLabel.setText("Preamp", juce::dontSendNotification);
    preampBox.addItem("Not Specified", 1);
    preampBox.addItem("Interface Built-in", 2);
    // Neve style
    preampBox.addItem("Neve 1073", 3);
    preampBox.addItem("Neve 1073LB", 4);
    preampBox.addItem("Neve 1073LBQ", 5);
    preampBox.addItem("Neve 1084", 6);
    preampBox.addItem("Neve 1081", 7);
    // API style
    preampBox.addItem("API 512", 8);
    preampBox.addItem("API 512c", 9);
    preampBox.addItem("API 312", 10);
    preampBox.addItem("API 3124+", 11);
    preampBox.addItem("API The Channel Strip", 12);
    // SSL
    preampBox.addItem("SSL VHD", 13);
    preampBox.addItem("SSL Alpha Channel", 14);
    preampBox.addItem("SSL SiX", 15);
    preampBox.addItem("SSL Fusion", 16);
    // Universal Audio
    preampBox.addItem("UA 610", 17);
    preampBox.addItem("UA Solo/610", 18);
    preampBox.addItem("UA 710 Twin-Finity", 19);
    preampBox.addItem("UA LA-610 MkII", 20);
    preampBox.addItem("UA 4-710d", 21);
    preampBox.addItem("UA SP-1", 22);
    // Warm Audio
    preampBox.addItem("Warm Audio WA73", 23);
    preampBox.addItem("Warm Audio WA73-EQ", 24);
    preampBox.addItem("Warm Audio WA12 MkII", 25);
    preampBox.addItem("Warm Audio WA-2A", 26);
    preampBox.addItem("Warm Audio TB12", 27);
    preampBox.addItem("Warm Audio WA76", 28);
    preampBox.addItem("Warm Audio WA-MPX", 29);
    // Focusrite
    preampBox.addItem("Focusrite ISA One", 30);
    preampBox.addItem("Focusrite ISA 428", 31);
    preampBox.addItem("Focusrite ISA Two", 32);
    // Golden Age
    preampBox.addItem("Golden Age Pre-73 MKIV", 33);
    preampBox.addItem("Golden Age Pre-73 Premier", 34);
    preampBox.addItem("Golden Age Pre-573 MKII", 35);
    // Gain boosters
    preampBox.addItem("Cloudlifter CL-1", 36);
    preampBox.addItem("Cloudlifter CL-2", 37);
    preampBox.addItem("Cloudlifter CL-Z", 38);
    preampBox.addItem("Fethead", 39);
    preampBox.addItem("sE Dynamite DM1", 40);
    // Budget/Prosumer
    preampBox.addItem("DBX 286s", 41);
    preampBox.addItem("DBX 286A", 42);
    preampBox.addItem("Art TubeMP", 43);
    preampBox.addItem("Art TubeMP Studio", 44);
    preampBox.addItem("Art Voice Channel", 45);
    preampBox.addItem("PreSonus BlueTube DP", 46);
    // High-end
    preampBox.addItem("Grace Design m101", 47);
    preampBox.addItem("Grace Design m201", 48);
    preampBox.addItem("BAE 1073", 49);
    preampBox.addItem("BAE 1084", 50);
    preampBox.addItem("Chandler TG2", 51);
    preampBox.addItem("Chandler REDD.47", 52);
    preampBox.addItem("Rupert Neve Designs 511", 53);
    preampBox.addItem("Rupert Neve Designs Shelford", 54);
    preampBox.addItem("Avalon VT-737sp", 55);
    preampBox.addItem("Avalon M5", 56);
    preampBox.addItem("Manley VoxBox", 57);
    preampBox.addItem("Millennia HV-3C", 58);
    preampBox.addItem("Great River ME-1NV", 59);
    preampBox.addItem("Vintech X73i", 60);
    // Generic categories
    preampBox.addItem("Other Tube", 61);
    preampBox.addItem("Other Solid State", 62);
    preampBox.addItem("Other Channel Strip", 63);
    // Custom option - must be last
    preampBox.addItem("Custom...", 100);
    preampBox.setSelectedId(1);
    
    // Custom preamp text editor
    customPreampEditor.setMultiLine(false);
    customPreampEditor.setTextToShowWhenEmpty("Enter custom preamp name...", theme.textDim);
    customPreampEditor.onReturnKey = [this] { 
        processor.setSelectedPreamp(customPreampEditor.getText());
    };
    customPreampEditor.onFocusLost = [this] {
        if (customPreampEditor.getText().isNotEmpty())
            processor.setSelectedPreamp(customPreampEditor.getText());
    };
    addChildComponent(customPreampEditor);
    
    addChildComponent(preampBox); addChildComponent(preampLabel);
    
    interfaceLabel.setText("Audio Interface", juce::dontSendNotification);
    interfaceBox.addItem("Not Specified", 1);
    // Universal Audio Apollo
    interfaceBox.addItem("UA Apollo Twin", 2);
    interfaceBox.addItem("UA Apollo Twin X", 3);
    interfaceBox.addItem("UA Apollo Twin X Duo", 4);
    interfaceBox.addItem("UA Apollo Twin X Quad", 5);
    interfaceBox.addItem("UA Apollo x4", 6);
    interfaceBox.addItem("UA Apollo x6", 7);
    interfaceBox.addItem("UA Apollo x8", 8);
    interfaceBox.addItem("UA Apollo x8p", 9);
    interfaceBox.addItem("UA Apollo x16", 10);
    interfaceBox.addItem("UA Apollo Solo", 11);
    interfaceBox.addItem("UA Apollo Solo USB", 12);
    interfaceBox.addItem("UA Volt 1", 13);
    interfaceBox.addItem("UA Volt 2", 14);
    interfaceBox.addItem("UA Volt 176", 15);
    interfaceBox.addItem("UA Volt 276", 16);
    interfaceBox.addItem("UA Volt 476", 17);
    interfaceBox.addItem("UA Volt 476P", 18);
    // Focusrite
    interfaceBox.addItem("Focusrite Scarlett Solo", 19);
    interfaceBox.addItem("Focusrite Scarlett 2i2", 20);
    interfaceBox.addItem("Focusrite Scarlett 4i4", 21);
    interfaceBox.addItem("Focusrite Scarlett 8i6", 22);
    interfaceBox.addItem("Focusrite Scarlett 18i8", 23);
    interfaceBox.addItem("Focusrite Scarlett 18i20", 24);
    interfaceBox.addItem("Focusrite Clarett+ 2Pre", 25);
    interfaceBox.addItem("Focusrite Clarett+ 4Pre", 26);
    interfaceBox.addItem("Focusrite Clarett+ 8Pre", 27);
    interfaceBox.addItem("Focusrite Vocaster One", 28);
    interfaceBox.addItem("Focusrite Vocaster Two", 29);
    // PreSonus
    interfaceBox.addItem("PreSonus Studio 24c", 30);
    interfaceBox.addItem("PreSonus Studio 26c", 31);
    interfaceBox.addItem("PreSonus Studio 68c", 32);
    interfaceBox.addItem("PreSonus Studio 1824c", 33);
    interfaceBox.addItem("PreSonus Quantum", 34);
    interfaceBox.addItem("PreSonus Revelator io24", 35);
    // RME
    interfaceBox.addItem("RME Babyface Pro FS", 36);
    interfaceBox.addItem("RME Fireface UFX II", 37);
    interfaceBox.addItem("RME UCX II", 38);
    interfaceBox.addItem("RME ADI-2 Pro", 39);
    interfaceBox.addItem("RME ADI-2 DAC", 40);
    // MOTU
    interfaceBox.addItem("MOTU M2", 41);
    interfaceBox.addItem("MOTU M4", 42);
    interfaceBox.addItem("MOTU M6", 43);
    interfaceBox.addItem("MOTU 828es", 44);
    interfaceBox.addItem("MOTU UltraLite mk5", 45);
    interfaceBox.addItem("MOTU 8pre-es", 46);
    // Audient
    interfaceBox.addItem("Audient iD4 MkII", 47);
    interfaceBox.addItem("Audient iD14 MkII", 48);
    interfaceBox.addItem("Audient iD24", 49);
    interfaceBox.addItem("Audient iD44 MkII", 50);
    interfaceBox.addItem("Audient EVO 4", 51);
    interfaceBox.addItem("Audient EVO 8", 52);
    interfaceBox.addItem("Audient EVO 16", 53);
    // SSL
    interfaceBox.addItem("SSL 2", 54);
    interfaceBox.addItem("SSL 2+", 55);
    interfaceBox.addItem("SSL UF8", 56);
    interfaceBox.addItem("SSL 12", 57);
    // Apogee
    interfaceBox.addItem("Apogee Duet 3", 58);
    interfaceBox.addItem("Apogee Symphony Desktop", 59);
    interfaceBox.addItem("Apogee Element 24", 60);
    interfaceBox.addItem("Apogee Boom", 61);
    // Antelope
    interfaceBox.addItem("Antelope Zen Go", 62);
    interfaceBox.addItem("Antelope Discrete 4", 63);
    interfaceBox.addItem("Antelope Discrete 8", 64);
    // Arturia
    interfaceBox.addItem("Arturia AudioFuse 8Pre", 65);
    interfaceBox.addItem("Arturia AudioFuse Studio", 66);
    interfaceBox.addItem("Arturia MiniFuse 1", 67);
    interfaceBox.addItem("Arturia MiniFuse 2", 68);
    // Steinberg
    interfaceBox.addItem("Steinberg UR22C", 69);
    interfaceBox.addItem("Steinberg UR24C", 70);
    interfaceBox.addItem("Steinberg UR44C", 71);
    interfaceBox.addItem("Steinberg AXR4T", 72);
    // Behringer
    interfaceBox.addItem("Behringer UMC202HD", 73);
    interfaceBox.addItem("Behringer UMC204HD", 74);
    interfaceBox.addItem("Behringer UMC404HD", 75);
    interfaceBox.addItem("Behringer UMC1820", 76);
    // Native Instruments
    interfaceBox.addItem("NI Komplete Audio 1", 77);
    interfaceBox.addItem("NI Komplete Audio 2", 78);
    interfaceBox.addItem("NI Komplete Audio 6 MkII", 79);
    // Other
    interfaceBox.addItem("Zoom UAC-2", 80);
    interfaceBox.addItem("Zoom LiveTrak L-8", 81);
    interfaceBox.addItem("Mackie Onyx Producer", 82);
    interfaceBox.addItem("Roland Rubix22", 83);
    interfaceBox.addItem("Roland Rubix24", 84);
    interfaceBox.addItem("Lewitt Connect 6", 85);
    // Podcast specific
    interfaceBox.addItem("Rodecaster Pro II", 86);
    interfaceBox.addItem("Rodecaster Duo", 87);
    interfaceBox.addItem("Zoom PodTrak P4", 88);
    interfaceBox.addItem("Zoom PodTrak P8", 89);
    interfaceBox.addItem("Tascam Mixcast 4", 90);
    // Generic
    interfaceBox.addItem("Other USB", 91);
    interfaceBox.addItem("Other Thunderbolt", 92);
    // Custom option - must be last
    interfaceBox.addItem("Custom...", 100);
    interfaceBox.setSelectedId(1);
    
    // Custom interface text editor
    customInterfaceEditor.setMultiLine(false);
    customInterfaceEditor.setTextToShowWhenEmpty("Enter custom interface name...", theme.textDim);
    customInterfaceEditor.onReturnKey = [this] { 
        processor.setSelectedInterface(customInterfaceEditor.getText());
    };
    customInterfaceEditor.onFocusLost = [this] {
        if (customInterfaceEditor.getText().isNotEmpty())
            processor.setSelectedInterface(customInterfaceEditor.getText());
    };
    addChildComponent(customInterfaceEditor);
    
    interfaceBox.onChange = [this] { 
        showCustomInterfaceEditor = (interfaceBox.getSelectedId() == 100);
        if (!showCustomInterfaceEditor)
            processor.setSelectedInterface(interfaceBox.getText()); 
        resized();
    };
    addChildComponent(interfaceBox); addChildComponent(interfaceLabel);
    
    // Restore equipment selections from saved settings
    auto savedMic = processor.getSelectedMic();
    auto savedPreamp = processor.getSelectedPreamp();
    auto savedInterface = processor.getSelectedInterface();
    
    // Find and select saved mic
    for (int i = 0; i < micBox.getNumItems(); ++i)
    {
        if (micBox.getItemText(i) == savedMic)
        {
            micBox.setSelectedId(micBox.getItemId(i), juce::dontSendNotification);
            break;
        }
    }
    micBox.onChange = [this] { 
        showCustomMicEditor = (micBox.getSelectedId() == 100);
        if (!showCustomMicEditor)
            processor.setSelectedMic(micBox.getText()); 
        resized();
    };
    
    // Find and select saved preamp
    for (int i = 0; i < preampBox.getNumItems(); ++i)
    {
        if (preampBox.getItemText(i) == savedPreamp)
        {
            preampBox.setSelectedId(preampBox.getItemId(i), juce::dontSendNotification);
            break;
        }
    }
    preampBox.onChange = [this] { 
        showCustomPreampEditor = (preampBox.getSelectedId() == 100);
        if (!showCustomPreampEditor)
            processor.setSelectedPreamp(preampBox.getText()); 
        resized();
    };
    
    // Find and select saved interface
    for (int i = 0; i < interfaceBox.getNumItems(); ++i)
    {
        if (interfaceBox.getItemText(i) == savedInterface)
        {
            interfaceBox.setSelectedId(interfaceBox.getItemId(i), juce::dontSendNotification);
            break;
        }
    }
    
    // Language settings
    languageLabel.setText("Language", juce::dontSendNotification);
    languageBox.addItemList(Localization::getLanguageNames(), 1);
    languageBox.setSelectedId(static_cast<int>(processor.getLanguage()) + 1, juce::dontSendNotification);
    languageBox.onChange = [this] { 
        auto lang = static_cast<Language>(languageBox.getSelectedId() - 1);
        processor.setLanguage(lang);
        Localization::getInstance().setLanguage(lang);
        updateLabels();  // Update all UI labels with new language
        repaint();
    };
    addChildComponent(languageBox); addChildComponent(languageLabel);

    // Add chat components
    addChildComponent(chatLabel); addChildComponent(chatHistory);
    addChildComponent(chatInput); addChildComponent(sendChatBtn);

    // Style labels
    for (auto* lbl : { &gainLabel, &targetDbLabel, &riderAmountLabel, &genreLabel, &sourceLabel,
                       &situationLabel,
                       &analysisLabel, &aiStatusLabel, &aiProviderLabel, &modelLabel, &apiKeyLabel,
                       &themeLabel, &knobStyleLabel, &accentColorLabel, &bgColorLabel, &chatLabel,
                       &micLabel, &preampLabel, &interfaceLabel, &languageLabel })
    {
        lbl->setColour(juce::Label::textColourId, theme.textBright);
        lbl->setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    }

    resizeConstrainer.setSizeLimits(960, 640, 1800, 1200);
    resizeConstrainer.setFixedAspectRatio(static_cast<double>(baseWidth) / baseHeight);
    setConstrainer(&resizeConstrainer);
    setResizable(true, true);

    resizeCorner = std::make_unique<juce::ResizableCornerComponent>(this, &resizeConstrainer);
    addAndMakeVisible(*resizeCorner);
    resizeCorner->setAlwaysOnTop(true);

    // Load saved theme
    currentThemeIndex = processor.getThemeIndex();
    themeBox.setSelectedId(currentThemeIndex + 1, juce::dontSendNotification);
    applyTheme();

    setSize(baseWidth, baseHeight);
    
    showMainView();
    processor.requestOllamaModelList();

    // Attach child mouse forwarder to enable moving/resizing of interactive elements
    childForwarder = std::make_unique<ChildMouseForwarder>(*this);
    juce::Component* forwardables[] = {
        &gainSlider, &gainLabel, &targetDbSlider, &targetDbLabel, &ceilingSlider, &ceilingLabel,
        &autoToggle, &riderToggle, &riderAmountSlider, &riderAmountLabel,
        &phaseMeter, &grMeter, &ioMeter, &vuMeter, &freqBalance, &advancedMeters, &quickIssues, &satellitePanel,
        &genreBox, &genreLabel, &sourceBox, &sourceLabel, &situationBox, &situationLabel, &standaloneModeToggle,
        &autoSetButton, &aiSuggestButton, &applyAiButton, &analysisLabel, &aiStatusLabel, &aiNotes, &meterModeButton,
        &modelBox, &modelLabel, &aiProviderBox, &aiProviderLabel, &refreshModelsButton, &apiKeyEditor, &apiKeyLabel,
        &themeBox, &themeLabel, &knobStyleBox, &knobStyleLabel, &accentHueSlider, &accentColorLabel, &bgBrightnessSlider, &bgColorLabel,
        &saveSettingsBtn, &saveLayoutBtn, &loadLayoutBtn, &micBox, &micLabel, &preampBox, &preampLabel, &interfaceBox, &interfaceLabel,
        &languageBox, &languageLabel, &platformTargetBox, &platformTargetLabel, &autoToggle
    };
    for (auto* c : forwardables)
    {
        if (c == nullptr) continue;
        // Skip text editors to preserve typing behavior
        if (dynamic_cast<juce::TextEditor*>(c) != nullptr) continue;
        c->addMouseListener(childForwarder.get(), false);
    }

    startTimerHz(30);  // 30Hz is plenty for readable meters

}

SimpleGainAudioProcessorEditor::~SimpleGainAudioProcessorEditor() 
{ 
    gainSlider.removeListener(this);
    targetDbSlider.removeListener(this);
    setLookAndFeel(nullptr); 
}

void SimpleGainAudioProcessorEditor::applyTheme()
{
    // Get theme colors from shared ThemeData
    auto themeType = static_cast<ThemeType>(juce::jlimit(0, static_cast<int>(ThemeType::COUNT) - 1, currentThemeIndex));
    ThemeColors tc = getTheme(themeType);
    
    // Apply to our local theme struct
    theme.bgDark = tc.bgDark;
    theme.bgPanel = tc.bgPanel;
    theme.accent = tc.accent;
    theme.accentAlt = tc.accentAlt;
    theme.textBright = tc.textBright;
    theme.textDim = tc.textDim;
    theme.meterGreen = tc.meterGreen;
    theme.meterYellow = tc.meterYellow;
    theme.meterRed = tc.meterRed;
    
    // Apply accent hue adjustment from slider
    float hueShift = static_cast<float>(accentHueSlider.getValue());
    if (std::abs(hueShift) > 0.01f)
    {
        float h, s, b;
        theme.accent.getHSB(h, s, b);
        h = std::fmod(h + hueShift + 1.0f, 1.0f);  // Wrap around
        theme.accent = juce::Colour::fromHSV(h, s, b, theme.accent.getFloatAlpha());
        
        theme.accentAlt.getHSB(h, s, b);
        h = std::fmod(h + hueShift + 1.0f, 1.0f);
        theme.accentAlt = juce::Colour::fromHSV(h, s, b, theme.accentAlt.getFloatAlpha());
    }
    
    // Apply brightness adjustment from slider
    float brightnessShift = static_cast<float>(bgBrightnessSlider.getValue());
    if (std::abs(brightnessShift) > 0.01f)
    {
        if (brightnessShift > 0)
        {
            theme.bgDark = theme.bgDark.brighter(brightnessShift * 0.5f);
            theme.bgPanel = theme.bgPanel.brighter(brightnessShift * 0.5f);
        }
        else
        {
            theme.bgDark = theme.bgDark.darker(-brightnessShift * 0.5f);
            theme.bgPanel = theme.bgPanel.darker(-brightnessShift * 0.5f);
        }
    }

    aresLaf->updateColours();
    repaint();
}

void SimpleGainAudioProcessorEditor::updateLabels()
{
    auto& loc = Localization::getInstance();
    
    // Tab buttons
    mainTabBtn.setButtonText(loc.get(StringKey::TabMain));
    settingsTabBtn.setButtonText(loc.get(StringKey::TabSettings));
    chatTabBtn.setButtonText(loc.get(StringKey::TabAssistant));
    
    // Main controls
    gainLabel.setText(loc.get(StringKey::Gain), juce::dontSendNotification);
    targetDbLabel.setText(loc.get(StringKey::Target), juce::dontSendNotification);
    autoToggle.setButtonText(loc.get(StringKey::AutoGain));
    riderToggle.setButtonText(loc.get(StringKey::VocalRider));
    riderAmountLabel.setText(loc.get(StringKey::Rider), juce::dontSendNotification);
    
    // Context
    genreLabel.setText(loc.get(StringKey::Genre), juce::dontSendNotification);
    sourceLabel.setText(loc.get(StringKey::Source), juce::dontSendNotification);
    situationLabel.setText(loc.get(StringKey::Stage), juce::dontSendNotification);
    
    // Buttons
    autoSetButton.setButtonText(loc.get(StringKey::AutoSet));
    aiSuggestButton.setButtonText(loc.get(StringKey::AskAI));
    applyAiButton.setButtonText(loc.get(StringKey::ApplyAI));
    saveSettingsBtn.setButtonText(loc.get(StringKey::SaveSettings));
    refreshModelsButton.setButtonText(loc.get(StringKey::RefreshModels));
    sendChatBtn.setButtonText(loc.get(StringKey::Send));
    
    // Settings labels
    aiProviderLabel.setText(loc.get(StringKey::AIProvider), juce::dontSendNotification);
    modelLabel.setText(loc.get(StringKey::Model), juce::dontSendNotification);
    apiKeyLabel.setText(loc.get(StringKey::APIKey), juce::dontSendNotification);
    themeLabel.setText(loc.get(StringKey::Theme), juce::dontSendNotification);
    knobStyleLabel.setText(loc.get(StringKey::KnobStyle), juce::dontSendNotification);
    accentColorLabel.setText(loc.get(StringKey::AccentColor), juce::dontSendNotification);
    bgColorLabel.setText(loc.get(StringKey::Background), juce::dontSendNotification);
    languageLabel.setText(loc.get(StringKey::Language), juce::dontSendNotification);
    
    // Equipment
    micLabel.setText(loc.get(StringKey::Microphone), juce::dontSendNotification);
    preampLabel.setText(loc.get(StringKey::Preamp), juce::dontSendNotification);
    interfaceLabel.setText(loc.get(StringKey::Interface), juce::dontSendNotification);
}

void SimpleGainAudioProcessorEditor::showMainView()
{
    gainSlider.setVisible(true); gainLabel.setVisible(true);
    targetDbSlider.setVisible(true); targetDbLabel.setVisible(true);
    ceilingSlider.setVisible(true); ceilingLabel.setVisible(true);
    autoToggle.setVisible(true); riderToggle.setVisible(true);
    riderAmountSlider.setVisible(true); riderAmountLabel.setVisible(true);
    genreBox.setVisible(true); genreLabel.setVisible(true);
    // Source shown only in standalone mode
    sourceBox.setVisible(standaloneMode); sourceLabel.setVisible(standaloneMode);
    standaloneModeToggle.setVisible(true);
    situationBox.setVisible(true); situationLabel.setVisible(true);
    autoSetButton.setVisible(true);
    aiSuggestButton.setVisible(true); applyAiButton.setVisible(true);
    analysisLabel.setVisible(true); aiStatusLabel.setVisible(true);
    aiNotes.setVisible(true);
    meterModeButton.setVisible(true);  // PRE/POST button only on main view


    // Ensure meter components are visible in main view
    vuMeter.setVisible(true); phaseMeter.setVisible(true); grMeter.setVisible(true);
    ioMeter.setVisible(true); freqBalance.setVisible(true); advancedMeters.setVisible(true);
    quickIssues.setVisible(true); satellitePanel.setVisible(true);

    aiProviderBox.setVisible(false); aiProviderLabel.setVisible(false);
    modelBox.setVisible(false); modelLabel.setVisible(false);
    apiKeyEditor.setVisible(false); apiKeyLabel.setVisible(false);
    refreshModelsButton.setVisible(false); themeBox.setVisible(false);
    themeLabel.setVisible(false); knobStyleBox.setVisible(false);
    knobStyleLabel.setVisible(false); accentColorLabel.setVisible(false);
    accentHueSlider.setVisible(false); bgColorLabel.setVisible(false);
    bgBrightnessSlider.setVisible(false); saveSettingsBtn.setVisible(false);
    saveLayoutBtn.setVisible(false); loadLayoutBtn.setVisible(false);
    
    // Equipment and language settings hidden in main view
    micBox.setVisible(false); micLabel.setVisible(false);
    preampBox.setVisible(false); preampLabel.setVisible(false);
    interfaceBox.setVisible(false); interfaceLabel.setVisible(false);
    languageBox.setVisible(false); languageLabel.setVisible(false);
    
    // Platform target visible only on main view
    platformTargetBox.setVisible(true); platformTargetLabel.setVisible(true);

    chatLabel.setVisible(false); chatHistory.setVisible(false);
    chatInput.setVisible(false); sendChatBtn.setVisible(false);
}

void SimpleGainAudioProcessorEditor::showSettingsView()
{
    gainSlider.setVisible(false); gainLabel.setVisible(false);
    targetDbSlider.setVisible(false); targetDbLabel.setVisible(false);
    ceilingSlider.setVisible(false); ceilingLabel.setVisible(false);
    autoToggle.setVisible(false); riderToggle.setVisible(false);
    riderAmountSlider.setVisible(false); riderAmountLabel.setVisible(false);
    genreBox.setVisible(false); genreLabel.setVisible(false);
    sourceBox.setVisible(false); sourceLabel.setVisible(false);
    standaloneModeToggle.setVisible(false);
    situationBox.setVisible(false); situationLabel.setVisible(false);
    autoSetButton.setVisible(false);
    aiSuggestButton.setVisible(false); applyAiButton.setVisible(false);
    analysisLabel.setVisible(false); aiStatusLabel.setVisible(false);
    aiNotes.setVisible(false);
    meterModeButton.setVisible(false);  // Hide PRE/POST button


    // Hide meter components in settings view
    vuMeter.setVisible(false); phaseMeter.setVisible(false); grMeter.setVisible(false);
    ioMeter.setVisible(false); freqBalance.setVisible(false); advancedMeters.setVisible(false);
    quickIssues.setVisible(false); satellitePanel.setVisible(false);

    aiProviderBox.setVisible(true); aiProviderLabel.setVisible(true);
    modelBox.setVisible(true); modelLabel.setVisible(true);
    refreshModelsButton.setVisible(true); themeBox.setVisible(true);
    themeLabel.setVisible(true); knobStyleBox.setVisible(true);
    knobStyleLabel.setVisible(true); accentColorLabel.setVisible(true);
    accentHueSlider.setVisible(true); bgColorLabel.setVisible(true);
    bgBrightnessSlider.setVisible(true); saveSettingsBtn.setVisible(true);
    saveLayoutBtn.setVisible(true); loadLayoutBtn.setVisible(true);
    
    // Equipment and language settings visible in settings
    micBox.setVisible(true); micLabel.setVisible(true);
    preampBox.setVisible(true); preampLabel.setVisible(true);
    interfaceBox.setVisible(true); interfaceLabel.setVisible(true);
    languageBox.setVisible(true); languageLabel.setVisible(true);
    
    // Platform target hidden in settings view
    platformTargetBox.setVisible(false); platformTargetLabel.setVisible(false);

    bool needsKey = aiProviderBox.getSelectedId() > 1;
    apiKeyEditor.setVisible(needsKey);
    apiKeyLabel.setVisible(needsKey);

    chatLabel.setVisible(false); chatHistory.setVisible(false);
    chatInput.setVisible(false); sendChatBtn.setVisible(false);
}

void SimpleGainAudioProcessorEditor::showChatView()
{
    gainSlider.setVisible(false); gainLabel.setVisible(false);
    targetDbSlider.setVisible(false); targetDbLabel.setVisible(false);
    ceilingSlider.setVisible(false); ceilingLabel.setVisible(false);
    autoToggle.setVisible(false); riderToggle.setVisible(false);
    riderAmountSlider.setVisible(false); riderAmountLabel.setVisible(false);
    genreBox.setVisible(false); genreLabel.setVisible(false);
    sourceBox.setVisible(false); sourceLabel.setVisible(false);
    standaloneModeToggle.setVisible(false);
    situationBox.setVisible(false); situationLabel.setVisible(false);
    autoSetButton.setVisible(false);
    aiSuggestButton.setVisible(false); applyAiButton.setVisible(false);
    analysisLabel.setVisible(false); aiStatusLabel.setVisible(false);
    aiNotes.setVisible(false);
    meterModeButton.setVisible(false);  // Hide PRE/POST button


    // Hide meter components in chat view
    vuMeter.setVisible(false); phaseMeter.setVisible(false); grMeter.setVisible(false);
    ioMeter.setVisible(false); freqBalance.setVisible(false); advancedMeters.setVisible(false);
    quickIssues.setVisible(false); satellitePanel.setVisible(false);

    aiProviderBox.setVisible(false); aiProviderLabel.setVisible(false);
    modelBox.setVisible(false); modelLabel.setVisible(false);
    apiKeyEditor.setVisible(false); apiKeyLabel.setVisible(false);
    refreshModelsButton.setVisible(false); themeBox.setVisible(false);
    themeLabel.setVisible(false); knobStyleBox.setVisible(false);
    knobStyleLabel.setVisible(false); accentColorLabel.setVisible(false);
    accentHueSlider.setVisible(false); bgColorLabel.setVisible(false);
    bgBrightnessSlider.setVisible(false); saveSettingsBtn.setVisible(false);
    saveLayoutBtn.setVisible(false); loadLayoutBtn.setVisible(false);
    
    // Equipment and language settings hidden in chat
    micBox.setVisible(false); micLabel.setVisible(false);
    preampBox.setVisible(false); preampLabel.setVisible(false);
    interfaceBox.setVisible(false); interfaceLabel.setVisible(false);
    languageBox.setVisible(false); languageLabel.setVisible(false);
    
    // Platform target hidden in chat
    platformTargetBox.setVisible(false); platformTargetLabel.setVisible(false);

    chatLabel.setVisible(true); chatHistory.setVisible(true);
    chatInput.setVisible(true); sendChatBtn.setVisible(true);
}

void SimpleGainAudioProcessorEditor::sendChatMessage()
{
    auto msg = chatInput.getText().trim();
    if (msg.isEmpty()) return;

    chatHistory.moveCaretToEnd();
    chatHistory.insertTextAtCaret("\nYou: " + msg + "\n");
    chatInput.clear();

    // Send to AI via processor using free-form chat method
    processor.requestChatMessage(msg);

    chatHistory.insertTextAtCaret("ARES: Thinking...\n");
}

void SimpleGainAudioProcessorEditor::drawMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const float minDb = -60.0f, maxDb = 6.0f;

    // Dark panel background
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds.expanded(2), 8.0f);
    g.setColour(theme.bgPanel.darker(0.3f));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(theme.accent.withAlpha(0.2f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // Title at top
    auto titleArea = bounds.removeFromTop(26);
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.setColour(theme.accent);
    g.drawText("LEVEL", titleArea, juce::Justification::centred);

    // Reduce inner padding so internals are larger
    bounds = bounds.reduced(6.0f, 4.0f);
    g.reduceClipRegion(bounds.toNearestInt());
    
    // Readouts at top (above the meter bars)
    auto readoutsTop = bounds.removeFromTop(52);
    
    // RMS value - bigger and centered
    g.setFont(juce::FontOptions(34.0f).withStyle("Bold"));
    g.setColour(smoothedRmsDb > -6 ? theme.meterRed : (smoothedRmsDb > -18 ? theme.meterYellow : theme.textBright));
    juce::String rmsText = smoothedRmsDb > -100 ? juce::String(smoothedRmsDb, 1) + " dB" : "-inf";
    g.drawText(rmsText, readoutsTop.removeFromTop(36), juce::Justification::centred);
    
    // RMS label
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(theme.textDim);
    g.drawText("RMS", readoutsTop, juce::Justification::centred);
    
    bounds.removeFromTop(4);
    
    // Main meter area - two vertical LED bars with scale in between
    auto meterArea = bounds.removeFromTop(bounds.getHeight() - 50);
    
    // Calculate bar widths
    float totalWidth = meterArea.getWidth();
    float barWidth = (totalWidth - 20) / 2;  // smaller middle scale so bars can be wider

    auto leftBar = meterArea.removeFromLeft(barWidth).reduced(2, 0);
    auto scaleArea = meterArea.removeFromLeft(20);
    auto rightBar = meterArea.reduced(2, 0);
    
    // LED meter constants - fewer larger LEDs
    const int numLeds = 20;  // larger LEDs for better readability
    const float ledGap = 3.0f;  // more gap to make LEDs distinct
    const float ledHeight = (leftBar.getHeight() - (numLeds - 1) * ledGap) / numLeds;
    // Compute normalized levels
    float rmsNorm = juce::jlimit(0.0f, 1.0f, (smoothedRmsDb - minDb) / (maxDb - minDb));
    float peakNorm = juce::jlimit(0.0f, 1.0f, (smoothedPeakDb - minDb) / (maxDb - minDb));
    int litLeds = static_cast<int>(rmsNorm * numLeds);
    int peakLed = static_cast<int>(peakNorm * numLeds);
    
    // Draw LED segments - clean solid bars
    for (auto& bar : { leftBar, rightBar })
    {
        for (int i = 0; i < numLeds; ++i)
        {
            float ledY = bar.getBottom() - (i + 1) * (ledHeight + ledGap) + ledGap;
            auto ledRect = juce::Rectangle<float>(bar.getX() + 2, ledY, bar.getWidth() - 4, ledHeight);
            
            // Color zones based on dB position
            float pos = (float)i / numLeds;
            juce::Colour ledColor;
            if (pos < 0.55f)
                ledColor = juce::Colour::fromRGB(40, 200, 80);     // Green
            else if (pos < 0.75f)
                ledColor = juce::Colour::fromRGB(220, 200, 40);    // Yellow
            else if (pos < 0.90f)
                ledColor = juce::Colour::fromRGB(240, 140, 40);    // Orange
            else
                ledColor = juce::Colour::fromRGB(230, 55, 50);     // Red
            
            bool lit = (i < litLeds);
            bool isPeakLed = (i == peakLed && peakLed > 0);
            
            if (lit || isPeakLed)
            {
                // Lit LED - solid color with subtle highlight
                g.setColour(ledColor);
                g.fillRoundedRectangle(ledRect, 1.0f);
                
                // Top highlight for 3D
                g.setColour(ledColor.brighter(0.35f));
                g.fillRoundedRectangle(ledRect.getX(), ledRect.getY(), 
                                       ledRect.getWidth(), ledRect.getHeight() * 0.4f, 1.0f);
                
                // Peak LED gets extra brightness
                if (isPeakLed)
                {
                    g.setColour(juce::Colours::white.withAlpha(0.3f));
                    g.fillRoundedRectangle(ledRect, 1.0f);
                }
            }
            else
            {
                // Unlit LED - dark with very subtle color hint
                g.setColour(juce::Colour::fromRGB(22, 24, 28));
                g.fillRoundedRectangle(ledRect, 1.0f);
            }
        }
    }
    
    // dB scale in center
    g.setFont(juce::FontOptions(9.0f));
    g.setColour(theme.textDim);
    const float dbVals[] = { 0, -6, -12, -18, -24, -36, -48 };
    for (float db : dbVals)
    {
        float norm = (db - minDb) / (maxDb - minDb);
        float y = leftBar.getBottom() - norm * leftBar.getHeight();
        g.drawText(juce::String(static_cast<int>(db)), 
                   juce::Rectangle<float>(scaleArea.getX(), y - 6, scaleArea.getWidth(), 12),
                   juce::Justification::centred);
    }
    
    // User Target marker line (from slider) - RED regardless of theme
    float targetNorm = juce::jlimit(0.0f, 1.0f, (presetSuggestedDb - minDb) / (maxDb - minDb));
    float targetY = leftBar.getBottom() - targetNorm * leftBar.getHeight();
    g.setColour(juce::Colours::red);
    g.fillRect(leftBar.getX(), targetY - 1.0f, rightBar.getRight() - leftBar.getX(), 2.0f);
    // Label for user target
    g.setFont(juce::FontOptions(8.0f).withStyle("Bold"));
    g.setColour(juce::Colours::red);
    g.drawText("TGT", juce::Rectangle<float>(rightBar.getRight() + 2, targetY - 6, 24, 12), juce::Justification::left);
    
    // AI Target marker line (from AI suggestion) - BLUE regardless of theme
    if (std::abs(aiSuggestedDb - presetSuggestedDb) > 0.5f && aiSuggestedDb > -50.0f)
    {
        float aiTargetNorm = juce::jlimit(0.0f, 1.0f, (aiSuggestedDb - minDb) / (maxDb - minDb));
        float aiTargetY = leftBar.getBottom() - aiTargetNorm * leftBar.getHeight();
        g.setColour(juce::Colours::dodgerblue);
        g.fillRect(leftBar.getX(), aiTargetY - 1.0f, rightBar.getRight() - leftBar.getX(), 2.0f);
        // Label for AI target
        g.setColour(juce::Colours::dodgerblue);
        g.drawText("AI", juce::Rectangle<float>(rightBar.getRight() + 2, aiTargetY - 6, 24, 12), juce::Justification::left);
    }
    
    // Bottom readouts: Peak and Target
    auto bottomRow = bounds;
    auto peakArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 3);
    auto targetArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    auto aiArea = bottomRow;
    
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.setColour(smoothedPeakDb > -3 ? theme.meterRed : theme.textBright);
    juce::String peakText = smoothedPeakDb > -100 ? juce::String(smoothedPeakDb, 1) : "-inf";
    g.drawText(peakText, peakArea.removeFromTop(18), juce::Justification::centred);
    g.setFont(juce::FontOptions(7.0f));
    g.setColour(theme.textDim);
    g.drawText("PEAK", peakArea, juce::Justification::centred);
    
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.setColour(juce::Colours::red);
    g.drawText(juce::String(presetSuggestedDb, 1), targetArea.removeFromTop(18), juce::Justification::centred);
    g.setFont(juce::FontOptions(7.0f));
    g.setColour(theme.textDim);
    g.drawText("TARGET", targetArea, juce::Justification::centred);
    
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.setColour(juce::Colours::dodgerblue);
    juce::String aiText = aiSuggestedDb > -50.0f ? juce::String(aiSuggestedDb, 1) : "--";
    g.drawText(aiText, aiArea.removeFromTop(18), juce::Justification::centred);
    g.setFont(juce::FontOptions(7.0f));
    g.setColour(theme.textDim);
    g.drawText("AI", aiArea, juce::Justification::centred);
}

void SimpleGainAudioProcessorEditor::drawPhaseMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Phase meter background
    g.setColour(theme.bgDark);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    // Title
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    g.drawText("PHASE", bounds.removeFromTop(18), juce::Justification::centred);
    
    // Phase meter track
    auto phaseTrack = bounds.reduced(12, 6);
    g.setColour(theme.bgPanel.brighter(0.1f));
    g.fillRoundedRectangle(phaseTrack, 4.0f);
    
    // Phase correlation indicator (-1 to +1)
    float phaseNorm = (phaseCorrelation + 1.0f) * 0.5f; // Convert -1..+1 to 0..1
    float indicatorX = phaseTrack.getX() + phaseNorm * phaseTrack.getWidth();
    
    // Color based on phase: green = good (+1), yellow = mono (0), red = out of phase (-1)
    juce::Colour phaseColor = phaseCorrelation > 0.5f ? theme.meterGreen 
                            : (phaseCorrelation > -0.3f ? theme.meterYellow : theme.meterRed);
    
    // Draw filled portion from center
    float centerX = phaseTrack.getX() + phaseTrack.getWidth() * 0.5f;
    float fillStart = std::min(centerX, indicatorX);
    float fillWidth = std::abs(indicatorX - centerX);
    g.setColour(phaseColor.withAlpha(0.6f));
    g.fillRoundedRectangle(fillStart, phaseTrack.getY(), fillWidth, phaseTrack.getHeight(), 2.0f);
    
    // Draw indicator dot
    g.setColour(theme.textBright);
    g.fillEllipse(indicatorX - 4, phaseTrack.getCentreY() - 4, 8, 8);
    
    // Phase labels below track
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    g.setColour(theme.textDim);
    auto labelRow = bounds.removeFromBottom(14);
    g.drawText("-1", labelRow.removeFromLeft(labelRow.getWidth() / 3), juce::Justification::centred);
    g.drawText("0", labelRow.removeFromLeft(labelRow.getWidth() / 2), juce::Justification::centred);
    g.drawText("+1", labelRow, juce::Justification::centred);
    
    // Border
    g.setColour(theme.accent.withAlpha(0.25f));
    g.drawRoundedRectangle(bounds.expanded(0, 18), 6.0f, 1.0f);
}

void SimpleGainAudioProcessorEditor::drawSatellitePanel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Panel background
    g.setColour(theme.bgPanel);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(theme.accent.withAlpha(0.25f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    
    // Title bar - larger font
    auto titleArea = bounds.removeFromTop(28);
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.setColour(theme.accent);
    
    int satCount = processor.getActiveSatelliteCount();
    g.drawText("SATELLITES (" + juce::String(satCount) + ")", titleArea.reduced(10, 0), juce::Justification::left);
    
    // Reserve space for controls if satellite is selected
    if (selectedSatellite >= 0)
        bounds.removeFromBottom(70);
    
    if (satCount == 0)
    {
        g.setFont(juce::FontOptions(12.0f));
        g.setColour(theme.textDim);
        g.drawText("Load AR3S Satellite on tracks", bounds, juce::Justification::centred);
        return;
    }
    
    // Source type names
    const juce::StringArray sourceNames = { "Vox", "BG Vox", "Kick", "Snare", "HiHat", 
                                            "Drums", "Bass", "E.Gtr", "A.Gtr", "Keys", 
                                            "Synth", "Str", "Other" };
    
    bounds = bounds.reduced(6, 4);
    const float rowHeight = 28.0f;  // Increased row height
    
    for (int i = 0; i < MAX_SATELLITES && bounds.getHeight() >= rowHeight; ++i)
    {
        auto info = processor.getSatelliteInfo(i);
        if (!info.active) continue;
        
        auto row = bounds.removeFromTop(rowHeight);
        bool isSelected = (i == selectedSatellite);
        
        // Row background - highlight selected
        if (isSelected)
        {
            g.setColour(theme.accent.withAlpha(0.3f));
            g.fillRoundedRectangle(row, 4.0f);
            g.setColour(theme.accent);
            g.drawRoundedRectangle(row, 4.0f, 1.5f);
        }
        else
        {
            g.setColour(theme.bgDark.withAlpha(0.4f));
            g.fillRoundedRectangle(row, 4.0f);
        }
        
        // Store row bounds for click detection
        auto rowInt = row.toNearestInt();
        
        // Status indicator - larger
        g.setColour(isSelected ? theme.accent : theme.meterGreen);
        g.fillEllipse(row.getX() + 6, row.getCentreY() - 4, 8, 8);
        row.removeFromLeft(20);
        
        // Channel name - larger font
        g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
        g.setColour(isSelected ? theme.accent : theme.textBright);
        juce::String displayName = info.channelName.isEmpty() ? ("Sat " + juce::String(i + 1)) : info.channelName;
        if (displayName.length() > 10) displayName = displayName.substring(0, 9) + "...";
        g.drawText(displayName, row.removeFromLeft(70), juce::Justification::left);
        
        // Source type - larger font
        g.setFont(juce::FontOptions(11.0f));
        g.setColour(isSelected ? theme.accentAlt : theme.textDim);
        juce::String sourceName = (info.sourceType >= 0 && info.sourceType < sourceNames.size()) 
                                  ? sourceNames[info.sourceType] : "Other";
        g.drawText(sourceName, row.removeFromLeft(50), juce::Justification::left);
        
        // Mini level bar - taller
        auto levelBar = row.removeFromLeft(50).reduced(2, 6);
        g.setColour(theme.bgDark);
        g.fillRoundedRectangle(levelBar, 3.0f);
        
        float levelNorm = juce::jlimit(0.0f, 1.0f, (info.rmsDb + 60.0f) / 60.0f);
        juce::Colour levelColor = info.rmsDb > -6.0f ? theme.meterRed 
                                : (info.rmsDb > -18.0f ? theme.meterYellow : theme.meterGreen);
        g.setColour(levelColor);
        g.fillRoundedRectangle(levelBar.withWidth(levelBar.getWidth() * levelNorm), 3.0f);
        
        // Gain display - larger font
        g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
        float gainDb = 20.0f * std::log10(std::max(0.0001f, info.currentGain));
        g.setColour(gainDb > 0 ? theme.meterYellow : theme.textBright);
        juce::String gainStr = (gainDb >= 0 ? "+" : "") + juce::String(gainDb, 1);
        g.drawText(gainStr, row, juce::Justification::centred);
        
        bounds.removeFromTop(3);  // Increased spacing
    }
    
    // No inline controls - use right-click context menu instead
}

void SimpleGainAudioProcessorEditor::drawCrestMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Background panel
    g.setColour(theme.bgPanel);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Title
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(theme.accent);
    auto titleArea = bounds.removeFromTop(16);
    g.drawText("CREST FACTOR", titleArea, juce::Justification::centred);
    
    bounds.reduce(6, 4);
    
    // Crest value display
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    juce::String crestText = juce::String(crestFactorDb, 1) + " dB";
    g.drawText(crestText, bounds.removeFromTop(26), juce::Justification::centred);
    
    // Visual bar (0-20 dB range)
    bounds.removeFromTop(4);
    auto barArea = bounds.removeFromTop(14);
    g.setColour(theme.bgDark);
    g.fillRoundedRectangle(barArea, 3.0f);
    
    float crestNorm = juce::jlimit(0.0f, 1.0f, crestFactorDb / 20.0f);
    auto filledBar = barArea.withWidth(barArea.getWidth() * crestNorm);
    
    // Color based on crest: green = 12-20dB (dynamic), yellow = 6-12dB, red = 0-6dB (squashed)
    juce::Colour barColor = crestFactorDb > 12.0f ? theme.meterGreen
                          : (crestFactorDb > 6.0f ? theme.meterYellow : theme.meterRed);
    g.setColour(barColor.withAlpha(0.8f));
    g.fillRoundedRectangle(filledBar, 3.0f);
    
    // Border
    g.setColour(theme.accent.withAlpha(0.25f));
    g.drawRoundedRectangle(bounds.getUnion(titleArea).expanded(2, 2), 4.0f, 1.0f);
}

void SimpleGainAudioProcessorEditor::drawGainReductionMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Background panel
    g.setColour(theme.bgPanel);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Title
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(theme.accent);
    auto titleArea = bounds.removeFromTop(16);
    g.drawText("GAIN ADJUST", titleArea, juce::Justification::centred);
    
    bounds.reduce(6, 4);
    
    // GR value display
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    juce::String grText = (gainReductionDb > 0 ? "-" : "+") + juce::String(std::abs(gainReductionDb), 1) + " dB";
    g.setColour(gainReductionDb > 0 ? theme.meterRed : theme.meterGreen);
    g.drawText(grText, bounds.removeFromTop(26), juce::Justification::centred);
    
    // Visual bar (-18 to +18 dB range)
    bounds.removeFromTop(4);
    auto barArea = bounds.removeFromTop(14);
    g.setColour(theme.bgDark);
    g.fillRoundedRectangle(barArea, 3.0f);
    
    // Center line
    float centerX = barArea.getCentreX();
    g.setColour(theme.textDim.withAlpha(0.3f));
    g.drawLine(centerX, barArea.getY(), centerX, barArea.getBottom(), 1.0f);
    
    // Draw bar from center
    float grNorm = juce::jlimit(-1.0f, 1.0f, gainReductionDb / 18.0f);
    float barWidth = std::abs(grNorm) * barArea.getWidth() * 0.5f;
    juce::Rectangle<float> filledBar;
    
    if (gainReductionDb > 0)  // Reduction
    {
        filledBar = juce::Rectangle<float>(centerX - barWidth, barArea.getY(), barWidth, barArea.getHeight());
        g.setColour(theme.meterRed.withAlpha(0.7f));
    }
    else  // Boost
    {
        filledBar = juce::Rectangle<float>(centerX, barArea.getY(), barWidth, barArea.getHeight());
        g.setColour(theme.meterGreen.withAlpha(0.7f));
    }
    g.fillRoundedRectangle(filledBar, 3.0f);
    
    // Border
    g.setColour(theme.accent.withAlpha(0.25f));
    g.drawRoundedRectangle(bounds.getUnion(titleArea).expanded(2, 2), 4.0f, 1.0f);
}

void SimpleGainAudioProcessorEditor::drawIOComparisonMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Panel background - dark bezel with depth
    juce::ColourGradient bezelGrad(juce::Colour::fromRGB(35, 38, 42), bounds.getX(), bounds.getY(),
                                   juce::Colour::fromRGB(20, 22, 26), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bezelGrad);
    g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour(juce::Colour::fromRGB(60, 65, 72));
    g.drawRoundedRectangle(bounds, 10.0f, 2.0f);
    
    auto innerArea = bounds.reduced(8, 8);
    
    // Title with IN/OUT toggle button
    auto titleRow = innerArea.removeFromTop(22);
    auto btnArea = titleRow.removeFromRight(50);

    
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    g.drawText(vuShowInput ? "INPUT" : "OUTPUT", titleRow, juce::Justification::centred);
    
    innerArea.removeFromTop(4);
    
    // Bottom readouts: LUFS (with target), Peak, RMS
    auto bottomRow = innerArea.removeFromBottom(26);
    float third = bottomRow.getWidth() / 3.0f;
    
    // LUFS - show current and target when LUFS mode is active
    auto lufsArea = bottomRow.removeFromLeft(third);
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    
    // Color based on how close we are to target
    juce::Colour lufsColor = theme.textBright;
    if (lufsEnabled)
    {
        float diff = std::abs(shortTermLufs - lufsTarget);
        if (diff < 1.0f)
            lufsColor = theme.meterGreen;  // On target
        else if (diff < 3.0f)
            lufsColor = theme.meterYellow; // Close
        else
            lufsColor = theme.meterRed;    // Off target
    }
    else if (shortTermLufs > -14)
    {
        lufsColor = theme.meterRed;
    }
    
    g.setColour(lufsColor);
    juce::String lufsText = shortTermLufs > -100 ? juce::String(shortTermLufs, 1) : "-inf";
    g.drawText(lufsText, lufsArea.removeFromTop(14), juce::Justification::centred);
    g.setFont(juce::FontOptions(8.0f));
    g.setColour(lufsEnabled ? theme.accent : theme.textDim);
    juce::String lufsLabel = lufsEnabled ? ("LUFS \u2192 " + juce::String(static_cast<int>(lufsTarget))) : "LUFS";
    g.drawText(lufsLabel, lufsArea, juce::Justification::centred);
    
    // Peak
    float displayPeak = vuShowInput ? inputRmsDb + crestFactorDb : smoothedPeakDb;
    auto peakArea = bottomRow.removeFromLeft(third);
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.setColour(displayPeak > -3 ? theme.meterRed : theme.textBright);
    juce::String peakText = displayPeak > -100 ? juce::String(displayPeak, 1) : "-inf";
    g.drawText(peakText, peakArea.removeFromTop(14), juce::Justification::centred);
    g.setFont(juce::FontOptions(8.0f));
    g.setColour(theme.textDim);
    g.drawText("PEAK", peakArea, juce::Justification::centred);
    
    // RMS
    float displayRms = vuShowInput ? inputRmsDb : outputRmsDb;
    auto rmsArea = bottomRow;
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.setColour(displayRms > -12 ? theme.meterYellow : theme.textBright);
    juce::String rmsText = displayRms > -100 ? juce::String(displayRms, 1) : "-inf";
    g.drawText(rmsText, rmsArea.removeFromTop(14), juce::Justification::centred);
    g.setFont(juce::FontOptions(8.0f));
    g.setColour(theme.textDim);
    g.drawText("RMS", rmsArea, juce::Justification::centred);
    
    innerArea.removeFromBottom(3);
    
    // === REALISTIC VU METER ===
    auto meterArea = innerArea.reduced(2);
    float rmsDb = vuShowInput ? inputRmsDb : outputRmsDb;
    // Ensure we don't draw outside this component's bounds
    g.reduceClipRegion(bounds.toNearestInt());

    // VU meter outer frame with wood/metal look
    juce::ColourGradient frameGrad(juce::Colour::fromRGB(50, 48, 45), meterArea.getX(), meterArea.getY(),
                                   juce::Colour::fromRGB(30, 28, 25), meterArea.getX(), meterArea.getBottom(), false);
    g.setGradientFill(frameGrad);
    g.fillRoundedRectangle(meterArea, 8.0f);
    
    // Inner bezel
    g.setColour(juce::Colour::fromRGB(25, 23, 20));
    g.drawRoundedRectangle(meterArea.reduced(2), 6.0f, 2.0f);
    g.setColour(juce::Colour::fromRGB(70, 65, 58));
    g.drawRoundedRectangle(meterArea.reduced(1), 7.0f, 1.0f);
    
    // VU face - warm backlit amber with realistic lighting
    auto display = meterArea.reduced(8);
    
    // Base warm color with gradient for depth
    juce::ColourGradient faceGrad(
        juce::Colour::fromRGB(255, 240, 200), display.getCentreX(), display.getY(),
        juce::Colour::fromRGB(235, 200, 145), display.getCentreX(), display.getBottom(), false);
    g.setGradientFill(faceGrad);
    g.fillRoundedRectangle(display, 5.0f);
    
    // Subtle backlight glow from behind
    juce::ColourGradient backlightGrad(
        juce::Colour::fromRGB(255, 248, 220).withAlpha(0.9f), display.getCentreX(), display.getCentreY(),
        juce::Colour::fromRGB(240, 210, 160).withAlpha(0.6f), display.getX(), display.getY(), true);
    g.setGradientFill(backlightGrad);
    g.fillRoundedRectangle(display.reduced(3), 4.0f);
    
    // Inner shadow at top for depth
    juce::ColourGradient shadowTop(
        juce::Colours::black.withAlpha(0.15f), display.getX(), display.getY(),
        juce::Colours::transparentBlack, display.getX(), display.getY() + 25, false);
    g.setGradientFill(shadowTop);
    g.fillRect(display.getX(), display.getY(), display.getWidth(), 25.0f);
    
    // Geometry for needle arc - sized to comfortably fit inside the display
    float centerX = display.getCentreX();
    // Pivot slightly below the vertical center but still inside the display to avoid overflow
    float centerY = display.getBottom() - (display.getHeight() * 0.12f);
    // Radius sized to fit within display width/height
    float radius = std::min(display.getHeight() * 0.48f, display.getWidth() * 0.48f);
    
    const float startAngle = 2.45f;
    const float endAngle = 0.69f;
    const float totalAngle = startAngle - endAngle;
    
    // Main arc - thicker and bolder
    juce::Path arcPath;
    for (float a = startAngle; a >= endAngle; a -= 0.012f)
    {
        float x = centerX + radius * std::cos(a);
        float y = centerY - radius * std::sin(a);
        if (a == startAngle) arcPath.startNewSubPath(x, y);
        else arcPath.lineTo(x, y);
    }
    g.setColour(juce::Colour::fromRGB(40, 38, 32));
    g.strokePath(arcPath, juce::PathStrokeType(3.0f));
    
    // Scale marks: VU -20 to +3 - BIGGER FONT AND TICK MARKS
    struct VUMark { float vu; const char* label; bool major; };
    const VUMark marks[] = {
        {-20, "-20", true}, {-10, "-10", true}, {-7, "-7", false}, 
        {-5, "-5", true}, {-3, "-3", false}, {-2, "", false}, {-1, "", false},
        {0, "0", true}, {1, "+1", false}, {2, "+2", false}, {3, "+3", true}
    };
    
    for (const auto& m : marks)
    {
        float t = (m.vu + 20.0f) / 23.0f;
        float angle = startAngle - t * totalAngle;
        float tickOut = radius + 5;
        float tickIn = radius - (m.major ? 18 : 10);  // Much longer tick marks
        float x1 = centerX + tickIn * std::cos(angle);
        float y1 = centerY - tickIn * std::sin(angle);
        float x2 = centerX + tickOut * std::cos(angle);
        float y2 = centerY - tickOut * std::sin(angle);
        
        bool isRed = m.vu >= 0;
        g.setColour(isRed ? juce::Colour::fromRGB(190, 45, 40) : juce::Colour::fromRGB(35, 32, 28));
        g.drawLine(x1, y1, x2, y2, m.major ? 2.8f : 1.8f);  // Thicker lines
        
        if (m.major && strlen(m.label) > 0)
        {
            float labelR = radius - 22;  // Offset adjusted to fit labels inside display
            float lx = centerX + labelR * std::cos(angle);
            float ly = centerY - labelR * std::sin(angle);
            g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
            g.setColour(isRed ? juce::Colour::fromRGB(180, 40, 35) : juce::Colour::fromRGB(30, 28, 24));
            g.drawText(m.label, juce::Rectangle<float>(lx - 18, ly - 8, 36, 16), juce::Justification::centred);
        }
    }
    
    // Red zone arc - thicker with glow
    juce::Path redZone;
    float redStart = startAngle - (20.0f / 23.0f) * totalAngle;
    for (float a = redStart; a >= endAngle; a -= 0.012f)
    {
        float x = centerX + (radius + 3) * std::cos(a);
        float y = centerY - (radius + 3) * std::sin(a);
        if (a == redStart) redZone.startNewSubPath(x, y);
        else redZone.lineTo(x, y);
    }
    // Red zone glow
    g.setColour(juce::Colour::fromRGB(200, 50, 45).withAlpha(0.35f));
    g.strokePath(redZone, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(juce::Colour::fromRGB(210, 55, 50));
    g.strokePath(redZone, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // "VU" label with vintage styling - BIGGER
    g.setFont(juce::FontOptions(18.0f).withStyle("Bold"));  // Much bigger VU text
    g.setColour(juce::Colour::fromRGB(40, 35, 30));
    g.drawText("VU", juce::Rectangle<float>(display.getX(), display.getY() + 6, display.getWidth(), 22), juce::Justification::centred);
    
    // Needle - 0 VU = -18 dBFS
    float vuValue = rmsDb + 18.0f;
    vuValue = juce::jlimit(-20.0f, 3.0f, vuValue);
    float t = (vuValue + 20.0f) / 23.0f;
    float needleAngle = startAngle - t * totalAngle;
    
    // Needle length should reach just past the tick marks but NOT extend beyond arc
    float needleLen = radius - 6;  // Slightly shorter than radius to stay within arc
    float nx = centerX + needleLen * std::cos(needleAngle);
    float ny = centerY - needleLen * std::sin(needleAngle);
    
    // Needle shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawLine(centerX + 2, centerY + 2, nx + 2, ny + 2, 3.5f);
    
    // Red needle with gradient - classic VU style
    juce::Path needlePath;
    needlePath.startNewSubPath(centerX, centerY);
    needlePath.lineTo(nx, ny);
    g.setColour(juce::Colour::fromRGB(200, 50, 45));
    g.strokePath(needlePath, juce::PathStrokeType(3.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    // Needle highlight
    g.setColour(juce::Colour::fromRGB(235, 85, 78));
    g.drawLine(centerX, centerY, 
               centerX + needleLen * 0.6f * std::cos(needleAngle),
               centerY - needleLen * 0.6f * std::sin(needleAngle), 1.8f);
    
    // Pivot with metallic look
    float pivotR = std::min(11.0f, display.getHeight() * 0.08f);
    juce::ColourGradient pivotGrad(juce::Colour::fromRGB(70, 65, 60), centerX, centerY - pivotR,
                                   juce::Colour::fromRGB(28, 26, 22), centerX, centerY + pivotR, false);
    g.setGradientFill(pivotGrad);
    g.fillEllipse(centerX - pivotR, centerY - pivotR, pivotR * 2, pivotR * 2);
    g.setColour(juce::Colour::fromRGB(95, 90, 82));
    g.drawEllipse(centerX - pivotR, centerY - pivotR, pivotR * 2, pivotR * 2, 2.0f);
    // Pivot shine
    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.fillEllipse(centerX - pivotR * 0.45f, centerY - pivotR * 0.55f, pivotR * 0.65f, pivotR * 0.5f);
}

void SimpleGainAudioProcessorEditor::drawAdvancedMeters(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Background
    g.setColour(theme.bgPanel);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(theme.accent.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    
    auto area = bounds.reduced(10, 8);
    
    // Title
    auto titleRow = area.removeFromTop(16);
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    g.drawText("SIGNAL ANALYSIS", titleRow, juce::Justification::centred);
    area.removeFromTop(6);
    
    // Grid of values - 2 columns, larger text
    const float rowHeight = 20.0f;
    float colWidth = area.getWidth() / 2.0f;
    
    auto drawValue = [&](const juce::String& label, const juce::String& value, juce::Colour valueColor, bool leftCol, int row)
    {
        float x = leftCol ? area.getX() : area.getX() + colWidth;
        float y = area.getY() + row * rowHeight;
        auto labelArea = juce::Rectangle<float>(x, y, colWidth * 0.55f, rowHeight);
        auto valueArea = juce::Rectangle<float>(x + colWidth * 0.55f, y, colWidth * 0.45f, rowHeight);
        
        g.setFont(juce::FontOptions(10.0f));
        g.setColour(theme.textDim);
        g.drawText(label, labelArea, juce::Justification::centredLeft);
        g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
        g.setColour(valueColor);
        g.drawText(value, valueArea, juce::Justification::centredLeft);
    };
    
    // Left column - Loudness metrics
    juce::String headroomStr = headroom < 100 ? juce::String(headroom, 1) + " dB" : "inf";
    drawValue("Headroom:", headroomStr, headroom < 3 ? theme.meterYellow : theme.meterGreen, true, 0);
    
    juce::String drStr = juce::String(crestFactorDb, 1) + " dB";
    drawValue("Dynamics:", drStr, crestFactorDb < 6 ? theme.meterYellow : theme.textBright, true, 1);
    
    juce::String clipStr = clipCount > 0 ? juce::String(clipCount) + "!" : "0";
    drawValue("Clips:", clipStr, clipCount > 0 ? theme.meterRed : theme.meterGreen, true, 2);
    
    juce::String peakStr = smoothedPeakDb > -100 ? juce::String(smoothedPeakDb, 1) : "-inf";
    drawValue("Peak:", peakStr, smoothedPeakDb > -3 ? theme.meterRed : theme.textBright, true, 3);
    
    // Right column - Stereo metrics
    juce::String widthStr = juce::String(static_cast<int>(stereoWidth)) + "%";
    drawValue("Width:", widthStr, stereoWidth > 150 ? theme.meterYellow : theme.textBright, false, 0);
    
    juce::String monoStr = monoCompatible ? "OK" : "ISSUE";
    drawValue("Mono Compat:", monoStr, monoCompatible ? theme.meterGreen : theme.meterRed, false, 1);
    
    juce::String phaseStr = juce::String(phaseCorrelation, 2);
    drawValue("Correlation:", phaseStr, phaseCorrelation < 0 ? theme.meterRed : theme.textBright, false, 2);
    
    juce::String rmsStr = smoothedRmsDb > -100 ? juce::String(smoothedRmsDb, 1) : "-inf";
    drawValue("RMS:", rmsStr, theme.textBright, false, 3);
}

void SimpleGainAudioProcessorEditor::drawFrequencyBalance(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Background with subtle gradient
    juce::ColourGradient bgGrad(theme.bgPanel, bounds.getX(), bounds.getY(),
                                 theme.bgDark, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(theme.accent.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    
    auto area = bounds.reduced(8, 6);
    
    // Title
    auto titleRow = area.removeFromTop(14);
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    g.drawText("SPECTRUM ANALYZER", titleRow, juce::Justification::centred);
    area.removeFromTop(4);
    
    // Spectrum display area
    auto spectrumArea = area.reduced(2, 2);
    
    // Dark background for spectrum
    g.setColour(theme.bgDark.darker(0.3f));
    g.fillRoundedRectangle(spectrumArea, 4.0f);
    
    // Draw grid lines and frequency labels
    g.setColour(theme.textDim.withAlpha(0.15f));
    
    // Horizontal dB lines (-60, -40, -20, 0)
    const float minDb = -60.0f;
    const float maxDb = 0.0f;
    const float dbRange = maxDb - minDb;
    
    for (float db = -60.0f; db <= 0.0f; db += 20.0f)
    {
        float y = spectrumArea.getY() + spectrumArea.getHeight() * (1.0f - (db - minDb) / dbRange);
        g.drawLine(spectrumArea.getX(), y, spectrumArea.getRight(), y, 0.5f);
        
        // dB labels
        if (spectrumArea.getHeight() > 40)
        {
            g.setFont(juce::FontOptions(7.0f));
            g.setColour(theme.textDim.withAlpha(0.5f));
            g.drawText(juce::String((int)db), 
                       juce::Rectangle<float>(spectrumArea.getX() + 2, y - 5, 18, 10), 
                       juce::Justification::left);
            g.setColour(theme.textDim.withAlpha(0.15f));
        }
    }
    
    // Vertical frequency lines (logarithmic: 100Hz, 1kHz, 10kHz)
    double sampleRate = processor.getSampleRateValue();
    if (sampleRate < 1.0) sampleRate = 44100.0;
    
    const float minFreq = 20.0f;
    const float maxFreq = static_cast<float>(sampleRate) / 2.0f;
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float logRange = logMax - logMin;
    
    // Draw frequency markers at key points
    float freqMarkers[] = { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };
    const char* freqLabels[] = { "50", "100", "200", "500", "1K", "2K", "5K", "10K", "20K" };
    
    for (int i = 0; i < 9; ++i)
    {
        if (freqMarkers[i] >= minFreq && freqMarkers[i] <= maxFreq)
        {
            float logFreq = std::log10(freqMarkers[i]);
            float normX = (logFreq - logMin) / logRange;
            float x = spectrumArea.getX() + normX * spectrumArea.getWidth();
            
            g.setColour(theme.textDim.withAlpha(0.1f));
            g.drawLine(x, spectrumArea.getY(), x, spectrumArea.getBottom(), 0.5f);
            
            // Frequency labels at bottom
            g.setFont(juce::FontOptions(7.0f));
            g.setColour(theme.textDim.withAlpha(0.5f));
            g.drawText(freqLabels[i], 
                       juce::Rectangle<float>(x - 12, spectrumArea.getBottom() - 10, 24, 10), 
                       juce::Justification::centred);
        }
    }
    
    // Get spectrum data from processor
    auto spectrumData = processor.getSpectrumData();
    int numBins = static_cast<int>(spectrumData.size());
    
    if (numBins > 0)
    {
        // Create the spectrum path
        juce::Path spectrumPath;
        juce::Path fillPath;
        
        bool pathStarted = false;
        float lastX = 0, lastY = 0;
        
        // Draw spectrum with logarithmic frequency scale
        for (int i = 1; i < numBins; ++i)
        {
            // Calculate frequency for this bin
            float binFreq = static_cast<float>(i) * static_cast<float>(sampleRate) / static_cast<float>(numBins * 2);
            
            if (binFreq < minFreq || binFreq > maxFreq)
                continue;
            
            // Logarithmic X position
            float logFreq = std::log10(binFreq);
            float normX = (logFreq - logMin) / logRange;
            float x = spectrumArea.getX() + normX * spectrumArea.getWidth();
            
            // Smooth the data a bit by averaging nearby bins
            float db = spectrumData[i];
            if (i > 0 && i < numBins - 1)
                db = (spectrumData[i-1] + spectrumData[i] * 2 + spectrumData[i+1]) / 4.0f;
            
            // Normalize Y position
            float normY = (db - minDb) / dbRange;
            normY = juce::jlimit(0.0f, 1.0f, normY);
            float y = spectrumArea.getBottom() - normY * spectrumArea.getHeight();
            
            if (!pathStarted)
            {
                spectrumPath.startNewSubPath(x, y);
                fillPath.startNewSubPath(x, spectrumArea.getBottom());
                fillPath.lineTo(x, y);
                pathStarted = true;
            }
            else
            {
                spectrumPath.lineTo(x, y);
                fillPath.lineTo(x, y);
            }
            
            lastX = x;
            lastY = y;
        }
        
        if (pathStarted)
        {
            // Close fill path
            fillPath.lineTo(lastX, spectrumArea.getBottom());
            fillPath.closeSubPath();
            
            // Draw filled area with gradient
            juce::ColourGradient fillGrad(theme.accent.withAlpha(0.4f), spectrumArea.getX(), spectrumArea.getY(),
                                          theme.accent.withAlpha(0.05f), spectrumArea.getX(), spectrumArea.getBottom(), false);
            g.setGradientFill(fillGrad);
            g.fillPath(fillPath);
            
            // Draw spectrum line with glow
            g.setColour(theme.accent.withAlpha(0.3f));
            g.strokePath(spectrumPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            
            g.setColour(theme.accent);
            g.strokePath(spectrumPath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            
            // Draw bezel around spectrum area
            g.setColour(theme.bgDark);
            g.drawRoundedRectangle(spectrumArea, 4.0f, 1.0f);
        }
    }
    
    // Draw bezel around spectrum area
    g.setColour(theme.bgDark);
    g.drawRoundedRectangle(spectrumArea, 4.0f, 1.0f);
}

void SimpleGainAudioProcessorEditor::drawQuickIssues(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Background
    g.setColour(theme.bgPanel);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(theme.accent.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    
    auto area = bounds.reduced(8, 6);
    
    // Title
    auto titleRow = area.removeFromTop(12);
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    g.setColour(theme.textBright);
    g.drawText("QUICK CHECK", titleRow, juce::Justification::centred);
    area.removeFromTop(4);
    
    // Analyze and show issues
    std::vector<std::pair<juce::String, juce::Colour>> issues;
    
    // Check levels - use target as reference
    float targetLufs = targetDbSlider.getValue();
    if (smoothedRmsDb < -40)
        issues.push_back({"No signal", theme.textDim});
    else if (smoothedRmsDb < -30)
        issues.push_back({"Signal very quiet", theme.meterYellow});
    else if (smoothedPeakDb > -1)
        issues.push_back({"CLIPPING!", theme.meterRed});
    else if (smoothedPeakDb > -3)
        issues.push_back({"Peaks hot", theme.meterYellow});
    else
        issues.push_back({"Levels OK", theme.meterGreen});
    
    // Check dynamics
    if (crestFactorDb < 4)
        issues.push_back({"Over-compressed", theme.meterYellow});
    else if (crestFactorDb > 18)
        issues.push_back({"Very dynamic", theme.textDim});
    else
        issues.push_back({"Good dynamics", theme.meterGreen});
    
    // Check phase/mono
    if (phaseCorrelation < -0.3f)
        issues.push_back({"Phase problems!", theme.meterRed});
    else if (!monoCompatible)
        issues.push_back({"Check mono", theme.meterYellow});
    else
        issues.push_back({"Mono OK", theme.meterGreen});
    
    // Check stereo width
    if (stereoWidth > 150)
        issues.push_back({"Very wide", theme.meterYellow});
    else if (stereoWidth < 10 && smoothedRmsDb > -40)
        issues.push_back({"Mono signal", theme.textDim});
    
    // Draw issues with larger text
    g.setFont(juce::FontOptions(10.0f));
    float y = area.getY();
    for (const auto& [text, color] : issues)
    {
        if (y + 16 > area.getBottom()) break;
        g.setColour(color);
        g.drawText("> " + text, juce::Rectangle<float>(area.getX(), y, area.getWidth(), 16), juce::Justification::centredLeft);
        y += 16;
    }
}

void SimpleGainAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    // PRIORITY: Check for drag/resize operations first (these take precedence)
    if (e.mods.isCommandDown() || e.mods.isCtrlDown())
    {
        // Check ALL components for dragging - everything should be movable!
        juce::Component* components[] = {
            &gainSlider, &targetDbSlider, &ceilingSlider,
            &autoToggle, &riderToggle, &riderAmountSlider,
            &genreBox, &sourceBox, &situationBox,
            &autoSetButton, &aiSuggestButton, &applyAiButton,
            &analysisLabel, &aiStatusLabel, &aiNotes,
            &vuMeter, &phaseMeter, &grMeter, &ioMeter,
            &freqBalance, &advancedMeters, &quickIssues, &satellitePanel,
            &meterModeButton, &vuModeButton
        };

        for (auto* comp : components)
        {
            if (comp && comp->isVisible() && comp->getBounds().contains(e.getPosition()))
            {
                draggedComponent = comp;
                dragStartPos = e.getPosition();
                componentStartBounds = comp->getBounds();
                return; // Exit early - drag operation started
            }
        }
    }
    else if (e.mods.isAltDown())
    {
        // Check ALL components for resize handles
        juce::Component* components[] = {
            &gainSlider, &targetDbSlider, &ceilingSlider,
            &autoToggle, &riderToggle, &riderAmountSlider,
            &genreBox, &sourceBox, &situationBox,
            &autoSetButton, &aiSuggestButton, &applyAiButton,
            &analysisLabel, &aiStatusLabel, &aiNotes,
            &vuMeter, &phaseMeter, &grMeter, &ioMeter,
            &freqBalance, &advancedMeters, &quickIssues, &satellitePanel,
            &meterModeButton, &vuModeButton
        };

        for (auto* comp : components)
        {
            if (comp && comp->isVisible())
            {
                auto bounds = comp->getBounds();
                // Check if mouse is near bottom-right corner (resize handle area)
                juce::Rectangle<int> resizeHandle(bounds.getRight() - 10, bounds.getBottom() - 10, 10, 10);
                if (resizeHandle.contains(e.getPosition()))
                {
                    resizedComponent = comp;
                    resizeStartBounds = bounds;
                    dragStartPos = e.getPosition(); // Set drag start pos for resize delta calculation
                    isResizing = true;
                    return; // Exit early - resize operation started
                }
            }
        }
    }

    // SECONDARY: Handle satellite panel clicks (only if no drag/resize operation)
    // Get position in local coordinates
    auto localPos = e.getPosition().toFloat();
    
    // Check if click is in satellite panel area (only when not in standalone mode)
    if (currentTab == 0 && !standaloneMode && satellitePanelBounds.contains(localPos))
    {
        // Calculate which satellite row was clicked
        auto panelArea = satellitePanelBounds;
        panelArea.removeFromTop(22);  // Title
        panelArea = panelArea.reduced(4, 2);
        
        const float rowHeight = 28.0f;  // Match actual row height
        int clickedRow = static_cast<int>((localPos.y - panelArea.getY()) / rowHeight);
        
        // Find the actual satellite at this row
        int visibleRow = 0;
        for (int i = 0; i < MAX_SATELLITES; ++i)
        {
            auto info = processor.getSatelliteInfo(i);
            if (!info.active) continue;
            
            if (visibleRow == clickedRow)
            {
                // Right-click - show context menu (check multiple conditions for reliability)
                if (e.mods.isRightButtonDown() || e.mods.isPopupMenu() || e.mods.isCtrlDown())
                {
                    showSatelliteContextMenu(i, e.getScreenPosition());
                    return;
                }
                else
                {
                    // Left-click - select/deselect satellite
                    selectedSatellite = (selectedSatellite == i) ? -1 : i;
                    repaint();
                    return;
                }
            }
            visibleRow++;
        }
        
        // Clicked in panel but not on a satellite row - deselect
        if (clickedRow >= 0)
        {
            selectedSatellite = -1;
            repaint();
        }
    }
}

void SimpleGainAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e)
{
    // If we have a candidate and haven't officially started a drag/resize, check threshold
    if (draggedComponent == nullptr && !isResizing && candidateComponent != nullptr)
    {
        auto delta = e.getPosition() - candidateStartPos;
        if (std::abs(delta.getX()) > dragThreshold || std::abs(delta.getY()) > dragThreshold)
        {
            // Promote candidate to active drag or resize
            if (candidateIsResize)
            {
                resizedComponent = candidateComponent;
                resizeStartBounds = candidateStartBounds;
                dragStartPos = candidateStartPos;
                isResizing = true;
                DBG("Promoted candidate to RESIZE: " << resizedComponent->getName());
            }
            else
            {
                draggedComponent = candidateComponent;
                dragStartPos = candidateStartPos;
                componentStartBounds = candidateStartBounds;
                DBG("Promoted candidate to DRAG: " << draggedComponent->getName());
            }

            // Clear candidate state now that it's promoted
            candidateComponent = nullptr;
            candidateIsResize = false;
        }
    }

    // Delegate to helper that accepts simple point coordinates
    handleMouseDragAtPoint(e.getPosition());
}

void SimpleGainAudioProcessorEditor::handleMouseDragAtPoint(juce::Point<int> pos)
{
    if (draggedComponent != nullptr)
    {
        auto delta = pos - dragStartPos;
        auto newBounds = componentStartBounds.translated(delta.getX(), delta.getY());

        // Constrain to window bounds with some margin
        auto windowBounds = getLocalBounds().reduced(10);

        // No snapping: simply move the component and clamp to window bounds
        (void)0; // explicit no-op to keep line numbers stable for debug


        // Clamp movement smoothly using start bounds to avoid flip/flop jitter when near edges
        int minX = windowBounds.getX();
        int maxX = windowBounds.getRight() - componentStartBounds.getWidth();
        if (maxX < minX) maxX = minX; // in case component is wider than window, lock to left edge
        int clampedX = juce::jlimit(minX, maxX, componentStartBounds.getX() + (pos.getX() - dragStartPos.getX()));
        newBounds.setX(clampedX);

        int minY = windowBounds.getY() + 60; // keep header area clear
        int maxY = windowBounds.getBottom() - componentStartBounds.getHeight();
        if (maxY < minY) maxY = minY;
        int clampedY = juce::jlimit(minY, maxY, componentStartBounds.getY() + (pos.getY() - dragStartPos.getY()));
        newBounds.setY(clampedY);

        // No snapping/auto-push: do not modify newBounds to avoid proximity snapping / jumping.
        // User requested snapping removed — movement is free and only clamped to the window.
        // Overlap detection and correction are intentionally disabled here.
        draggedComponent->setBounds(newBounds);
        layoutDirty = true; // mark layout changed
        repaint();
    }
    else if (isResizing && resizedComponent != nullptr)
    {
        auto delta = pos - dragStartPos;
        auto newBounds = resizeStartBounds;
        newBounds.setWidth(juce::jmax(50, resizeStartBounds.getWidth() + delta.getX()));
        newBounds.setHeight(juce::jmax(20, resizeStartBounds.getHeight() + delta.getY()));

        // Constrain to window bounds
        auto windowBounds = getLocalBounds().reduced(10);
        if (newBounds.getRight() > windowBounds.getRight())
            newBounds.setWidth(windowBounds.getRight() - newBounds.getX());
        if (newBounds.getBottom() > windowBounds.getBottom())
            newBounds.setHeight(windowBounds.getBottom() - newBounds.getY());

        // Prevent overlap: clamp width/height so the resized component doesn't intrude into others
        juce::Component* components[] = { &vuMeter, &phaseMeter, &grMeter, &ioMeter, &freqBalance, &advancedMeters, &quickIssues, &satellitePanel };
        for (auto* comp : components)
        {
            if (!comp || comp == resizedComponent || !comp->isVisible()) continue;
            auto ob = comp->getBounds();
            // If other component is to the right and vertically overlaps, clamp width
            if (ob.getX() >= newBounds.getX())
            {
                if (!(ob.getBottom() <= newBounds.getY() || ob.getY() >= newBounds.getBottom()))
                {
                    int maxW = ob.getX() - newBounds.getX();
                    newBounds.setWidth(juce::jmax(50, juce::jmin(newBounds.getWidth(), maxW)));
                }
            }
            // If other component is below and horizontally overlaps, clamp height
            if (ob.getY() >= newBounds.getY())
            {
                if (!(ob.getRight() <= newBounds.getX() || ob.getX() >= newBounds.getRight()))
                {
                    int maxH = ob.getY() - newBounds.getY();
                    newBounds.setHeight(juce::jmax(20, juce::jmin(newBounds.getHeight(), maxH)));
                }
            }
        }

        resizedComponent->setBounds(newBounds);
        layoutDirty = true; // mark layout changed
        repaint();
    }
}


void SimpleGainAudioProcessorEditor::mouseUp(const juce::MouseEvent& e)
{
    draggedComponent = nullptr;
    resizedComponent = nullptr;
    isResizing = false;
    candidateComponent = nullptr;
    candidateIsResize = false;

    // Snapping/auto-resolve disabled by user request — do not automatically move other components on mouseUp.

    if (layoutDirty)
    {
        // Persist the new layout
        saveLayoutToFile();
        layoutLoaded = true;
        layoutDirty = false;
        DBG("Layout auto-saved after move/resize");
    }
}


void SimpleGainAudioProcessorEditor::showSatelliteContextMenu(int satIndex, juce::Point<int> screenPos)
{
    auto info = processor.getSatelliteInfo(satIndex);
    if (!info.active) return;
    
    juce::PopupMenu menu;
    
    juce::String satName = info.channelName.isEmpty() ? ("Satellite " + juce::String(satIndex + 1)) : info.channelName;
    menu.addSectionHeader(satName);
    menu.addSeparator();
    
    // Gain submenu
    juce::PopupMenu gainMenu;
    float currentGainDb = 20.0f * std::log10(std::max(0.0001f, info.currentGain));
    gainMenu.addItem("Current: " + juce::String(currentGainDb, 1) + " dB", false, false, nullptr);
    gainMenu.addSeparator();
    gainMenu.addItem("+6 dB", [this, satIndex] { setSatelliteGain(satIndex, 6.0f); });
    gainMenu.addItem("+3 dB", [this, satIndex] { setSatelliteGain(satIndex, 3.0f); });
    gainMenu.addItem("0 dB", [this, satIndex] { setSatelliteGain(satIndex, 0.0f); });
    gainMenu.addItem("-3 dB", [this, satIndex] { setSatelliteGain(satIndex, -3.0f); });
    gainMenu.addItem("-6 dB", [this, satIndex] { setSatelliteGain(satIndex, -6.0f); });
    gainMenu.addItem("-12 dB", [this, satIndex] { setSatelliteGain(satIndex, -12.0f); });
    menu.addSubMenu("Set Gain", gainMenu);
    
    // Target submenu
    juce::PopupMenu targetMenu;
    targetMenu.addItem("Current: " + juce::String(info.targetDb, 1) + " dB", false, false, nullptr);
    targetMenu.addSeparator();
    targetMenu.addItem("-6 dB", [this, satIndex] { setSatelliteTarget(satIndex, -6.0f); });
    targetMenu.addItem("-12 dB", [this, satIndex] { setSatelliteTarget(satIndex, -12.0f); });
    targetMenu.addItem("-18 dB", [this, satIndex] { setSatelliteTarget(satIndex, -18.0f); });
    targetMenu.addItem("-24 dB", [this, satIndex] { setSatelliteTarget(satIndex, -24.0f); });
    targetMenu.addItem("-30 dB", [this, satIndex] { setSatelliteTarget(satIndex, -30.0f); });
    menu.addSubMenu("Set Target", targetMenu);
    
    // Ceiling submenu
    juce::PopupMenu ceilingMenu;
    ceilingMenu.addItem("Current: " + juce::String(info.ceilingDb, 1) + " dB", false, false, nullptr);
    ceilingMenu.addSeparator();
    ceilingMenu.addItem("0 dB (No Limit)", [this, satIndex] { setSatelliteCeiling(satIndex, 0.0f); });
    ceilingMenu.addItem("-3 dB", [this, satIndex] { setSatelliteCeiling(satIndex, -3.0f); });
    ceilingMenu.addItem("-6 dB", [this, satIndex] { setSatelliteCeiling(satIndex, -6.0f); });
    ceilingMenu.addItem("-12 dB", [this, satIndex] { setSatelliteCeiling(satIndex, -12.0f); });
    ceilingMenu.addItem("-18 dB", [this, satIndex] { setSatelliteCeiling(satIndex, -18.0f); });
    menu.addSubMenu("Set Ceiling", ceilingMenu);
    
    // Auto-Set to -18dB target
    float targetDb = -18.0f;
    float neededGain = targetDb - info.rmsDb;
    neededGain = juce::jlimit(-24.0f, 12.0f, neededGain);
    menu.addItem("Auto-Set (" + juce::String(neededGain, 1) + " dB)", [this, satIndex, neededGain] {
        setSatelliteGain(satIndex, neededGain);
    });
    
    // AI Suggest
    menu.addItem("AI Suggest", [this, satIndex, info] {
        const juce::StringArray sourceNames = { "Lead Vocal", "Background Vocal", "Kick", "Snare", "Hi-Hat", 
                                                "Full Drums", "Bass", "Electric Guitar", "Acoustic Guitar", "Keys/Piano", 
                                                "Synth", "Strings", "Other" };
        juce::String sourceName = (info.sourceType >= 0 && info.sourceType < sourceNames.size()) 
                                  ? sourceNames[info.sourceType] : "Other";
        processor.requestAiSuggestion(genreBox.getText(), sourceName, situationBox.getText());
    });
    
    menu.addSeparator();
    
    // Release master control
    if (info.controlledByMaster)
    {
        menu.addItem("Release Control", [this, satIndex] {
            processor.releaseSatelliteControl(satIndex);
        });
    }
    
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea({screenPos.x, screenPos.y, 1, 1}));
}

void SimpleGainAudioProcessorEditor::setSatelliteGain(int satIndex, float gainDb)
{
    SimpleGainAudioProcessor::SatelliteControl ctrl;
    ctrl.gainDb = gainDb;
    ctrl.targetDb = -18.0f; // Default, can be customized
    ctrl.autoEnabled = false;
    ctrl.riderAmount = 0.0f;
    processor.setSatelliteControl(satIndex, ctrl);
}

void SimpleGainAudioProcessorEditor::setSatelliteTarget(int satIndex, float targetDb)
{
    SimpleGainAudioProcessor::SatelliteControl ctrl;
    ctrl.gainDb = 0.0f; // Leave gain unchanged
    ctrl.targetDb = targetDb;
    ctrl.autoEnabled = true;
    ctrl.riderAmount = 0.0f;
    processor.setSatelliteControl(satIndex, ctrl);
}

void SimpleGainAudioProcessorEditor::setSatelliteCeiling(int satIndex, float ceilingDb)
{
    SimpleGainAudioProcessor::SatelliteControl ctrl;
    ctrl.gainDb = 0.0f; // Leave gain unchanged
    ctrl.targetDb = -18.0f; // Default
    ctrl.ceilingDb = ceilingDb;
    ctrl.autoEnabled = false;
    ctrl.riderAmount = 0.0f;
    processor.setSatelliteControl(satIndex, ctrl);
}

void SimpleGainAudioProcessorEditor::setSatelliteAuto(int satIndex, bool enabled)
{
    SimpleGainAudioProcessor::SatelliteControl ctrl;
    ctrl.gainDb = 0.0f;
    ctrl.targetDb = -18.0f;
    ctrl.autoEnabled = enabled;
    ctrl.riderAmount = 0.0f;
    processor.setSatelliteControl(satIndex, ctrl);
}

void SimpleGainAudioProcessorEditor::setSatelliteRider(int satIndex, float amount)
{
    SimpleGainAudioProcessor::SatelliteControl ctrl;
    ctrl.gainDb = 0.0f;
    ctrl.targetDb = -18.0f;
    ctrl.autoEnabled = false;
    ctrl.riderAmount = amount;
    processor.setSatelliteControl(satIndex, ctrl);
}

void SimpleGainAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    juce::ColourGradient bg(theme.bgDark, 0, 0, theme.bgDark.darker(0.25f), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(bg);
    g.fillAll();

    // Draw drag hint when Ctrl/Cmd is held
    if (juce::ModifierKeys::currentModifiers.isCommandDown() || 
        juce::ModifierKeys::currentModifiers.isCtrlDown())
    {
        g.setColour(theme.accent.withAlpha(0.3f));
        g.setFont(juce::FontOptions(12.0f));
        g.drawText("Hold Ctrl/Cmd + drag to move components", 
                  getLocalBounds().reduced(20), juce::Justification::bottomRight);
    }
    // Draw resize hint when Alt is held
    else if (juce::ModifierKeys::currentModifiers.isAltDown())
    {
        g.setColour(theme.accent.withAlpha(0.3f));
        g.setFont(juce::FontOptions(12.0f));
        g.drawText("Hold Alt + drag bottom-right corner to resize components", 
                  getLocalBounds().reduced(20), juce::Justification::bottomRight);
        
        // Draw resize handles for ALL components
        juce::Component* components[] = {
            &gainSlider, &targetDbSlider, &ceilingSlider,
            &autoToggle, &riderToggle, &riderAmountSlider,
            &genreBox, &sourceBox, &situationBox,
            &autoSetButton, &aiSuggestButton, &applyAiButton,
            &analysisLabel, &aiStatusLabel, &aiNotes,
            &vuMeter, &phaseMeter, &grMeter, &ioMeter,
            &freqBalance, &advancedMeters, &quickIssues, &satellitePanel,
            &meterModeButton, &vuModeButton
        };

        g.setColour(theme.accent.withAlpha(0.7f));
        for (auto* comp : components)
        {
            if (comp && comp->isVisible())
            {
                auto bounds = comp->getBounds();
                // Draw resize handle at bottom-right corner
                g.fillRect(bounds.getRight() - 8, bounds.getBottom() - 8, 8, 8);
            }
        }
    }
}

void SimpleGainAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // If in standalone mode, controls work directly on this track (no satellite control)
    if (standaloneMode)
        return;
    
    // If a satellite is selected, control that satellite; otherwise control all satellites
    if (selectedSatellite >= 0)
    {
        if (slider == &gainSlider)
        {
            float gainDb = static_cast<float>(gainSlider.getValue());
            setSatelliteGain(selectedSatellite, gainDb);
        }
        else if (slider == &targetDbSlider)
        {
            float targetDb = static_cast<float>(targetDbSlider.getValue());
            setSatelliteTarget(selectedSatellite, targetDb);
        }
        else if (slider == &ceilingSlider)
        {
            float ceilingDb = static_cast<float>(ceilingSlider.getValue());
            setSatelliteCeiling(selectedSatellite, ceilingDb);
        }
    }
    else
    {
        // No satellite selected - control all satellites
        if (slider == &gainSlider)
        {
            float gainDb = static_cast<float>(gainSlider.getValue());
            processor.setAllSatellitesGain(gainDb);
        }
        else if (slider == &targetDbSlider)
        {
            float targetDb = static_cast<float>(targetDbSlider.getValue());
            processor.setAllSatellitesTarget(targetDb);
        }
        else if (slider == &ceilingSlider)
        {
            float ceilingDb = static_cast<float>(ceilingSlider.getValue());
            processor.setAllSatellitesCeiling(ceilingDb);
        }
    }
}

void SimpleGainAudioProcessorEditor::timerCallback()
{
    const auto analysis = processor.getAnalysisSnapshot();
    
    // Select pre or post levels based on toggle
    const float displayRmsDb = showPreMeter ? analysis.preRmsDb : analysis.postRmsDb;
    const float displayPeakDb = showPreMeter ? analysis.prePeakDb : analysis.postPeakDb;
    const float displayCrestDb = showPreMeter ? analysis.preCrestDb : analysis.postCrestDb;
    const float displayPhase = showPreMeter ? analysis.prePhaseCorrelation : analysis.postPhaseCorrelation;
    
    // Moderate smoothing for readable but responsive numeric displays
    const float textSmooth = 0.85f;
    
    smoothedRmsDb = textSmooth * smoothedRmsDb + (1.0f - textSmooth) * displayRmsDb;
    
    // Peak uses fast attack (instant), slow release for accurate peak reading
    if (displayPeakDb > smoothedPeakDb)
        smoothedPeakDb = displayPeakDb;  // Instant attack for accurate peak
    else
        smoothedPeakDb = 0.92f * smoothedPeakDb + 0.08f * displayPeakDb;  // Slow release
    
    phaseCorrelation = textSmooth * phaseCorrelation + (1.0f - textSmooth) * displayPhase;
    crestFactorDb = textSmooth * crestFactorDb + (1.0f - textSmooth) * displayCrestDb;
    
    // Track input and output RMS
    inputRmsDb = analysis.preRmsDb;
    outputRmsDb = analysis.postRmsDb;
    
    // Calculate gain reduction
    gainReductionDb = inputRmsDb - outputRmsDb;
    
    // Advanced metering updates - slower smoothing for readability
    stereoWidth = 0.97f * stereoWidth + 0.03f * analysis.stereoWidth;
    shortTermLufs = 0.97f * shortTermLufs + 0.03f * analysis.shortTermLufs;
    integratedLufs = analysis.integratedLufs;
    truePeakDb = std::max(truePeakDb * 0.9995f, analysis.truePeak);  // Slow decay
    clipCount = analysis.clipCount;
    headroom = 0.0f - analysis.postPeakDb;
    lowEnergy = 0.95f * lowEnergy + 0.05f * analysis.lowEnergy;
    midEnergy = 0.95f * midEnergy + 0.05f * analysis.midEnergy;
    highEnergy = 0.95f * highEnergy + 0.05f * analysis.highEnergy;
    monoCompatible = analysis.monoCompatible;

    if (analysis.postPeakDb > peakHoldDb) { peakHoldDb = analysis.postPeakDb; peakHoldCounter = 60; }
    else if (peakHoldCounter > 0) peakHoldCounter--;
    else peakHoldDb = std::max(peakHoldDb - 0.4f, analysis.postPeakDb);

    if (auto* p = processor.getValueTreeState().getRawParameterValue("auto_target_db"))
        presetSuggestedDb = p->load();
    
    // Track LUFS target and enabled state
    if (auto* p = processor.getValueTreeState().getRawParameterValue("lufs_target"))
        lufsTarget = p->load();
    if (auto* p = processor.getValueTreeState().getRawParameterValue("lufs_enabled"))
        lufsEnabled = p->load() > 0.5f;
    
    // Update AI suggested target from processor
    aiSuggestedDb = processor.getAiSuggestedTargetDb();

    // Show pre or post in the analysis label based on toggle
    juce::String modeLabel = showPreMeter ? "[PRE] " : "[POST] ";
    analysisLabel.setText(modeLabel + "RMS: " + juce::String(displayRmsDb, 1) + " dB  |  Peak: " +
                          juce::String(displayPeakDb, 1) + " dB  |  Crest: " +
                          juce::String(displayCrestDb, 1) + " dB", juce::dontSendNotification);

    auto notes = processor.getAiNotes();
    aiNotes.setText(notes, false);
    aiStatusLabel.setText("Status: " + processor.getAiStatus(), juce::dontSendNotification);

    // Update chat with AI response (using separate chat response tracking)
    if (currentTab == 2)
    {
        const auto chatVer = processor.getChatResponseVersion();
        if (chatVer != lastChatVersion)
        {
            lastChatVersion = chatVer;
            auto chatResp = processor.getChatResponse();
            if (chatResp.isNotEmpty())
            {
                auto currentText = chatHistory.getText();
                if (currentText.contains("ARES: Thinking..."))
                {
                    chatHistory.setText(currentText.replace("ARES: Thinking...", "ARES: " + chatResp));
                }
                else
                {
                    chatHistory.moveCaretToEnd();
                    chatHistory.insertTextAtCaret("\nARES: " + chatResp + "\n");
                }
            }
        }
    }

    const auto modelsVer = processor.getAvailableModelsVersion();
    if (modelsVer != lastModelsVersion)
    {
        lastModelsVersion = modelsVer;
        auto models = processor.getAvailableModels();
        auto savedModel = processor.getSelectedModel();  // Get saved model from processor
        auto currentText = modelBox.getText();
        
        modelBox.clear(juce::dontSendNotification);
        modelBox.addItemList(models, 1);
        
        // Try to select saved model first, then current text
        int selectId = 1;
        auto savedIdx = models.indexOf(savedModel);
        auto currentIdx = models.indexOf(currentText);
        
        if (savedIdx >= 0)
            selectId = savedIdx + 1;
        else if (currentIdx >= 0)
            selectId = currentIdx + 1;
        
        if (modelBox.getNumItems() > 0) 
            modelBox.setSelectedId(selectId, juce::dontSendNotification);
    }

    // Clear satellite selection if it becomes inactive
    if (selectedSatellite >= 0)
    {
        auto info = processor.getSatelliteInfo(selectedSatellite);
        if (!info.active)
            selectedSatellite = -1;
    }

    repaint();
}

void SimpleGainAudioProcessorEditor::handleSatelliteClick(int index)
{
    // Toggle selection for visual highlight
    auto info = processor.getSatelliteInfo(index);
    if (!info.active) return;
    
    selectedSatellite = (selectedSatellite == index) ? -1 : index;
    repaint();
}

void SimpleGainAudioProcessorEditor::saveLayoutToFile()
{
    auto layoutFile = getLayoutFile();
    juce::XmlDocument doc(layoutFile);
    
    auto root = std::make_unique<juce::XmlElement>("AR3S_Layout");
    // persist auto-fit and edit mode
    root->setAttribute("autoFit", autoFitMode);
    root->setAttribute("editMode", editMode);
    
    // Save component positions and sizes
    juce::Component* components[] = {
        &gainSlider, &targetDbSlider, &ceilingSlider,
        &autoToggle, &riderToggle, &riderAmountSlider,
        &genreBox, &sourceBox, &situationBox,
        &autoSetButton, &aiSuggestButton, &applyAiButton,
        &analysisLabel, &aiStatusLabel, &aiNotes,
        &vuMeter, &phaseMeter, &grMeter, &ioMeter,
        &freqBalance, &advancedMeters, &quickIssues, &satellitePanel,
        &meterModeButton, &vuModeButton
    };
    
    const char* componentNames[] = {
        "gainSlider", "targetDbSlider", "ceilingSlider",
        "autoToggle", "riderToggle", "riderAmountSlider",
        "genreBox", "sourceBox", "situationBox",
        "autoSetButton", "aiSuggestButton", "applyAiButton",
        "analysisLabel", "aiStatusLabel", "aiNotes",
        "vuMeter", "phaseMeter", "grMeter", "ioMeter",
        "freqBalance", "advancedMeters", "quickIssues", "satellitePanel",
        "meterModeButton", "vuModeButton"
    };
    
    for (int i = 0; i < sizeof(components) / sizeof(components[0]); ++i)
    {
        if (components[i])
        {
            auto compElement = std::make_unique<juce::XmlElement>(componentNames[i]);
            auto bounds = components[i]->getBounds();
            compElement->setAttribute("x", bounds.getX());
            compElement->setAttribute("y", bounds.getY());
            compElement->setAttribute("width", bounds.getWidth());
            compElement->setAttribute("height", bounds.getHeight());
            compElement->setAttribute("visible", components[i]->isVisible());
            if (auto* m = dynamic_cast<SimpleGainAudioProcessorEditor::MovableComponent*>(components[i]))
                compElement->setAttribute("locked", m->isLocked());
            root->addChildElement(compElement.release());
        }
    }
    
    if (root->writeTo(layoutFile))
    {
        DBG("Layout saved to: " + layoutFile.getFullPathName());
    }
}

void SimpleGainAudioProcessorEditor::loadLayoutFromFile()
{
    auto layoutFile = getLayoutFile();
    if (!layoutFile.existsAsFile())
        return;
    
    std::unique_ptr<juce::XmlElement> root(juce::XmlDocument::parse(layoutFile));
    if (!root || root->getTagName() != "AR3S_Layout")
        return;
    
    // Load component positions and sizes
    juce::Component* components[] = {
        &gainSlider, &targetDbSlider, &ceilingSlider,
        &autoToggle, &riderToggle, &riderAmountSlider,
        &genreBox, &sourceBox, &situationBox,
        &autoSetButton, &aiSuggestButton, &applyAiButton,
        &analysisLabel, &aiStatusLabel, &aiNotes,
        &vuMeter, &phaseMeter, &grMeter, &ioMeter,
        &freqBalance, &advancedMeters, &quickIssues, &satellitePanel,
        &meterModeButton, &vuModeButton
    };
    
    const char* componentNames[] = {
        "gainSlider", "targetDbSlider", "ceilingSlider",
        "autoToggle", "riderToggle", "riderAmountSlider",
        "genreBox", "sourceBox", "situationBox",
        "autoSetButton", "aiSuggestButton", "applyAiButton",
        "analysisLabel", "aiStatusLabel", "aiNotes",
        "vuMeter", "phaseMeter", "grMeter", "ioMeter",
        "freqBalance", "advancedMeters", "quickIssues", "satellitePanel",
        "meterModeButton", "vuModeButton"
    };
    
    for (int i = 0; i < sizeof(components) / sizeof(components[0]); ++i)
    {
        if (auto compElement = root->getChildByName(componentNames[i]))
        {
            juce::Rectangle<int> bounds(
                compElement->getIntAttribute("x"),
                compElement->getIntAttribute("y"),
                compElement->getIntAttribute("width"),
                compElement->getIntAttribute("height")
            );
            
            components[i]->setBounds(bounds);
            components[i]->setVisible(compElement->getBoolAttribute("visible", true));
            if (auto* m = dynamic_cast<SimpleGainAudioProcessorEditor::MovableComponent*>(components[i]))
                m->setLocked(compElement->getBoolAttribute("locked", false));
        }
    }
    
    DBG("Layout loaded from: " + layoutFile.getFullPathName());
    // restore auto-fit/edit mode if present
    autoFitMode = root->getBoolAttribute("autoFit", autoFitMode);
    editMode = root->getBoolAttribute("editMode", editMode);
    autoFitToggle.setToggleState(autoFitMode, juce::dontSendNotification);
    editModeToggle.setToggleState(editMode, juce::dontSendNotification);

    layoutLoaded = true;
    // Ensure loaded layout is clamped to the current window
    resized();
    repaint();

    // Record the current window bounds to support proportional scaling on future window resizes
    lastWindowBounds = getLocalBounds().reduced(10);
}

void SimpleGainAudioProcessorEditor::applyAutoLayout()
{
    // Apply the default 'auto-fit' layout for Main tab
    auto area = getLocalBounds().reduced(15);
    auto headerArea = area.removeFromTop(55);

    // Column widths tuned for auto-fit
    int totalWidth = area.getWidth();
    int controlsWidth = 220;
    int vuAnalysisWidth = 320;
    int meterWidth = 520;

    auto controlsCol = area.removeFromLeft(controlsWidth).reduced(5);
    auto vuCol = area.removeFromLeft(vuAnalysisWidth).reduced(5);
    auto meterCol = area.removeFromLeft(meterWidth).reduced(5);
    auto rightCol = area.reduced(5);

    // Controls column
    auto gainArea = controlsCol.removeFromTop(110).reduced(3);
    gainLabel.setBounds(gainArea.removeFromTop(16));
    gainSlider.setBounds(gainArea);
    controlsCol.removeFromTop(4);
    auto targetArea = controlsCol.removeFromTop(110).reduced(3);
    targetDbLabel.setBounds(targetArea.removeFromTop(16));
    targetDbSlider.setBounds(targetArea);
    controlsCol.removeFromTop(4);
    auto ceilingArea = controlsCol.removeFromTop(110).reduced(3);
    ceilingLabel.setBounds(ceilingArea.removeFromTop(16));
    ceilingSlider.setBounds(ceilingArea);

    // VU + analysis column
    ioMeter.setBounds(vuCol.removeFromTop(220).reduced(3));
    vuCol.removeFromTop(6);
    freqBalance.setBounds(vuCol.removeFromTop(200).reduced(3));
    vuCol.removeFromTop(6);
    advancedMeters.setBounds(vuCol.removeFromTop(60).reduced(3));
    vuCol.removeFromTop(4);
    quickIssues.setBounds(vuCol.reduced(3));

    // Meter column
    auto meterModeArea = meterCol.removeFromTop(28);
    int btnW = 60;
    meterModeButton.setBounds(meterModeArea.removeFromLeft(btnW).withTrimmedTop(2));
    vuMeter.setBounds(meterCol);

    // Right column
    auto contextArea = rightCol.removeFromTop(120);
    auto row1 = contextArea.removeFromTop(42);
    auto genreArea = row1.removeFromLeft(row1.getWidth() / 2).reduced(3);
    genreLabel.setBounds(genreArea.removeFromTop(16));
    genreBox.setBounds(genreArea);
    auto sourceArea = row1.reduced(3);
    sourceLabel.setBounds(sourceArea.removeFromTop(16));
    sourceBox.setBounds(sourceArea);
    contextArea.removeFromTop(4);
    auto row2 = contextArea.removeFromTop(42);
    auto sitArea = row2.removeFromLeft(row2.getWidth() / 2).reduced(3);
    situationLabel.setBounds(sitArea.removeFromTop(16));
    situationBox.setBounds(sitArea);
    auto standAloneArea = row2.reduced(3);
    standaloneModeToggle.setBounds(standAloneArea.removeFromTop(26).withTrimmedTop(10));

    rightCol.removeFromTop(10);
    auto platformRow = rightCol.removeFromTop(26);
    platformTargetLabel.setBounds(platformRow.removeFromLeft(55).reduced(2, 0));
    platformTargetBox.setBounds(platformRow.reduced(2, 0));

    rightCol.removeFromTop(6);
    analysisLabel.setBounds(rightCol.removeFromTop(18).reduced(3, 0));
    aiStatusLabel.setBounds(rightCol.removeFromTop(18).reduced(3, 0));
    rightCol.removeFromTop(4);
    auto aiNotesArea = rightCol.removeFromTop(rightCol.getHeight() / 2 - 5);
    aiNotes.setBounds(aiNotesArea.reduced(3));
    rightCol.removeFromTop(8);
    satellitePanel.setBounds(rightCol.reduced(3));

    // Ensure resize handle sits on top
    resizeCorner->toFront(true);
}

void SimpleGainAudioProcessorEditor::resolveAllOverlaps(bool animate)
{
    juce::Component* components[] = { &vuMeter, &phaseMeter, &grMeter, &ioMeter, &freqBalance, &advancedMeters, &quickIssues, &satellitePanel };

    auto intersectsAny = [&](juce::Rectangle<int> r, juce::Component* skip){
        for (auto* other : components)
        {
            if (!other || other == skip || !other->isVisible()) continue;
            if (r.intersects(other->getBounds())) return true;
        }
        return false;
    };

    for (auto* comp : components)
    {
        if (!comp || !comp->isVisible()) continue;
        auto b = comp->getBounds();
        // If it overlaps anybody, find minimal shift to resolve
        bool changed = false;
        for (auto* other : components)
        {
            if (!other || other == comp || !other->isVisible()) continue;
            auto ob = other->getBounds();
            if (!b.intersects(ob)) continue;

            // Candidate positions: left, right, top, bottom
            juce::Rectangle<int> left(ob.getX() - b.getWidth(), b.getY(), b.getWidth(), b.getHeight());
            juce::Rectangle<int> right(ob.getRight(), b.getY(), b.getWidth(), b.getHeight());
            juce::Rectangle<int> top(b.getX(), ob.getY() - b.getHeight(), b.getWidth(), b.getHeight());
            juce::Rectangle<int> bottom(b.getX(), ob.getBottom(), b.getWidth(), b.getHeight());

            // Filter candidates which do not intersect any other components and are within window
            juce::Rectangle<int> windowBounds = getLocalBounds().reduced(10);
            std::vector<std::pair<int, juce::Rectangle<int>>> candidates;
            auto pushIfValid = [&](juce::Rectangle<int> candidate){
                if (!candidate.intersects(ob) && candidate.getX() >= windowBounds.getX() && candidate.getRight() <= windowBounds.getRight()
                    && candidate.getY() >= windowBounds.getY() + 60 && candidate.getBottom() <= windowBounds.getBottom() && !intersectsAny(candidate, comp))
                {
                    int dist = std::abs(candidate.getX() - b.getX()) + std::abs(candidate.getY() - b.getY());
                    candidates.emplace_back(dist, candidate);
                }
            };
            pushIfValid(left); pushIfValid(right); pushIfValid(top); pushIfValid(bottom);

            if (!candidates.empty())
            {
                // choose minimal distance candidate
                std::sort(candidates.begin(), candidates.end(), [](auto&a, auto&b){ return a.first < b.first; });
                auto chosen = candidates.front().second;
                if (animate)
                    animator.animateComponent(comp, chosen, 1.0f, 120, false, 0.3, 1.0);
                else
                    comp->setBounds(chosen);
                changed = true;
                b = chosen;
            }
        }

        // if changed then continue checking others in next pass, loop until stable
        if (changed) resolveAllOverlaps(animate);
    }
}

void SimpleGainAudioProcessorEditor::handleSatellitePanelMouse(const juce::MouseEvent& e, juce::Rectangle<float> bounds)
{
    // Handle mouse events for the satellite panel component
    // This method can be extended for satellite panel specific interactions
    // For now, just pass through to normal component handling
    startComponentDrag(static_cast<juce::Component*>(&satellitePanel), e);
}

// New helper that performs the same logic as startComponentDrag but accepts coordinates and modifier keys
void SimpleGainAudioProcessorEditor::startComponentDragAt(juce::Component* component, juce::Point<int> pos, juce::ModifierKeys mods)
{
    DBG("startComponentDragAt called for component");
    if (!component) return;

    // If component supports locking, respect it (no drag/resize when locked)
    if (auto* m = dynamic_cast<SimpleGainAudioProcessorEditor::MovableComponent*>(component))
    {
        if (m->isLocked())
        {
            DBG("Component is locked, ignoring drag/resize request");
            return;
        }
    }

    // If Auto-fit mode is active and not in edit mode, disallow manual drag/resize to prevent layout conflicts
    if (autoFitMode && !editMode)
    {
        DBG("Auto-fit active and edit mode off — ignoring manual drag/resize request");
        return;
    }

    // Use realtime modifiers to determine behavior and require modifiers (Cmd/Ctrl to move, Alt to resize)
    auto realMods = juce::ModifierKeys::getCurrentModifiersRealtime();

    if (realMods.isCommandDown() || realMods.isCtrlDown())
    {
        DBG("Starting drag operation (modifier)");
        draggedComponent = component;
        dragStartPos = pos;
        componentStartBounds = component->getBounds();
        // clear any prior candidate
        candidateComponent = nullptr;
        candidateIsResize = false;
        return;
    }

    if (realMods.isAltDown())
    {
        auto bounds = component->getBounds();
        // Larger resize handle to make option-resize easier
        juce::Rectangle<int> resizeHandle(bounds.getRight() - 16, bounds.getBottom() - 16, 16, 16);
        if (resizeHandle.contains(pos))
        {
            DBG("Starting resize operation (modifier)");
            resizedComponent = component;
            resizeStartBounds = bounds;
            dragStartPos = pos; // Set drag start pos for resize delta calculation
            isResizing = true;
            candidateComponent = nullptr;
            candidateIsResize = false;
            return;
        }
    }

    // No modifiers: register as a candidate for delayed drag/resize (click-and-hold or move beyond threshold will promote)
    candidateComponent = component;
    candidateStartPos = pos;
    candidateStartBounds = component->getBounds();
    // Detect if click is on the larger resize handle area
    auto bounds = component->getBounds();
    juce::Rectangle<int> resizeHandle(bounds.getRight() - 16, bounds.getBottom() - 16, 16, 16);
    candidateIsResize = resizeHandle.contains(pos);
    DBG("Candidate registered: " << component->getName() << " at " << candidateStartPos.getX() << "," << candidateStartPos.getY() << (candidateIsResize?" (resize)":" (drag)"));
    return;
}

// Backwards-compatible wrapper used by actual mouse events
void SimpleGainAudioProcessorEditor::startComponentDrag(juce::Component* component, const juce::MouseEvent& e)
{
    startComponentDragAt(component, e.getPosition(), e.mods);
}



juce::File SimpleGainAudioProcessorEditor::getLayoutFile() const
{
    auto userAppData = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    auto aresDir = userAppData.getChildFile("ARES");
    aresDir.createDirectory();
    return aresDir.getChildFile("ar3s_layout.xml");
}

