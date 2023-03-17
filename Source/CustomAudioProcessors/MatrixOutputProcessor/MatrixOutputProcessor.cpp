/* Copyright (c) 2020-2023, Christian Ahrens
 *
 * This file is part of SpaConBridge <https://github.com/ChristianAhrens/SpaConBridge>
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
#include "../../SpaConBridgeCommon.h"


namespace SpaConBridge
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
	: ProcessorBase()
{
	// Automation parameters.
	// level meter param
	auto lmR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixOutput_LevelMeterPostMute);
	m_matrixOutputLevelMeter = new GestureManagedAudioParameterFloat("MatrixOutput_LevelMeterPostMute", "levelMeter", lmR.getStart(), lmR.getEnd(), 0.1f, lmR.getStart());
	m_matrixOutputLevelMeter->addListener(this);
	addParameter(m_matrixOutputLevelMeter);

	// gain param
	auto gR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixOutput_Gain);
	m_matrixOutputGain = new GestureManagedAudioParameterFloat("MatrixOutput_Gain", "gain", gR.getStart(), gR.getEnd(), 0.1f, 0.0f); // exception: dont use the min range as default - for a gain fader, 0dB is nicer
	m_matrixOutputGain->addListener(this);
	addParameter(m_matrixOutputGain);

	// mute param
	auto mR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixOutput_Mute);
	m_matrixOutputMute = new GestureManagedAudioParameterInt("MatrixOutput_mute", "mute", static_cast<int>(mR.getStart()), static_cast<int>(mR.getEnd()), static_cast<int>(mR.getStart()));
	m_matrixOutputMute->addListener(this);
	addParameter(m_matrixOutputMute);

	m_matrixOutputId = MatrixOutput_ID_MIN; // This default sourceId will be overwritten by ctrl->AddProcessor() below.
	m_processorId = INVALID_PROCESSOR_ID;

	// Register this new processor instance to the singleton Controller object's internal list.
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		m_processorId = ctrl->AddMatrixOutputProcessor(insertToConfig ? DCP_Host : DCP_Init, this);
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
void MatrixOutputProcessor::SetProcessorId(DataChangeParticipant changeSource, MatrixOutputProcessorId processorId)
{
	ignoreUnused(changeSource);
	if (m_processorId != processorId && processorId != INVALID_PROCESSOR_ID)
	{
		m_processorId = processorId;
	}
}

/**
 * Method which will be called every time a parameter or property has been changed.
 * @param changeSource	The application module which is causing the property change.
 * @param changeTypes	Defines which parameter or property has been changed.
 */
void MatrixOutputProcessor::SetParameterChanged(const DataChangeParticipant& changeSource, const DataChangeType& changeTypes)
{
	SetLastSourceForChangeType(changeSource, changeTypes);

	// Set the specified change flag for all DataChangeTargets.
	for (auto changeTarget = 0; changeTarget < DCP_Max; changeTarget++)
	{
		if ((changeSource != changeTarget)
			// specialitiesy: if the source is the processor or multislider, it must also be set as target,
			// since both UIs uses DCP_MatrixOutputProcessor/DCP_MultiSlider for querying as well.
			|| (changeSource == DCP_MatrixOutputProcessor)
			|| (changeSource == DCP_MultiSlider))
			m_dataChangesByTarget[static_cast<DataChangeParticipant>(changeTarget)] |= changeTypes;
	}
}

/**
 * Get the current value of a specific automation parameter.
 * @param paramIdx	The index of the desired parameter.
 * @param normalized If true, the returned value will be normalized to a 0.0f to 1.0f range. False per default.
 * @return	The desired parameter value, as float.
 */
float MatrixOutputProcessor::GetParameterValue(MatrixOutputParameterIndex paramIdx, bool normalized) const
{
	float ret = 0.0f;

	switch (paramIdx)
	{
		case MOI_ParamIdx_LevelMeterPostMute:
			{
				ret = m_matrixOutputLevelMeter->get();
				if (normalized)
					ret = m_matrixOutputLevelMeter->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case MOI_ParamIdx_Gain:
			{
				ret = m_matrixOutputGain->get();
				if (normalized)
					ret = m_matrixOutputGain->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case MOI_ParamIdx_Mute:
			{
				ret = static_cast<float>(m_matrixOutputMute->get());
				if (normalized)
					ret = m_matrixOutputMute->getNormalisableRange().convertTo0to1(ret);
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
void MatrixOutputProcessor::SetParameterValue(DataChangeParticipant changeSource, MatrixOutputParameterIndex paramIdx, float newValue)
{
	// The reimplemented method AudioProcessor::parameterValueChanged() will trigger a SetParameterChanged() call.
	// We need to ensure that this change is registered to the correct source. 
	// We set the source here, so that it can be used in parameterValueChanged(). 
	m_currentChangeSource = changeSource;

	switch (paramIdx)
	{
	case MOI_ParamIdx_LevelMeterPostMute:
		m_matrixOutputLevelMeter->SetParameterValue(newValue);
		break;
	case MOI_ParamIdx_Gain:
		m_matrixOutputGain->SetParameterValue(newValue);
		break;
	case MOI_ParamIdx_Mute:
		m_matrixOutputMute->SetParameterValue(static_cast<int>(newValue));
		break;
	default:
		jassertfalse; // Unknown parameter index!
		break;
	}
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

	for (int pIdx = 0; pIdx < MOI_ParamIdx_MaxIndex; pIdx++)
	{
		switch (pIdx)
		{
		case MOI_ParamIdx_LevelMeterPostMute:
			m_matrixOutputLevelMeter->Tick();
			break;
		case MOI_ParamIdx_Gain:
			m_matrixOutputGain->Tick();
			break;
		case MOI_ParamIdx_Mute:
			m_matrixOutputMute->Tick();
			break;
		default:
			jassert(false); // missing implementation!
			break;
		}
	}
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

	processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCHANNELID), static_cast<int>(GetMatrixOutputId()));
    processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE), static_cast<int>(GetComsMode()));

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
	// sanity check, if the incoming xml does make sense for this method
	if (!stateXml || (stateXml->getTagName() != (AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()))))
		return false;

	// To prevent that we end up in a recursive ::setStateXml situation, verify that this setStateXml method is not called by itself
	const ScopedXmlChangeLock lock(IsXmlChangeLocked());
	if (!lock.isLocked())
		return false;

	SetMatrixOutputId(DCP_Init, static_cast<MatrixOutputId>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCHANNELID))));
    SetComsMode(DCP_Init, static_cast<ComsMode>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE))));

	return true;
}

/**
 * Setter function for the MatrixOutput Id
 * @param changeSource	The application module which is causing the property change.
 * @param matrixOutputId	The new ID
 */
void MatrixOutputProcessor::SetMatrixOutputId(DataChangeParticipant changeSource, MatrixOutputId matrixOutputId)
{

	if (m_matrixOutputId != matrixOutputId)
	{
		// Ensure it's within allowed range.
		m_matrixOutputId = jmin(MatrixOutput_ID_MAX, jmax(MatrixOutput_ID_MIN, matrixOutputId));

		// Signal change to other modules in the processor.
		SetParameterChanged(changeSource, DCT_MatrixOutputID);
        
        // finally trigger config update
        if (changeSource != DCP_Init)
            triggerConfigurationUpdate(false);
	}
}

/**
 * Getter function for the MatrixOutput Id
 * @return	The current MatrixOutput ID
 */
MatrixOutputId MatrixOutputProcessor::GetMatrixOutputId() const
{
	return m_matrixOutputId;
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
		SetMatrixOutputId(DCP_Init, MatrixOutputId);
		SetComsMode(DCP_Init, newMode);
	}
}

/**
 * Method to get a list of remote object identifiers that are used by this soundsource processing object.
 * @return	The requested list of remote object identifiers.
 */
const std::vector<RemoteObjectIdentifier>	MatrixOutputProcessor::GetUsedRemoteObjects()
{
	return std::vector<RemoteObjectIdentifier>{
		ROI_MatrixOutput_LevelMeterPostMute,
		ROI_MatrixOutput_Gain,
		ROI_MatrixOutput_Mute };
};

/**
 * Method to get a list of non-flicering remote object identifiers that are used by this MatrixOutput processing object.
 * @return	The requested list of remote object identifiers.
 */
const std::vector<RemoteObjectIdentifier>	MatrixOutputProcessor::GetStaticRemoteObjects()
{
	return std::vector<RemoteObjectIdentifier>{
		ROI_MatrixOutput_ChannelName };
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
		case MOI_ParamIdx_LevelMeterPostMute:
			{
				if (m_matrixOutputLevelMeter->get() != m_matrixOutputLevelMeter->GetLastValue())
					changed = DCT_MatrixOutputLevelMeter;
			}
			break;
		case MOI_ParamIdx_Gain:
			{
				if (m_matrixOutputGain->get() != m_matrixOutputGain->GetLastValue())
					changed = DCT_MatrixOutputGain;
			}
			break;
		case MOI_ParamIdx_Mute:
			{
				int newValueDenorm = static_cast<int>(m_matrixOutputMute->getNormalisableRange().convertFrom0to1(newValue));
				if (newValueDenorm != m_matrixOutputMute->GetLastValue())
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
 * Creates the procssor's GUI.
 * This can return nullptr if you want a UI-less filter, in which case the host may create a generic UI that lets the user twiddle the parameters directly.
 * @return	A pointer to the newly created editor component.
 */
AudioProcessorEditor* MatrixOutputProcessor::createEditor()
{
	AudioProcessorEditor* editor = new MatrixOutputProcessorEditor(*this);

	// Initialize GUI with current IP address, etc.
	SetParameterChanged(DCP_Protocol, (DCT_MatrixOutputProcessorConfig | DCT_CommunicationConfig | DCT_MatrixOutputParameters)); // We use 'DCP_Protocol' as source here, to not have the initial update be resent as new values via protocol

	return editor;
}


} // namespace SpaConBridge

