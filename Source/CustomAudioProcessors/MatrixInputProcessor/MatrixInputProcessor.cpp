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


#include "MatrixInputProcessor.h"

#include "MatrixInputProcessorEditor.h"			//<USE MatrixInputProcessorEditor

#include "../Parameters.h"

#include "../../Controller.h"					//<USE Controller
#include "../../PagedUI/PageComponentManager.h"	//<USE PageComponentManager
#include "../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


static constexpr MatrixInputId MatrixInput_ID_MIN = 1;		//< Minimum maxtrix input number
static constexpr MatrixInputId MatrixInput_ID_MAX = 128;		//< Highest maxtrix input number

/*
===============================================================================
 Class MatrixInputProcessor
===============================================================================
*/

/**
 * Class constructor for the processor.
 */
MatrixInputProcessor::MatrixInputProcessor(bool insertToConfig)
	: ProcessorBase()
{
	// Automation parameters.
	// level meter param
	auto lmR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_LevelMeterPreMute);
	m_matrixInputLevelMeter = new GestureManagedAudioParameterFloat("MatrixInput_LevelMeterPreMute", "levelMeter", lmR.getStart(), lmR.getEnd(), 0.1f, lmR.getStart());
	m_matrixInputLevelMeter->addListener(this);
	addParameter(m_matrixInputLevelMeter);

	// gain param
	auto gR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_Gain);
	m_matrixInputGain = new GestureManagedAudioParameterFloat("MatrixInput_Gain", "gain", gR.getStart(), gR.getEnd(), 0.1f, 0.0f); // exception: dont use the min range as default - for a gain fader, 0dB is nicer
	m_matrixInputGain->addListener(this);
	addParameter(m_matrixInputGain);

	// mute param
	auto mR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_Mute);
	m_matrixInputMute = new GestureManagedAudioParameterInt("MatrixInput_mute", "mute", static_cast<int>(mR.getStart()), static_cast<int>(mR.getEnd()), static_cast<int>(mR.getStart()));
	m_matrixInputMute->addListener(this);
	addParameter(m_matrixInputMute);

	m_matrixInputId = MatrixInput_ID_MIN; // This default sourceId will be overwritten by ctrl->AddProcessor() below.
	m_processorId = INVALID_PROCESSOR_ID;

	// Default communication mode.
	SetComsMode(DCP_Init, (CM_Rx | CM_Tx));

	// Register this new processor instance to the singleton Controller object's internal list.
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		m_processorId = ctrl->AddMatrixInputProcessor(insertToConfig ? DCP_Host : DCP_Init, this);
}

/**
 * Class destructor for the processor.
 */
MatrixInputProcessor::~MatrixInputProcessor()
{
	// Erase this new processor instance from the singleton Controller object's internal list.
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->RemoveMatrixInputProcessor(this);
}

/**
 * Get the id of this processor instance 
 */
int MatrixInputProcessor::GetProcessorId() const
{
	return m_processorId;
}

/**
 * Setter function for the processors' Id
 * @param changeSource	The application module which is causing the property change.
 * @param processorId	The new ID
 */
void MatrixInputProcessor::SetProcessorId(DataChangeParticipant changeSource, MatrixInputProcessorId processorId)
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
void MatrixInputProcessor::SetParameterChanged(const DataChangeParticipant& changeSource, const DataChangeType& changeTypes)
{
	SetLastSourceForChangeType(changeSource, changeTypes);

	// Set the specified change flag for all DataChangeTargets.
	for (auto changeTarget = 0; changeTarget < DCP_Max; changeTarget++)
	{
		if ((changeSource != changeTarget)
			// specialitiesy: if the source is the processor or multislider, it must also be set as target,
			// since both UIs uses DCP_MatrixInputProcessor/DCP_MultiSlider for querying as well.
			|| (changeSource == DCP_MatrixInputProcessor)
			|| (changeSource == DCP_MultiSlider))
			m_dataChangesByTarget[static_cast<DataChangeParticipant>(changeTarget)] |= changeTypes;
	}

	if (auto ctrl = Controller::GetInstance())
		ctrl->EnqueueTickTrigger();
}

/**
 * Get the current value of a specific automation parameter.
 * @param paramIdx	The index of the desired parameter.
 * @param normalized If true, the returned value will be normalized to a 0.0f to 1.0f range. False per default.
 * @return	The desired parameter value, as float.
 */
float MatrixInputProcessor::GetParameterValue(MatrixInputParameterIndex paramIdx, bool normalized) const
{
	float ret = 0.0f;

	switch (paramIdx)
	{
		case MII_ParamIdx_LevelMeterPreMute:
			{
				ret = m_matrixInputLevelMeter->get();
				if (normalized)
					ret = m_matrixInputLevelMeter->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case MII_ParamIdx_Gain:
			{
				ret = m_matrixInputGain->get();
				if (normalized)
					ret = m_matrixInputGain->getNormalisableRange().convertTo0to1(ret);
			}
			break;
		case MII_ParamIdx_Mute:
			{
				ret = static_cast<float>(m_matrixInputMute->get());
				if (normalized)
					ret = m_matrixInputMute->getNormalisableRange().convertTo0to1(ret);
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
void MatrixInputProcessor::SetParameterValue(DataChangeParticipant changeSource, MatrixInputParameterIndex paramIdx, float newValue)
{
	// The reimplemented method AudioProcessor::parameterValueChanged() will trigger a SetParameterChanged() call.
	// We need to ensure that this change is registered to the correct source. 
	// We set the source here, so that it can be used in parameterValueChanged(). 
	m_currentChangeSource = changeSource;

	switch (paramIdx)
	{
	case MII_ParamIdx_LevelMeterPreMute:
		m_matrixInputLevelMeter->SetParameterValue(newValue);
		break;
	case MII_ParamIdx_Gain:
		m_matrixInputGain->SetParameterValue(newValue);
		break;
	case MII_ParamIdx_Mute:
		m_matrixInputMute->SetParameterValue(static_cast<int>(newValue));
		break;
	default:
		jassertfalse; // Unknown parameter index!
		break;
	}

	if (auto miEditor = dynamic_cast<MatrixInputProcessorEditor*>(getActiveEditor()))
		miEditor->EnqueueTickTrigger();
}

/**
 * This method should be called once every timer callback tick of the Controller. 
 * The signal is passed on to all automation parameters. This is used to trigger gestures for touch automation.
 */
void MatrixInputProcessor::Tick()
{
	for (int pIdx = 0; pIdx < MII_ParamIdx_MaxIndex; pIdx++)
	{
		switch (pIdx)
		{
		case MII_ParamIdx_LevelMeterPreMute:
			m_matrixInputLevelMeter->Tick();
			break;
		case MII_ParamIdx_Gain:
			m_matrixInputGain->Tick();
			break;
		case MII_ParamIdx_Mute:
			m_matrixInputMute->Tick();
			break;
		default:
			jassert(false); // missing implementation!
			break;
		}
	}
}

/**
 * Called by the host to rename a program.
 * @param index		Index of the desired program
 * @param newName	Desired new program name.
 */
void MatrixInputProcessor::changeProgramName(int index, const String& newName)
{
	if (index != getCurrentProgram())
		return;
	if (newName == m_processorDisplayName)
		return;

	m_processorDisplayName = newName;

	// Signal change to other modules in the procssor.
	SetParameterChanged(DCP_Host, DCT_MatrixInputID);
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to dump this objects' settings
 * to a XML element structure that is returned and written to config file by the
 * singleton AppConfiguration class implementation.
 * @return	The XML element data that was created.
 */
std::unique_ptr<XmlElement> MatrixInputProcessor::createStateXml()
{
	auto processorInstanceXmlElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()));

	processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCHANNELID), static_cast<int>(GetMatrixInputId()));
    processorInstanceXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE), static_cast<int>(GetComsMode()));

    return processorInstanceXmlElement;
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to set this objects' settings
 * from a XML element structure that passed as argument.
 * @param stateXml	The XML element containing this objects' configuration data
 * @return	True if the data was read and handled successfuly, false if not.
 */
bool MatrixInputProcessor::setStateXml(XmlElement* stateXml)
{
	// sanity check, if the incoming xml does make sense for this method
	if (!stateXml || (stateXml->getTagName() != (AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE) + String(GetProcessorId()))))
		return false;

	// To prevent that we end up in a recursive ::setStateXml situation, verify that this setStateXml method is not called by itself
	const ScopedXmlChangeLock lock(IsXmlChangeLocked());
	if (!lock.isLocked())
		return false;

	SetMatrixInputId(DCP_Init, static_cast<MatrixInputId>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCHANNELID))));
    SetComsMode(DCP_Init, static_cast<ComsMode>(stateXml->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::PROCESSORCOMSMODE))));

	return true;
}

/**
 * Setter function for the MatrixInput Id
 * @param changeSource	The application module which is causing the property change.
 * @param matrixInputId	The new ID
 */
void MatrixInputProcessor::SetMatrixInputId(DataChangeParticipant changeSource, MatrixInputId matrixInputId)
{
	if (m_matrixInputId != matrixInputId)
	{
		// Ensure it's within allowed range.
		m_matrixInputId = juce::jlimit(MatrixInput_ID_MIN, MatrixInput_ID_MAX, matrixInputId);

		// Signal change to other modules in the processor.
		SetParameterChanged(changeSource, DCT_MatrixInputID);
        
        // finally trigger config update
        if (changeSource != DCP_Init)
            triggerConfigurationUpdate(false);
	}
}

/**
 * Getter function for the MatrixInput Id
 * @return	The current MatrixInput ID
 */
MatrixInputId MatrixInputProcessor::GetMatrixInputId() const
{
	return m_matrixInputId;
}

/**
 * Method to initialize config setting, without risking overwriting with the defaults.
 * @param matrixInputId		New SourceID or matrix input number to use for this processor instance.
 * @param mappingId		New coordinate mapping to use for this procssor instance.
 * @param ipAddress		New IP address of the DS100 device.
 * @param newMode		New OSC communication mode (Rx/Tx).
 */
void MatrixInputProcessor::InitializeSettings(MatrixInputId matrixInputId, String ipAddress, ComsMode newMode)
{
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
	{
		jassert(matrixInputId > 128);
		SetMatrixInputId(DCP_Init, matrixInputId);
		SetComsMode(DCP_Init, newMode);
	}
}

/**
 * Method to get a list of remote object identifiers that are used by this MatrixInput processing object.
 * @return	The requested list of remote object identifiers.
 */
const std::vector<RemoteObjectIdentifier>	MatrixInputProcessor::GetUsedRemoteObjects()
{
	return std::vector<RemoteObjectIdentifier>{
		ROI_MatrixInput_LevelMeterPreMute, 
		ROI_MatrixInput_Gain, 
		ROI_MatrixInput_Mute };
};

/**
 * Method to get a list of non-flicering remote object identifiers that are used by this MatrixInput processing object.
 * @return	The requested list of remote object identifiers.
 */
const std::vector<RemoteObjectIdentifier>	MatrixInputProcessor::GetStaticRemoteObjects()
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
void MatrixInputProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	DataChangeType changed = DCT_None;

	switch (parameterIndex)
	{
		case MII_ParamIdx_LevelMeterPreMute:
			{
				if (m_matrixInputLevelMeter->get() != m_matrixInputLevelMeter->GetLastValue())
					changed = DCT_MatrixInputLevelMeter;
			}
			break;
		case MII_ParamIdx_Gain:
			{
				if (m_matrixInputGain->get() != m_matrixInputGain->GetLastValue())
					changed = DCT_MatrixInputGain;
			}
			break;
		case MII_ParamIdx_Mute:
			{
				int newValueDenorm = static_cast<int>(m_matrixInputMute->getNormalisableRange().convertFrom0to1(newValue));
				if (newValueDenorm != m_matrixInputMute->GetLastValue())
					changed = DCT_MatrixInputMute;
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
AudioProcessorEditor* MatrixInputProcessor::createEditor()
{
	auto editor = new MatrixInputProcessorEditor(*this);

	// Initialize GUI with current IP address, etc.
	SetParameterChanged(DCP_Protocol, DCT_MatrixInputParameters); // We use 'DCP_Protocol' as source here, to not have the initial update be resent as new values via protocol

	return editor;
}


} // namespace SpaConBridge

