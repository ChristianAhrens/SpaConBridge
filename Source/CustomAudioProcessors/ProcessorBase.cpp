/* Copyright (c) 2023, Christian Ahrens
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


#include "ProcessorBase.h"

#include "Parameters.h"

#include "../Controller.h"					//<USE Controller
#include "../PagedUI/PageComponentManager.h"	//<USE PageComponentManager
#include "../SpaConBridgeCommon.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class MatrixInputProcessor
===============================================================================
*/

/**
 * Class constructor for the processor.
 */
ProcessorBase::ProcessorBase()
{
	// Start with all parameter changed flags cleared. Function setStateInformation() 
	// will check whether or not we should initialize parameters when starting up.
	for (auto changeTarget = 0; changeTarget < DCP_Max; changeTarget++)
		m_dataChangesByTarget[static_cast<DataChangeParticipant>(changeTarget)] = DCT_None;
}

/**
 * Class destructor for the processor.
 */
ProcessorBase::~ProcessorBase()
{
}

/**
 * Get the state of the desired flag (or flags) for the desired change source.
 * @param changeTarget	The application module querying the change flag.
 * @param changeTypes	The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value
 *			since the last time PopParameterChanged() was called.
 */
bool ProcessorBase::GetParameterChanged(const DataChangeParticipant& changeTarget, const DataChangeType& changeTypes)
{
	return ((m_dataChangesByTarget[changeTarget] & changeTypes) != 0);
}

/**
 * Reset the state of the desired flag (or flags) for the desired change source.
 * Will return the state of the flag before the resetting.
 * @param changeTarget	The application module querying the change flag.
 * @param changeTypes	The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value
 *			since the last time PopParameterChanged() was called.
 */
bool ProcessorBase::PopParameterChanged(const DataChangeParticipant& changeTarget, const DataChangeType& changeTypes)
{
	bool ret((m_dataChangesByTarget[changeTarget] & changeTypes) != 0);
	m_dataChangesByTarget[changeTarget] &= ~changeTypes; // Reset flag.
	return ret;
}

/**
 * Method to mark the last source of a change for every known change type.
 * @param changeSource	The application module which is causing the property change.
 * @param changeTypes	Defines which parameter or property has been changed.
 */
void ProcessorBase::SetLastSourceForChangeType(const DataChangeParticipant& changeSource, const DataChangeType& changeTypes)
{
	if ((changeTypes & DCT_NumProcessors) == DCT_NumProcessors)
		m_dataChangeTypesByLastChangeSource[DCT_NumProcessors] = changeSource;
	if ((changeTypes & DCT_IPAddress) == DCT_IPAddress)
		m_dataChangeTypesByLastChangeSource[DCT_IPAddress] = changeSource;
	if ((changeTypes & DCT_RefreshInterval) == DCT_RefreshInterval)
		m_dataChangeTypesByLastChangeSource[DCT_RefreshInterval] = changeSource;
	if ((changeTypes & DCT_Connected) == DCT_Connected)
		m_dataChangeTypesByLastChangeSource[DCT_Connected] = changeSource;
	if ((changeTypes & DCT_CommunicationConfig) == DCT_CommunicationConfig)
		m_dataChangeTypesByLastChangeSource[DCT_CommunicationConfig] = changeSource;
	if ((changeTypes & DCT_SoundobjectID) == DCT_SoundobjectID)
		m_dataChangeTypesByLastChangeSource[DCT_SoundobjectID] = changeSource;
	if ((changeTypes & DCT_MappingID) == DCT_MappingID)
		m_dataChangeTypesByLastChangeSource[DCT_MappingID] = changeSource;
	if ((changeTypes & DCT_ComsMode) == DCT_ComsMode)
		m_dataChangeTypesByLastChangeSource[DCT_ComsMode] = changeSource;
	if ((changeTypes & DCT_SoundobjectColourAndSize) == DCT_SoundobjectColourAndSize)
		m_dataChangeTypesByLastChangeSource[DCT_SoundobjectColourAndSize] = changeSource;
	if ((changeTypes & DCT_MatrixInputID) == DCT_MatrixInputID)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixInputID] = changeSource;
	if ((changeTypes & DCT_MatrixOutputID) == DCT_MatrixOutputID)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixOutputID] = changeSource;
	if ((changeTypes & DCT_SoundobjectProcessorConfig) == DCT_SoundobjectProcessorConfig)
		m_dataChangeTypesByLastChangeSource[DCT_SoundobjectProcessorConfig] = changeSource;
	if ((changeTypes & DCT_MatrixInputProcessorConfig) == DCT_MatrixInputProcessorConfig)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixInputProcessorConfig] = changeSource;
	if ((changeTypes & DCT_MatrixOutputProcessorConfig) == DCT_MatrixOutputProcessorConfig)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixOutputProcessorConfig] = changeSource;
	if ((changeTypes & DCT_SoundobjectPosition) == DCT_SoundobjectPosition)
		m_dataChangeTypesByLastChangeSource[DCT_SoundobjectPosition] = changeSource;
	if ((changeTypes & DCT_ReverbSendGain) == DCT_ReverbSendGain)
		m_dataChangeTypesByLastChangeSource[DCT_ReverbSendGain] = changeSource;
	if ((changeTypes & DCT_SoundobjectSpread) == DCT_SoundobjectSpread)
		m_dataChangeTypesByLastChangeSource[DCT_SoundobjectSpread] = changeSource;
	if ((changeTypes & DCT_DelayMode) == DCT_DelayMode)
		m_dataChangeTypesByLastChangeSource[DCT_DelayMode] = changeSource;
	if ((changeTypes & DCT_SoundobjectParameters) == DCT_SoundobjectParameters)
		m_dataChangeTypesByLastChangeSource[DCT_SoundobjectParameters] = changeSource;
	if ((changeTypes & DCT_MatrixInputLevelMeter) == DCT_MatrixInputLevelMeter)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixInputLevelMeter] = changeSource;
	if ((changeTypes & DCT_MatrixInputGain) == DCT_MatrixInputGain)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixInputGain] = changeSource;
	if ((changeTypes & DCT_MatrixInputMute) == DCT_MatrixInputMute)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixInputMute] = changeSource;
	if ((changeTypes & DCT_MatrixInputParameters) == DCT_MatrixInputParameters)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixInputParameters] = changeSource;
	if ((changeTypes & DCT_MatrixOutputLevelMeter) == DCT_MatrixOutputLevelMeter)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixOutputLevelMeter] = changeSource;
	if ((changeTypes & DCT_MatrixOutputGain) == DCT_MatrixOutputGain)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixOutputGain] = changeSource;
	if ((changeTypes & DCT_MatrixOutputMute) == DCT_MatrixOutputMute)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixOutputMute] = changeSource;
	if ((changeTypes & DCT_MatrixOutputParameters) == DCT_MatrixOutputParameters)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixOutputParameters] = changeSource;
	if ((changeTypes & DCT_MuteState) == DCT_MuteState)
		m_dataChangeTypesByLastChangeSource[DCT_MuteState] = changeSource;
	if ((changeTypes & DCT_NumBridgingModules) == DCT_NumBridgingModules)
		m_dataChangeTypesByLastChangeSource[DCT_NumBridgingModules] = changeSource;
	if ((changeTypes & DCT_BridgingConfig) == DCT_BridgingConfig)
		m_dataChangeTypesByLastChangeSource[DCT_BridgingConfig] = changeSource;
	if ((changeTypes & DCT_DebugMessage) == DCT_DebugMessage)
		m_dataChangeTypesByLastChangeSource[DCT_DebugMessage] = changeSource;
	if ((changeTypes & DCT_ProcessorSelection) == DCT_ProcessorSelection)
		m_dataChangeTypesByLastChangeSource[DCT_ProcessorSelection] = changeSource;
	if ((changeTypes & DCT_TabPageSelection) == DCT_TabPageSelection)
		m_dataChangeTypesByLastChangeSource[DCT_TabPageSelection] = changeSource;
	if ((changeTypes & DCT_AllConfigParameters) == DCT_AllConfigParameters)
		m_dataChangeTypesByLastChangeSource[DCT_AllConfigParameters] = changeSource;
	if ((changeTypes & DCT_MatrixInputName) == DCT_MatrixInputName)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixInputName] = changeSource;
	if ((changeTypes & DCT_MatrixOutputName) == DCT_MatrixOutputName)
		m_dataChangeTypesByLastChangeSource[DCT_MatrixOutputName] = changeSource;
}

/**
 * Getter for the member defining the origin of the last occured change for a given data type.
 * @param	changeType	The data type for which the last change origin shall be determined.
 * @return	The DCP identification of the last change origin.
 */
const DataChangeParticipant ProcessorBase::GetParameterChangeSource(const DataChangeType& changeType)
{
	auto changeSource = DataChangeParticipant(DCP_Max);

	// unique DCTs
	if (((changeType & DCT_NumProcessors) == DCT_NumProcessors) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_NumProcessors))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_NumProcessors);
	else if (((changeType & DCT_IPAddress) == DCT_IPAddress) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_IPAddress))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_IPAddress);
	else if (((changeType & DCT_RefreshInterval) == DCT_RefreshInterval) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_RefreshInterval))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_RefreshInterval);
	else if (((changeType & DCT_Connected) == DCT_Connected) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_Connected))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_Connected);
	else if (((changeType & DCT_SoundobjectID) == DCT_SoundobjectID) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_SoundobjectID))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_SoundobjectID);
	else if (((changeType & DCT_MappingID) == DCT_MappingID) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MappingID))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MappingID);
	else if (((changeType & DCT_ComsMode) == DCT_ComsMode) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_ComsMode))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_ComsMode);
	else if (((changeType & DCT_SoundobjectColourAndSize) == DCT_SoundobjectColourAndSize) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_SoundobjectColourAndSize))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_SoundobjectColourAndSize);
	else if (((changeType & DCT_MatrixInputID) == DCT_MatrixInputID) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixInputID))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixInputID);
	else if (((changeType & DCT_MatrixOutputID) == DCT_MatrixOutputID) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixOutputID))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixOutputID);
	else if (((changeType & DCT_SoundobjectPosition) == DCT_SoundobjectPosition) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_SoundobjectPosition))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_SoundobjectPosition);
	else if (((changeType & DCT_ReverbSendGain) == DCT_ReverbSendGain) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_ReverbSendGain))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_ReverbSendGain);
	else if (((changeType & DCT_SoundobjectSpread) == DCT_SoundobjectSpread) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_SoundobjectSpread))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_SoundobjectSpread);
	else if (((changeType & DCT_DelayMode) == DCT_DelayMode) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_DelayMode))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_DelayMode);
	else if (((changeType & DCT_MatrixInputLevelMeter) == DCT_MatrixInputLevelMeter) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixInputLevelMeter))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixInputLevelMeter);
	else if (((changeType & DCT_MatrixInputGain) == DCT_MatrixInputGain) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixInputGain))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixInputGain);
	else if (((changeType & DCT_MatrixInputMute) == DCT_MatrixInputMute) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixInputMute))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixInputMute);
	else if (((changeType & DCT_MatrixOutputLevelMeter) == DCT_MatrixOutputLevelMeter) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixOutputLevelMeter))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixOutputLevelMeter);
	else if (((changeType & DCT_MatrixOutputGain) == DCT_MatrixOutputGain) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixOutputGain))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixOutputGain);
	else if (((changeType & DCT_MatrixOutputMute) == DCT_MatrixOutputMute) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixOutputMute))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixOutputMute);
	else if (((changeType & DCT_MuteState) == DCT_MuteState) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MuteState))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MuteState);
	else if (((changeType & DCT_NumBridgingModules) == DCT_NumBridgingModules) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_NumBridgingModules))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_NumBridgingModules);
	else if (((changeType & DCT_DebugMessage) == DCT_DebugMessage) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_DebugMessage))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_DebugMessage);
	else if (((changeType & DCT_ProcessorSelection) == DCT_ProcessorSelection) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_ProcessorSelection))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_ProcessorSelection);
	else if (((changeType & DCT_TabPageSelection) == DCT_TabPageSelection) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_TabPageSelection))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_TabPageSelection);
	else if (((changeType & DCT_MatrixInputName) == DCT_MatrixInputName) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixInputName))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixInputName);
	else if (((changeType & DCT_MatrixOutputName) == DCT_MatrixOutputName) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixOutputName))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixOutputName);
	// accumulated DCTs - Config
	else if (((changeType & DCT_CommunicationConfig) == DCT_CommunicationConfig) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_CommunicationConfig))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_CommunicationConfig);
	else if (((changeType & DCT_SoundobjectProcessorConfig) == DCT_SoundobjectProcessorConfig) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_SoundobjectProcessorConfig))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_SoundobjectProcessorConfig);
	else if (((changeType & DCT_MatrixInputProcessorConfig) == DCT_MatrixInputProcessorConfig) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixInputProcessorConfig))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixInputProcessorConfig);
	else if (((changeType & DCT_MatrixOutputProcessorConfig) == DCT_MatrixOutputProcessorConfig) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixOutputProcessorConfig))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixOutputProcessorConfig);
	else if (((changeType & DCT_BridgingConfig) == DCT_BridgingConfig) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_BridgingConfig))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_BridgingConfig);
	// accumulated DCTs - Parameters
	else if (((changeType & DCT_SoundobjectParameters) == DCT_SoundobjectParameters) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_SoundobjectParameters))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_SoundobjectParameters);
	else if (((changeType & DCT_MatrixInputParameters) == DCT_MatrixInputParameters) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixInputParameters))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixInputParameters);
	else if (((changeType & DCT_MatrixOutputParameters) == DCT_MatrixOutputParameters) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_MatrixOutputParameters))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_MatrixOutputParameters);
	else if (((changeType & DCT_AllConfigParameters) == DCT_AllConfigParameters) && 0 < m_dataChangeTypesByLastChangeSource.count(DCT_AllConfigParameters))
		changeSource = m_dataChangeTypesByLastChangeSource.at(DCT_AllConfigParameters);

	return changeSource;
}

/**
 * The host will call this method when it wants to save the processor's internal state.
 * This must copy any info about the processor's state into the block of memory provided, 
 * so that the host can store this and later restore it using setStateInformation().
 * @param destData		Stream where the processor parameters will be written to.
 */
void ProcessorBase::getStateInformation(MemoryBlock& destData)
{
	ignoreUnused(destData);
}

/**
 * This method is called when project is loaded, or when a snapshot is recalled.
 * Use this method to restore your parameters from this memory block,
 * whose contents will have been created by the getStateInformation() call.
 * @sa MatrixInputProcessor::DisablePollingForTicks()
 * @param data			Stream where the processor parameters will be read from.
 * @param sizeInBytes	Size of stream buffer.
 */
void ProcessorBase::setStateInformation(const void* data, int sizeInBytes)
{
	ignoreUnused(data);
	ignoreUnused(sizeInBytes);
}

/**
 * Set the new OSC communication mode (sending and/or receiving).
 * @param changeSource	The application module which is causing the property change.
 * @param newMode	The new OSC communication mode.
 */
void ProcessorBase::SetComsMode(DataChangeParticipant changeSource, ComsMode newMode)
{
	if (m_comsMode != newMode)
	{
		m_comsMode = newMode;

		// Signal change to other modules in the processor.
		SetParameterChanged(changeSource, DCT_ComsMode);
	}
}

/**
 * Get the current OSC communication mode (either sending or receiving).
 * @return The current OSC communication mode.
 */
ComsMode ProcessorBase::GetComsMode() const
{
	return m_comsMode;
}


//==============================================================================
// Overriden functions of class AudioProcessorParameter::Listener

/**
 * REIMPLEMENTED from AudioProcessorParameter::Listener::parameterGestureChanged()
 * Indicates that a parameter change gesture has started / ended. 
 * This reimplementation does nothing. See GestureManagedAudioParameterFloat::BeginGuiGesture().
 * @param parameterIndex	Index of the procssor parameter being changed.
 * @param gestureIsStarting	True if starting, false if ending.
 */
void ProcessorBase::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
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
const String ProcessorBase::getName() const
{
	return JUCEApplication::getInstance()->getApplicationName();
}

/**
 * Returns true if the processor wants midi messages.
 * @return	True if the processor wants midi messages.
 */
bool ProcessorBase::acceptsMidi() const
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
bool ProcessorBase::producesMidi() const
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

double ProcessorBase::getTailLengthSeconds() const
{
	return 0.0;
}

/**
 * Returns the number of preset programs the filter supports.
 * The value returned must be valid as soon as this object is created, and must not change over its lifetime.
 * @return Number of preset programs the filter supports. This value shouldn't be less than 1.
 */
int ProcessorBase::getNumPrograms()
{
	return 1;
}

/**
 * Returns the number of the currently active program.
 * @return Returns the number of the currently active program.
 */
int ProcessorBase::getCurrentProgram()
{
	return 0;
}

/**
 * Called by the host to change the current program.
 * @param index		New program index.
 */
void ProcessorBase::setCurrentProgram(int index)
{
	ignoreUnused(index);
}

/**
 * Returns the name of a given program.
 * @param index		Index of the desired program
 * @return			Desired program name.
 */
const String ProcessorBase::getProgramName(int index)
{
	ignoreUnused(index);
	return m_processorDisplayName;
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
void ProcessorBase::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	ignoreUnused(sampleRate, samplesPerBlock);
}

/**
 * Called after playback has stopped, to let the filter free up any resources it no longer needs.
 * When playback stops, you can use this as an opportunity to free up any spare memory, etc.
 */
void ProcessorBase::releaseResources()
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
void ProcessorBase::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	ignoreUnused(buffer, midiMessages);
}

/**
 * This function returns true if the procssor can create an editor component.
 * @return True.
 */
bool ProcessorBase::hasEditor() const
{
	return true;
}


} // namespace SpaConBridge

