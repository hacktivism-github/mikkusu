/*
  ==============================================================================

    WaveformDisplay.cpp
    Created: 3 Feb 2026 3:31:01am
    Author:  Bruno Teixeira

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "WaveformDisplay.h"
#include <cmath>

//==============================================================================
WaveformDisplay::WaveformDisplay(juce::AudioFormatManager & formatManagerToUse,
                                 juce::AudioThumbnailCache & cacheToUse)
    : audioThumb(1000, formatManagerToUse, cacheToUse),
      fileLoaded(false),
      position(0.0)
{
    audioThumb.addChangeListener(this);
    setTooltip("");
}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);

    if (fileLoaded)
    {
        g.setColour(juce::Colours::orange);
        audioThumb.drawChannel(g,
                               getLocalBounds(),
                               0,
                               audioThumb.getTotalLength(),
                               0,
                               1.0f);

        double safePosition = position;
        if (safePosition < 0.0 || safePosition > 1.0 || ! std::isfinite(safePosition))
            safePosition = 0.0;

        const int x = (int) std::round(safePosition * (double) getWidth());

        g.setColour(juce::Colours::lightgreen);
        g.drawLine((float) x, 0.0f, (float) x, (float) getHeight(), 2.0f);
    }
    else
    {
        g.setColour(juce::Colours::lightgrey.withAlpha(0.75f));
        g.setFont(juce::FontOptions(18.0f));
        g.drawText("Drop or load a track", getLocalBounds(),
                   juce::Justification::centred, true);
    }
}

void WaveformDisplay::resized()
{
}

void WaveformDisplay::loadURL(juce::URL audioURL)
{
    audioThumb.clear();
    fileLoaded = audioThumb.setSource(new juce::URLInputSource(audioURL));
    position = 0.0;
    setTooltip("");

    if (fileLoaded)
    {
        std::cout << "WaveformDisplay: loaded! " << std::endl;
        repaint();
    }
    else
    {
        std::cout << "WaveformDisplay: not loaded! " << std::endl;
    }
}

void WaveformDisplay::clear()
{
    audioThumb.clear();
    fileLoaded = false;
    position = 0.0;
    setTooltip("");
    repaint();
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    juce::ignoreUnused(source);
    std::cout << "WaveformDisplay: change received! " << std::endl;
    repaint();
}

void WaveformDisplay::setPositionRelative(double pos)
{
    if (! std::isfinite(pos))
        pos = 0.0;

    pos = juce::jlimit(0.0, 1.0, pos);

    if (pos != position)
    {
        position = pos;
        repaint();
    }
}

void WaveformDisplay::handleSeekFromMouse(float mouseX)
{
    if (! fileLoaded || getWidth() <= 0)
        return;

    const double clickedPos = juce::jlimit(0.0, 1.0, (double) mouseX / (double) getWidth());

    if (onSeekRequested)
        onSeekRequested(clickedPos);

    updateTooltipForMouse(mouseX);
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& e)
{
    handleSeekFromMouse(e.position.x);
}

void WaveformDisplay::mouseDrag(const juce::MouseEvent& e)
{
    handleSeekFromMouse(e.position.x);
}

void WaveformDisplay::mouseMove(const juce::MouseEvent& e)
{
    updateTooltipForMouse(e.position.x);
}

void WaveformDisplay::mouseExit(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
    setTooltip("");
}

juce::String WaveformDisplay::formatTime(double seconds) const
{
    if (! std::isfinite(seconds) || seconds < 0.0)
        seconds = 0.0;

    const int totalSeconds = (int) std::floor(seconds);
    const int minutes = totalSeconds / 60;
    const int secs = totalSeconds % 60;

    return juce::String::formatted("%02d:%02d", minutes, secs);
}

void WaveformDisplay::updateTooltipForMouse(float mouseX)
{
    if (! fileLoaded || getWidth() <= 0)
    {
        setTooltip("");
        return;
    }

    const float playheadX = (float) position * (float) getWidth();
    const float hoverTolerancePx = 10.0f;

    if (std::abs(mouseX - playheadX) <= hoverTolerancePx)
    {
        const double totalLength = audioThumb.getTotalLength();
        const double currentSeconds = position * totalLength;

        //setTooltip("Playhead: " + formatTime(currentSeconds) +
        setTooltip("Position: " + formatTime(currentSeconds) +
                   " / " + formatTime(totalLength));
    }
    else
    {
        setTooltip("");
    }
}
