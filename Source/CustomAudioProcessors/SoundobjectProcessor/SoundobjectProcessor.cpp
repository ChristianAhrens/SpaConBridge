/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in
and now in a derived version is part of SpaConBridge.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include "SoundobjectProcessor.h"

#include "SoundobjectProcessorEditor.h"				//<USE SoundobjectProcessorEditor

#include "../Parameters.h"

#include "../../Controller.h"						//<USE Controller
#include "../../PagedUI/PageComponentManager.h"		//<USE PageComponentManager
#include "../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


static constexpr SoundobjectId SOURCE_ID_MIN = 1;	//< Minimum maxtrix input number / SourceId
static constexpr SoundobjectId SOURCE_ID_MAX = 128;	//< Highest maxtrix input number / SourceId
static constexpr int DEFAULT_COORD_MAPPING = 1;		//< Default coordinate mapping

/*
===============================================================================
 Class SoundobjectProcessor
===============================================================================
*/

/**
 * Class constructor for the processor.
 */
SoundobjectProcessor::SoundobjectProcessor(bool insertToConfig)
{
	// Automation parameters.
	// x coord. param
	auto xR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_CoordinateMapping_SourcePosition_X);
	m_xPos = new GestureManagedAudioParameterFloat("x_pos", "x", xR.getStart(), xR.getEnd(), 0.001f, 0.5f);
	m_xPos->addListener(this);
	addParameter(m_xPos);

	// x coord. param
	auto yR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_CoordinateMapping_SourcePosition_Y);
	m_yPos = new GestureManagedAudioParameterFloat("y_pos", "y", yR.getStart(), yR.getEnd(), 0.001f, 0.5f);
	m_yPos->addListener(this);
	addParameter(m_yPos);

	// EnSpace send gain param
	auto rsgR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_ReverbSendGain);
	m_reverbSendGain = new GestureManagedAudioParameterFloat("ReverbSendGain", "Reverb", rsgR.getStart(), rsgR.getEnd(), 0.1f, 0.0f);
	m_reverbSendGain->addListener(this);
	addParameter(m_reverbSendGain);

	// sound object spread param
	auto ssR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_Positioning_SourceSpread);
	m_sourceSpread = new GestureManagedAudioParameterFloat("SourceSpread", "Spread", ssR.getStart(), ssR.getEnd(), 0.01f, 0.5f);
	m_sourceSpread->addListener(this);
	addParameter(m_sourceSpread);

	// sound object delay mode param
	StringArray delayModeChoices("Off", "Tight", "Full");
	m_delayMode = new GestureManagedAudioParameterChoice("DelayMode", "Delay", delayModeChoices, 1);
	m_delayMode->addListener(this);
	addParameter(m_delayMode);

	// display name is empty per default.
	m_processorDisplayName = String();

	m_soundobjectId = SOURCE_ID_MIN; // This default sourceId will be overwritten by ctrl->AddProcessor() below.
	m_mappingId = DEFAULT_COORD_MAPPING; // Default: coordinate mapping 1.
	m_processorId = INVALID_PROCESSOR_ID;

	// Default OSC communication mode.
	m_comsMode = CM_Off;

	// Default painting parameters
	m_soundobjectColour = Colours::black;
	m_soundobjectSize = 0.5f;

	// Start with all parameter changed flags cleared. Function setStateInformation() 
	// will check whether or not we should initialize parameters when starting up.
	for (int cs = 0; cs < DCP_Max; cs++)
		m_parametersChanged[cs] = DCT_None;

	// Register this new procssor instance to the singleton Controller object's internal list.
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		m_processorId = ctrl->AddSoundobjectProcessor(insertToConfig ? DCP_Host : DCP_Init, this);
}

/**
 * Class destructor for the processor.
 */
SoundobjectProcessor::~SoundobjectProcessor()
{
	// Erase this new procssor instance from the singleton Controller object's internal list.
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->RemoveSoundobjectProcessor(this);
}

/**
 * Get the id of this processor instance 
 */
int SoundobjectProcessor::GetProcessorId() const
{
	return m_processorId;
}

/**
 * Setter function for the processors' Id
 * @param changeSource	The application module which is causing the property change.
 * @param processorId	The new ID
 */
void SoundobjectProcessor::SetProcessorId(DataChangeParticipant changeSource, SoundobjectProcessorId processorId)
{
	ignoreUnused(changeSource);
	if (m_processorId != processorId && processorId != INVALID_PROCESSOR_ID)
	{
		m_processorId = processorId;
	}
}

/**
 * Get the state of the desired flag (or flags) for the desired change source.
 * @param changeTarget	The application module querying the change flag.
 * @param change	The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value 
 *			since the last time PopParameterChanged() was called.
 */
bool SoundobjectProcessor::GetParameterChanged(DataChangeParticipant changeTarget, DataChangeType change)
{
	return ((m_parametersChanged[changeTarget] & change) != 0);
}

/**
 * Reset the state of the desired flag (or flags) for the desired change source.
 * Will return the state of the flag before the resetting.
 * @param changeTarget	The application module querying the change flag.
 * @param change	The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value 
 *			since the last time PopParameterChanged() was called.
 */
bool SoundobjectProcessor::PopParameterChanged(DataChangeParticipant changeTarget, DataChangeType change)
{
	bool ret((m_parametersChanged[changeTarget] & change) != 0);
	m_parametersChanged[changeTarget] &= ~change; // Reset flag.
	return ret;
}

/**
 * Method which will be called every time a parameter or property has been changed.
 * @param changeSource	The application module which is causing the property change.
 * @param changeTypes	Defines which parameter or property has been changed.
 */
void SoundobjectProcessor::SetParameterChanged(DataChangeParticipant changeSource, DataChangeType changeTypes)
{
	// Set the specified change flag for all DataChangeSources.
	for (int cs = 0; cs < DCP_Max; cs++)
	{
		// If the change came from OSC (received message with new param value), 
		// do not set the specified change flag for OSC. This would trigger an 
		// OSC Set command to go out for every received message.
		if ((changeSource != DCP_Protocol) || (cs != DCP_Protocol))
			m_parametersChanged[cs] |= changeTypes;
	}
}

/**
 * Get the current value of a specific automation parameter.
 * @param paramIdx	The index of the desired parameter.
 * @param normalized If true, the returned value will be normalized to a 0.0f to 1.0f range. False per default.
 * @return	The desired parameter value, as float.
 */
float SoundobjectProcessor::GetParameterValue(SoundobjectParameterIndex paramIdx, bool normalized) const
{
	float ret = 0.0f;

	switch (paramIdx)
	{
		case SPI_ParamIdx_X:
			{
				ret = m_xPos->get();
				if (normalized)
					ret = m_xPos->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case SPI_ParamIdx_Y:
			{
				ret = m_yPos->get();
				if (normalized)
					ret = m_yPos->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case SPI_ParamIdx_ReverbSendGain:
			{
				ret = m_reverbSendGain->get();
				if (normalized)
					ret = m_reverbSendGain->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case SPI_ParamIdx_ObjectSpread:
			{
				ret = m_sourceSpread->get();
				if (normalized)
					ret = m_sourceSpread->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case SPI_ParamIdx_DelayMode:
			{
				// AudioParameterChoice::getIndex() maps the internal 0.0f - 1.0f value to the 0 to N-1 range.
				ret = static_cast<float>(m_delayMode->getIndex());
				if (normalized)
					ret = m_delayMode->getNormalisableRange().convertTo0to1(ret);
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
void SoundobjectProcessor::SetParameterValue(DataChangeParticipant changeSource, SoundobjectParameterIndex paramIdx, float newValue)
{
	// The reimplemented method AudioProcessor::parameterValueChanged() will trigger a SetParameterChanged() call.
	// We need to ensure that this change is registered to the correct source. 
	// We set the source here, so that it can be used in parameterValueChanged(). 
	m_currentChangeSource = changeSource;

	switch (paramIdx)
	{
	case SPI_ParamIdx_X:
		m_xPos->SetParameterValue(newValue);
		break;
	case SPI_ParamIdx_Y:
		m_yPos->SetParameterValue(newValue);
		break;
	case SPI_ParamIdx_ReverbSendGain:
		m_reverbSendGain->SetParameterValue(newValue);
		break;
	case SPI_ParamIdx_ObjectSpread:
		m_sourceSpread->SetParameterValue(newValue);
		break;
	case SPI_ParamIdx_DelayMode:
		m_delayMode->SetParameterValue(newValue);
		break;
	default:
		jassertfalse; // Unknown parameter index!
		break;
	}

	// After the SetParameterChanged() call has been triggered, set the change source to the default.
	// The host is the only one which can call parameterValueChanged directly. All other modules of the
	// application do it over this method.
	m_currentChangeSource = DCP_Host;
}

/**
 * This method should be called once every timer callback tick of the Controller. 
 * The signal is passed on to all automation parameters. This is used to trigger gestures for touch automation.
 */
void SoundobjectProcessor::Tick()
{
	// Reset the flags indicating when a parameter's SET command is out on the network. 
	// These flags are set during Controller::timerCallback() and queried in Controller::oscMessageReceived()
	m_paramSetCommandsInTransit = DCT_None;

	for (int pIdx = 0; pIdx < SPI_ParamIdx_MaxIndex; pIdx++)
	{
		switch (pIdx)
		{
		case SPI_ParamIdx_X:
			m_xPos->Tick();
			break;
		case SPI_ParamIdx_Y:
			m_yPos->Tick();
			break;
		case SPI_ParamIdx_ReverbSendGain:
			m_reverbSendGain->Tick();
			break;
		case SPI_ParamIdx_ObjectSpread:
			m_sourceSpread->Tick();
			break;
		case SPI_ParamIdx_DelayMode:
			m_delayMode->Tick();
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
void SoundobjectProcessor::SetParamInTransit(DataChangeType paramsChanged)
{
	m_paramSetCommandsInTransit |= paramsChanged;
}

/**
 * Check if the given parameter(s) have a SET command message which has just been sent out on the network.
 * @return True if the specified paranmeter(s) are marked as having a SET command in transit.
 */
bool SoundobjectProcessor::IsParamInTransit(DataChangeType paramsChanged) const
{
	return ((m_paramSetCommandsInTransit & paramsChanged) != DCT_None);
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to dump this objects' settings
 * to a XML element structure that is returned and written to config file by the
 * singleton AppConfiguration class implementation.
 * @return	The XML element data that was created.
 */
std::unique_ptr<XmlElement> SoundobjectProcessor::createStateXml()
{
	auto processorInstanceXmlElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()));
	if (processorInstanceXmlElement)
	{
		processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCHANNELID), static_cast<int>(GetSoundobjectId()));
        processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORRECORDID), static_cast<int>(GetMappingId()));
        processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE), static_cast<int>(GetComsMode()));
		processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOLOUR), GetSoundobjectColour().toString());
		processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORSIZE), GetSoundobjectSize());
	}

    return processorInstanceXmlElement;
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to set this objects' settings
 * from a XML element structure that passed as argument.
 * @param stateXml	The XML element containing this objects' configuration data
 * @return	True if the data was read and handled successfuly, false if not.
 */
bool SoundobjectProcessor::setStateXml(XmlElement* stateXml)
{
	// sanity check, if the incoming xml does make sense for this method
	if (!stateXml || (stateXml->getTagName() != (AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()))))
		return false;

	// To prevent that we end up in a recursive ::setStateXml situation, verify that this setStateXml method is not called by itself
	const ScopedXmlChangeLock lock(IsXmlChangeLocked());
	if (!lock.isLocked())
		return false;

	SetSoundobjectId(DCP_Init, static_cast<SoundobjectId>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCHANNELID))));
    SetMappingId(DCP_Init, static_cast<MappingId>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORRECORDID))));
    SetComsMode(DCP_Init, static_cast<ComsMode>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE))));
	if (stateXml->hasAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOLOUR)) && stateXml->hasAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORSIZE)))
	{
		SetSoundobjectColour(DCP_Init, juce::Colour::fromString(stateXml->getStringAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOLOUR))));
		SetSoundobjectSize(DCP_Init, stateXml->getDoubleAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORSIZE)));
	}

	return true;
}

/**
 * The host will call this method when it wants to save the processor's internal state.
 * This must copy any info about the processor's state into the block of memory provided, 
 * so that the host can store this and later restore it using setStateInformation().
 * @param destData		Stream where the procssor parameters will be written to.
 */
void SoundobjectProcessor::getStateInformation(MemoryBlock& destData)
{
	ignoreUnused(destData);
}

/**
 * This method is called when project is loaded, or when a snapshot is recalled.
 * Use this method to restore your parameters from this memory block,
 * whose contents will have been created by the getStateInformation() call.
 * @sa SoundobjectProcessor::DisablePollingForTicks()
 * @param data			Stream where the procssor parameters will be read from.
 * @param sizeInBytes	Size of stream buffer.
 */
void SoundobjectProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	ignoreUnused(data);
	ignoreUnused(sizeInBytes);
}

/**
 * Set the new OSC communication mode (sending and/or receiving).
 * @param changeSource	The application module which is causing the property change.
 * @param newMode	The new OSC communication mode.
 */
void SoundobjectProcessor::SetComsMode(DataChangeParticipant changeSource, ComsMode newMode)
{
	if (m_comsMode != newMode)
	{
		m_comsMode = newMode;

		// Reset response-ignoring mechanism.
		m_paramSetCommandsInTransit = DCT_None;

		// Signal change to other modules in the procssor.
		SetParameterChanged(changeSource, DCT_ComsMode);
	}
}

/**
 * Get the current OSC communication mode (either sending or receiving).
 * @return The current OSC communication mode.
 */
ComsMode SoundobjectProcessor::GetComsMode() const
{
	return m_comsMode;
}

/**
 * Setter function for the coordinate mapping idx.
 * @param changeSource	The application module which is causing the property change.
 * @param mappingId		The new coordinate mapping ID
 */
void SoundobjectProcessor::SetMappingId(DataChangeParticipant changeSource, MappingId mappingId)
{
	if (m_mappingId != mappingId)
	{
		m_mappingId = mappingId;

		// Signal change to other modules in the procssor.
		SetParameterChanged(changeSource, DCT_MappingID);
        
        // finally trigger config update
        if (changeSource != DCP_Init)
            triggerConfigurationUpdate(false);
	}
}

/**
 * Getter function for the coordinate mapping Id
 * @return	The current coordinate mapping ID
 */
MappingId SoundobjectProcessor::GetMappingId() const
{
	return m_mappingId;
}

/**
 * Setter function for the source Id
 * @param changeSource	The application module which is causing the property change.
 * @param soundobjectId	The new ID
 */
void SoundobjectProcessor::SetSoundobjectId(DataChangeParticipant changeSource, SoundobjectId soundobjectId)
{
	if (m_soundobjectId != soundobjectId)
	{
		// Ensure it's within allowed range.
		m_soundobjectId = jmin(SOURCE_ID_MAX, jmax(SOURCE_ID_MIN, soundobjectId));

		// Signal change to other modules in the procssor.
		SetParameterChanged(changeSource, DCT_SoundobjectID);
        
        // finally trigger config update
        if (changeSource != DCP_Init)
            triggerConfigurationUpdate(false);
	}
}

/**
 * Getter method for the sound object Id
 * @return	The sound object ID of this processor instance
 */
SoundobjectId SoundobjectProcessor::GetSoundobjectId() const
{
	return m_soundobjectId;
}

/**
 * Setter method for the sound object painting colour
 * @param	changeSource	The application module which is causing the property change.
 * @param	colour			The new colour to set
 */
void SoundobjectProcessor::SetSoundobjectColour(DataChangeParticipant changeSource, const juce::Colour& colour)
{
	if (m_soundobjectColour != colour)
	{
		m_soundobjectColour = colour;

		// Signal change to other modules in the procssor.
		SetParameterChanged(changeSource, DCT_SoundobjectColourAndSize);

		// finally trigger config update
		if (changeSource != DCP_Init)
			triggerConfigurationUpdate(false);
	}
}

/**
 * Getter method for the sound object painting colour
 * @return	The sound object painting colour for this processor instance
 */
const juce::Colour& SoundobjectProcessor::GetSoundobjectColour() const
{
	return m_soundobjectColour;
}

/**
 * Setter method for the sound object painting size
 * @param	changeSource	The application module which is causing the property change.
 * @param	size			The new size to set
 */
void SoundobjectProcessor::SetSoundobjectSize(DataChangeParticipant changeSource, double size)
{
	if (m_soundobjectSize != size)
	{
		m_soundobjectSize = size;

		// Signal change to other modules in the procssor.
		SetParameterChanged(changeSource, DCT_SoundobjectColourAndSize);

		// finally trigger config update
		if (changeSource != DCP_Init)
			triggerConfigurationUpdate(false);
	}
}

/**
 * Getter method for the sound object painting size
 * @return	The sound object painting size for this processor instance
 */
double SoundobjectProcessor::GetSoundobjectSize() const
{
	return m_soundobjectSize;
}

/**
 * Method to initialize config setting, without risking overwriting with the defaults.
 * @param soundobjectId	New SoundobjectID or matrix input number to use for this processor instance.
 * @param mappingId		New coordinate mapping to use for this procssor instance.
 * @param ipAddress		New IP address of the DS100 device.
 * @param newMode		New protocol communication mode (Rx/Tx).
 */
void SoundobjectProcessor::InitializeSettings(SoundobjectId soundobjectId, MappingId mappingId, String ipAddress, ComsMode newMode)
{
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
	{
		jassert(soundobjectId > 128);
		SetSoundobjectId(DCP_Init, soundobjectId);
		jassert(mappingId > 4);
		SetMappingId(DCP_Init, mappingId);
		SetComsMode(DCP_Init, newMode);
	}
}

/**
 * Method to get a list of remote object identifiers that are used by this soundsource processing object.
 * @return	The requested list of remote object identifiers.
 */
const std::vector<RemoteObjectIdentifier>	SoundobjectProcessor::GetUsedRemoteObjects()
{
	return std::vector<RemoteObjectIdentifier>{
		ROI_CoordinateMapping_SourcePosition_XY,
		ROI_CoordinateMapping_SourcePosition_X, 
		ROI_CoordinateMapping_SourcePosition_Y,
		ROI_MatrixInput_ReverbSendGain, 
		ROI_Positioning_SourceSpread, 
		ROI_Positioning_SourceDelayMode };
};

/**
 * Method to get a list of non-flicering remote object identifiers that are used by this soundsource processing object.
 * @return	The requested list of remote object identifiers.
 */
const std::vector<RemoteObjectIdentifier>	SoundobjectProcessor::GetStaticRemoteObjects()
{
	return std::vector<RemoteObjectIdentifier>{
		ROI_MatrixInput_ChannelName };
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
void SoundobjectProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	DataChangeType changed = DCT_None;

	switch (parameterIndex)
	{
		case SPI_ParamIdx_X:
			{
				if (m_xPos->get() != m_xPos->GetLastValue())
					changed = DCT_SoundobjectPosition;
			}
			break;
		case SPI_ParamIdx_Y:
			{
				if (m_yPos->get() != m_yPos->GetLastValue())
					changed = DCT_SoundobjectPosition;
			}
			break;
		case SPI_ParamIdx_ReverbSendGain:
			{
				if (m_reverbSendGain->get() != m_reverbSendGain->GetLastValue())
					changed = DCT_ReverbSendGain;
			}
			break;
		case SPI_ParamIdx_ObjectSpread:
			{
				if (m_sourceSpread->get() != m_sourceSpread->GetLastValue())
					changed = DCT_SoundobjectSpread;
			}
			break;
		case SPI_ParamIdx_DelayMode:
			{
				int newValueDenorm = static_cast<int>(m_delayMode->getNormalisableRange().convertFrom0to1(newValue));
				if (newValueDenorm != m_delayMode->GetLastIndex())
					changed = DCT_DelayMode;
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
void SoundobjectProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
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
const String SoundobjectProcessor::getName() const
{
	return JUCEApplication::getInstance()->getApplicationName();
}

/**
 * Returns true if the processor wants midi messages.
 * @return	True if the processor wants midi messages.
 */
bool SoundobjectProcessor::acceptsMidi() const
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
bool SoundobjectProcessor::producesMidi() const
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

double SoundobjectProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

/**
 * Returns the number of preset programs the filter supports.
 * The value returned must be valid as soon as this object is created, and must not change over its lifetime.
 * @return Number of preset programs the filter supports. This value shouldn't be less than 1.
 */
int SoundobjectProcessor::getNumPrograms()
{
	return 1;
}

/**
 * Returns the number of the currently active program.
 * @return Returns the number of the currently active program.
 */
int SoundobjectProcessor::getCurrentProgram()
{
	return 0;
}

/**
 * Called by the host to change the current program.
 * @param index		New program index.
 */
void SoundobjectProcessor::setCurrentProgram(int index)
{
	ignoreUnused(index);
}

/**
 * Returns the name of a given program.
 * @param index		Index of the desired program
 * @return			Desired program name.
 */
const String SoundobjectProcessor::getProgramName(int index)
{
	ignoreUnused(index);
	return m_processorDisplayName;
}

/**
 * Called by the host to rename a program.
 * @param index		Index of the desired program
 * @param newName	Desired new program name.
 */
void SoundobjectProcessor::changeProgramName(int index, const String& newName)
{
	if (index != getCurrentProgram())
		return;
	if (newName == m_processorDisplayName)
		return;

	m_processorDisplayName = newName;

	// Signal change to other modules in the procssor.
	SetParameterChanged(DCP_Host, DCT_SoundobjectID);
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
void SoundobjectProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	ignoreUnused(sampleRate, samplesPerBlock);
}

/**
 * Called after playback has stopped, to let the filter free up any resources it no longer needs.
 * When playback stops, you can use this as an opportunity to free up any spare memory, etc.
 */
void SoundobjectProcessor::releaseResources()
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
void SoundobjectProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	ignoreUnused(buffer, midiMessages);
}

/**
 * This function returns true if the procssor can create an editor component.
 * @return True.
 */
bool SoundobjectProcessor::hasEditor() const
{
	return true;
}

/**
 * Creates the procssor's GUI.
 * This can return nullptr if you want a UI-less filter, in which case the host may create a generic UI that lets the user twiddle the parameters directly.
 * @return	A pointer to the newly created editor component.
 */
AudioProcessorEditor* SoundobjectProcessor::createEditor()
{
	AudioProcessorEditor* editor = new SoundobjectProcessorEditor(*this);

	// Initialize GUI with current IP address, etc.
	SetParameterChanged(DCP_Host, (DCT_SoundobjectProcessorConfig | DCT_CommunicationConfig | DCT_SoundobjectParameters));

	return editor;
}


} // namespace SpaConBridge

