/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in
and now in a derived version is part of SoundscapeBridgeApp.

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


#pragma once

#include "../../SoundscapeBridgeAppCommon.h"
#include "../../AppConfiguration.h"

#include <RemoteProtocolBridgeCommon.h>


namespace SoundscapeBridgeApp
{


/**
 * Forward declarations.
 */
class GestureManagedAudioParameterFloat;
class GestureManagedAudioParameterChoice;


/**
 * Class MainProcessor, a derived AudioProcessor which can be wrapped as VST, AU, or AAX. 
 */
class SoundobjectProcessor :
	public AudioProcessor,
	public AudioProcessorParameter::Listener,
	public AppConfiguration::XmlConfigurableElement
{
public:
	SoundobjectProcessor(bool insertToConfig = true);
	~SoundobjectProcessor() override;

	int GetProcessorId() const;
	void SetProcessorId(DataChangeSource changeSource, int processorId);

	void InitializeSettings(SoundobjectId sourceId, MappingId mappingId, String ipAddress, ComsMode newMode);

	static const std::vector<RemoteObjectIdentifier>	GetUsedRemoteObjects();

	SoundobjectId GetSoundobjectId() const;
	void SetSoundobjectId(DataChangeSource changeSource, SoundobjectId sourceId);

	MappingId GetMappingId() const;
	void SetMappingId(DataChangeSource changeSource, MappingId mappingId);

	int GetMessageRate() const;
	void SetMessageRate(DataChangeSource changeSource, int oscMsgRate);

	ComsMode GetComsMode() const;
	void SetComsMode(DataChangeSource changeSource, ComsMode newMode);

	float GetParameterValue(SoundobjectParameterIndex paramIdx, bool normalized = false) const;
	void SetParameterValue(DataChangeSource changeSource, SoundobjectParameterIndex paramIdx, float newValue);

	bool GetParameterChanged(DataChangeSource changeSource, DataChangeType change);
	bool PopParameterChanged(DataChangeSource changeSource, DataChangeType change);
	void SetParameterChanged(DataChangeSource changeSource, DataChangeType changeTypes);

	void Tick();
	void SetParamInTransit(DataChangeType paramsChanged);
	bool IsParamInTransit(DataChangeType paramsChanged) const;

	// Overriden functions of class AppConfiguration::XmlConfigurableElement
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	// Overriden functions of class AudioProcessor
	virtual void getStateInformation(MemoryBlock& destData) override;
	virtual void setStateInformation(const void* data, int sizeInBytes) override;
	// Overriden functions of class AudioProcessorParameter::Listener
	virtual void parameterValueChanged(int parameterIndex, float newValue) override;
	virtual void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

	// Functions which need to be reimplemented from class AudioProcessor, but which 
	// aren't relevant for our use.
	bool acceptsMidi() const override;
	void changeProgramName(int index, const String& newName) override;
	AudioProcessorEditor* createEditor() override;
	int getCurrentProgram() override;
	int getNumPrograms() override;
	const String getProgramName(int index) override;
	const String getName() const override;
	double getTailLengthSeconds() const override;
	bool hasEditor() const override;
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void processBlock(AudioSampleBuffer&, MidiBuffer&) override;
	bool producesMidi() const override;
	void releaseResources() override;
	void setCurrentProgram(int index) override;

protected:
	/**
	 * X coordinate in meters.
	 * NOTE: not using std::unique_ptr here, see addParameter().
	 */
	GestureManagedAudioParameterFloat*		m_xPos;

	/**
	 * Y coordinate in meters.
	 */
	GestureManagedAudioParameterFloat*		m_yPos;

	/**
	 * Matrix input En-Space gain.
	 */
	GestureManagedAudioParameterFloat*		m_reverbSendGain;

	/**
	 * Sound object spread.
	 */
	GestureManagedAudioParameterFloat*		m_sourceSpread;

	/**
	 * Sound object delay mode (Off, Tight, Full).
	 */
	GestureManagedAudioParameterChoice*		m_delayMode;

	/**
	 * Current OSC communication mode, sending and/or receiving.
	 */
	ComsMode					m_comsMode;

	/*
	 * Coordinate mapping index (1 to 4).
	 */
	MappingId					m_mappingId;

	/*
	 * SoundobjectID, or matrix input number.
	 */
	SoundobjectId				m_soundobjectId;

	/**
	 * Unique ID of this Processor instance. 
	 * This is also this Processor's index within the Controller::m_processors array.
	 */
	SoundobjectProcessorId		m_processorId;

	/**
	 * Keep track of which automation parameters have changed recently. 
	 * The array has one entry for each application module (see enum DataChangeSource).
	 */
	DataChangeType				m_parametersChanged[DCS_Max];

	/**
	 * Flags used to indicate when a SET command for a parameter is currently out on the network.
	 * Until such a flag is cleared (in the Tick() method), calls to IsParamInTransit will return true.
	 * This mechanism is used to ensure that parameters aren't overwritten right after having been
	 * changed via the Gui or the host.
	 */
	DataChangeType				m_paramSetCommandsInTransit = DCT_None;

	/**
	 * User friendly name for this processor instance
	 */
	String						m_processorDisplayName;

	/**
	 * Member used to ensure that property changes are registered to the correct source.
	 * See MainProcessor::SetParameterValue().
	 */
	DataChangeSource			m_currentChangeSource = DCS_Host;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundobjectProcessor)
};


} // namespace SoundscapeBridgeApp