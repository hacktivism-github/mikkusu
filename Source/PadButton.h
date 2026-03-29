/*
  ==============================================================================

    PadButton.h
    Created: 24 Jan 2026 9:58:09am
    Author:  Bruno Teixeira

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PadButton : public juce::Button
{
public:
    PadButton(const juce::String& name) : juce::Button(name) {}

    void setPadText(const juce::String& newText)
    {
        padText = newText;
        repaint();
    }

    void setPadActive(bool shouldBeActive)
    {
        isActive = shouldBeActive;
        repaint();
    }
  
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto raw = getLocalBounds().toFloat();
        if (raw.getWidth() <= 10.0f || raw.getHeight() <= 10.0f)
            return;

        auto area = raw.reduced(1.5f);
        if (area.getWidth() <= 0.0f || area.getHeight() <= 0.0f)
            return;

        auto bg = juce::Colour(0xff141414);
        auto border = isActive ? juce::Colours::orange : juce::Colour(0xff3a3a3a);

        if (isButtonDown)
            bg = bg.brighter(0.16f);
        else if (isMouseOverButton)
            bg = bg.brighter(0.08f);

        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(area.translated(0.0f, 1.0f), 2.0f);

        g.setColour(bg);
        g.fillRoundedRectangle(area, 2.0f);

        g.setColour(border);
        g.drawRoundedRectangle(area, 2.0f, 1.5f);

        g.setColour(juce::Colours::white.withAlpha(0.75f));
        g.setFont(juce::FontOptions(10.5f));
        g.drawFittedText(padText, getLocalBounds().reduced(4),
                         juce::Justification::centred, 1);
    }
    
private:
    juce::String padText { "PAD" };
    bool isActive { false };
};
