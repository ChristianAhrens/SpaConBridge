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

#include "SpaConBridgeCommon.h"
#include "AppConfiguration.h"
#include "ProtocolBridgingWrapper.h"


namespace SpaConBridge
{


/**
 * Forward declarations.
 */
class SoundobjectProcessor;
class MatrixInputProcessor;
class MatrixOutputProcessor;


/**
 * Class Controller which takes care of protocol communication through protocolbridging wrapper, including connection establishment
 * and sending/receiving of messages over the network.
 * NOTE: This is a singleton class, i.e. there is only one instance.
 */
class Controller :
	private Timer,
	public AppConfiguration::XmlConfigurableElement,
	public ProtocolBridgingWrapper::Listener
{
public:
	Controller();
	~Controller() override;
	static Controller* GetInstance();
	void DestroyInstance();

	bool GetParameterChanged(DataChangeParticipant changeTarget, DataChangeType change);
	bool PopParameterChanged(DataChangeParticipant changeTarget, DataChangeType change);
	void SetParameterChanged(DataChangeParticipant changeSource, DataChangeType changeTypes);

	juce::int32 GetNextProcessorId();

	//==========================================================================
	void createNewSoundobjectProcessor();
	void createNewSoundobjectProcessors(int newProcessorsCount);
	SoundobjectProcessorId AddSoundobjectProcessor(DataChangeParticipant changeSource, SoundobjectProcessor* p);
	void RemoveSoundobjectProcessor(SoundobjectProcessor* p);
	int GetSoundobjectProcessorCount() const;
	SoundobjectProcessor* GetSoundobjectProcessor(SoundobjectProcessorId processorId) const;
	std::vector<SoundobjectProcessorId> GetSoundobjectProcessorIds() const;

	JUCE_DEPRECATED(void ActivateSoundobjectId(SoundobjectId soundobjectId, MappingId mappingId));
	JUCE_DEPRECATED(void DeactivateSoundobjectId(SoundobjectId soundobjectId, MappingId mappingId));
	void UpdateActiveSoundobjects();

	void SetSelectedSoundobjectProcessorIds(const std::vector<SoundobjectProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<SoundobjectProcessorId> GetSelectedSoundobjectProcessorIds();
	void SetSoundobjectIdSelectState(SoundobjectId soundobjectId, bool selected);
	bool IsSoundobjectIdSelected(SoundobjectId soundobjectId);

	//==========================================================================
	void createNewMatrixInputProcessor();
	void createNewMatrixInputProcessors(int newProcessorsCount);
	MatrixInputProcessorId AddMatrixInputProcessor(DataChangeParticipant changeSource, MatrixInputProcessor* p);
	void RemoveMatrixInputProcessor(MatrixInputProcessor* p);
	int GetMatrixInputProcessorCount() const;
	MatrixInputProcessor* GetMatrixInputProcessor(MatrixInputProcessorId processorId) const;
	std::vector<MatrixInputProcessorId> GetMatrixInputProcessorIds() const;

	JUCE_DEPRECATED(void ActivateMatrixInputId(MatrixInputId matrixInputId));
	JUCE_DEPRECATED(void DeactivateMatrixInputId(MatrixInputId matrixInputId));
	void UpdateActiveMatrixInputs();

	void SetSelectedMatrixInputProcessorIds(const std::vector<MatrixInputProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<MatrixInputProcessorId> GetSelectedMatrixInputProcessorIds();
	void SetMatrixInputIdSelectState(MatrixInputId matrixInputId, bool selected);
	bool IsMatrixInputIdSelected(MatrixInputId matrixInputId);

	//==========================================================================
	void createNewMatrixOutputProcessor();
	void createNewMatrixOutputProcessors(int newProcessorsCount);
	MatrixOutputProcessorId AddMatrixOutputProcessor(DataChangeParticipant changeSource, MatrixOutputProcessor* p);
	void RemoveMatrixOutputProcessor(MatrixOutputProcessor* p);
	int GetMatrixOutputProcessorCount() const;
	MatrixOutputProcessor* GetMatrixOutputProcessor(MatrixOutputProcessorId processorId) const;
	std::vector<MatrixOutputProcessorId> GetMatrixOutputProcessorIds() const;

	JUCE_DEPRECATED(void ActivateMatrixOutputId(MatrixOutputId matrixOutputId));
	JUCE_DEPRECATED(void DeactivateMatrixOutputId(MatrixOutputId matrixOutputId));
	void UpdateActiveMatrixOutputs();

	void SetSelectedMatrixOutputProcessorIds(const std::vector<MatrixOutputProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<MatrixOutputProcessorId> GetSelectedMatrixOutputProcessorIds();
	void SetMatrixOutputIdSelectState(MatrixOutputId matrixOutputId, bool selected);
	bool IsMatrixOutputIdSelected(MatrixOutputId matrixOutputId);

	//==========================================================================
	static String GetDefaultDS100IpAddress();
	String GetDS100IpAddress() const;
	void SetDS100IpAddress(DataChangeParticipant changeSource, String ipAddress, bool dontSendNotification = false);
	String GetSecondDS100IpAddress() const;
	void SetSecondDS100IpAddress(DataChangeParticipant changeSource, String ipAddress, bool dontSendNotification = false);

	//==========================================================================
	int GetRefreshInterval() const;
	void SetRefreshInterval(DataChangeParticipant changeSource, int refreshInterval, bool dontSendNotification = false);
	static std::pair<int, int> GetSupportedRefreshIntervalRange();

	//==========================================================================
	ExtensionMode GetExtensionMode() const;
	void SetExtensionMode(DataChangeParticipant changeSource, ExtensionMode mode, bool dontSendNotification = false);

	//==========================================================================
	ActiveParallelModeDS100 GetActiveParallelModeDS100() const;
	void SetActiveParallelModeDS100(DataChangeParticipant changeSource, ActiveParallelModeDS100 activeParallelModeDS100, bool dontSendNotification = false);

	//==========================================================================
	const std::vector<RemoteObject> GetActivatedSoundObjectRemoteObjects();
	const std::vector<RemoteObject> GetActivatedMatrixInputRemoteObjects();
	const std::vector<RemoteObject> GetActivatedMatrixOutputRemoteObjects();

	//==========================================================================
	ProtocolBridgingType GetActiveProtocolBridging();
	void SetActiveProtocolBridging(ProtocolBridgingType bridgingType);
	int GetActiveProtocolBridgingCount();
	
	bool GetMuteBridgingSoundobjectId(ProtocolBridgingType bridgingType, SoundobjectId soundobjectId);
	bool SetMuteBridgingSoundobjectId(ProtocolBridgingType bridgingType, SoundobjectId soundobjectId, bool mute);
	bool SetMuteBridgingSoundobjectIds(ProtocolBridgingType bridgingType, const std::vector<SoundobjectId>& soundobjectIds, bool mute);

	bool GetMuteBridgingMatrixInputId(ProtocolBridgingType bridgingType, MatrixInputId matrixInputId);
	bool SetMuteBridgingMatrixInputId(ProtocolBridgingType bridgingType, MatrixInputId matrixInputId, bool mute);
	bool SetMuteBridgingMatrixInputIds(ProtocolBridgingType bridgingType, const std::vector<MatrixInputId>& matrixInputIds, bool mute);

	bool GetMuteBridgingMatrixOutputId(ProtocolBridgingType bridgingType, MatrixOutputId matrixOutputId);
	bool SetMuteBridgingMatrixOutputId(ProtocolBridgingType bridgingType, MatrixOutputId matrixOutputId, bool mute);
	bool SetMuteBridgingMatrixOutputIds(ProtocolBridgingType bridgingType, const std::vector<MatrixOutputId>& matrixOutputIds, bool mute);

	String GetBridgingIpAddress(ProtocolBridgingType bridgingType);
	bool SetBridgingIpAddress(ProtocolBridgingType bridgingType, String ipAddress, bool dontSendNotification = false);
	int GetBridgingListeningPort(ProtocolBridgingType bridgingType);
	bool SetBridgingListeningPort(ProtocolBridgingType bridgingType, int listeningPort, bool dontSendNotification = false);
	int GetBridgingRemotePort(ProtocolBridgingType bridgingType);
	bool SetBridgingRemotePort(ProtocolBridgingType bridgingType, int remotePort, bool dontSendNotification = false);
	int GetBridgingMappingArea(ProtocolBridgingType bridgingType);
	bool SetBridgingMappingArea(ProtocolBridgingType bridgingType, int mappingAreaId, bool dontSendNotification = false);
	String GetBridgingInputDeviceIdentifier(ProtocolBridgingType bridgingType);
	bool SetBridgingInputDeviceIdentifier(ProtocolBridgingType bridgingType, const String& inputDeviceIdentifier, bool dontSendNotification = false);
	String GetBridgingOutputDeviceIdentifier(ProtocolBridgingType bridgingType);
	bool SetBridgingOutputDeviceIdentifier(ProtocolBridgingType bridgingType, const String& outputDeviceIdentifier, bool dontSendNotification = false);
	JUCEAppBasics::MidiCommandRangeAssignment GetBridgingMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId);
	bool SetBridgingMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId, const JUCEAppBasics::MidiCommandRangeAssignment& assignmentMapping, bool dontSendNotification = false);

	//==========================================================================
	void InitGlobalSettings(DataChangeParticipant changeSource, String ipAddress, int rate);

	//==========================================================================
	void Disconnect();
	void Reconnect();
	bool IsConnected() const;
	bool IsFirstDS100Connected() const;
	bool IsFirstDS100Master() const;
	bool IsSecondDS100Connected() const;
	bool IsSecondDS100Master() const;

	//==========================================================================
	void SetOnline(DataChangeParticipant changeSource, bool online);
	bool IsOnline() const;

	//==========================================================================
	bool LoadConfigurationFile(const File& fileToLoadFrom);
	bool SaveConfigurationFile(const File& fileToSaveTo);

	//==========================================================================
	void AddProtocolBridgingWrapperListener(ProtocolBridgingWrapper::Listener* listener);

	//==========================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==========================================================================
	void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData) override;

	//==========================================================================
	bool SendMessageDataDirect(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData);

private:
	void timerCallback() override;

protected:
	
	static Controller				*m_singleton;					/**< The one and only instance of CController. */

	int								m_refreshInterval;				/**< Interval at which the controller internal update is triggered, in ms. */
	bool							m_onlineState{ false };			/**< State of the protocol bridging/communication. This is only the expected state, not the actual connected state. */

	Array<SoundobjectProcessor*>	m_soundobjectProcessors;		/**< List of registered processor instances. */
	Array<MatrixInputProcessor*>	m_matrixInputProcessors;		/**< List of registered processor instances. */
	Array<MatrixOutputProcessor*>	m_matrixOutputProcessors;		/**< List of registered processor instances. */

	ProtocolBridgingWrapper			m_protocolBridge;				/**< The wrapper for protocol bridging node, allowing to easily interface with it. */

	String							m_DS100IpAddress;				/**< IP Address where OSC messages will be sent to / received from. */
	ExtensionMode					m_DS100ExtensionMode;			/**< Current extension mode. This has impact on if second DS100 is active or not. */
	ActiveParallelModeDS100			m_DS100ActiveParallelModeDS100;	/**< Currently active DS100 when in extension mode "parallel". */
	String							m_SecondDS100IpAddress;			/**< IP Address where OSC messages will be sent to / received from. */

	DataChangeType					m_parametersChanged[DCP_Max];	/**< Keep track of which OSC parameters have changed recently.
																	 * The array has one entry for each application module (see enum DataChangeSource). */

	CriticalSection					m_mutex;						/**< A re-entrant mutex. */

	std::map<SoundobjectId, bool>	m_soundObjectSelection;			/**< The current select state of sound objects. */
	std::map<MatrixInputId, bool>	m_matrixInputSelection;			/**< The current select state of matrix inputs. */
	std::map<MatrixOutputId, bool>	m_matrixOutputSelection;		/**< The current select state of matrix outputs. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller)
};


} // namespace SpaConBridge
