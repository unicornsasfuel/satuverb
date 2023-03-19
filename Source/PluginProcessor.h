/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DspFaust.cpp"

//==============================================================================
/**
*/
class FaustSkeletonAudioProcessor  : public foleys::MagicProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
                             , private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    FaustSkeletonAudioProcessor();
    ~FaustSkeletonAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
   
    void parameterChanged (const juce::String &param, float value) override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

private:
    //==============================================================================
    mydsp* faustDsp = new mydsp();
    std::unique_ptr<::midi_handler> faustMidi = std::make_unique<::midi_handler>();
    std::unique_ptr<::FaustPolyEngine> faustEngine = std::make_unique<::FaustPolyEngine>(faustDsp, nullptr, faustMidi.get());

    float** faustIO;

    juce::AudioProcessorValueTreeState treeState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaustSkeletonAudioProcessor)
};
