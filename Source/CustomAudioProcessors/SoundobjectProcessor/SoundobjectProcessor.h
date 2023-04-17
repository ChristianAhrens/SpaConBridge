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
class GestureManagedAudioParameterChoice;


/**
 * Class MainProcessor, a derived AudioProcessor which can be wrapped as VST, AU, or AAX. 
 */
class SoundobjectProcessor : public ProcessorBase
{
public:
	explicit SoundobjectProcessor(bool insertToConfig = true);
	~SoundobjectProcessor() override;

	int GetProcessorId() const;
	void SetProcessorId(DataChangeParticipant changeSource, int processorId);

	void InitializeSettings(SoundobjectId sourceId, MappingId mappingId, String ipAddress, ComsMode newMode);

	static const std::vector<RemoteObjectIdentifier>	GetUsedRemoteObjects();
	static const std::vector<RemoteObjectIdentifier>	GetStaticRemoteObjects();

	SoundobjectId GetSoundobjectId() const;
	void SetSoundobjectId(DataChangeParticipant changeSource, SoundobjectId sourceId);

	MappingId GetMappingId() const;
	void SetMappingId(DataChangeParticipant changeSource, MappingId mappingId);

	const juce::Colour& GetSoundobjectColour() const;
	void SetSoundobjectColour(DataChangeParticipant changeSource, const juce::Colour &colour);

	double GetSoundobjectSize() const;
	void SetSoundobjectSize(DataChangeParticipant changeSource, double size);

	float GetParameterValue(SoundobjectParameterIndex paramIdx, bool normalized = false) const;
	void SetParameterValue(DataChangeParticipant changeSource, SoundobjectParameterIndex paramIdx, float newValue);

	void SetParameterChanged(const DataChangeParticipant& changeSource, const DataChangeType& changeTypes) override;

	void Tick();

	// Overriden functions of class AppConfiguration::XmlConfigurableElement
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	// Overriden functions of class AudioProcessorParameter::Listener
	virtual void parameterValueChanged(int parameterIndex, float newValue) override;
	AudioProcessorEditor* createEditor() override;

private:
	GestureManagedAudioParameterFloat*				m_xPos;									/**< X coordinate in meters. NOTE: not using std::unique_ptr here, see addParameter(). */
	GestureManagedAudioParameterFloat*				m_yPos;									/**< Y coordinate in meters. */
	GestureManagedAudioParameterFloat*				m_reverbSendGain;						/**< Matrix input En-Space gain. */
	GestureManagedAudioParameterFloat*				m_sourceSpread;							/**< Sound object spread. */
	GestureManagedAudioParameterChoice*				m_delayMode;							/**< Sound object delay mode (Off, Tight, Full). */
	MappingId										m_mappingId;							/**< Coordinate mapping index (1 to 4). */
	SoundobjectId									m_soundobjectId;						/**< SoundobjectID, or matrix input number. */
	juce::Colour									m_soundobjectColour;					/**< The colour to be used to paint this soundobject on ui. */
	double											m_soundobjectSize;						/**< The size to be used to paint this soundobject on ui. */
	SoundobjectProcessorId							m_processorId;							/**< Unique ID of this Processor instance. This is also this Processor's index within the Controller::m_processors array. */


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundobjectProcessor)
};


} // namespace SpaConBridge
