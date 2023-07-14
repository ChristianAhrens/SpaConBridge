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

#include "../ProcessorBase.h"

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
class MatrixInputProcessor : public ProcessorBase
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

	float GetParameterValue(MatrixInputParameterIndex paramIdx, bool normalized = false) const;
	void SetParameterValue(DataChangeParticipant changeSource, MatrixInputParameterIndex paramIdx, float newValue);

	void SetParameterChanged(const DataChangeParticipant& changeSource, const DataChangeType& changeTypes) override;

	void Tick();

	// Overriden functions of class AudioProcessor
	void changeProgramName(int index, const String& newName) override;

	// Overriden functions of class AppConfiguration::XmlConfigurableElement
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	// Overriden functions of class AudioProcessorParameter::Listener
	virtual void parameterValueChanged(int parameterIndex, float newValue) override;
	AudioProcessorEditor* createEditor() override;

private:
	GestureManagedAudioParameterFloat*				m_matrixInputLevelMeter;				/**< MatrixInput levelmeter value. NOTE: not using std::unique_ptr here, see addParameter(). */
	GestureManagedAudioParameterFloat*				m_matrixInputGain;						/**< MatrixInput gain value. */
	GestureManagedAudioParameterInt*				m_matrixInputMute;						/**< MatrixInput mute value. */
	MatrixInputId									m_matrixInputId;						/**< matrix input id. */
	MatrixInputProcessorId							m_processorId;							/**< Unique ID of this Processor instance. This is also this Processor's index within the Controller::m_processors array. */
	

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixInputProcessor)
};


} // namespace SpaConBridge
