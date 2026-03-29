/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (850, 600);

    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted)
                                           {
                                               if (granted)
                                                   setAudioChannels (0, 2);
                                           });
    }
    else
    {
        setAudioChannels (0, 2);
    }

    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);

    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
    
    deckGUI1.setSyncTargetPlayer(&player2);
    deckGUI2.setSyncTargetPlayer(&player1);

    addAndMakeVisible(playlistComponent);

    formatManager.registerBasicFormats();
}

MainComponent::~MainComponent()
{
    mixerSource.removeAllInputs();
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay (samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay (samplesPerBlockExpected, sampleRate);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    mixerSource.releaseResources();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    const int playlistH = (int) (getHeight() * 0.28f);
    const int deckH = getHeight() - playlistH;

    deckGUI1.setBounds(0, 0, getWidth() / 2, deckH);
    deckGUI2.setBounds(getWidth() / 2, 0, getWidth() / 2, deckH);

    playlistComponent.setBounds(0, deckH, getWidth(), playlistH);
}
