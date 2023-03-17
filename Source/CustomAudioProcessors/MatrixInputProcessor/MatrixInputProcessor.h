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


#pragma once

#include "../../SpaConBridgeCommon.h"
#include "../../AppConfiguration.h"

#include <RemoteProtocolBridgeCommon.h>


namespace SpaConBridge
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
	explicit MatrixInputProcessor(bool insertToConfig = true);
	~MatrixInputProcessor() override;

	int GetProcessorId() const;
	void SetProcessorId(DataChangeParticipant changeSource, int processorId);

	void InitializeSettings(MatrixInputId matrixInputId, String ipAddress, ComsMode newMode);

	static const std::vector<RemoteObjectIdentifier>	GetUsedRemoteObjects();
	static const std::vector<RemoteObjectIdentifier>	GetStaticRemoteObjects();

	MatrixInputId GetMatrixInputId() const;
	void SetMatrixInputId(DataChangeParticipant changeSource, MatrixInputId matrixInputId);

	ComsMode GetComsMode() const;
	void SetComsMode(DataChangeParticipant changeSource, ComsMode newMode);

	float GetParameterValue(MatrixInputParameterIndex paramIdx, bool normalized = false) const;
	void SetParameterValue(DataChangeParticipant changeSource, MatrixInputParameterIndex paramIdx, float newValue);

	bool GetParameterChanged(const DataChangeParticipant& changeTarget, const DataChangeType& changeTypes);
	bool PopParameterChanged(const DataChangeParticipant& changeTarget, const DataChangeType& changeTypes);
	void SetParameterChanged(const DataChangeParticipant& changeSource, const DataChangeType& changeTypes);

	const DataChangeParticipant GetParameterChangeSource(const DataChangeType& changeType);

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

private:
	void SetLastSourceForChangeType(const DataChangeParticipant& changeSource, const DataChangeType& changeTypes);

	GestureManagedAudioParameterFloat*				m_matrixInputLevelMeter;				/**< MatrixInput levelmeter value. NOTE: not using std::unique_ptr here, see addParameter(). */
	GestureManagedAudioParameterFloat*				m_matrixInputGain;						/**< MatrixInput gain value. */
	GestureManagedAudioParameterInt*				m_matrixInputMute;						/**< MatrixInput mute value. */
	ComsMode										m_comsMode;								/**< Current OSC communication mode, sending and/or receiving. */
	MatrixInputId									m_matrixInputId;						/**< matrix input id. */
	juce::Colour									m_soundobjectColour;					/**< The colour to be used to paint this soundobject on ui. */
	double											m_soundobjectSize;						/**< The size to be used to paint this soundobject on ui. */
	MatrixInputProcessorId							m_processorId;							/**< Unique ID of this Processor instance. This is also this Processor's index within the Controller::m_processors array. */
	std::map<DataChangeParticipant, DataChangeType>	m_dataChangesByTarget;					/**< Keep track of which automation parameters have changed recently. */
	std::map<DataChangeType, DataChangeParticipant>	m_dataChangeTypesByLastChangeSource;	/**< Keep track of who has last changed which automation parameters. */
	DataChangeType									m_paramSetCommandsInTransit = DCT_None;	/**< Flags used to indicate when a SET command for a parameter is currently out on the network.
																							 * Until such a flag is cleared (in the Tick() method), calls to IsParamInTransit will return true.
																							 * This mechanism is used to ensure that parameters aren't overwritten right after having been
																							 * changed via the Gui or the host.
																							 */
	String											m_processorDisplayName;					/**< User friendly name for this processor instance. */
	DataChangeParticipant							m_currentChangeSource = DCP_Host;		/**< Member used to ensure that property changes are registered to the correct source. See MainProcessor::SetParameterValue(). */


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixInputProcessor)
};


} // namespace SpaConBridge
