/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

auto getSelectedTabName() { return juce::String("Selected Tab"); }

auto getPhaserRateName() { return juce::String("Phaser RateHz"); }
auto getPhaserCenterFreqName() { return juce::String("Phaser Center FreqHz"); }
auto getPhaserDepthName() { return juce::String("Phaser Depth %"); }
auto getPhaserFeedbackName() { return juce::String("Phaser Feedback %"); }
auto getPhaserMixName() { return juce::String("Phaser Mix %"); }
auto getPhaserBypassName() { return juce::String("Phaser Bypass"); }

auto getChorusRateName() { return juce::String("Chorus RateHz"); }
auto getChorusDepthName() { return juce::String("Chorus Depth %"); }
auto getChorusCenterDelayName() { return juce::String("Chorus Center Delay ms"); }
auto getChorusFeedbackName() { return juce::String("Chorus Feedback %"); }
auto getChorusMixName() { return juce::String("Chorus Mix %"); }
auto getChorusBypassName() { return juce::String("Chorus Bypass"); }

auto getOverdriveSaturationName() { return juce::String("OverDrive Saturation");}
auto getOverdriveBypassName() { return juce::String("Overdrive Bypass"); }

auto getLadderFilterModeName() { return juce::String("Ladder Filter Mode"); }
auto getLadderFilterCutoffName() { return juce::String("Ladder Filter Cutoff Hz"); }
auto getLadderFilterResonanceName() { return juce::String("Ladder Filter Resonance"); }
auto getLadderFilterDriveName() { return juce::String("Ladder Filter Drive"); }
auto getLadderFilterBypassName() { return juce::String("Ladder Filter Bypass"); }

auto getLadderFilterChoices()
{
    return juce::StringArray
    {
        "LPF12",
        "HPF12",
        "BPF12",
        "LPF24",
        "HPF24",
        "BPF24",
    };
}

auto getGeneralFilterChoices()
{
    return juce::StringArray
    {
        "Peak",
        "bandpass",
        "notch",
        "allpass",
    };
}
auto getGeneralFilterModeName() { return juce::String("General Filter Mode"); }
auto getGeneralFilterFreqName() { return juce::String("General Filter Freq hz"); }
auto getGeneralFilterQualityName() { return juce::String("General Filter Quality"); }
auto getGeneralFilterGainName() { return juce::String("General Filter Gain"); }
auto getGeneralFilterBypassName() { return juce::String("General Filter Bypass"); }

//==============================================================================
Project13AudioProcessor::Project13AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

    for (size_t i = 0; i < static_cast<size_t>(DSP_Option::END_OF_LIST); ++i)
    {
        dspOrder[i] = static_cast<DSP_Option>(i);
    }
    restoreDspOrderFifo.push(dspOrder);
    /*
     cached params
     */
    auto floatParams = std::array
    {
        &phaserRateHz,
        &phaserCenterFreqHz,
        &phaserDepthPercent,
        &phaserFeedbackPercent,
        &phaserMixPercent,

        &chorusRateHz,
        &chorusDepthPercent,
        &chorusCenterDelayMs,
        &chorusFeedbackPercent,
        &chorusMixPercent,

        &overdriveSaturation,

        &ladderFilterCutoffHz,
        &ladderFilterResonance,
        &ladderFilterDrive,

        &generalFilterFreqHz,
        &generalFilterQuality,
        &generalFilterGain,
    };
    auto floatNameFuncs = std::array
    {
        &getPhaserRateName,
        &getPhaserCenterFreqName,
        &getPhaserDepthName,
        &getPhaserFeedbackName,
        &getPhaserMixName,

        &getChorusRateName,
        &getChorusDepthName,
        &getChorusCenterDelayName,
        &getChorusFeedbackName,
        &getChorusMixName,

        &getOverdriveSaturationName,

        &getLadderFilterCutoffName,
        &getLadderFilterResonanceName,
        &getLadderFilterDriveName,

        &getGeneralFilterFreqName,
        &getGeneralFilterQualityName,
        &getGeneralFilterGainName,
    };

    auto choiceParams = std::array
    {
        &ladderFilterMode,

        &generalFilterMode,
    };

    auto choiceNameFuncs = std::array
    {
        &getLadderFilterModeName,

        &getGeneralFilterModeName,
    };

    auto bypassParams = std::array
    {
        &phaserBypass,
        &chorusBypass,
        &overdriveBypass,
        &ladderFilterBypass,
        &generalFilterBypass,
    };

    auto bypassNameFuncs = std::array
    {
        &getPhaserBypassName,
        &getChorusBypassName,
        &getOverdriveBypassName,
        &getLadderFilterBypassName,
        &getGeneralFilterBypassName,
    };

    auto intParams = std::array
    {
        &selectedTab,
    };

    auto intFuncs = std::array
    {
        &getSelectedTabName,
    };

    initCachedParams<juce::AudioParameterInt*>(intParams, intFuncs);
    initCachedParams<juce::AudioParameterBool*>(bypassParams, bypassNameFuncs);
    jassert(floatParams.size() == floatNameFuncs.size());
    initCachedParams<juce::AudioParameterFloat*>(floatParams, floatNameFuncs);
    initCachedParams<juce::AudioParameterChoice*>(choiceParams, choiceNameFuncs);
}

Project13AudioProcessor::~Project13AudioProcessor()
{
}

//==============================================================================
const juce::String Project13AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Project13AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Project13AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Project13AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Project13AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Project13AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Project13AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Project13AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Project13AudioProcessor::getProgramName (int index)
{
    return {};
}

void Project13AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Project13AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    leftChannel.prepare(spec);
    rightChannel.prepare(spec);

    for (auto smoother : getSmoothers())
    {
        smoother->reset(sampleRate, 0.005); //5 ms smoothing time
    }

    updateSmoothersFromParams(1, SmootherUpdateMode::initialize);
}

std::vector<juce::SmoothedValue<float>*> Project13AudioProcessor::getSmoothers()
{
    auto smoothers = std::vector
    {
        &phaserRateHzSmoother,
        &phaserCenterFreqHzSmoother,
        &phaserDepthPercentSmoother,
        &phaserFeedbackPercentSmoother,
        &phaserMixPercentSmoother,
        &chorusRateHzSmoother,
        &chorusDepthPercentSmoother,
        &chorusCenterDelayMsSmoother,
        &chorusFeedbackPercentSmoother,
        &chorusMixPercentSmoother,
        &overdriveSaturationSmoother,
        &ladderFilterCutoffHzSmoother,
        &ladderFilterResonanceSmoother,
        &ladderFilterDriveSmoother,
        &generalFilterFreqHzSmoother,
        &generalFilterQualitySmoother,
        &generalFilterGainSmoother,
    };

    return smoothers;
}

void Project13AudioProcessor::updateSmoothersFromParams(int numSamplesToSkip, SmootherUpdateMode init)
{
    auto paramsNeedingSmoothing = std::array
    {
        phaserRateHz,
        phaserCenterFreqHz,
        phaserDepthPercent,
        phaserFeedbackPercent,
        phaserMixPercent,
        chorusRateHz,
        chorusDepthPercent,
        chorusCenterDelayMs,
        chorusFeedbackPercent,
        chorusMixPercent,
        overdriveSaturation,
        ladderFilterCutoffHz,
        ladderFilterResonance,
        ladderFilterDrive,
        generalFilterFreqHz,
        generalFilterQuality,
        generalFilterGain,
    };

    auto smoothers = getSmoothers();

    jassert(smoothers.size() == paramsNeedingSmoothing.size());

    for (size_t i = 0; i < smoothers.size(); ++i)
    {
        auto smoother = smoothers[i];
        auto param = paramsNeedingSmoothing[i];

        if (init == SmootherUpdateMode::initialize)
            smoother->setCurrentAndTargetValue(param->get());
        else
            smoother->setTargetValue(param->get());

        smoother->skip(numSamplesToSkip);
    }
}

std::vector<juce::RangedAudioParameter*>
Project13AudioProcessor::getParamsForOptions(Project13AudioProcessor::DSP_Option option)
{
    switch (option)
    {
    case DSP_Option::Phase:
    {
        return
        {
            phaserRateHz,
            phaserCenterFreqHz,
            phaserDepthPercent,
            phaserFeedbackPercent,
            phaserMixPercent,
            phaserBypass,
        };
    }
    case DSP_Option::Chorus:
    {
        return
        {
            chorusRateHz,
            chorusDepthPercent,
            chorusCenterDelayMs,
            chorusFeedbackPercent,
            chorusMixPercent,
            chorusBypass,
        };
    }
    case DSP_Option::OverDrive:
    {
        return
        {
            overdriveSaturation,
            overdriveBypass,
        };
    }
    case DSP_Option::LadderFilter:
    {
        return
        {
            ladderFilterMode,
            ladderFilterCutoffHz,
            ladderFilterResonance,
            ladderFilterDrive,
            ladderFilterBypass,
        };
    }
    case DSP_Option::GeneralFilter:
    {
        return
        {
            generalFilterMode,
            generalFilterFreqHz,
            generalFilterQuality,
            generalFilterGain,
            generalFilterBypass,
        };
    }
    case DSP_Option::END_OF_LIST:
        break;
    }

    jassertfalse;
    return {};
}

void Project13AudioProcessor::MonoChannelDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    jassert(spec.numChannels == 1);

    std::vector<juce::dsp::ProcessorBase*> dsp
    {
        &phaser,
        &chorus,
        &overdrive,
        &ladderFilter,
        &generalFilter
    };

    for (auto p : dsp)
    {
        p->prepare(spec);
        p->reset();
    }
}

void Project13AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Project13AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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

juce::AudioProcessorValueTreeState::ParameterLayout Project13AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const int versionHint = 1;

    auto name = getSelectedTabName();
    layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ name, versionHint }, name, 0, static_cast<int>(DSP_Option::END_OF_LIST) - 1, static_cast<int>(DSP_Option::Chorus)));

    /*
     phaser:
     rate: Hz
     depth: 0 to 1
     center freq: Hz
     feedback: -1 to 1
     mix: 0 to 1
     */

     //phaser rate: LFO Hz
    name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f), 0.2f, "Hz"));
    //phaser depth: 0 - 1
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f), 5.f, "%"));
    //phaser center freq: audio Hz
    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 1000.f, "Hz"));
    //phaser feedback: -1 to 1
    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(-100.f, 100.f, 0.10f, 1.f), 0.0f, "%"));
    //phaser mix: 0 - 1
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f), 5.f, "%"));
    name = getPhaserBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ name, versionHint }, name, false));
    /*
     Chorus:
     rate: Hz (less than 100hz)
     depth: 0 to 1
     centre delay: milliseconds (1 to 100)
     feedback: -1 to 1
     mix: 0 to 1
     */

     //rate: Hz
    name = getChorusRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(0.01f, 50.f, 0.01f, 1.f), 0.2f, "Hz"));
    //depth: 0 to 1
    name = getChorusDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(0.f, 100.f, 0.01f, 1.f), 5.f, "%"));
    //centre delay: milliseconds (1 to 100)
    name = getChorusCenterDelayName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(1.f, 50.f, 0.1f, 1.f), 7.f, "ms"));
    //feedback: -1 to 1
    name = getChorusFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(-100.f, 100.f, 0.1f, 1.f), 0.0f, "%"));
    //mix: 0 to 1
    name = getChorusMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f), 5.0f, "%"));
    name = getChorusBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ name, versionHint }, name, false));
    /*
     overdrive
     uses the drive portion of the LadderFilter class for now
     drive: 1 - 100
     */
     //drive: 1-100
    name = getOverdriveSaturationName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f), 1.f, ""));
    name = getOverdriveBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ name, versionHint }, name, false));
    /*
     ladder filter:
     mode: LadderFilterMode enum (int)
     cutoff: hz
     resonance: 0 to 1
     drive: 1 - 100
     */

     //mode: LadderFilterMode enum (int)
    name = getLadderFilterModeName();
    auto choices = getLadderFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ name, versionHint }, name, choices, 0));
    //cutoff: hz
    name = getLadderFilterCutoffName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(20.f, 20000.f, 0.1f, 1.f), 20000.f, "Hz"));
    //resonance: 0 to 1
    name = getLadderFilterResonanceName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(0.f, 100.f, 0.1f, 1.f), 0.f, "%"));

    //drive: 1 - 100
    name = getLadderFilterDriveName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint }, name, juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f), 1.f, ""));
    name = getLadderFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ name, versionHint }, name, false));
    /*
     general filter: https://docs.juce.com/develop/structdsp_1_1IIR_1_1Coefficients.html
     Mode: Peak, bandpass, notch, allpass,
     freq:
     Q:
     gain:
     */
     //Mode: Peak, bandpass, notch, allpass,
    name = getGeneralFilterModeName();
    choices = getGeneralFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ name, versionHint },
        name, choices, 0));
    //freq:
    name = getGeneralFilterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint },
        name,
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
        750.f, "Hz"));
    //Q:
    name = getGeneralFilterQualityName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 100.f, 0.01f, 1.f),
        0.72f, ""));
    //gain:
    name = getGeneralFilterGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name, versionHint },
        name,
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f, "dB"));
    name = getGeneralFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ name, versionHint }, name, false));
    return layout;
}

void Project13AudioProcessor::MonoChannelDSP::updateDSPFromParams()
{
    phaser.dsp.setRate(p.phaserRateHzSmoother.getCurrentValue());
    phaser.dsp.setCentreFrequency(p.phaserCenterFreqHzSmoother.getCurrentValue());
    phaser.dsp.setDepth(p.phaserDepthPercentSmoother.getCurrentValue() * 0.01f);
    phaser.dsp.setFeedback(p.phaserFeedbackPercentSmoother.getCurrentValue() * 0.01f);
    phaser.dsp.setMix(p.phaserMixPercentSmoother.getCurrentValue() * 0.01f);

    chorus.dsp.setRate(p.chorusRateHzSmoother.getCurrentValue());
    chorus.dsp.setDepth(p.chorusDepthPercentSmoother.getCurrentValue() * 0.01f);
    chorus.dsp.setCentreDelay(p.chorusCenterDelayMsSmoother.getCurrentValue());
    chorus.dsp.setFeedback(p.chorusFeedbackPercentSmoother.getCurrentValue() * 0.01f);
    chorus.dsp.setMix(p.chorusMixPercentSmoother.getCurrentValue() * 0.01f);

    overdrive.dsp.setDrive(p.overdriveSaturationSmoother.getCurrentValue());

    ladderFilter.dsp.setMode(static_cast<juce::dsp::LadderFilterMode>(p.ladderFilterMode->getIndex()));
    ladderFilter.dsp.setCutoffFrequencyHz(p.ladderFilterCutoffHzSmoother.getCurrentValue());
    ladderFilter.dsp.setResonance(p.ladderFilterResonanceSmoother.getCurrentValue() * 0.01f);
    ladderFilter.dsp.setDrive(p.ladderFilterDriveSmoother.getCurrentValue());

    auto sampleRate = p.getSampleRate();
    //update generalFilter coefficients
    //choices: Peak, bandpass, notch, allpass,
    auto genMode = p.generalFilterMode->getIndex();
    auto genHz = p.generalFilterFreqHzSmoother.getCurrentValue();
    auto genQ = p.generalFilterQualitySmoother.getCurrentValue();
    auto genGain = p.generalFilterGainSmoother.getCurrentValue();

    bool filterChanged = false;
    filterChanged |= (filterFreq != genHz);
    filterChanged |= (filterQ != genQ);
    filterChanged |= (filterGain != genGain);

    auto updatedMode = static_cast<GeneralFilterMode>(genMode);
    filterChanged |= (filterMode != updatedMode);
    if (filterChanged)
    {
        filterMode = updatedMode;
        filterFreq = genHz;
        filterQ = genQ;
        filterGain = genGain;
        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
        switch (filterMode)
        {
            case GeneralFilterMode::Peak:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, filterFreq, filterQ, juce::Decibels::decibelsToGain(filterGain));
                break;
            }
            case GeneralFilterMode::Bandpass:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, filterFreq, filterQ);
                break;
            }
            case GeneralFilterMode::Notch:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, filterFreq, filterQ);
                break;
            }
            case GeneralFilterMode::Allpass:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate, filterFreq, filterQ);
                break;
            }
            case GeneralFilterMode::END_OF_LIST:
            {
                jassertfalse;
                break;
            }
        }

        if (coefficients != nullptr)
        {

            *generalFilter.dsp.coefficients = *coefficients;
            generalFilter.reset();
        }
    }

}

void Project13AudioProcessor::MonoChannelDSP::process(juce::dsp::AudioBlock<float> block, const DSP_Order& dspOrder)
{
    DSP_Pointers dspPointers;
    dspPointers.fill({});
    for (size_t i = 0; i < dspPointers.size(); ++i)
    {
        switch (dspOrder[i])
        {
        case DSP_Option::Phase:
            dspPointers[i].processor = &phaser;
            dspPointers[i].bypassed = p.phaserBypass->get();
            break;
        case DSP_Option::Chorus:
            dspPointers[i].processor = &chorus;
            dspPointers[i].bypassed = p.chorusBypass->get();
            break;
        case DSP_Option::OverDrive:
            dspPointers[i].processor = &overdrive;
            dspPointers[i].bypassed = p.overdriveBypass->get();
            break;
        case DSP_Option::LadderFilter:
            dspPointers[i].processor = &ladderFilter;
            dspPointers[i].bypassed = p.ladderFilterBypass->get();
            break;
        case DSP_Option::GeneralFilter:
            dspPointers[i].processor = &generalFilter;
            dspPointers[i].bypassed = p.generalFilterBypass->get();
            break;
        case DSP_Option::END_OF_LIST:
            jassertfalse;
            break;
        }
    }

    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for (size_t i = 0; i < dspPointers.size(); ++i)
    {
        if (dspPointers[i].processor != nullptr)
        {
            juce::ScopedValueSetter<bool> svs(context.isBypassed,
                dspPointers[i].bypassed);
#if VERIFY_BYPASS_FUNCTIONALITY
            if (context.isBypassed)
            {
                jassertfalse;
            }

            if (dspPointers[i].processor == &bandFilter)
            {
                continue;
            }
#endif

            dspPointers[i].processor->process(context);
        }
    }
}

void Project13AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //[DONE]: add APVTS
    //[DONE]: create audio parameters for all dsp choices
    //[DONE]: update DSP here from audio parameters
    //[DONE]: update generalFilter coefficients
    //[DONE]: add smoothers for all param updates 
    //[DONE]: save/load settings
    //[DONE]: save/load DSP order
    //[DONE]: bypass params for each DSP element
    //TODO: Filters are mono, not stereo
    //[DONE]: Drag-To-Reorder GUI
    //TODO: GUI design for each DSP instance?
    //[DONE]: metering
    //TODO: restore tabs in GUI when loading settings
    //[DONE]: prepare all DSP
    //[DONE]: snap dropped tabs to correct position
    //[DONE]: hide dragged tab image or stop dragging the tag and constrain dragged image to X axis only. 
    //TODO: wet/dry knob [BONUS]
    //TODO: mono & stereo versions [mono is BONUS]
    //TODO: modulators [BONUS]
    //TODO: thread-safe filter updating [BONUS]
    //TODO: pre/post filtering [BONUS]
    //TODO: delay module [BONUS]
    //TODO: save/load presets [BONUS]

    leftChannel.updateDSPFromParams();
    rightChannel.updateDSPFromParams();

    //temp instance to pull into
    auto newDSPOrder = DSP_Order();

    //try to pull
    while (dspOrderFifo.pull(newDSPOrder))
    {
#if VERIFY_BYPASS_FUNCTIONALITY
        jassertfalse;
#endif
    }

    //if you pulled, replace dspOrder
    if (newDSPOrder != DSP_Order())
        dspOrder = newDSPOrder;


        /*
         process max 64 samples at a time.

        (1) get the number of samples that need processing.
        (2) compute the max number of samples to process at a time (max 64)
        (3) do the following with the current block, as long as there are samples to process
        (4) compute the number of samples to process during this sub-block
        (5) update the smoothers from params
        (6) update the mono channels from the smoothers
        (7) create a sub-block that holds max samples of audio from the original block
        (8) process each channel
        (9) increment the number of samples processed, decrement the number of samples remaining
        We will also need a counter to keep track of the start sample for a particular sub-block (10).
         */

    if (guiNeedsLatestDspOrder.compareAndSetBool(false, true))
    {
        restoreDspOrderFifo.push(dspOrder);
    }

    const auto numSamples = buffer.getNumSamples();
    auto samplesRemaining = numSamples;
    auto maxSamplesToProcess = juce::jmin(numSamples, 64);

    leftPreRMS.set(buffer.getRMSLevel(0, 0, numSamples));
    rightPreRMS.set(buffer.getRMSLevel(1, 0, numSamples));

    auto block = juce::dsp::AudioBlock<float>(buffer);
    size_t startSample = 0; // (10) 
    while (samplesRemaining > 0) // (3) 
    {
        /*
         figure out how many samples to actually process.
         i.e., you might have a buffer size of 72.
         The first time through this loop samplesToProcess will be 64, because maxSmplesToProcess is set to 64, and samplesRamaining is 72.
         the 2nd time this loop runs, samplesToProcess will be 8, because the previous loop consumed 64 of the 72 samples.
         */
        auto samplesToProcess = juce::jmin(samplesRemaining, maxSamplesToProcess); // (4) 
        //advance each smoother 'samplesToProcess' samples
        updateSmoothersFromParams(samplesToProcess, SmootherUpdateMode::liveInRealtime); // (5)

        //update the DSP 
        leftChannel.updateDSPFromParams();  // (6)
        rightChannel.updateDSPFromParams();

        //create a sub block from the buffer, and
        auto subBlock = block.getSubBlock(startSample, samplesToProcess); // (7)

        //now process
        leftChannel.process(subBlock.getSingleChannelBlock(0), dspOrder); // (8)
        rightChannel.process(subBlock.getSingleChannelBlock(1), dspOrder);

        startSample += samplesToProcess; // (9)
        samplesRemaining -= samplesToProcess;
    }

    leftPostRMS.set(buffer.getRMSLevel(0, 0, numSamples));
    rightPostRMS.set(buffer.getRMSLevel(1, 0, numSamples));
}

//==============================================================================
bool Project13AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Project13AudioProcessor::createEditor()
{
    return new Project13AudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
template<>
struct juce::VariantConverter<Project13AudioProcessor::DSP_Order>
{
    static Project13AudioProcessor::DSP_Order fromVar(const juce::var& v)
    {
        using T = Project13AudioProcessor::DSP_Order;
        T dspOrder;

        jassert(v.isBinaryData());
        if (!v.isBinaryData())
        {
            dspOrder.fill(Project13AudioProcessor::DSP_Option::END_OF_LIST);
        }
        else
        {
            auto mb = *v.getBinaryData();

            juce::MemoryInputStream mis(mb, false);
            std::vector<int> arr;
            while (!mis.isExhausted())
            {
                arr.push_back(mis.readInt());
            }
            jassert(arr.size() == dspOrder.size());
            for (size_t i = 0; i < dspOrder.size(); ++i)
            {
                dspOrder[i] = static_cast<Project13AudioProcessor::DSP_Option>(arr[i]);
            }
        }

        return dspOrder;
    }
    static juce::var toVar(const Project13AudioProcessor::DSP_Order& t)
    {
        juce::MemoryBlock mb;
        //juce MOS uses scoping to complete writing to the memory block correctly.
        {
            juce::MemoryOutputStream mos(mb, false);

            for (auto& v : t)
            {
                mos.writeInt(static_cast<int>(v));
            }
        }
        return mb;
    }
};
//==============================================================================
void Project13AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    apvts.state.setProperty("dspOrder", 
        juce::VariantConverter<Project13AudioProcessor::DSP_Order>::toVar(dspOrder), nullptr);

    juce::MemoryOutputStream mos(destData, false);
    apvts.state.writeToStream(mos);
}

void Project13AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        if (apvts.state.hasProperty("dspOrder"))
        {
            auto order = juce::VariantConverter<Project13AudioProcessor::DSP_Order>::fromVar(apvts.state.getProperty("dspOrder"));
            dspOrderFifo.push(order);
            restoreDspOrderFifo.push(order);
        }
        DBG(apvts.state.toXmlString());

#if VERIFY_BYPASS_FUNCTIONALITY
        juce::Timer::callAfterDelay(1000, [this]()
            {
                DSP_Order order;
        order.fill(DSP_Option::LadderFilter);
        order[0] = DSP_Option::Chorus;

        //bypass the Chorus
        chorusBypass->setValueNotifyingHost(1.f);
        dspOrderFifo.push(order);
            });
#endif

    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Project13AudioProcessor();
}
