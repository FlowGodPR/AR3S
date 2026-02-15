#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Theme IDs - shared between master and satellites
enum class ThemeType
{
    Vintage = 0,      // Rust, beat up, analog warmth
    ModernDark,       // Clean dark modern
    Futuristic,       // Neon, sci-fi, glowing
    UFO,              // Alien green, otherworldly
    SleekBlack,       // Pure black with minimal color
    UADConsole,       // Professional console style (like UAD/Luna)
    SSL,              // SSL inspired gray/blue
    Neve,             // Neve inspired warm gray/red
    Sunset,           // Warm orange/pink gradient feel
    Ocean,            // Deep blue ocean vibes
    Midnight,         // Deep purple/blue night
    RedRoom,          // Dark red studio vibe
    COUNT
};

// Complete theme color set
struct ThemeColors
{
    juce::Colour bgDark;
    juce::Colour bgPanel;
    juce::Colour bgPanelAlt;     // For alternating or highlight panels
    juce::Colour accent;
    juce::Colour accentAlt;
    juce::Colour textBright;
    juce::Colour textDim;
    juce::Colour meterGreen;
    juce::Colour meterYellow;
    juce::Colour meterRed;
    juce::Colour knobFill;       // Knob body color
    juce::Colour knobOutline;    // Knob edge
    juce::Colour vuBackground;   // VU meter background
    juce::Colour vuNeedle;       // VU needle color
    bool hasTexture;             // For vintage worn look
    float cornerRadius;          // Sharp (0) to rounded (8+)
};

inline ThemeColors getTheme(ThemeType type)
{
    ThemeColors t;
    
    switch (type)
    {
        case ThemeType::Vintage:
            // Warm, rusty, beat-up analog look
            t.bgDark       = juce::Colour::fromRGB(35, 28, 22);       // Dark brown
            t.bgPanel      = juce::Colour::fromRGB(55, 45, 35);       // Warm brown
            t.bgPanelAlt   = juce::Colour::fromRGB(70, 55, 42);       // Lighter brown
            t.accent       = juce::Colour::fromRGB(180, 90, 40);      // Rust orange
            t.accentAlt    = juce::Colour::fromRGB(140, 70, 30);      // Darker rust
            t.textBright   = juce::Colour::fromRGB(235, 220, 190);    // Cream
            t.textDim      = juce::Colour::fromRGB(145, 125, 100);    // Faded tan
            t.meterGreen   = juce::Colour::fromRGB(90, 160, 70);      // Vintage green
            t.meterYellow  = juce::Colour::fromRGB(200, 160, 60);     // Amber
            t.meterRed     = juce::Colour::fromRGB(180, 60, 50);      // Vintage red
            t.knobFill     = juce::Colour::fromRGB(60, 50, 40);       // Worn metal
            t.knobOutline  = juce::Colour::fromRGB(100, 85, 70);      // Brass edge
            t.vuBackground = juce::Colour::fromRGB(230, 200, 150);    // Aged paper
            t.vuNeedle     = juce::Colour::fromRGB(40, 35, 30);       // Dark needle
            t.hasTexture   = true;
            t.cornerRadius = 2.0f;
            break;
            
        case ThemeType::ModernDark:
            // Clean, professional dark UI
            t.bgDark       = juce::Colour::fromRGB(18, 18, 22);       // Near black
            t.bgPanel      = juce::Colour::fromRGB(28, 30, 36);       // Dark gray
            t.bgPanelAlt   = juce::Colour::fromRGB(38, 42, 50);       // Lighter gray
            t.accent       = juce::Colour::fromRGB(80, 160, 255);     // Blue
            t.accentAlt    = juce::Colour::fromRGB(100, 180, 255);    // Lighter blue
            t.textBright   = juce::Colour::fromRGB(240, 242, 248);    // White
            t.textDim      = juce::Colour::fromRGB(110, 115, 130);    // Gray
            t.meterGreen   = juce::Colour::fromRGB(50, 200, 100);     // Clean green
            t.meterYellow  = juce::Colour::fromRGB(255, 200, 60);     // Yellow
            t.meterRed     = juce::Colour::fromRGB(255, 70, 80);      // Red
            t.knobFill     = juce::Colour::fromRGB(45, 48, 55);       // Dark metal
            t.knobOutline  = juce::Colour::fromRGB(70, 75, 85);       // Metal edge
            t.vuBackground = juce::Colour::fromRGB(35, 38, 45);       // Dark display
            t.vuNeedle     = juce::Colour::fromRGB(255, 100, 80);     // Orange needle
            t.hasTexture   = false;
            t.cornerRadius = 6.0f;
            break;
            
        case ThemeType::Futuristic:
            // Neon, sci-fi, glowing effects
            t.bgDark       = juce::Colour::fromRGB(8, 10, 20);        // Deep blue-black
            t.bgPanel      = juce::Colour::fromRGB(15, 20, 35);       // Dark blue
            t.bgPanelAlt   = juce::Colour::fromRGB(22, 28, 48);       // Lighter blue
            t.accent       = juce::Colour::fromRGB(0, 255, 220);      // Cyan
            t.accentAlt    = juce::Colour::fromRGB(180, 80, 255);     // Purple
            t.textBright   = juce::Colour::fromRGB(220, 255, 250);    // Cyan-white
            t.textDim      = juce::Colour::fromRGB(80, 120, 140);     // Blue-gray
            t.meterGreen   = juce::Colour::fromRGB(0, 255, 150);      // Neon green
            t.meterYellow  = juce::Colour::fromRGB(255, 220, 0);      // Neon yellow
            t.meterRed     = juce::Colour::fromRGB(255, 50, 100);     // Neon pink
            t.knobFill     = juce::Colour::fromRGB(20, 25, 40);       // Dark blue
            t.knobOutline  = juce::Colour::fromRGB(0, 200, 180);      // Cyan edge
            t.vuBackground = juce::Colour::fromRGB(10, 15, 25);       // Dark display
            t.vuNeedle     = juce::Colour::fromRGB(0, 255, 200);      // Cyan needle
            t.hasTexture   = false;
            t.cornerRadius = 8.0f;
            break;
            
        case ThemeType::UFO:
            // Alien, otherworldly green
            t.bgDark       = juce::Colour::fromRGB(5, 12, 8);         // Dark green-black
            t.bgPanel      = juce::Colour::fromRGB(12, 25, 18);       // Dark forest
            t.bgPanelAlt   = juce::Colour::fromRGB(18, 35, 25);       // Lighter forest
            t.accent       = juce::Colour::fromRGB(80, 255, 120);     // Alien green
            t.accentAlt    = juce::Colour::fromRGB(150, 255, 100);    // Lime green
            t.textBright   = juce::Colour::fromRGB(200, 255, 210);    // Light green
            t.textDim      = juce::Colour::fromRGB(80, 130, 90);      // Muted green
            t.meterGreen   = juce::Colour::fromRGB(100, 255, 100);    // Bright green
            t.meterYellow  = juce::Colour::fromRGB(200, 255, 80);     // Yellow-green
            t.meterRed     = juce::Colour::fromRGB(255, 100, 80);     // Orange-red
            t.knobFill     = juce::Colour::fromRGB(15, 30, 20);       // Dark green
            t.knobOutline  = juce::Colour::fromRGB(60, 180, 80);      // Green edge
            t.vuBackground = juce::Colour::fromRGB(8, 18, 12);        // Dark display
            t.vuNeedle     = juce::Colour::fromRGB(100, 255, 120);    // Green needle
            t.hasTexture   = false;
            t.cornerRadius = 4.0f;
            break;
            
        case ThemeType::SleekBlack:
            // Pure black, minimal color - only knobs/meters/text visible
            t.bgDark       = juce::Colour::fromRGB(0, 0, 0);          // Pure black
            t.bgPanel      = juce::Colour::fromRGB(8, 8, 8);          // Nearly black
            t.bgPanelAlt   = juce::Colour::fromRGB(15, 15, 15);       // Slight gray
            t.accent       = juce::Colour::fromRGB(255, 255, 255);    // White accent
            t.accentAlt    = juce::Colour::fromRGB(180, 180, 180);    // Gray accent
            t.textBright   = juce::Colour::fromRGB(255, 255, 255);    // Pure white
            t.textDim      = juce::Colour::fromRGB(80, 80, 80);       // Dark gray
            t.meterGreen   = juce::Colour::fromRGB(60, 220, 100);     // Green
            t.meterYellow  = juce::Colour::fromRGB(255, 200, 50);     // Yellow
            t.meterRed     = juce::Colour::fromRGB(255, 60, 70);      // Red
            t.knobFill     = juce::Colour::fromRGB(25, 25, 25);       // Dark gray
            t.knobOutline  = juce::Colour::fromRGB(60, 60, 60);       // Medium gray
            t.vuBackground = juce::Colour::fromRGB(5, 5, 5);          // Near black
            t.vuNeedle     = juce::Colour::fromRGB(255, 80, 60);      // Orange needle
            t.hasTexture   = false;
            t.cornerRadius = 0.0f;  // Sharp corners
            break;
            
        case ThemeType::UADConsole:
            // Professional console style (like UAD/Luna)
            t.bgDark       = juce::Colour::fromRGB(32, 34, 38);       // Console gray
            t.bgPanel      = juce::Colour::fromRGB(48, 52, 58);       // Panel gray
            t.bgPanelAlt   = juce::Colour::fromRGB(58, 62, 70);       // Lighter panel
            t.accent       = juce::Colour::fromRGB(255, 180, 60);     // Gold/amber
            t.accentAlt    = juce::Colour::fromRGB(255, 140, 40);     // Darker gold
            t.textBright   = juce::Colour::fromRGB(248, 248, 245);    // Off-white
            t.textDim      = juce::Colour::fromRGB(140, 145, 155);    // Gray text
            t.meterGreen   = juce::Colour::fromRGB(80, 200, 90);      // Console green
            t.meterYellow  = juce::Colour::fromRGB(255, 200, 50);     // Amber
            t.meterRed     = juce::Colour::fromRGB(255, 65, 65);      // Alert red
            t.knobFill     = juce::Colour::fromRGB(55, 58, 65);       // Metal gray
            t.knobOutline  = juce::Colour::fromRGB(90, 95, 105);      // Light edge
            t.vuBackground = juce::Colour::fromRGB(40, 42, 48);       // Dark display
            t.vuNeedle     = juce::Colour::fromRGB(255, 160, 50);     // Gold needle
            t.hasTexture   = false;
            t.cornerRadius = 4.0f;
            break;
            
        case ThemeType::SSL:
            // SSL Console inspired - professional gray with blue accents
            t.bgDark       = juce::Colour::fromRGB(38, 42, 48);
            t.bgPanel      = juce::Colour::fromRGB(52, 58, 66);
            t.bgPanelAlt   = juce::Colour::fromRGB(65, 72, 82);
            t.accent       = juce::Colour::fromRGB(90, 160, 220);     // SSL blue
            t.accentAlt    = juce::Colour::fromRGB(70, 140, 200);
            t.textBright   = juce::Colour::fromRGB(245, 248, 252);
            t.textDim      = juce::Colour::fromRGB(130, 140, 155);
            t.meterGreen   = juce::Colour::fromRGB(70, 200, 100);
            t.meterYellow  = juce::Colour::fromRGB(255, 210, 60);
            t.meterRed     = juce::Colour::fromRGB(255, 70, 70);
            t.knobFill     = juce::Colour::fromRGB(60, 65, 75);
            t.knobOutline  = juce::Colour::fromRGB(100, 108, 120);
            t.vuBackground = juce::Colour::fromRGB(45, 50, 58);
            t.vuNeedle     = juce::Colour::fromRGB(90, 160, 220);
            t.hasTexture   = false;
            t.cornerRadius = 4.0f;
            break;
            
        case ThemeType::Neve:
            // Neve inspired - warm gray with red/maroon accents
            t.bgDark       = juce::Colour::fromRGB(42, 38, 36);
            t.bgPanel      = juce::Colour::fromRGB(58, 52, 48);
            t.bgPanelAlt   = juce::Colour::fromRGB(72, 65, 60);
            t.accent       = juce::Colour::fromRGB(180, 60, 50);      // Neve red
            t.accentAlt    = juce::Colour::fromRGB(150, 45, 40);
            t.textBright   = juce::Colour::fromRGB(248, 242, 238);
            t.textDim      = juce::Colour::fromRGB(145, 135, 125);
            t.meterGreen   = juce::Colour::fromRGB(80, 180, 80);
            t.meterYellow  = juce::Colour::fromRGB(240, 190, 60);
            t.meterRed     = juce::Colour::fromRGB(220, 55, 50);
            t.knobFill     = juce::Colour::fromRGB(55, 50, 45);
            t.knobOutline  = juce::Colour::fromRGB(95, 85, 78);
            t.vuBackground = juce::Colour::fromRGB(48, 44, 40);
            t.vuNeedle     = juce::Colour::fromRGB(180, 60, 50);
            t.hasTexture   = true;
            t.cornerRadius = 3.0f;
            break;
            
        case ThemeType::Sunset:
            // Warm sunset - orange/pink gradient feel
            t.bgDark       = juce::Colour::fromRGB(28, 20, 25);
            t.bgPanel      = juce::Colour::fromRGB(45, 32, 38);
            t.bgPanelAlt   = juce::Colour::fromRGB(60, 42, 50);
            t.accent       = juce::Colour::fromRGB(255, 130, 80);     // Sunset orange
            t.accentAlt    = juce::Colour::fromRGB(255, 100, 120);    // Pink
            t.textBright   = juce::Colour::fromRGB(255, 245, 240);
            t.textDim      = juce::Colour::fromRGB(160, 130, 135);
            t.meterGreen   = juce::Colour::fromRGB(100, 220, 120);
            t.meterYellow  = juce::Colour::fromRGB(255, 200, 80);
            t.meterRed     = juce::Colour::fromRGB(255, 80, 100);
            t.knobFill     = juce::Colour::fromRGB(50, 38, 42);
            t.knobOutline  = juce::Colour::fromRGB(255, 130, 80);
            t.vuBackground = juce::Colour::fromRGB(35, 28, 32);
            t.vuNeedle     = juce::Colour::fromRGB(255, 100, 80);
            t.hasTexture   = false;
            t.cornerRadius = 6.0f;
            break;
            
        case ThemeType::Ocean:
            // Deep ocean blue
            t.bgDark       = juce::Colour::fromRGB(12, 20, 32);
            t.bgPanel      = juce::Colour::fromRGB(20, 35, 55);
            t.bgPanelAlt   = juce::Colour::fromRGB(28, 48, 72);
            t.accent       = juce::Colour::fromRGB(60, 180, 220);     // Ocean blue
            t.accentAlt    = juce::Colour::fromRGB(80, 200, 240);
            t.textBright   = juce::Colour::fromRGB(230, 245, 255);
            t.textDim      = juce::Colour::fromRGB(100, 140, 170);
            t.meterGreen   = juce::Colour::fromRGB(60, 220, 140);
            t.meterYellow  = juce::Colour::fromRGB(255, 220, 80);
            t.meterRed     = juce::Colour::fromRGB(255, 90, 90);
            t.knobFill     = juce::Colour::fromRGB(25, 40, 60);
            t.knobOutline  = juce::Colour::fromRGB(60, 140, 180);
            t.vuBackground = juce::Colour::fromRGB(18, 30, 48);
            t.vuNeedle     = juce::Colour::fromRGB(60, 200, 220);
            t.hasTexture   = false;
            t.cornerRadius = 5.0f;
            break;
            
        case ThemeType::Midnight:
            // Deep purple/blue night
            t.bgDark       = juce::Colour::fromRGB(15, 12, 25);
            t.bgPanel      = juce::Colour::fromRGB(28, 22, 42);
            t.bgPanelAlt   = juce::Colour::fromRGB(40, 32, 58);
            t.accent       = juce::Colour::fromRGB(160, 100, 255);    // Purple
            t.accentAlt    = juce::Colour::fromRGB(200, 120, 255);
            t.textBright   = juce::Colour::fromRGB(240, 235, 255);
            t.textDim      = juce::Colour::fromRGB(120, 110, 150);
            t.meterGreen   = juce::Colour::fromRGB(80, 220, 150);
            t.meterYellow  = juce::Colour::fromRGB(255, 200, 100);
            t.meterRed     = juce::Colour::fromRGB(255, 80, 120);
            t.knobFill     = juce::Colour::fromRGB(32, 26, 48);
            t.knobOutline  = juce::Colour::fromRGB(140, 90, 200);
            t.vuBackground = juce::Colour::fromRGB(22, 18, 35);
            t.vuNeedle     = juce::Colour::fromRGB(180, 100, 255);
            t.hasTexture   = false;
            t.cornerRadius = 6.0f;
            break;
            
        case ThemeType::RedRoom:
            // Dark red studio vibe
            t.bgDark       = juce::Colour::fromRGB(22, 15, 15);
            t.bgPanel      = juce::Colour::fromRGB(38, 25, 25);
            t.bgPanelAlt   = juce::Colour::fromRGB(52, 35, 35);
            t.accent       = juce::Colour::fromRGB(220, 60, 60);      // Studio red
            t.accentAlt    = juce::Colour::fromRGB(180, 45, 45);
            t.textBright   = juce::Colour::fromRGB(255, 245, 245);
            t.textDim      = juce::Colour::fromRGB(150, 120, 120);
            t.meterGreen   = juce::Colour::fromRGB(80, 200, 100);
            t.meterYellow  = juce::Colour::fromRGB(255, 200, 70);
            t.meterRed     = juce::Colour::fromRGB(255, 50, 50);
            t.knobFill     = juce::Colour::fromRGB(42, 28, 28);
            t.knobOutline  = juce::Colour::fromRGB(180, 60, 60);
            t.vuBackground = juce::Colour::fromRGB(30, 20, 20);
            t.vuNeedle     = juce::Colour::fromRGB(220, 60, 60);
            t.hasTexture   = false;
            t.cornerRadius = 4.0f;
            break;
            
        default:
            // Default to Modern Dark
            return getTheme(ThemeType::ModernDark);
    }
    
    return t;
}

inline juce::StringArray getThemeNames()
{
    return { "Vintage", "Modern Dark", "Futuristic", "UFO", "Sleek Black", "UAD Console", 
             "SSL", "Neve", "Sunset", "Ocean", "Midnight", "Red Room" };
}
