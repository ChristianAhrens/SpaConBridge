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
class GestureManagedAudioParameterInt;


/**
 * Class MatrixInputProcessor. 
 */
class MatrixInputProcessor :
	public AudioProcessor,
	public AudioProcessorParameter::Listener,
	public AppConfiguration::XmlConfigurableElement
{
public:
	MatrixInputProcessor(bool insertToConfig = true);
	~MatrixInputProcessor() override;

	int GetProcessorId() const;
	void SetProcessorId(DataChangeSource changeSource, int processorId);

	void InitializeSettings(MatrixInputId channelId, String ipAddress, ComsMode newMode);

	static const std::vector<RemoteObjectIdentifier>	GetUsedRemoteObjects();

	MatrixInputId GetMatrixInputId() const;
	void SetMatrixInputId(DataChangeSource changeSource, MatrixInputId MatrixInputId);

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
	 * Matrix input En-Space gain.
	 */
	GestureManagedAudioParameterFloat*	m_MatrixInputLevelMeter;

	/**
	 * Sound object spread.
	 */
	GestureManagedAudioParameterFloat*	m_MatrixInputGain;

	/**
	 * Sound object delay mode (Off, Tight, Full).
	 */
	GestureManagedAudioParameterInt*	m_MatrixInputMute;

	/**
	 * Current OSC communication mode, sending and/or receiving.
	 */
	ComsMode					m_comsMode;

	/*
	 * MatrixInputProcessor, or matrix input number.
	 */
	MatrixInputProcessorId	m_MatrixInputId;

	/**
	 * Unique ID of this Processor instance. 
	 * This is also this Processor's index within the Controller::m_processors array.
	 */
	MatrixInputProcessorId	m_processorId;

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


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixInputProcessor)
};


} // namespace SoundscapeBridgeApp