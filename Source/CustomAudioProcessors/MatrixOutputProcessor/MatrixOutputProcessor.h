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
 * Class MatrixOutputProcessor. 
 */
class MatrixOutputProcessor : public ProcessorBase
{
public:
	explicit MatrixOutputProcessor(bool insertToConfig = true);
	~MatrixOutputProcessor() override;

	int GetProcessorId() const;
	void SetProcessorId(DataChangeParticipant changeSource, int processorId);

	void InitializeSettings(MatrixOutputId channelId, String ipAddress, ComsMode newMode);

	static const std::vector<RemoteObjectIdentifier>	GetUsedRemoteObjects();
	static const std::vector<RemoteObjectIdentifier>	GetStaticRemoteObjects();

	MatrixOutputId GetMatrixOutputId() const;
	void SetMatrixOutputId(DataChangeParticipant changeSource, MatrixOutputId MatrixOutputId);

	float GetParameterValue(MatrixOutputParameterIndex paramIdx, bool normalized = false) const;
	void SetParameterValue(DataChangeParticipant changeSource, MatrixOutputParameterIndex paramIdx, float newValue);

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
	GestureManagedAudioParameterFloat*				m_matrixOutputLevelMeter;				/**< MatrixOutput levelmeter value. NOTE: not using std::unique_ptr here, see addParameter(). */
	GestureManagedAudioParameterFloat*				m_matrixOutputGain;						/**< MatrixOutput gain value. */
	GestureManagedAudioParameterInt*				m_matrixOutputMute;						/**< MatrixOutput mute value. */
	MatrixOutputId									m_matrixOutputId;						/**< matrix output id. */
	MatrixOutputProcessorId							m_processorId;							/**< Unique ID of this Processor instance. This is also this Processor's index within the Controller::m_processors array. */


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixOutputProcessor)
};


} // namespace SpaConBridge
