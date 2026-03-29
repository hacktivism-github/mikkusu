/*
  ==============================================================================

    DeckGUI.cpp
    Created: 2 Feb 2026 10:58:51pm
    Author:  Bruno Teixeira

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "DeckGUI.h"

DeckGUI::DeckGUI(DJAudioPlayer* _player,
                 juce::AudioFormatManager& formatManagerToUse,
                 juce::AudioThumbnailCache& cacheToUse,
                 bool isLeftDeck)
    : player(_player),
      leftDeck(isLeftDeck),
      waveformDisplay(formatManagerToUse, cacheToUse)
{
    setLookAndFeel(&deckLookAndFeel);

    addAndMakeVisible(jogWheel);

    jogWheel.onDrag = [this](float deltaX)
    {
        if (player == nullptr)
            return;

        const double deltaSeconds = (double) deltaX * 0.01;
        player->nudgePosition(deltaSeconds);
    };

    addAndMakeVisible(cueMainButton);
    cueMainButton.addListener(this);
    cueMainButton.setTooltip("Jump to track start / cue");

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(unloadButton);
    addAndMakeVisible(clearCuesButton);
    addAndMakeVisible(resetEQButton);
    addAndMakeVisible(syncButton);

    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);
    unloadButton.addListener(this);
    clearCuesButton.addListener(this);
    resetEQButton.addListener(this);
    syncButton.addListener(this);

    playButton.setTooltip("Start or pause playback");
    stopButton.setTooltip("Stop playback");
    loadButton.setTooltip("Load an audio track");
    unloadButton.setTooltip("Unload current track from this deck");
    clearCuesButton.setTooltip("Clear all hot cues");
    resetEQButton.setTooltip("Reset equaliser to default");
    syncButton.setTooltip("Sync this deck tempo to the other deck");

    //playButton.setButtonText(">");
    //playButton.setName("playButton");
    unloadButton.setButtonText("UNLOAD");
    clearCuesButton.setButtonText("RESET CUE");
    resetEQButton.setButtonText("RESET EQ");

    playButton.setClickingTogglesState(false);
    //styleTransportButton(playButton, juce::Colour(0xff1d1d1d));
    playButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff56d61d));
    playButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff56d61d));

    styleTransportButton(stopButton, juce::Colour(0xff9a3232));
    styleTransportButton(loadButton, juce::Colour(0xff2f3438));
    styleTransportButton(unloadButton, juce::Colour(0xff2f3438));
    styleTransportButton(clearCuesButton, juce::Colour(0xff2c3136));
    styleTransportButton(resetEQButton, juce::Colour(0xff2c3136));
    styleTransportButton(syncButton, juce::Colour(0xff2f3438));
//    syncButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff4da6ff));
//    syncButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff4da6ff));
    syncButton.setName("syncButton");

    syncButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1d1d1d));
    syncButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff4da6ff));
    syncButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff4da6ff));

    addAndMakeVisible(prevCueButton);
    addAndMakeVisible(selectedCueButton);
    addAndMakeVisible(nextCueButton);

    prevCueButton.addListener(this);
    selectedCueButton.addListener(this);
    nextCueButton.addListener(this);

    styleTransportButton(prevCueButton, juce::Colour(0xff1d1d1d));
    styleTransportButton(nextCueButton, juce::Colour(0xff1d1d1d));
    prevCueButton.setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    nextCueButton.setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);

    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM: --", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centred);
    bpmLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    bpmLabel.setFont(juce::FontOptions(18.0f));

    addAndMakeVisible(timeLabel);
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    timeLabel.setJustificationType(juce::Justification::centredRight);
    timeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey.withAlpha(0.85f));
    timeLabel.setFont(juce::FontOptions(12.0f));

    addAndMakeVisible(volLabel);
    addAndMakeVisible(speedLabel);
    addAndMakeVisible(posLabel);
    addAndMakeVisible(lowEQLabel);
    addAndMakeVisible(midEQLabel);
    addAndMakeVisible(highEQLabel);

    volLabel.setText("VOL", juce::dontSendNotification);
    speedLabel.setText("TEMPO", juce::dontSendNotification);
    posLabel.setText("POS", juce::dontSendNotification);

    lowEQLabel.setText("L", juce::dontSendNotification);
    midEQLabel.setText("M", juce::dontSendNotification);
    highEQLabel.setText("H", juce::dontSendNotification);

    for (auto* label : { &volLabel, &speedLabel, &posLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        label->setFont(juce::FontOptions(12.0f));
    }

    for (auto* eqLabel : { &lowEQLabel, &midEQLabel, &highEQLabel })
    {
        eqLabel->setJustificationType(juce::Justification::centred);
        eqLabel->setColour(juce::Label::backgroundColourId, juce::Colour(0xff111111));
        eqLabel->setColour(juce::Label::outlineColourId, juce::Colour(0xff2f2f2f));
        eqLabel->setColour(juce::Label::textColourId, juce::Colour(0xffb89d7a));
        eqLabel->setFont(juce::FontOptions(12.0f));
    }

    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(posSlider);
    addAndMakeVisible(lowEQSlider);
    addAndMakeVisible(midEQSlider);
    addAndMakeVisible(highEQSlider);

    speedSlider.setTooltip("Adjust playback tempo. Double-click to reset to normal speed.");

    volSlider.setName("volSlider");
    speedSlider.setName("tempoSlider");
    posSlider.setName("posSlider");

    volSlider.addListener(this);
    speedSlider.addListener(this);
    posSlider.addListener(this);
    lowEQSlider.addListener(this);
    midEQSlider.addListener(this);
    highEQSlider.addListener(this);

    volSlider.setRange(0.0, 1.0);
    volSlider.setValue(0.5);

    speedSlider.setRange(0.5, 2.0);
    speedSlider.setValue(1.0);
    speedSlider.setDoubleClickReturnValue(true, 1.0);

    posSlider.setRange(0.0, 1.0);
    posSlider.setValue(0.0);

    lowEQSlider.setRange(-24.0, 24.0);
    midEQSlider.setRange(-24.0, 24.0);
    highEQSlider.setRange(-24.0, 24.0);

    lowEQSlider.setValue(0.0);
    midEQSlider.setValue(0.0);
    highEQSlider.setValue(0.0);

    volSlider.setSliderStyle(juce::Slider::LinearVertical);
    volSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    speedSlider.setSliderStyle(juce::Slider::LinearVertical);
    speedSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    posSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    posSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    lowEQSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    midEQSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    highEQSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);

    lowEQSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midEQSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    highEQSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(waveformDisplay);

    waveformDisplay.onSeekRequested = [this](double relativePos)
    {
        if (player != nullptr)
            player->setPositionRelative(relativePos);
    };

    updateCueButtonLabels();
    startTimer(40);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void DeckGUI::setSyncTargetPlayer(DJAudioPlayer* targetPlayer)
{
    syncTargetPlayer = targetPlayer;
}

void DeckGUI::loadTrackFromURL(const juce::URL& url)
{
    if (player == nullptr)
        return;

    player->loadURL(url);
    waveformDisplay.loadURL(url);

//    bpmLabel.setText("BPM: " + juce::String(player->getBPM(), 1),
//                     juce::dontSendNotification);
    
    refreshBPMLabel();

    timeLabel.setText("00:00 / " + formatTime(player->getTrackLengthInSeconds()),
                      juce::dontSendNotification);

    syncEQSlidersFromPlayer();
    updateCueButtonLabels();
}

void DeckGUI::unloadTrack()
{
    if (player == nullptr)
        return;

    player->unloadTrack();
    waveformDisplay.clear();

    bpmLabel.setText("BPM: --", juce::dontSendNotification);
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);

    lowEQSlider.setValue(0.0, juce::dontSendNotification);
    midEQSlider.setValue(0.0, juce::dontSendNotification);
    highEQSlider.setValue(0.0, juce::dontSendNotification);

    volSlider.setValue(0.5, juce::dontSendNotification);
    speedSlider.setValue(1.0, juce::dontSendNotification);
    posSlider.setValue(0.0, juce::dontSendNotification);

    selectedCueIndex = 0;
    updateCueButtonLabels();
    
    refreshBPMLabel();
    
    playButton.setToggleState(false, juce::dontSendNotification);
}

juce::String DeckGUI::getCurrentTrackPath() const
{
    if (player == nullptr)
        return {};

    return player->getCurrentTrackPath();
}

juce::String DeckGUI::formatTime(double seconds) const
{
    if (! std::isfinite(seconds) || seconds < 0.0)
        seconds = 0.0;

    const int totalSeconds = (int) std::floor(seconds + 0.5);
    const int mins = totalSeconds / 60;
    const int secs = totalSeconds % 60;

    return juce::String(mins).paddedLeft('0', 2) + ":"
         + juce::String(secs).paddedLeft('0', 2);
}

void DeckGUI::styleTransportButton(juce::TextButton& button, juce::Colour colour)
{
    button.setColour(juce::TextButton::buttonColourId, colour);
    button.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
}

void DeckGUI::paint (juce::Graphics& g)
{
    juce::ColourGradient bg(juce::Colour(0xff2b2b2b), 0.0f, 0.0f,
                            juce::Colour(0xff0c0c0c), 0.0f, (float) getHeight(), false);
    g.setGradientFill(bg);
    g.fillAll();

    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    g.setColour(juce::Colours::black.withAlpha(0.70f));
    g.drawRoundedRectangle(bounds, 14.0f, 2.5f);

    g.setColour(juce::Colours::white.withAlpha(0.04f));
    g.drawLine(bounds.getWidth() * 0.5f, 14.0f,
               bounds.getWidth() * 0.5f, bounds.getHeight() - 14.0f, 1.0f);

    for (auto* eqLabel : { &lowEQLabel, &midEQLabel, &highEQLabel })
    {
        auto r = eqLabel->getBounds().toFloat();
        if (! r.isEmpty())
        {
            g.setColour(juce::Colours::black.withAlpha(0.25f));
            g.fillRoundedRectangle(r.translated(0.0f, 1.0f), 2.0f);
        }
    }

    if (! leftDeck)
    {
        auto r = getLocalBounds().reduced(12);
        auto brandArea = juce::Rectangle<int>(
            r.getRight() - 250,
            r.getBottom() - 14,
            235,
            24
        );

        const juce::String brandText = "Xperiments (by Bruno)";

        g.setColour(juce::Colour(0xffffb84d).withAlpha(0.10f));
        g.setFont(juce::FontOptions(12.5f));
        g.drawText(brandText,
                   brandArea.translated(0, 0),
                   juce::Justification::centredRight,
                   true);

        g.setColour(juce::Colour(0xffffb84d).withAlpha(0.08f));
        g.drawText(brandText,
                   brandArea.translated(1, 0),
                   juce::Justification::centredRight,
                   true);

        g.setColour(juce::Colour(0xffffb84d).withAlpha(0.08f));
        g.drawText(brandText,
                   brandArea.translated(-1, 0),
                   juce::Justification::centredRight,
                   true);

        g.setColour(juce::Colour(0xffffb84d).withAlpha(0.06f));
        g.drawText(brandText,
                   brandArea.translated(0, 1),
                   juce::Justification::centredRight,
                   true);

        g.setColour(juce::Colour(0xffffb84d).withAlpha(0.06f));
        g.drawText(brandText,
                   brandArea.translated(0, -1),
                   juce::Justification::centredRight,
                   true);

        g.setColour(juce::Colour(0xffffd580).withAlpha(0.30f));
        g.drawText(brandText,
                   brandArea,
                   juce::Justification::centredRight,
                   true);

        g.setColour(juce::Colour(0xfffff1cc).withAlpha(0.78f));
        g.setFont(juce::FontOptions(12.5f));
        g.drawText(brandText,
                   brandArea,
                   juce::Justification::centredRight,
                   true);
    }
}

void DeckGUI::resized()
{
    auto area = getLocalBounds().reduced(10);
    if (area.getWidth() <= 0 || area.getHeight() <= 0)
        return;

    auto topArea = area.removeFromTop((int) (getHeight() * 0.48f));
    area.removeFromTop(10);

    auto waveformArea = area.removeFromTop(64);
    area.removeFromTop(8);

    auto utilityRowArea = area.removeFromTop(34);
    area.removeFromTop(8);

    auto bottomArea = area;

    const int outerTempoW = 54;
    const int centerControlsWidth = (int) (topArea.getWidth() * 0.28f);

    juce::Rectangle<int> outerTempoArea;
    juce::Rectangle<int> jogArea;
    juce::Rectangle<int> centerArea;

    if (leftDeck)
    {
        outerTempoArea = topArea.removeFromLeft(outerTempoW).reduced(2);
        centerArea = topArea.removeFromRight(centerControlsWidth).reduced(4);
        jogArea = topArea.reduced(4);
    }
    else
    {
        centerArea = topArea.removeFromLeft(centerControlsWidth).reduced(4);
        outerTempoArea = topArea.removeFromRight(outerTempoW).reduced(2);
        jogArea = topArea.reduced(4);
    }

    speedLabel.setBounds(outerTempoArea.removeFromTop(18));
    auto tempoBounds = outerTempoArea.reduced(6, 6);
    speedSlider.setBounds(tempoBounds.getWidth() > 14 && tempoBounds.getHeight() > 30
                              ? tempoBounds
                              : juce::Rectangle<int>());

    auto jogZone = jogArea.reduced(6);

    int jogSize = juce::jmax(60, juce::jmin(jogZone.getWidth(), jogZone.getHeight()));
    auto jogBounds = juce::Rectangle<int>(
        jogZone.getCentreX() - jogSize / 2,
        jogZone.getCentreY() - jogSize / 2,
        jogSize,
        jogSize
    );
    jogWheel.setBounds(jogBounds);

    auto bpmRow = centerArea.removeFromTop(28);
    auto bpmBox = juce::Rectangle<int>(0, 0, 96, 24).withCentre(bpmRow.getCentre());
    bpmLabel.setBounds(bpmBox);

    centerArea.removeFromTop(14);
    auto mixerArea = centerArea.reduced(2);

    const int volW = 36;
    const int eqBlockW = 74;
    const int clusterGap = 12;
    const int clusterW = eqBlockW + clusterGap + volW;
    const int clusterH = 190;

    auto layoutVerticalSlider = [](juce::Rectangle<int> col, juce::Label& label, juce::Slider& slider)
    {
        const int labelH = 16;
        const int sliderH = 145;
        const int spacing = 8;

        auto content = juce::Rectangle<int>(0, 0, col.getWidth(), labelH + spacing + sliderH)
                           .withCentre(col.getCentre());

        auto labelArea = content.removeFromTop(labelH);
        label.setBounds(labelArea);

        content.removeFromTop(spacing);

        auto sliderBounds = juce::Rectangle<int>(content.getX(),
                                                 content.getY(),
                                                 col.getWidth(),
                                                 sliderH);

        if (sliderBounds.getWidth() > 14 && sliderBounds.getHeight() > 30)
            slider.setBounds(sliderBounds);
        else
            slider.setBounds(juce::Rectangle<int>());
    };

    auto layoutEqBlock = [](juce::Rectangle<int> eqBlock,
                            juce::Label& highLabel, juce::Slider& highKnob,
                            juce::Label& midLabel,  juce::Slider& midKnob,
                            juce::Label& lowLabel,  juce::Slider& lowKnob,
                            bool labelsOnLeft)
    {
        const int rowGap = 8;
        const int rowH = 52;
        const int totalH = rowH * 3 + rowGap * 2;

        eqBlock = juce::Rectangle<int>(eqBlock.getX(),
                                       eqBlock.getY() + 8,
                                       eqBlock.getWidth(),
                                       totalH);

        auto highRow = eqBlock.removeFromTop(rowH);
        eqBlock.removeFromTop(rowGap);
        auto midRow = eqBlock.removeFromTop(rowH);
        eqBlock.removeFromTop(rowGap);
        auto lowRow = eqBlock.removeFromTop(rowH);

        auto layoutEqRow = [labelsOnLeft](juce::Rectangle<int> row, juce::Label& label, juce::Slider& knob)
        {
            const int labelW = 20;
            const int labelH = 28;
            const int gap = 3;
            const int knobSize = 40;

            if (labelsOnLeft)
            {
                auto labelBox = juce::Rectangle<int>(row.getX(),
                                                     row.getCentreY() - labelH / 2,
                                                     labelW,
                                                     labelH);
                label.setBounds(labelBox);

                auto knobBox = juce::Rectangle<int>(labelBox.getRight() + gap,
                                                    row.getCentreY() - knobSize / 2,
                                                    knobSize,
                                                    knobSize);
                knob.setBounds(knobBox);
            }
            else
            {
                auto knobBox = juce::Rectangle<int>(row.getX(),
                                                    row.getCentreY() - knobSize / 2,
                                                    knobSize,
                                                    knobSize);
                knob.setBounds(knobBox);

                auto labelBox = juce::Rectangle<int>(knobBox.getRight() + gap,
                                                     row.getCentreY() - labelH / 2,
                                                     labelW,
                                                     labelH);
                label.setBounds(labelBox);
            }
        };

        layoutEqRow(highRow, highLabel, highKnob);
        layoutEqRow(midRow,  midLabel,  midKnob);
        layoutEqRow(lowRow,  lowLabel,  lowKnob);
    };

    auto clusterArea = juce::Rectangle<int>(0, 0, clusterW, clusterH)
                           .withCentre({ mixerArea.getCentreX(), mixerArea.getCentreY() + 18 });

    if (leftDeck)
    {
        auto eqBlock = clusterArea.removeFromLeft(eqBlockW);
        clusterArea.removeFromLeft(clusterGap);
        auto volCol = clusterArea.removeFromLeft(volW);

        layoutEqBlock(eqBlock,
                      highEQLabel, highEQSlider,
                      midEQLabel,  midEQSlider,
                      lowEQLabel,  lowEQSlider,
                      true);

        layoutVerticalSlider(volCol, volLabel, volSlider);
    }
    else
    {
        auto volCol = clusterArea.removeFromLeft(volW);
        clusterArea.removeFromLeft(clusterGap);
        auto eqBlock = clusterArea.removeFromLeft(eqBlockW);

        layoutVerticalSlider(volCol, volLabel, volSlider);

        layoutEqBlock(eqBlock,
                      highEQLabel, highEQSlider,
                      midEQLabel,  midEQSlider,
                      lowEQLabel,  lowEQSlider,
                      false);
    }

    posLabel.setBounds(0, 0, 0, 0);
    posSlider.setBounds(0, 0, 0, 0);

    auto waveformHeader = waveformArea.removeFromTop(16);
    timeLabel.setBounds(waveformHeader.reduced(4, 0));

    auto waveformBounds = waveformArea.reduced(2, 2);
    waveformDisplay.setBounds(waveformBounds);

    auto utilityHost = utilityRowArea.reduced(8, 0);

    clearCuesButton.setButtonText("CLR CUE");
    resetEQButton.setButtonText("EQ RESET");

    const int utilGap = 4;
    //const int loadW = 78;
    const int loadW = 70;
    //const int unloadW = 96;
    const int unloadW = 88;
    //const int syncW = 70;
    //const int resetCueW = 108;
    const int resetCueW = 92;
    //const int resetEqW = 96;
    const int resetEqW = 88;
    //const int utilityRowW = loadW + utilGap + unloadW + utilGap + syncW + utilGap + resetCueW + utilGap + resetEqW;
    const int utilityRowW = loadW + utilGap + unloadW + utilGap + resetCueW + utilGap + resetEqW;
    const int utilityRowH = 25;

    auto utilityRow = juce::Rectangle<int>(0, 0, utilityRowW, utilityRowH)
                          .withCentre(utilityHost.getCentre());

    if (utilityRow.getX() < utilityHost.getX())
        utilityRow.setX(utilityHost.getX());

    if (utilityRow.getRight() > utilityHost.getRight())
        utilityRow.setX(utilityHost.getRight() - utilityRow.getWidth());

    auto loadBounds = utilityRow.removeFromLeft(loadW);
    utilityRow.removeFromLeft(utilGap);

    auto unloadBounds = utilityRow.removeFromLeft(unloadW);
    utilityRow.removeFromLeft(utilGap);
    
    //auto syncBounds = utilityRow.removeFromLeft(syncW);
    //utilityRow.removeFromLeft(utilGap);

    auto clearCueBounds = utilityRow.removeFromLeft(resetCueW);
    utilityRow.removeFromLeft(utilGap);

    auto resetEqBounds = utilityRow.removeFromLeft(resetEqW);

    loadButton.setBounds(loadBounds);
    unloadButton.setBounds(unloadBounds);
    //syncButton.setBounds(syncBounds);
    clearCuesButton.setBounds(clearCueBounds);
    resetEQButton.setBounds(resetEqBounds);

    auto stripArea = bottomArea.reduced(10, 4);

    const int perfRowH = 40;
    auto perfRow = juce::Rectangle<int>(0, 0,
                                        //juce::jmin(stripArea.getWidth() - 8, 380),
                                        juce::jmin(stripArea.getWidth() - 8, 440),
                                        perfRowH)
                       .withCentre(stripArea.getCentre());

    //const int perfGap = 8;
    const int perfGap = 6;
    //const int playW = 78;
    const int playW = 78;
    //const int cueW = 96;
    const int cueW = 64;
    //const int arrowW = 40;
    const int arrowW = 34;
    //const int cueSlotW = 78;
    const int cueSlotW = 78;
    const int syncW = 68;

    auto playBounds = perfRow.removeFromLeft(playW);
    perfRow.removeFromLeft(perfGap);

    auto cueMainBounds = perfRow.removeFromLeft(cueW);
    perfRow.removeFromLeft(perfGap);

    auto prevBounds = perfRow.removeFromLeft(arrowW);
    perfRow.removeFromLeft(perfGap);

    auto cueSlotBounds = perfRow.removeFromLeft(cueSlotW);
    perfRow.removeFromLeft(perfGap);

    auto nextBounds = perfRow.removeFromLeft(arrowW);
    perfRow.removeFromLeft(perfGap);
    
    auto syncBounds = perfRow.removeFromLeft(syncW);

    playButton.setBounds(playBounds);
    cueMainButton.setBounds(cueMainBounds);
    prevCueButton.setBounds(prevBounds);
    selectedCueButton.setBounds(cueSlotBounds);
    nextCueButton.setBounds(nextBounds);
    syncButton.setBounds(syncBounds);

    stopButton.setBounds(0, 0, 0, 0);
}

void DeckGUI::buttonClicked (juce::Button* button)
{
    if (player == nullptr)
        return;

//    if (button == &playButton)
//    {
//        if (player->isPlaying())
//            player->stop();
//        else
//            player->start();
//        return;
//    }
    
    if (button == &playButton)
    {
        if (player->isPlaying())
        {
            player->stop();
            playButton.setToggleState(false, juce::dontSendNotification);
        }
        else
        {
            player->start();
            playButton.setToggleState(true, juce::dontSendNotification);
        }
        return;
    }

    if (button == &stopButton)
    {
        player->stop();
        return;
    }

    if (button == &cueMainButton)
    {
        player->setPosition(0.0);
        return;
    }

    if (button == &prevCueButton)
    {
        selectedCueIndex = (selectedCueIndex + 7) % 8;
        updateCueButtonLabels();
        return;
    }

    if (button == &nextCueButton)
    {
        selectedCueIndex = (selectedCueIndex + 1) % 8;
        updateCueButtonLabels();
        return;
    }

    if (button == &selectedCueButton)
    {
        const bool shiftDown = juce::ModifierKeys::getCurrentModifiers().isShiftDown();

        if (shiftDown)
            player->updateHotCue(selectedCueIndex);
        else if (player->hotCueIsSet(selectedCueIndex))
            player->triggerHotCue(selectedCueIndex);
        else
            player->setHotCue(selectedCueIndex, player->getCurrentPositionInSeconds());

        updateCueButtonLabels();
        return;
    }

    if (button == &loadButton)
    {
        auto fileChooserFlags = juce::FileBrowserComponent::canSelectFiles;

        fChooser.launchAsync(fileChooserFlags,
                             [this](const juce::FileChooser& chooser)
                             {
                                 auto chosenFile = chooser.getResult();

                                 if (chosenFile.existsAsFile())
                                     loadTrackFromURL(juce::URL{ chosenFile });
                             });
        return;
    }

    if (button == &unloadButton)
    {
        unloadTrack();
        return;
    }

    if (button == &clearCuesButton)
    {
        player->clearHotCues();
        updateCueButtonLabels();
        return;
    }

    if (button == &resetEQButton)
    {
        player->resetEQToOriginal();
        syncEQSlidersFromPlayer();
        return;
    }
    if (button == &syncButton)
    {
        if (player != nullptr && syncTargetPlayer != nullptr)
        {
            const double targetBPM = syncTargetPlayer->getEffectiveBPM();

            if (player->syncToTargetBPM(targetBPM))
            {
                speedSlider.setValue(player->getSpeed(), juce::dontSendNotification);
                refreshBPMLabel();
            }
        }
        return;
    }
}

void DeckGUI::sliderValueChanged (juce::Slider* slider)
{
    if (player == nullptr)
        return;

    if (slider == &volSlider)
        player->setGain(volSlider.getValue());

    if (slider == &speedSlider)
        player->setSpeed(speedSlider.getValue());
        refreshBPMLabel();

    if (slider == &posSlider)
        player->setPositionRelative(posSlider.getValue());

    if (slider == &lowEQSlider)
        player->setLowEQ(lowEQSlider.getValue());

    if (slider == &midEQSlider)
        player->setMidEQ(midEQSlider.getValue());

    if (slider == &highEQSlider)
        player->setHighEQ(highEQSlider.getValue());
}

bool DeckGUI::isInterestedInFileDrag (const juce::StringArray& files)
{
    juce::ignoreUnused(files);
    return true;
}

void DeckGUI::filesDropped (const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);

    if (player == nullptr || files.size() != 1)
        return;

    juce::URL url { juce::File { files[0] } };
    loadTrackFromURL(url);
}

void DeckGUI::timerCallback()
{
    if (player == nullptr)
        return;

    const double pos = player->getPositionRelative();
    waveformDisplay.setPositionRelative(pos);

    const double currentSeconds = player->getCurrentPositionInSeconds();
    const double totalSeconds = player->getTrackLengthInSeconds();

    timeLabel.setText(formatTime(currentSeconds) + " / " + formatTime(totalSeconds),
                      juce::dontSendNotification);

    if (player->isPlaying())
    {
        jogRotationAngle += 0.10f;

        if (jogRotationAngle > juce::MathConstants<float>::twoPi)
            jogRotationAngle -= juce::MathConstants<float>::twoPi;
    }
    else
    {
        const float turns = 30.0f;
        jogRotationAngle = (float) (pos * turns * juce::MathConstants<double>::twoPi);
    }

    jogWheel.setRotationAngle(jogRotationAngle);
    
    playButton.setToggleState(player->isPlaying(), juce::dontSendNotification);
}

void DeckGUI::updateCueButtonLabels()
{
    if (player == nullptr)
        return;

    const bool active = player->hotCueIsSet(selectedCueIndex);
    selectedCueButton.setPadActive(active);

    juce::String text = juce::String(selectedCueIndex + 1);
    if (active)
        text << "*";

    selectedCueButton.setPadText(text);
}

void DeckGUI::syncEQSlidersFromPlayer()
{
    if (player == nullptr)
        return;

    lowEQSlider.setValue(player->getLowEQ(), juce::dontSendNotification);
    midEQSlider.setValue(player->getMidEQ(), juce::dontSendNotification);
    highEQSlider.setValue(player->getHighEQ(), juce::dontSendNotification);
}

void DeckGUI::refreshBPMLabel()
{
    if (player == nullptr || ! player->hasTrackLoaded())
    {
        bpmLabel.setText("BPM: --", juce::dontSendNotification);
        return;
    }

    bpmLabel.setText("BPM: " + juce::String(player->getEffectiveBPM(), 1),
                     juce::dontSendNotification);
}
