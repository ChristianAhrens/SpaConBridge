/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include "MatrixOutputProcessor.h"

#include "MatrixOutputProcessorEditor.h"			//<USE MatrixOutputProcessorEditor

#include "../Parameters.h"

#include "../../Controller.h"					//<USE Controller
#include "../../PagedUI/PageComponentManager.h"	//<USE PageComponentManager
#include "../../SoundscapeBridgeAppCommon.h"


namespace SoundscapeBridgeApp
{


static constexpr MatrixOutputId MatrixOutput_ID_MIN = 1;		//< Minimum maxtrix input number
static constexpr MatrixOutputId MatrixOutput_ID_MAX = 128;		//< Highest maxtrix input number

/*
===============================================================================
 Class MatrixOutputProcessor
===============================================================================
*/

/**
 * Class constructor for the processor.
 */
MatrixOutputProcessor::MatrixOutputProcessor(bool insertToConfig)
{
	// Automation parameters.
	// level meter param
	auto lmR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixOutput_LevelMeterPostMute);
	m_MatrixOutputLevelMeter = new GestureManagedAudioParameterFloat("MatrixOutput_LevelMeterPostMute", "levelMeter", lmR.getStart(), lmR.getEnd(), 0.1f, 0.0f);
	m_MatrixOutputLevelMeter->addListener(this);
	addParameter(m_MatrixOutputLevelMeter);

	// gain param
	auto gR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixOutput_Gain);
	m_MatrixOutputGain = new GestureManagedAudioParameterFloat("MatrixOutput_Gain", "gain", gR.getStart(), gR.getEnd(), 0.1f, 0.0f);
	m_MatrixOutputGain->addListener(this);
	addParameter(m_MatrixOutputGain);

	// mute param
	auto mR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixOutput_Mute);
	m_MatrixOutputMute = new GestureManagedAudioParameterInt("MatrixOutput_mute", "mute", static_cast<int>(mR.getStart()), static_cast<int>(mR.getEnd()), 0);
	m_MatrixOutputMute->addListener(this);
	addParameter(m_MatrixOutputMute);

	m_MatrixOutputId = MatrixOutput_ID_MIN; // This default sourceId will be overwritten by ctrl->AddProcessor() below.
	m_processorId = INVALID_PROCESSOR_ID;

	// Default OSC communication mode.
	m_comsMode = CM_Off;

	// Start with all parameter changed flags cleared. Function setStateInformation() 
	// will check whether or not we should initialize parameters when starting up.
	for (int cs = 0; cs < DCS_Max; cs++)
		m_parametersChanged[cs] = DCT_None;

	// Register this new processor instance to the singleton Controller object's internal list.
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		m_processorId = ctrl->AddMatrixOutputProcessor(insertToConfig ? DCS_Host : DCS_Init, this);
}

/**
 * Class destructor for the processor.
 */
MatrixOutputProcessor::~MatrixOutputProcessor()
{
	// Erase this new processor instance from the singleton Controller object's internal list.
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->RemoveMatrixOutputProcessor(this);
}

/**
 * Get the id of this processor instance 
 */
int MatrixOutputProcessor::GetProcessorId() const
{
	return m_processorId;
}

/**
 * Setter function for the processors' Id
 * @param changeSource	The application module which is causing the property change.
 * @param processorId	The new ID
 */
void MatrixOutputProcessor::SetProcessorId(DataChangeSource changeSource, MatrixOutputProcessorId processorId)
{
	ignoreUnused(changeSource);
	if (m_processorId != processorId && processorId != INVALID_PROCESSOR_ID)
	{
		m_processorId = processorId;
	}
}

/**
 * Get the state of the desired flag (or flags) for the desired change source.
 * @param changeSource	The application module querying the change flag.
 * @param change	The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value 
 *			since the last time PopParameterChanged() was called.
 */
bool MatrixOutputProcessor::GetParameterChanged(DataChangeSource changeSource, DataChangeType change)
{
	return ((m_parametersChanged[changeSource] & change) != 0);
}

/**
 * Reset the state of the desired flag (or flags) for the desired change source.
 * Will return the state of the flag before the resetting.
 * @param changeSource	The application module querying the change flag.
 * @param change	The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value 
 *			since the last time PopParameterChanged() was called.
 */
bool MatrixOutputProcessor::PopParameterChanged(DataChangeSource changeSource, DataChangeType change)
{
	bool ret((m_parametersChanged[changeSource] & change) != 0);
	m_parametersChanged[changeSource] &= ~change; // Reset flag.
	return ret;
}

/**
 * Method which will be called every time a parameter or property has been changed.
 * @param changeSource	The application module which is causing the property change.
 * @param changeTypes	Defines which parameter or property has been changed.
 */
void MatrixOutputProcessor::SetParameterChanged(DataChangeSource changeSource, DataChangeType changeTypes)
{
	// Set the specified change flag for all DataChangeSources.
	for (int cs = 0; cs < DCS_Max; cs++)
	{
		// If the change came from OSC (received message with new param value), 
		// do not set the specified change flag for OSC. This would trigger an 
		// OSC Set command to go out for every received message.
		if ((changeSource != DCS_Protocol) || (cs != DCS_Protocol))
			m_parametersChanged[cs] |= changeTypes;
	}
}

/**
 * Get the current value of a specific automation parameter.
 * @param paramIdx	The index of the desired parameter.
 * @param normalized If true, the returned value will be normalized to a 0.0f to 1.0f range. False per default.
 * @return	The desired parameter value, as float.
 */
float MatrixOutputProcessor::GetParameterValue(SoundobjectParameterIndex paramIdx, bool normalized) const
{
	float ret = 0.0f;

	switch (paramIdx)
	{
		case MCI_ParamIdx_LevelMeterPreMute:
			{
				ret = m_MatrixOutputLevelMeter->get();
				if (normalized)
					ret = m_MatrixOutputLevelMeter->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case MCI_ParamIdx_Gain:
			{
				ret = m_MatrixOutputGain->get();
				if (normalized)
					ret = m_MatrixOutputGain->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case MCI_ParamIdx_Mute:
			{
				ret = static_cast<float>(m_MatrixOutputMute->get());
				if (normalized)
					ret = m_MatrixOutputMute->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		default:
			jassertfalse;
			break;
	}

	return ret;
}

/**
 * Set the value of a specific automation parameter.
 * @param changeSource	The application module which is causing the property change.
 * @param paramIdx	The index of the desired parameter.
 * @param newValue	The new value as a float.
 */
void MatrixOutputProcessor::SetParameterValue(DataChangeSource changeSource, SoundobjectParameterIndex paramIdx, float newValue)
{
	// The reimplemented method AudioProcessor::parameterValueChanged() will trigger a SetParameterChanged() call.
	// We need to ensure that this change is registered to the correct source. 
	// We set the source here, so that it can be used in parameterValueChanged(). 
	m_currentChangeSource = changeSource;

	switch (paramIdx)
	{
	case MCI_ParamIdx_LevelMeterPreMute:
		m_MatrixOutputLevelMeter->SetParameterValue(newValue);
		break;
	case MCI_ParamIdx_Gain:
		m_MatrixOutputGain->SetParameterValue(newValue);
		break;
	case MCI_ParamIdx_Mute:
		m_MatrixOutputMute->SetParameterValue(static_cast<int>(newValue));
		break;
	default:
		jassertfalse; // Unknown parameter index!
		break;
	}

	// After the SetParameterChanged() call has been triggered, set the change source to the default.
	// The host is the only one which can call parameterValueChanged directly. All other modules of the
	// application do it over this method.
	m_currentChangeSource = DCS_Host;
}

/**
 * This method should be called once every timer callback tick of the Controller. 
 * The signal is passed on to all automation parameters. This is used to trigger gestures for touch automation.
 */
void MatrixOutputProcessor::Tick()
{
	// Reset the flags indicating when a parameter's SET command is out on the network. 
	// These flags are set during Controller::timerCallback() and queried in Controller::oscMessageReceived()
	m_paramSetCommandsInTransit = DCT_None;

	for (int pIdx = 0; pIdx < MCI_ParamIdx_MaxIndex; pIdx++)
	{
		switch (pIdx)
		{
		case MCI_ParamIdx_LevelMeterPreMute:
			m_MatrixOutputLevelMeter->Tick();
			break;
		case MCI_ParamIdx_Gain:
			m_MatrixOutputGain->Tick();
			break;
		case MCI_ParamIdx_Mute:
			m_MatrixOutputMute->Tick();
			break;
		default:
			jassert(false); // missing implementation!
			break;
		}
	}
}

/**
 * The given parameter(s) have a SET command message which has just been sent out on the network.
 * @param paramsChanged		Which parameter(s) should be marked as having a SET command in transit.
 */
void MatrixOutputProcessor::SetParamInTransit(DataChangeType paramsChanged)
{
	m_paramSetCommandsInTransit |= paramsChanged;
}

/**
 * Check if the given parameter(s) have a SET command message which has just been sent out on the network.
 * @return True if the specified paranmeter(s) are marked as having a SET command in transit.
 */
bool MatrixOutputProcessor::IsParamInTransit(DataChangeType paramsChanged) const
{
	return ((m_paramSetCommandsInTransit & paramsChanged) != DCT_None);
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to dump this objects' settings
 * to a XML element structure that is returned and written to config file by the
 * singleton AppConfiguration class implementation.
 * @return	The XML element data that was created.
 */
std::unique_ptr<XmlElement> MatrixOutputProcessor::createStateXml()
{
	auto processorInstanceXmlElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()));
	if (processorInstanceXmlElement)
	{
		processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSOROBJECTID), static_cast<int>(GetMatrixOutputId()));
        processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE), static_cast<int>(GetComsMode()));
	}

    return processorInstanceXmlElement;
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to set this objects' settings
 * from a XML element structure that passed as argument.
 * @param stateXml	The XML element containing this objects' configuration data
 * @return	True if the data was read and handled successfuly, false if not.
 */
bool MatrixOutputProcessor::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || (stateXml->getTagName() != (AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()))))
		return false;

	SetMatrixOutputId(DCS_Init, static_cast<MatrixOutputId>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSOROBJECTID))));
    SetComsMode(DCS_Init, static_cast<ComsMode>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE))));

	return true;
}

/**
 * The host will call this method when it wants to save the processor's internal state.
 * This must copy any info about the processor's state into the block of memory provided, 
 * so that the host can store this and later restore it using setStateInformation().
 * @param destData		Stream where the processor parameters will be written to.
 */
void MatrixOutputProcessor::getStateInformation(MemoryBlock& destData)
{
	ignoreUnused(destData);
}

/**
 * This method is called when project is loaded, or when a snapshot is recalled.
 * Use this method to restore your parameters from this memory block,
 * whose contents will have been created by the getStateInformation() call.
 * @sa MatrixOutputProcessor::DisablePollingForTicks()
 * @param data			Stream where the processor parameters will be read from.
 * @param sizeInBytes	Size of stream buffer.
 */
void MatrixOutputProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	ignoreUnused(data);
	ignoreUnused(sizeInBytes);
}

/**
 * Set the new OSC communication mode (sending and/or receiving).
 * @param changeSource	The application module which is causing the property change.
 * @param newMode	The new OSC communication mode.
 */
void MatrixOutputProcessor::SetComsMode(DataChangeSource changeSource, ComsMode newMode)
{
	if (m_comsMode != newMode)
	{
		m_comsMode = newMode;

		// Reset response-ignoring mechanism.
		m_paramSetCommandsInTransit = DCT_None;

		// Signal change to other modules in the processor.
		SetParameterChanged(changeSource, DCT_ComsMode);
	}
}

/**
 * Get the current OSC communication mode (either sending or receiving).
 * @return The current OSC communication mode.
 */
ComsMode MatrixOutputProcessor::GetComsMode() const
{
	return m_comsMode;
}

/**
 * Setter function for the MatrixOutput Id
 * @param changeSource	The application module which is causing the property change.
 * @param MatrixOutputId	The new ID
 */
void MatrixOutputProcessor::SetMatrixOutputId(DataChangeSource changeSource, MatrixOutputId MatrixOutputId)
{
	if (m_MatrixOutputId != MatrixOutputId)
	{
		// Ensure it's within allowed range.
		m_MatrixOutputId = jmin(MatrixOutput_ID_MAX, jmax(MatrixOutput_ID_MIN, MatrixOutputId));

		// Signal change to other modules in the processor.
		SetParameterChanged(changeSource, DCT_MatrixOutputID);
        
        // finally trigger config update
        if (changeSource != DCS_Init)
            triggerConfigurationUpdate(false);
	}
}

/**
 * Getter function for the MatrixOutput Id
 * @return	The current MatrixOutput ID
 */
MatrixOutputId MatrixOutputProcessor::GetMatrixOutputId() const
{
	return m_MatrixOutputId;
}

/**
 * Setter function for the send rate used in the outgoing OSC messages.
 * @param changeSource	The application module which is causing the property change.
 * @param oscMsgRate	The interval at which OSC messages are sent, in ms.
 */
void MatrixOutputProcessor::SetMessageRate(DataChangeSource changeSource, int oscMsgRate)
{
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetRate(changeSource, oscMsgRate);
}

/**
 * Getter function for the send rate used in the outgoing OSC messages.
 * @return	The interval at which OSC messages are sent, in ms.
 */
int MatrixOutputProcessor::GetMessageRate() const
{
	int rate = 0;
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		rate = ctrl->GetRate();

	return rate;
}

/**
 * Method to initialize config setting, without risking overwriting with the defaults.
 * @param MatrixOutputId		New SourceID or matrix input number to use for this processor instance.
 * @param mappingId		New coordinate mapping to use for this procssor instance.
 * @param ipAddress		New IP address of the DS100 device.
 * @param newMode		New OSC communication mode (Rx/Tx).
 */
void MatrixOutputProcessor::InitializeSettings(MatrixOutputId MatrixOutputId, String ipAddress, ComsMode newMode)
{
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
	{
		jassert(MatrixOutputId > 128);
		SetMatrixOutputId(DCS_Init, MatrixOutputId);
		SetComsMode(DCS_Init, newMode);
	}
}

/**
 * Method to get a list of remote object identifiers that are used by this soundsource processing object.
 * @return	The requested list of remote object identifiers.
 */
const std::vector<RemoteObjectIdentifier>	MatrixOutputProcessor::GetUsedRemoteObjects()
{
	return std::vector<RemoteObjectIdentifier>{ROI_CoordinateMapping_SourcePosition_XY, ROI_CoordinateMapping_SourcePosition_X, ROI_CoordinateMapping_SourcePosition_Y, ROI_MatrixInput_ReverbSendGain, ROI_Positioning_SourceSpread, ROI_Positioning_SourceDelayMode};
};


//==============================================================================
// Overriden functions of class AudioProcessorParameter::Listener


/**
 * REIMPLEMENTED from AudioProcessorParameter::Listener::parameterValueChanged()
 * The host will call this method AFTER one of the filter's parameters has been changed.
 * The host may call this at any time, even when a parameter's value isn't actually being changed, 
 * including during the audio processing callback (avoid blocking!).
 * @param parameterIndex	Index of the procssor parameter being changed.
 * @param newValue			New parameter value, always between 0.0f and 1.0f.
 */
void MatrixOutputProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	DataChangeType changed = DCT_None;

	switch (parameterIndex)
	{
		case MCI_ParamIdx_LevelMeterPreMute:
			{
				if (m_MatrixOutputLevelMeter->get() != m_MatrixOutputLevelMeter->GetLastValue())
					changed = DCT_MatrixOutputLevelMeter;
			}
			break;
		case MCI_ParamIdx_Gain:
			{
				if (m_MatrixOutputGain->get() != m_MatrixOutputGain->GetLastValue())
					changed = DCT_MatrixOutputGain;
			}
			break;
		case MCI_ParamIdx_Mute:
			{
				int newValueDenorm = static_cast<int>(m_MatrixOutputMute->getNormalisableRange().convertFrom0to1(newValue));
				if (newValueDenorm != m_MatrixOutputMute->GetLastValue())
					changed = DCT_MatrixOutputMute;
			}
			break;
		default:
			jassertfalse;
			break;
	}

	if (changed != DCT_None)
	{
		// To ensure that this property change is registered with the correct source, 
		// m_currentChangeSource is set properly inside SetParameterValue
		SetParameterChanged(m_currentChangeSource, changed);
	}
}

/**
 * REIMPLEMENTED from AudioProcessorParameter::Listener::parameterGestureChanged()
 * Indicates that a parameter change gesture has started / ended. 
 * This reimplementation does nothing. See GestureManagedAudioParameterFloat::BeginGuiGesture().
 * @param parameterIndex	Index of the procssor parameter being changed.
 * @param gestureIsStarting	True if starting, false if ending.
 */
void MatrixOutputProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
	ignoreUnused(parameterIndex);
	ignoreUnused(gestureIsStarting);
}



//==============================================================================
// More overriden functions of class AudioProcessor


/**
 * Returns the name of this processor.
 * @return The procssor name.
 */
const String MatrixOutputProcessor::getName() const
{
	return JUCEApplication::getInstance()->getApplicationName();
}

/**
 * Returns true if the processor wants midi messages.
 * @return	True if the processor wants midi messages.
 */
bool MatrixOutputProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

/**
 * Returns true if the processor produces midi messages.
 * @return	True if the processor produces midi messages.
 */
bool MatrixOutputProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

/**
 * Returns the length of the filter's tail, in seconds.
 * @return	Zero, since no audio delay is introduced.
 */

double MatrixOutputProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

/**
 * Returns the number of preset programs the filter supports.
 * The value returned must be valid as soon as this object is created, and must not change over its lifetime.
 * @return Number of preset programs the filter supports. This value shouldn't be less than 1.
 */
int MatrixOutputProcessor::getNumPrograms()
{
	return 1;
}

/**
 * Returns the number of the currently active program.
 * @return Returns the number of the currently active program.
 */
int MatrixOutputProcessor::getCurrentProgram()
{
	return 0;
}

/**
 * Called by the host to change the current program.
 * @param index		New program index.
 */
void MatrixOutputProcessor::setCurrentProgram(int index)
{
	ignoreUnused(index);
}

/**
 * Returns the name of a given program.
 * @param index		Index of the desired program
 * @return			Desired program name.
 */
const String MatrixOutputProcessor::getProgramName(int index)
{
	ignoreUnused(index);
	return m_processorDisplayName;
}

/**
 * Called by the host to rename a program.
 * @param index		Index of the desired program
 * @param newName	Desired new program name.
 */
void MatrixOutputProcessor::changeProgramName(int index, const String& newName)
{
	ignoreUnused(index);
	m_processorDisplayName = newName;

	// Signal change to other modules in the procssor.
	SetParameterChanged(DCS_Host, DCT_MatrixOutputID);
}

/**
 * Called before playback starts, to let the filter prepare itself.
 * @param sampleRate	The sample rate is the target sample rate, and will remain constant until playback stops.
 *						You can call getTotalNumInputChannels and getTotalNumOutputChannels or query the busLayout member
 *						variable to find out the number of channels your processBlock callback must process.
 * @param samplesPerBlock	This value is a strong hint about the maximum number of samples that will be provided in each block.
 *							You may want to use this value to resize internal buffers. You should program defensively in case
 *							a buggy host exceeds this value. The actual block sizes that the host uses may be different each time
 *							the callback happens: completely variable block sizes can be expected from some hosts.
 */
void MatrixOutputProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	ignoreUnused(sampleRate, samplesPerBlock);
}

/**
 * Called after playback has stopped, to let the filter free up any resources it no longer needs.
 * When playback stops, you can use this as an opportunity to free up any spare memory, etc.
 */
void MatrixOutputProcessor::releaseResources()
{
}


/**
 * Renders the next block. This reimplementation does nothing.
 * @param buffer	When this method is called, the buffer contains a number of channels which is at least as great
 *					as the maximum number of input and output channels that this filter is using. It will be filled with the
 *					filter's input data and should be replaced with the filter's output.
 * @param midiMessages	If the filter is receiving a midi input, then the midiMessages array will be filled with the midi
 *						messages for this block. Each message's timestamp will indicate the message's time, as a number of samples
 *						from the start of the block. Any messages left in the midi buffer when this method has finished are assumed
 *						to be the filter's midi output. This means that your filter should be careful to clear any incoming
 *						messages from the array if it doesn't want them to be passed-on.
 */
void MatrixOutputProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	ignoreUnused(buffer, midiMessages);
}

/**
 * This function returns true if the procssor can create an editor component.
 * @return True.
 */
bool MatrixOutputProcessor::hasEditor() const
{
	return true;
}

/**
 * Creates the procssor's GUI.
 * This can return nullptr if you want a UI-less filter, in which case the host may create a generic UI that lets the user twiddle the parameters directly.
 * @return	A pointer to the newly created editor component.
 */
AudioProcessorEditor* MatrixOutputProcessor::createEditor()
{
	AudioProcessorEditor* editor = new MatrixOutputProcessorEditor(*this);

	// Initialize GUI with current IP address, etc.
	SetParameterChanged(DCS_Host, (DCT_ProcessorInstanceConfig | DCT_CommunicationConfig | DCT_SoundobjectParameters));

	return editor;
}


} // namespace SoundscapeBridgeApp
