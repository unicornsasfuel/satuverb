/*
  ==============================================================================

   This file implements a JUCE effect plugin that expects faust2api -juce generated
   DSP files, such as those exported from the FAUST WebIDE using the
   "source -> juce" option.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include <JuceHeader.h>

// Uncomment the #define directive below when your PGM XML file is ready
// Add it to the project files with the name "magic.xml"

//#define MAGIC_LOAD_BINARY 1

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(std::unique_ptr<::FaustPolyEngine> &faustEngine)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Build list of parameters as reported by Faust engine
    int numParams = faustEngine->getParamsCount();
    
    for (int i = 0; i < numParams; i++) {
        std::string paramName = faustEngine->getParamAddress(i);
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(paramName, 1),
            paramName,
            juce::NormalisableRange<float>(
                faustEngine->getParamMin(i),
                faustEngine->getParamMax(i)
                ),
            faustEngine->getParamInit(i))
        );
    }
    return layout;
}

//==============================================================================
FaustSkeletonAudioProcessor::FaustSkeletonAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : foleys::MagicProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    treeState(*this, nullptr, "PARAMETERS", createParameterLayout(faustEngine))
#endif

{
    //add parameter listeners
    int numParams = faustEngine->getParamsCount();

    for (int i = 0; i < numParams; i++) {
        treeState.addParameterListener(faustEngine->getParamAddress(i), this);
    }

#ifdef MAGIC_LOAD_BINARY    
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
#endif
    
}

FaustSkeletonAudioProcessor::~FaustSkeletonAudioProcessor()
{
    
}

//==============================================================================
const juce::String FaustSkeletonAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FaustSkeletonAudioProcessor::acceptsMidi() const
{
    return false;
}

bool FaustSkeletonAudioProcessor::producesMidi() const
{
    return false;
}

bool FaustSkeletonAudioProcessor::isMidiEffect() const
{
    return false;
}

double FaustSkeletonAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FaustSkeletonAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FaustSkeletonAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FaustSkeletonAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String FaustSkeletonAudioProcessor::getProgramName(int index)
{
    return {};
}

void FaustSkeletonAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void FaustSkeletonAudioProcessor::parameterChanged(const juce::String& param, float value)
{

    faustEngine->setParamValue(param.toRawUTF8(), value);

}

//==============================================================================
void FaustSkeletonAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Initialize the DSP
    faustDsp->init(sampleRate);

    // Parse Faust metadata to automatically set latency
    juce::var parsedJson;
    std::string jsonMeta = faustEngine->getJSONMeta();
    if (!juce::JSON::parse(jsonMeta, parsedJson).failed()) {
        auto meta = parsedJson["meta"];
        int metaSize = meta.size();

        for (int i = 0; i < metaSize; i++) {
            int foo = int(meta[i]["latency_samples"]);
            if (meta[i]["latency_samples"]) {
                setLatencySamples(int(meta[i]["latency_samples"]));
            }
            else if (meta[i]["latency_sec"]) {
                setLatencySamples(int(meta[i]["latency_sec"]) * sampleRate);
            }
        }
    }

}

void FaustSkeletonAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FaustSkeletonAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void FaustSkeletonAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Pass audio sample buffer to Faust DSP for processing
    faustIO = buffer.getArrayOfWritePointers();
    faustEngine->compute(buffer.getNumSamples(), faustIO, faustIO);
    
    // Pass MIDI messages along to Faust engine
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        const int time = message.getTimeStamp();
        int channel = message.getChannel();
        const juce::uint8* rawmidi = message.getRawData();
        int type = rawmidi[0] & 0xf0;
        int count = message.getMessageLengthFromFirstByte(rawmidi[0]);

        int data1 = NULL;
        int data2 = NULL;
        if (count > 1) {
            data1 = rawmidi[1];
            if (count > 2) {
                data2 = rawmidi[2];
            }
        }
        faustEngine->propagateMidi(count, time, type, channel, data1, data2);
    }
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FaustSkeletonAudioProcessor();
}
