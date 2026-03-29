/*
  ==============================================================================

    DeckGUI.h
    Created: 2 Feb 2026 10:58:51pm
    Author:  Bruno Teixeira

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "PadButton.h"

//==============================================================================
class JogWheelComponent : public juce::Component
{
public:
    void setRotationAngle(float newAngleRadians)
    {
        rotationAngle = newAngleRadians;
        repaint();
    }

    std::function<void(float deltaX)> onDrag;

    void mouseDown(const juce::MouseEvent& e) override
    {
        lastDragX = (float) e.position.x;
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        const float currentX = (float) e.position.x;
        const float delta = currentX - lastDragX;
        lastDragX = currentX;

        if (onDrag)
            onDrag(delta);
    }

    void paint(juce::Graphics& g) override
    {
        auto raw = getLocalBounds().toFloat();
        if (raw.getWidth() <= 30.0f || raw.getHeight() <= 30.0f)
            return;

        auto bounds = raw.reduced(8.0f);
        if (bounds.getWidth() <= 0.0f || bounds.getHeight() <= 0.0f)
            return;

        auto centre = bounds.getCentre();
        const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        if (radius <= 0.0f)
            return;

        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillEllipse(bounds.translated(0.0f, 5.0f));

        g.setColour(juce::Colour(0xffc8c8c8));
        g.fillEllipse(bounds);

        auto rim = bounds.reduced(radius * 0.05f);
        if (rim.getWidth() <= 0.0f || rim.getHeight() <= 0.0f)
            return;

        juce::ColourGradient rimGrad(juce::Colour(0xff666666), rim.getTopLeft(),
                                     juce::Colour(0xff171717), rim.getBottomRight(), false);
        g.setGradientFill(rimGrad);
        g.fillEllipse(rim);

        g.setColour(juce::Colour(0xff9d9d9d));
        g.drawEllipse(rim, 2.0f);

        for (int i = 0; i < 28; ++i)
        {
            float angle = juce::MathConstants<float>::twoPi * (float) i / 28.0f;
            float r1 = radius * 0.84f;
            float r2 = radius * 0.94f;

            g.setColour(juce::Colour(0xff5f5f5f));
            g.drawLine(centre.x + std::cos(angle) * r1,
                       centre.y + std::sin(angle) * r1,
                       centre.x + std::cos(angle) * r2,
                       centre.y + std::sin(angle) * r2,
                       3.0f);
        }

        auto platter = rim.reduced(radius * 0.16f);
        if (platter.getWidth() > 0.0f && platter.getHeight() > 0.0f)
        {
            juce::ColourGradient platterGrad(juce::Colour(0xff6f6f6f), platter.getTopLeft(),
                                             juce::Colour(0xff101010), platter.getBottomRight(), false);
            g.setGradientFill(platterGrad);
            g.fillEllipse(platter);

            {
                juce::Graphics::ScopedSaveState state(g);
                g.addTransform(juce::AffineTransform::rotation(rotationAngle, centre.x, centre.y));

                g.setColour(juce::Colours::white.withAlpha(0.08f));
                for (int i = 0; i < 20; ++i)
                {
                    float angle = juce::MathConstants<float>::twoPi * (float) i / 20.0f;
                    float r = platter.getWidth() * 0.46f;

                    g.drawLine(centre.x, centre.y,
                               centre.x + std::cos(angle) * r,
                               centre.y + std::sin(angle) * r,
                               1.0f);
                }

                g.setColour(juce::Colours::white.withAlpha(0.22f));
                g.drawLine(centre.x, centre.y,
                           centre.x,
                           centre.y - platter.getWidth() * 0.42f,
                           2.0f);
            }

            auto hub = platter.reduced(platter.getWidth() * 0.34f);
            if (hub.getWidth() > 0.0f && hub.getHeight() > 0.0f)
            {
                g.setColour(juce::Colour(0xff0d0d0d));
                g.fillEllipse(hub);

                g.setColour(juce::Colour(0xffc0c0c0));
                g.drawEllipse(hub, 2.0f);

                auto core = hub.reduced(hub.getWidth() * 0.24f);
                if (core.getWidth() > 0.0f && core.getHeight() > 0.0f)
                {
                    g.setColour(juce::Colour(0xff202020));
                    g.fillEllipse(core);
                }
            }
        }

        juce::Path highlight;
        highlight.addCentredArc(centre.x, centre.y,
                                radius * 0.75f, radius * 0.75f,
                                0.0f,
                                juce::MathConstants<float>::pi * 1.08f,
                                juce::MathConstants<float>::pi * 1.38f,
                                true);

        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.strokePath(highlight, juce::PathStrokeType(3.0f));
    }
    

private:
    float rotationAngle = 0.0f;
    float lastDragX = 0.0f;
};

//==============================================================================
class CueRectButton : public juce::Button
{
public:
    CueRectButton(const juce::String& name, const juce::String& textToShow)
        : juce::Button(name), buttonText(textToShow) {}

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto area = getLocalBounds().toFloat().reduced(1.5f);
        if (area.isEmpty())
            return;

        auto bg = juce::Colour(0xff141414);
        auto border = juce::Colour(0xff8f8f8f);

        if (isButtonDown)
            bg = bg.brighter(0.12f);
        else if (isMouseOverButton)
            bg = bg.brighter(0.05f);

        g.setColour(juce::Colours::black.withAlpha(0.30f));
        g.fillRoundedRectangle(area.translated(0.0f, 1.0f), 2.0f);

        g.setColour(bg);
        g.fillRoundedRectangle(area, 2.0f);

        g.setColour(border);
        g.drawRoundedRectangle(area, 2.0f, 1.2f);

        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.setFont(juce::FontOptions(15.0f));
        g.drawFittedText(buttonText, getLocalBounds().reduced(4),
                         juce::Justification::centred, 1);
    }

private:
    juce::String buttonText;
};

//class PlayPauseButton : public juce::Button
//{
//public:
//    PlayPauseButton(const juce::String& name) : juce::Button(name) {}
//
//    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
//    {
//        auto area = getLocalBounds().toFloat().reduced(1.5f);
//        if (area.isEmpty())
//            return;
//
//        auto bg = juce::Colour(0xff141414);
//        auto border = juce::Colour(0xff56d61d);
//
//        if (isButtonDown)
//            bg = bg.brighter(0.10f);
//        else if (isMouseOverButton)
//            bg = bg.brighter(0.05f);
//
//        g.setColour(juce::Colours::black.withAlpha(0.30f));
//        g.fillRoundedRectangle(area.translated(0.0f, 1.0f), 3.0f);
//
//        g.setColour(bg);
//        g.fillRoundedRectangle(area, 3.0f);
//
//        g.setColour(border);
//        g.drawRoundedRectangle(area, 3.0f, 1.6f);
//
//        auto iconArea = area.reduced(12.0f, 10.0f);
//        const auto centreY = iconArea.getCentreY();
//
//        // Play triangle
//        juce::Path playPath;
//        const float triW = iconArea.getWidth() * 0.22f;
//        const float triH = iconArea.getHeight() * 0.42f;
//        const float triX = iconArea.getX() + iconArea.getWidth() * 0.22f;
//
//        playPath.startNewSubPath(triX, centreY - triH * 0.5f);
//        playPath.lineTo(triX, centreY + triH * 0.5f);
//        playPath.lineTo(triX + triW, centreY);
//        playPath.closeSubPath();
//
//        g.setColour(juce::Colour(0xff56d61d));
//        g.fillPath(playPath);
//
//        // Pause bars
//        const float barW = 4.0f;
//        const float barH = triH;
//        const float pauseX1 = iconArea.getX() + iconArea.getWidth() * 0.58f;
//        const float pauseGap = 5.0f;
//
//        g.fillRoundedRectangle(pauseX1,
//                               centreY - barH * 0.5f,
//                               barW,
//                               barH,
//                               1.0f);
//
//        g.fillRoundedRectangle(pauseX1 + barW + pauseGap,
//                               centreY - barH * 0.5f,
//                               barW,
//                               barH,
//                               1.0f);
//    }
//};

//class PlayPauseButton : public juce::Button
//{
//public:
//    PlayPauseButton(const juce::String& name) : juce::Button(name) {}
//
//    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
//    {
//        auto area = getLocalBounds().toFloat().reduced(1.5f);
//        if (area.isEmpty())
//            return;
//
//        auto bg = juce::Colour(0xff141414);
//        auto border = juce::Colour(0xff56d61d);
//
//        if (isButtonDown)
//            bg = bg.brighter(0.10f);
//        else if (isMouseOverButton)
//            bg = bg.brighter(0.05f);
//
//        // Background
//        g.setColour(juce::Colours::black.withAlpha(0.30f));
//        g.fillRoundedRectangle(area.translated(0.0f, 1.0f), 3.0f);
//
//        g.setColour(bg);
//        g.fillRoundedRectangle(area, 3.0f);
//
//        g.setColour(border);
//        g.drawRoundedRectangle(area, 3.0f, 1.6f);
//
//        // ===== ICON (COMPACT) =====
//        auto icon = area.reduced(area.getWidth() * 0.28f,
//                                 area.getHeight() * 0.28f);
//
//        const float centreY = icon.getCentreY();
//
//        // --- PLAY TRIANGLE ---
//        juce::Path play;
//        const float triW = icon.getWidth() * 0.28f;
//        const float triH = icon.getHeight() * 0.45f;
//        const float triX = icon.getX();
//
//        play.startNewSubPath(triX, centreY - triH * 0.5f);
//        play.lineTo(triX, centreY + triH * 0.5f);
//        play.lineTo(triX + triW, centreY);
//        play.closeSubPath();
//
//        g.setColour(juce::Colour(0xff56d61d));
//        g.fillPath(play);
//
//        // --- PAUSE BARS ---
//        const float barW = 2.5f;
//        const float barH = triH;
//
//        const float pauseStartX = triX + triW + icon.getWidth() * 0.18f;
//        const float gap = 3.0f;
//
//        g.fillRect(pauseStartX,
//                   centreY - barH * 0.5f,
//                   barW,
//                   barH);
//
//        g.fillRect(pauseStartX + barW + gap,
//                   centreY - barH * 0.5f,
//                   barW,
//                   barH);
//    }
//};

class PlayPauseButton : public juce::Button
{
public:
    PlayPauseButton(const juce::String& name) : juce::Button(name) {}

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto area = getLocalBounds().toFloat().reduced(1.5f);
        if (area.isEmpty())
            return;

        auto bg = juce::Colour(0xff141414);
        auto border = juce::Colour(0xff56d61d);

        if (isButtonDown)
            bg = bg.brighter(0.10f);
        else if (isMouseOverButton)
            bg = bg.brighter(0.05f);

        g.setColour(juce::Colours::black.withAlpha(0.30f));
        g.fillRoundedRectangle(area.translated(0.0f, 1.0f), 3.0f);

        g.setColour(bg);
        g.fillRoundedRectangle(area, 3.0f);

        g.setColour(border);
        g.drawRoundedRectangle(area, 3.0f, 1.6f);

        auto icon = area.reduced(area.getWidth() * 0.30f,
                                 area.getHeight() * 0.28f);

        const float centreY = icon.getCentreY();
        g.setColour(juce::Colour(0xff56d61d));

        if (getToggleState())
        {
            // PAUSE icon
            const float barW = 3.0f;
            const float barH = icon.getHeight() * 0.48f;
            const float gap = 4.0f;

            const float totalW = barW * 2.0f + gap;
            const float startX = icon.getCentreX() - totalW * 0.5f;

            g.fillRect(startX,
                       centreY - barH * 0.5f,
                       barW,
                       barH);

            g.fillRect(startX + barW + gap,
                       centreY - barH * 0.5f,
                       barW,
                       barH);
        }
        else
        {
            // PLAY icon
            juce::Path play;
            const float triW = icon.getWidth() * 0.32f;
            const float triH = icon.getHeight() * 0.50f;
            const float triX = icon.getCentreX() - triW * 0.35f;

            play.startNewSubPath(triX, centreY - triH * 0.5f);
            play.lineTo(triX, centreY + triH * 0.5f);
            play.lineTo(triX + triW, centreY);
            play.closeSubPath();

            g.fillPath(play);
        }
    }
};

//==============================================================================
class DeckLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        juce::ignoreUnused(slider);

        if (width <= 8 || height <= 8)
            return;

        auto radius = (float) juce::jmin(width / 2, height / 2) - 3.0f;
        if (radius <= 0.0f)
            return;

        auto centreX = (float) x + (float) width * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillEllipse(rx, ry + 2.0f, rw, rw);

        g.setColour(juce::Colour(0xff101010));
        g.fillEllipse(rx, ry, rw, rw);

        g.setColour(juce::Colour(0xff2b2b2b));
        g.drawEllipse(rx, ry, rw, rw, 1.5f);

        const float innerInset = 4.0f;
        const float innerSize = rw - (innerInset * 2.0f);

        if (innerSize > 0.0f)
        {
            juce::ColourGradient grad(juce::Colour(0xff3a3a3a), centreX, ry,
                                      juce::Colour(0xff161616), centreX, ry + rw, false);
            g.setGradientFill(grad);
            g.fillEllipse(rx + innerInset, ry + innerInset, innerSize, innerSize);
        }

        juce::Path arc;
        arc.addCentredArc(centreX, centreY,
                          radius * 0.92f, radius * 0.92f,
                          0.0f,
                          rotaryStartAngle,
                          rotaryEndAngle,
                          true);

        g.setColour(juce::Colour(0xff1f1f1f));
        g.strokePath(arc, juce::PathStrokeType(2.0f));

        juce::Path p;
        auto pointerLength = radius * 0.62f;
        auto pointerThickness = 3.0f;

        if (pointerLength > 0.0f)
        {
            p.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength,
                                  pointerThickness, pointerLength, 1.2f);

            g.setColour(juce::Colour(0xffd8d8d8));
            g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        }
    }

    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          const juce::Slider::SliderStyle style,
                          juce::Slider& slider) override
    {
        juce::ignoreUnused(minSliderPos, maxSliderPos);

        if (style == juce::Slider::LinearVertical)
        {
            if (width <= 8 || height <= 20)
                return;

            const bool isVolume = (slider.getName() == "volSlider");
            const bool isTempo  = (slider.getName() == "tempoSlider");

            const float trackW = isVolume ? 4.0f : 5.0f;
            const float thumbW = isVolume ? 12.0f : 18.0f;
            const float thumbH = isVolume ? 8.0f  : 12.0f;
            const float thumbRadius = isVolume ? 1.5f : 2.5f;

            auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);

            auto track = juce::Rectangle<float>(
                bounds.getCentreX() - trackW * 0.5f,
                bounds.getY() + 6.0f,
                trackW,
                bounds.getHeight() - 12.0f);

            if (track.getWidth() <= 0.0f || track.getHeight() <= 0.0f)
                return;

            g.setColour(juce::Colour(0xff0d0d10));
            g.fillRoundedRectangle(track, 2.0f);

            auto innerTrack = track.reduced(1.0f, 2.0f);
            if (innerTrack.getWidth() > 0.0f && innerTrack.getHeight() > 0.0f)
            {
                g.setColour(juce::Colour(0xff2d2d32));
                g.fillRoundedRectangle(innerTrack, 1.5f);
            }

            g.setColour(juce::Colour(0xff4a4a50).withAlpha(0.7f));
            g.drawRoundedRectangle(track, 2.0f, 0.8f);

            auto thumb = juce::Rectangle<float>(
                bounds.getCentreX() - thumbW * 0.5f,
                sliderPos - thumbH * 0.5f,
                thumbW,
                thumbH);

            if (thumb.getWidth() <= 0.0f || thumb.getHeight() <= 0.0f)
                return;

            g.setColour(juce::Colours::black.withAlpha(0.25f));
            g.fillRoundedRectangle(thumb.translated(0.0f, 1.0f), thumbRadius);

            g.setColour(juce::Colour(0xffd9d9d9));
            g.fillRoundedRectangle(thumb, thumbRadius);

            g.setColour(juce::Colour(0xff8c8c8c));
            g.drawRoundedRectangle(thumb, thumbRadius, 0.8f);

            if (isTempo)
            {
                g.setColour(juce::Colour(0xff6f6f6f));
                g.drawLine(thumb.getCentreX(), thumb.getY() + 2.0f,
                           thumb.getCentreX(), thumb.getBottom() - 2.0f, 1.0f);
            }

            return;
        }

        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height,
                                         sliderPos, minSliderPos, maxSliderPos, style, slider);
    }

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton,
                              bool isButtonDown) override
    {
        auto raw = button.getLocalBounds().toFloat();
        if (raw.getWidth() <= 4.0f || raw.getHeight() <= 4.0f)
            return;

        auto bounds = raw.reduced(1.5f);
        if (bounds.getWidth() <= 0.0f || bounds.getHeight() <= 0.0f)
            return;

        auto base = backgroundColour;

        if (isButtonDown)
            base = base.brighter(0.15f);
        else if (isMouseOverButton)
            base = base.brighter(0.08f);

        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(bounds.translated(0.0f, 2.0f), 4.0f);

        g.setColour(base);
        g.fillRoundedRectangle(bounds, 4.0f);

        juce::Colour outline = juce::Colours::black.withAlpha(0.55f);

        if (button.getName() == "playButton")
        {
            outline = juce::Colour(0xff56d61d);

            if (isMouseOverButton)
                outline = outline.brighter(0.15f);

            if (isButtonDown)
                outline = outline.darker(0.10f);
        }
        
        if (button.getName() == "syncButton")
        {
            outline = juce::Colour(0xff4da6ff);

            if (isMouseOverButton)
                outline = outline.brighter(0.2f);

            if (isButtonDown)
                outline = outline.darker(0.1f);
        }

        g.setColour(outline);
        g.drawRoundedRectangle(bounds, 4.0f, 1.8f);
    }
};

//==============================================================================
class DeckGUI  : public juce::Component,
                 public juce::Button::Listener,
                 public juce::Slider::Listener,
                 public juce::FileDragAndDropTarget,
                 public juce::Timer
{
public:
    DeckGUI(DJAudioPlayer* player,
            juce::AudioFormatManager& formatManagerToUse,
            juce::AudioThumbnailCache& cacheToUse,
            bool isLeftDeck);
    ~DeckGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* button) override;
    void sliderValueChanged (juce::Slider* slider) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void timerCallback() override;

    void loadTrackFromURL(const juce::URL& url);
    void unloadTrack();
    juce::String getCurrentTrackPath() const;
    void setSyncTargetPlayer(DJAudioPlayer* targetPlayer);

private:
    juce::String formatTime(double seconds) const;
    void updateCueButtonLabels();
    void syncEQSlidersFromPlayer();
    void styleTransportButton(juce::TextButton& button, juce::Colour colour);
    
    void refreshBPMLabel();

    DJAudioPlayer* player { nullptr };
    DJAudioPlayer* syncTargetPlayer { nullptr };
    bool leftDeck = true;
    float jogRotationAngle = 0.0f;
    int selectedCueIndex = 0;

    DeckLookAndFeel deckLookAndFeel;

    JogWheelComponent jogWheel;
    CueRectButton cueMainButton { "cueMain", "CUE" };

    //juce::TextButton playButton { ">" };
    PlayPauseButton playButton { "playButton" };
    juce::TextButton stopButton { "S" };
    juce::TextButton loadButton { "LOAD" };
    juce::TextButton unloadButton { "UNLOAD" };
    juce::TextButton clearCuesButton { "CLEAR" };
    juce::TextButton resetEQButton { "RESET" };
    juce::TextButton syncButton { "SYNC" };

    juce::TextButton prevCueButton { "<" };
    PadButton selectedCueButton { "SelectedCue" };
    juce::TextButton nextCueButton { ">" };

    juce::Slider volSlider;
    juce::Slider speedSlider;
    juce::Slider posSlider;

    juce::Label bpmLabel;
    juce::Label timeLabel;

    juce::Slider lowEQSlider;
    juce::Slider midEQSlider;
    juce::Slider highEQSlider;

    juce::Label volLabel;
    juce::Label speedLabel;
    juce::Label posLabel;

    juce::Label lowEQLabel;
    juce::Label midEQLabel;
    juce::Label highEQLabel;

    juce::FileChooser fChooser { "Select a file..." };
    WaveformDisplay waveformDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckGUI)
};
