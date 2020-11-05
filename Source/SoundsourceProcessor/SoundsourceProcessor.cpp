/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

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


#include "SoundsourceProcessor.h"

#include "SoundsourceProcessorEditor.h"	//<USE SoundsourceProcessorEditor
#include "Parameters.h"

#include "../Controller.h"		//<USE CController
#include "../Overview/OverviewManager.h"		//<USE COverviewManager
#include "../SoundscapeBridgeAppCommon.h"
#include "../Version.h"		//<USE CVersion


namespace SoundscapeBridgeApp
{


static constexpr SourceId SOURCE_ID_MIN = 1;		//< Minimum maxtrix input number / SourceId
static constexpr SourceId SOURCE_ID_MAX = 64;		//< Highest maxtrix input number / SourceId
static constexpr int DEFAULT_COORD_MAPPING = 1;		//< Default coordinate mapping

/*
===============================================================================
 Class SoundsourceProcessor
===============================================================================
*/

/**
 * Class constructor for the processor.
 */
SoundsourceProcessor::SoundsourceProcessor(bool insertToConfig)
{
	// Automation parameters.
	m_xPos = new GestureManagedAudioParameterFloat("x_pos", "x", 0.0f, 1.0f, 0.001f, 0.5f);
	m_yPos = new GestureManagedAudioParameterFloat("y_pos", "y", 0.0f, 1.0f, 0.001f, 0.5f);
	m_xPos->addListener(this);
	m_yPos->addListener(this);
	addParameter(m_xPos);
	addParameter(m_yPos);

	m_reverbSendGain = new GestureManagedAudioParameterFloat("ReverbSendGain", "Reverb", -120.0f, 24.0f, 0.1f, 0.0f);
	m_sourceSpread = new GestureManagedAudioParameterFloat("SourceSpread", "Spread", 0.0f, 1.0f, 0.001f, 0.5f);
	StringArray delayModeChoices("Off", "Tight", "Full");
	m_delayMode = new GestureManagedAudioParameterChoice("DelayMode", "Delay", delayModeChoices, 1);

	m_reverbSendGain->addListener(this);
	m_sourceSpread->addListener(this);
	m_delayMode->addListener(this);
	addParameter(m_reverbSendGain);
	addParameter(m_sourceSpread);
	addParameter(m_delayMode);

	// Plugin's display name is empty per default.
	m_pluginDisplayName = String();

	m_sourceId = SOURCE_ID_MIN; // This default sourceId will be overwritten by ctrl->AddProcessor() below.
	m_mappingId = DEFAULT_COORD_MAPPING; // Default: coordinate mapping 1.
	m_processorId = INVALID_PROCESSOR_ID;

	// Default OSC communication mode.
	m_comsMode = CM_Off;
	m_comsModeWhenNotBypassed = m_comsMode;

	// Start with all parameter changed flags cleared. Function setStateInformation() 
	// will check whether or not we should initialize parameters when starting up.
	for (int cs = 0; cs < DCS_Max; cs++)
		m_parametersChanged[cs] = DCT_None;

	// Register this new plugin instance to the singleton CController object's internal list.
	CController* ctrl = CController::GetInstance();
	if (ctrl)
		m_processorId = ctrl->AddProcessor(insertToConfig ? DCS_Host : DCS_Init, this);
}

/**
 * Class destructor for the processor.
 */
SoundsourceProcessor::~SoundsourceProcessor()
{
	// Erase this new plugin instance from the singleton CController object's internal list.
	CController* ctrl = CController::GetInstance();
	if (ctrl)
		ctrl->RemoveProcessor(this);
}

/**
 * Get the id of this processor instance 
 */
int SoundsourceProcessor::GetProcessorId() const
{
	return m_processorId;
}

/**
 * Setter function for the processors' Id
 * @param changeSource	The application module which is causing the property change.
 * @param processorId	The new ID
 */
void SoundsourceProcessor::SetProcessorId(DataChangeSource changeSource, ProcessorId processorId)
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
bool SoundsourceProcessor::GetParameterChanged(DataChangeSource changeSource, DataChangeType change)
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
bool SoundsourceProcessor::PopParameterChanged(DataChangeSource changeSource, DataChangeType change)
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
void SoundsourceProcessor::SetParameterChanged(DataChangeSource changeSource, DataChangeType changeTypes)
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
float SoundsourceProcessor::GetParameterValue(AutomationParameterIndex paramIdx, bool normalized) const
{
	float ret = 0.0f;

	switch (paramIdx)
	{
		case ParamIdx_X:
			{
				ret = m_xPos->get();
				if (normalized)
					ret = m_xPos->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case ParamIdx_Y:
			{
				ret = m_yPos->get();
				if (normalized)
					ret = m_yPos->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case ParamIdx_ReverbSendGain:
			{
				ret = m_reverbSendGain->get();
				if (normalized)
					ret = m_reverbSendGain->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case ParamIdx_SourceSpread:
			{
				ret = m_sourceSpread->get();
				if (normalized)
					ret = m_sourceSpread->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case ParamIdx_DelayMode:
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
void SoundsourceProcessor::SetParameterValue(DataChangeSource changeSource, AutomationParameterIndex paramIdx, float newValue)
{
	// The reimplemented method AudioProcessor::parameterValueChanged() will trigger a SetParameterChanged() call.
	// We need to ensure that this change is registered to the correct source. 
	// We set the source here, so that it can be used in parameterValueChanged(). 
	m_currentChangeSource = changeSource;

	switch (paramIdx)
	{
	case ParamIdx_X:
		m_xPos->SetParameterValue(newValue);
		break;
	case ParamIdx_Y:
		m_yPos->SetParameterValue(newValue);
		break;
	case ParamIdx_ReverbSendGain:
		m_reverbSendGain->SetParameterValue(newValue);
		break;
	case ParamIdx_SourceSpread:
		m_sourceSpread->SetParameterValue(newValue);
		break;
	case ParamIdx_DelayMode:
		m_delayMode->SetParameterValue(newValue);
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
 * This method should be called once every timer callback tick of the CController. 
 * The signal is passed on to all automation parameters. This is used to trigger gestures for touch automation.
 */
void SoundsourceProcessor::Tick()
{
	// Reset the flags indicating when a parameter's SET command is out on the network. 
	// These flags are set during CController::timerCallback() and queried in CController::oscMessageReceived()
	m_paramSetCommandsInTransit = DCT_None;

	for (int pIdx = 0; pIdx < ParamIdx_MaxIndex; pIdx++)
	{
		switch (pIdx)
		{
		case ParamIdx_X:
			m_xPos->Tick();
			break;
		case ParamIdx_Y:
			m_yPos->Tick();
			break;
		case ParamIdx_ReverbSendGain:
			m_reverbSendGain->Tick();
			break;
		case ParamIdx_SourceSpread:
			m_sourceSpread->Tick();
			break;
		case ParamIdx_DelayMode:
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
void SoundsourceProcessor::SetParamInTransit(DataChangeType paramsChanged)
{
	m_paramSetCommandsInTransit |= paramsChanged;
}

/**
 * Check if the given parameter(s) have a SET command message which has just been sent out on the network.
 * @return True if the specified paranmeter(s) are marked as having a SET command in transit.
 */
bool SoundsourceProcessor::IsParamInTransit(DataChangeType paramsChanged) const
{
	return ((m_paramSetCommandsInTransit & paramsChanged) != DCT_None);
}

/**
 * Function called when the "Overview" button on the GUI is clicked.
 */
void SoundsourceProcessor::OnOverviewButtonClicked()
{
	COverviewManager* ovrMgr = COverviewManager::GetInstance();
	if (ovrMgr)
	{
		ovrMgr->OpenOverview();

		// Set the selected coordinate mapping on the Overview slider to this Plug-in's setting.
		ovrMgr->SetSelectedMapping(GetMappingId());
	}
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to dump this objects' settings
 * to a XML element structure that is returned and written to config file by the
 * singleton AppConfiguration class implementation.
 * @return	The XML element data that was created.
 */
std::unique_ptr<XmlElement> SoundsourceProcessor::createStateXml()
{
	auto processorInstanceXmlElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()));
	if (processorInstanceXmlElement)
	{
		processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORSOURCEID), static_cast<int>(GetSourceId()));
        processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORMAPPINGID), static_cast<int>(GetMappingId()));
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
bool SoundsourceProcessor::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || (stateXml->getTagName() != (AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()))))
		return false;

	SetSourceId(DCS_Init, static_cast<SourceId>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORSOURCEID))));
    SetMappingId(DCS_Init, static_cast<MappingId>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORMAPPINGID))));
    SetComsMode(DCS_Init, static_cast<ComsMode>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE))));

	return true;
}

/**
 * The host will call this method when it wants to save the processor's internal state.
 * This must copy any info about the processor's state into the block of memory provided, 
 * so that the host can store this and later restore it using setStateInformation().
 * @param destData		Stream where the plugin parameters will be written to.
 */
void SoundsourceProcessor::getStateInformation(MemoryBlock& destData)
{
	MemoryOutputStream stream(destData, true);

	String ip = GetIpAddress();
	CVersion version(String(JUCE_STRINGIFY(JUCE_APP_VERSION)));
	jassert(version.IsValid());
	stream.writeInt(version.ToInt());
	stream.writeFloat(*m_xPos);
	stream.writeFloat(*m_yPos);
	stream.writeInt(GetSourceId());
	stream.writeInt(GetMappingId());
	stream.writeString(ip);
	stream.writeInt(GetMessageRate());
	stream.writeInt(static_cast<int>(GetComsMode()));
	stream.writeFloat(m_reverbSendGain->get());
	stream.writeFloat(m_sourceSpread->get());
	stream.writeFloat(static_cast<float>(m_delayMode->getIndex()));
	stream.writeInt(m_processorId);

#ifdef JUCE_DEBUG
	PushDebugMessage("SoundsourceProcessor::getStateInformation");
#endif
}

/**
 * This method is called when project is loaded, or when a snapshot is recalled.
 * Use this method to restore your parameters from this memory block,
 * whose contents will have been created by the getStateInformation() call.
 * @sa SoundsourceProcessor::DisablePollingForTicks()
 * @param data			Stream where the plugin parameters will be read from.
 * @param sizeInBytes	Size of stream buffer.
 */
void SoundsourceProcessor::setStateInformation(const void* data, int sizeInBytes)
{
#ifdef JUCE_DEBUG
	PushDebugMessage("SoundsourceProcessor::setStateInformation");
#endif

	MemoryInputStream stream(data, static_cast<size_t> (sizeInBytes), false);

	// Only binary data from Plugin V2.0 onwards is supported.
	CVersion version(stream.readInt());
	CVersion minVersion(2, 0);
	if (version >= minVersion)
	{
		float xPos = stream.readFloat();
		float yPos = stream.readFloat();
		int sourceId = stream.readInt();
		int mapId =	stream.readInt();
		String ipAddress = stream.readString();
		int msgRate = stream.readInt();
		ComsMode newComMode = static_cast<ComsMode>(stream.readInt());
		float reverb = stream.readFloat();
		float spread = stream.readFloat();
		float delaym = stream.readFloat();
		Rectangle<int> overviewBounds;
		overviewBounds.setX(stream.readInt());
		overviewBounds.setY(stream.readInt());
		overviewBounds.setWidth(stream.readInt());
		overviewBounds.setHeight(stream.readInt());

		// PluginId was added in V2.8.0
		ProcessorId processorId = INVALID_PROCESSOR_ID;
		if (version >= CVersion(2, 8))
		{
			processorId = stream.readInt();
		}

		// Only apply the de-serialized data if the stored PluginID matches our own.
		// When loading projects and when adding new plugin instances, Pro Tools likes to call setStateInformation 
		// with data which does not necessarily belong to the correct instance, and which will overwrite the correct settings.
		if ((processorId == m_processorId) || (processorId == INVALID_PROCESSOR_ID))
		{
			InitializeSettings(sourceId, mapId, ipAddress, msgRate, newComMode);

			SetParameterValue(DCS_Host, ParamIdx_X, xPos);
			SetParameterValue(DCS_Host, ParamIdx_Y, yPos);
			SetParameterValue(DCS_Host, ParamIdx_ReverbSendGain, reverb);
			SetParameterValue(DCS_Host, ParamIdx_SourceSpread, spread);
			SetParameterValue(DCS_Host, ParamIdx_DelayMode, delaym);
		}
	}
}

/**
 * Set the new OSC communication mode (sending and/or receiving).
 * @param changeSource	The application module which is causing the property change.
 * @param newMode	The new OSC communication mode.
 */
void SoundsourceProcessor::SetComsMode(DataChangeSource changeSource, ComsMode newMode)
{
	if (m_comsMode != newMode)
	{
		m_comsMode = newMode;

		// Backup last non-bypass mode.
		if (newMode != CM_Off)
			m_comsModeWhenNotBypassed = newMode;

		// Reset response-ignoring mechanism.
		m_paramSetCommandsInTransit = DCT_None;

		// Signal change to other modules in the plugin.
		SetParameterChanged(changeSource, DCT_ComsMode);

		// Activate the corresponding soundsource id in controller
		CController* ctrl = CController::GetInstance();
		if (ctrl && (changeSource != DCS_Init))
		{
			if (m_comsMode & CM_Rx)
				ctrl->ActivateSoundSourceId(GetSourceId(), GetMappingId());
			else
				ctrl->DeactivateSoundSourceId(GetSourceId(), GetMappingId());
		}
	}
}

/**
 * Restore the Plugin's OSC Rx/Tx mode to whatever it was before going into Bypass.
 * @param changeSource	The application module which is causing the property change.
 */
void SoundsourceProcessor::RestoreComsMode(DataChangeSource changeSource)
{
	if (m_comsModeWhenNotBypassed != CM_Off)
		SetComsMode(changeSource, m_comsModeWhenNotBypassed);
}

/**
 * Get the current OSC communication mode (either sending or receiving).
 * @return The current OSC communication mode.
 */
ComsMode SoundsourceProcessor::GetComsMode() const
{
	return m_comsMode;
}

/**
 * Setter function for the coordinate mapping idx.
 * @param changeSource	The application module which is causing the property change.
 * @param mappingId		The new coordinate mapping ID
 */
void SoundsourceProcessor::SetMappingId(DataChangeSource changeSource, MappingId mappingId)
{
	if (m_mappingId != mappingId)
	{
		DataChangeType dct = DCT_MappingID;

		m_mappingId = mappingId;

		// If the user changes the coodinate mapping and we are in Receive mode, then the position
		// of the X/Y sliders will update automatically to reflect the new mapping in the DS100.
		// However, in Send-only mode we need to manually poll the DS100's position for the new mapping once.
		if ((GetComsMode() & CM_Rx) != CM_Rx)
		{
			dct |= DCT_ComsMode;
			m_comsMode |= CM_PollOnce;
		}

		// Signal change to other modules in the plugin.
		SetParameterChanged(changeSource, dct);
        
        // finally trigger config update
        if (changeSource != DCS_Init)
            triggerConfigurationUpdate(false);
	}
}

/**
 * Getter function for the coordinate mapping Id
 * @return	The current coordinate mapping ID
 */
MappingId SoundsourceProcessor::GetMappingId() const
{
	return m_mappingId;
}

/**
 * Setter function for the source Id
 * @param changeSource	The application module which is causing the property change.
 * @param sourceId	The new ID
 */
void SoundsourceProcessor::SetSourceId(DataChangeSource changeSource, SourceId sourceId)
{
	if (m_sourceId != sourceId)
	{
		// Ensure it's within allowed range.
		m_sourceId = jmin(SOURCE_ID_MAX, jmax(SOURCE_ID_MIN, sourceId));

		// Signal change to other modules in the plugin.
		SetParameterChanged(changeSource, DCT_SourceID);
        
        // finally trigger config update
        if (changeSource != DCS_Init)
            triggerConfigurationUpdate(false);
	}
}

/**
 * Getter function for the source Id
 * @return	The current source ID
 */
SourceId SoundsourceProcessor::GetSourceId() const
{
	return m_sourceId;
}

/**
 * Setter function for the IP address for outgoing OSC comnmunication.
 * @param changeSource	The application module which is causing the property change.
 * @param ipAddress	The new IP address as a string
 */
void SoundsourceProcessor::SetIpAddress(DataChangeSource changeSource, String ipAddress)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
		ctrl->SetIpAddress(changeSource, ipAddress);
}

/**
* Getter function for the IP address
* @return	The current IP address as a string
*/
String SoundsourceProcessor::GetIpAddress() const
{
	String ipAddress;
	CController* ctrl = CController::GetInstance();
	if (ctrl)
		ipAddress = ctrl->GetIpAddress();

	return ipAddress;
}

/**
 * Setter function for the send rate used in the outgoing OSC messages.
 * @param changeSource	The application module which is causing the property change.
 * @param oscMsgRate	The interval at which OSC messages are sent, in ms.
 */
void SoundsourceProcessor::SetMessageRate(DataChangeSource changeSource, int oscMsgRate)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
		ctrl->SetRate(changeSource, oscMsgRate);
}

/**
 * Getter function for the send rate used in the outgoing OSC messages.
 * @return	The interval at which OSC messages are sent, in ms.
 */
int SoundsourceProcessor::GetMessageRate() const
{
	int rate = 0;
	CController* ctrl = CController::GetInstance();
	if (ctrl)
		rate = ctrl->GetRate();

	return rate;
}

/**
 * Getter function for the last OSCSender connection status.
 * @return	True if the last message attempted by the OSCSender was successful, false if it failed.
 */
bool SoundsourceProcessor::GetOnline() const
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
		return ctrl->GetOnline();

	return false;
}

/**
 * Method to initialize config setting, without risking overwriting with the defaults.
 * @param sourceId		New SourceID or matrix input number to use for this plugin instance.
 * @param mappingId		New coordinate mapping to use for this plugin instance.
 * @param ipAddress		New IP address of the DS100 device.
 * @param oscMsgRate	New interval for OSC messages, in milliseconds.
 * @param newMode		New OSC communication mode (Rx/Tx).
 */
void SoundsourceProcessor::InitializeSettings(int sourceId, int mappingId, String ipAddress, int oscMsgRate, ComsMode newMode)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		jassert(sourceId > 128);
		SetSourceId(DCS_Init, static_cast<MappingId>(sourceId));
		jassert(mappingId > 4);
		SetMappingId(DCS_Init, static_cast<MappingId>(mappingId));
		SetComsMode(DCS_Init, newMode);

		// Only overwite the current IP settings if they haven't been changed from the defaults.
		if (GetIpAddress() == ctrl->GetDefaultIpAddress())
		{
			ctrl->InitGlobalSettings(DCS_Init, ipAddress, oscMsgRate);
		}
	}
}

/**
 * Informs the AudioProcessor that track properties such as the track's name or colour has been changed.
 * It's entirely up to the host when and how often this callback will be called.
 * The default implementation of this callback will do nothing.
 * @param properties	A struct containing information about the DAW track inside which your AudioProcessor is loaded.
 */
void SoundsourceProcessor::updateTrackProperties(const TrackProperties& properties)
{
	m_pluginDisplayName = properties.name;

	// Signal change to other modules in the plugin.
	SetParameterChanged(DCS_Host, DCT_SourceID);
}

#ifdef JUCE_DEBUG
/**
 * Helper method to append a message onto the debugging buffer. This buffer can then be flushed with FlushDebugMessages().
 * @param message Message to be printed. A timestamp will automatically be prepended.
 */
void SoundsourceProcessor::PushDebugMessage(String message)
{
	if (message.isNotEmpty())
	{
		String timestamp = Time::getCurrentTime().toString(false, true, true, true);
		message = timestamp + String(": ") + message + String("\n");
		m_debugMessageBuffer += message;

		SetParameterChanged(DCS_Host, DCT_DebugMessage);
	}
}

/**
 * Helper method to get the contents of the debug message buffer. This call also clears the buffer.
 * @ret		Messages to be printed, one per line. 
 */
String SoundsourceProcessor::FlushDebugMessages()
{
	String ret(m_debugMessageBuffer);
	m_debugMessageBuffer.clear();
	return ret;
}
#endif



//==============================================================================
// Overriden functions of class AudioProcessorParameter::Listener


/**
 * REIMPLEMENTED from AudioProcessorParameter::Listener::parameterValueChanged()
 * The host will call this method AFTER one of the filter's parameters has been changed.
 * The host may call this at any time, even when a parameter's value isn't actually being changed, 
 * including during the audio processing callback (avoid blocking!).
 * @param parameterIndex	Index of the plugin parameter being changed.
 * @param newValue			New parameter value, always between 0.0f and 1.0f.
 */
void SoundsourceProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	DataChangeType changed = DCT_None;

	switch (parameterIndex)
	{
		case ParamIdx_X:
			{
				if (m_xPos->get() != m_xPos->GetLastValue())
					changed = DCT_SourcePosition;
			}
			break;
		case ParamIdx_Y:
			{
				if (m_yPos->get() != m_yPos->GetLastValue())
					changed = DCT_SourcePosition;
			}
			break;
		case ParamIdx_ReverbSendGain:
			{
				if (m_reverbSendGain->get() != m_reverbSendGain->GetLastValue())
					changed = DCT_ReverbSendGain;
			}
			break;
		case ParamIdx_SourceSpread:
			{
				if (m_sourceSpread->get() != m_sourceSpread->GetLastValue())
					changed = DCT_SourceSpread;
			}
			break;
		case ParamIdx_DelayMode:
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
 * @param parameterIndex	Index of the plugin parameter being changed.
 * @param gestureIsStarting	True if starting, false if ending.
 */
void SoundsourceProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
	ignoreUnused(parameterIndex);
	ignoreUnused(gestureIsStarting);
}



//==============================================================================
// More overriden functions of class AudioProcessor


/**
 * Returns the name of this processor.
 * @return The plugin name.
 */
const String SoundsourceProcessor::getName() const
{
	return JUCEApplication::getInstance()->getApplicationName();
}

/**
 * Returns true if the processor wants midi messages.
 * @return	True if the processor wants midi messages.
 */
bool SoundsourceProcessor::acceptsMidi() const
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
bool SoundsourceProcessor::producesMidi() const
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

double SoundsourceProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

/**
 * Returns the number of preset programs the filter supports.
 * The value returned must be valid as soon as this object is created, and must not change over its lifetime.
 * @return Number of preset programs the filter supports. This value shouldn't be less than 1.
 */
int SoundsourceProcessor::getNumPrograms()
{
	return 1;
}

/**
 * Returns the number of the currently active program.
 * @return Returns the number of the currently active program.
 */
int SoundsourceProcessor::getCurrentProgram()
{
	return 0;
}

/**
 * Called by the host to change the current program.
 * @param index		New program index.
 */
void SoundsourceProcessor::setCurrentProgram(int index)
{
	ignoreUnused(index);
}

/**
 * Returns the name of a given program.
 * @param index		Index of the desired program
 * @return			Desired program name.
 */
const String SoundsourceProcessor::getProgramName(int index)
{
	ignoreUnused(index);
	return m_pluginDisplayName;
}

/**
 * Called by the host to rename a program.
 * @param index		Index of the desired program
 * @param newName	Desired new program name.
 */
void SoundsourceProcessor::changeProgramName(int index, const String& newName)
{
	ignoreUnused(index);
	m_pluginDisplayName = newName;

	// Signal change to other modules in the plugin.
	SetParameterChanged(DCS_Host, DCT_SourceID);
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
void SoundsourceProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	ignoreUnused(sampleRate, samplesPerBlock);
}

/**
 * Called after playback has stopped, to let the filter free up any resources it no longer needs.
 * When playback stops, you can use this as an opportunity to free up any spare memory, etc.
 */
void SoundsourceProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
/**
 * Callback to query if the AudioProcessor supports a specific layout.
 * This callback is called when the host probes the supported bus layouts via the checkBusesLayoutSupported method. 
 * Used to limit the layouts that the AudioProcessor supports. 
 * @param layouts	Bus channel layouts to check.
 * @returns			True if a given layout is supported by this plugin.
 */
bool SoundsourceProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
	ignoreUnused(layouts);
	return true;
}
#endif


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
void SoundsourceProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	ignoreUnused(buffer, midiMessages);
}

/**
 * This function returns true if the plugin can create an editor component.
 * @return True.
 */
bool SoundsourceProcessor::hasEditor() const
{
	return true;
}

/**
 * Creates the plugin's GUI.
 * This can return nullptr if you want a UI-less filter, in which case the host may create a generic UI that lets the user twiddle the parameters directly.
 * @return	A pointer to the newly created editor component.
 */
AudioProcessorEditor* SoundsourceProcessor::createEditor()
{
	AudioProcessorEditor* editor = new SoundsourceProcessorEditor(*this);

	// Initialize GUI with current IP address, etc.
	SetParameterChanged(DCS_Host, (DCT_PluginInstanceConfig | DCT_OscConfig | DCT_AutomationParameters));

	return editor;
}


} // namespace SoundscapeBridgeApp


/**
 * This global (i.e. outside the dbaudio namespace) function creates new instances of the plugin.
 * @return Creates and returns a new plugin instance.
 */
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new SoundscapeBridgeApp::SoundsourceProcessor();
}
