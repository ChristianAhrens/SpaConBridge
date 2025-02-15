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
 * @class Controller
 * @brief Class Controller which takes care of protocol communication through protocolbridging wrapper, including connection establishment
 * and sending/receiving of messages over the network.
 * 
 * @note This is a singleton class, i.e. there is only one instance.
 */
class Controller :
	public juce::MessageListener,
	public AppConfiguration::XmlConfigurableElement,
	public ProtocolBridgingWrapper::Listener
{
public:
	class StandaloneActiveObjectsListener
	{
	public:
		StandaloneActiveObjectsListener() {};
		virtual ~StandaloneActiveObjectsListener() {};

		/**
		 * Method to be overloaded to interface
		 * with incoming message data, prefiltered to originate from 
		 * devices of interest.
		 */
		virtual void HandleObjectDataInternal(const RemoteObjectIdentifier& roi, const RemoteObjectMessageData& msgData) = 0;
	};

public:
	Controller();
	~Controller() override;
	static Controller* GetInstance();
	void DestroyInstance();
	static bool Exists() { return bool(s_singleton) && s_constructionFinished; };

	//==========================================================================
	bool GetParameterChanged(DataChangeParticipant changeTarget, DataChangeType change);
	bool PopParameterChanged(DataChangeParticipant changeTarget, DataChangeType change);
	void SetParameterChanged(DataChangeParticipant changeSource, DataChangeType changeTypes);

	juce::int32 GetNextProcessorId();

	//==========================================================================
	std::vector<RemoteObject> GetAllStandaloneActiveRemoteObjectsToUse();
	bool IsStaticProcessorRemoteObjectsPollingEnabled();
	void SetStaticProcessorRemoteObjectsPollingEnabled(DataChangeParticipant changeSource, bool enabled);

	const std::vector<RemoteObject> GetStandaloneActiveRemoteObjects();
	const std::vector<RemoteObject>& GetStandaloneActiveRemoteObjects(Controller::StandaloneActiveObjectsListener* listener);
	bool ContainsStandaloneActiveRemoteObject(Controller::StandaloneActiveObjectsListener* listener, const RemoteObject& remoteObject);
	bool AddStandaloneActiveRemoteObject(Controller::StandaloneActiveObjectsListener* listener, const RemoteObject& remoteObject);
	bool RemoveStandaloneActiveRemoteObject(Controller::StandaloneActiveObjectsListener* listener, const RemoteObject& remoteObject);
	void TriggerConfirmStandaloneActiveObjects(Controller::StandaloneActiveObjectsListener* listener);

	bool AddStandaloneActiveObjectsListener(Controller::StandaloneActiveObjectsListener* listener);
	bool RemoveStandaloneActiveObjectsListener(Controller::StandaloneActiveObjectsListener* listener);

	//==========================================================================
	void UpdateActiveRemoteObjects(bool dontSendNotification = false);
	
	//==========================================================================
	void createNewSoundobjectProcessor();
	SoundobjectProcessorId AddSoundobjectProcessor(DataChangeParticipant changeSource, SoundobjectProcessor* p);
	void RemoveSoundobjectProcessor(SoundobjectProcessor* p);
	void RemoveSoundobjectProcessorIds(const std::vector<SoundobjectProcessorId>& sopIds);
	int GetSoundobjectProcessorCount() const;
	SoundobjectProcessor* GetSoundobjectProcessor(SoundobjectProcessorId processorId) const;
	std::vector<SoundobjectProcessorId> GetSoundobjectProcessorIds() const;
	std::vector<RemoteObject> GetSoundobjectProcessorRemoteObjects(SoundobjectProcessorId soundobjectProcessorId);

	//==========================================================================
	void createNewMatrixInputProcessor();
	MatrixInputProcessorId AddMatrixInputProcessor(DataChangeParticipant changeSource, MatrixInputProcessor* p);
	void RemoveMatrixInputProcessor(MatrixInputProcessor* p);
	void RemoveMatrixInputProcessorIds(const std::vector<MatrixInputProcessorId>& mipIds);
	int GetMatrixInputProcessorCount() const;
	MatrixInputProcessor* GetMatrixInputProcessor(MatrixInputProcessorId processorId) const;
	std::vector<MatrixInputProcessorId> GetMatrixInputProcessorIds() const;
	std::vector<RemoteObject> GetMatrixInputProcessorRemoteObjects(MatrixInputProcessorId matrixInputProcessorId);

	//==========================================================================
	void createNewMatrixOutputProcessor();
	MatrixOutputProcessorId AddMatrixOutputProcessor(DataChangeParticipant changeSource, MatrixOutputProcessor* p);
	void RemoveMatrixOutputProcessor(MatrixOutputProcessor* p);
	void RemoveMatrixOutputProcessorIds(const std::vector<MatrixOutputProcessorId>& mopIds);
	int GetMatrixOutputProcessorCount() const;
	MatrixOutputProcessor* GetMatrixOutputProcessor(MatrixOutputProcessorId processorId) const;
	std::vector<MatrixOutputProcessorId> GetMatrixOutputProcessorIds() const;
	std::vector<RemoteObject> GetMatrixOutputProcessorRemoteObjects(MatrixOutputProcessorId matrixOutputProcessorId);

	//==========================================================================
	ProtocolType GetDS100ProtocolType() const;
	void SetDS100ProtocolType(DataChangeParticipant changeSource, ProtocolType protocol, bool dontSendNotification = false);
	bool IsPollingDS100ProtocolType();

	//==========================================================================
	std::pair<juce::IPAddress, int> GetDS100IpAndPort() const;
	void SetDS100IpAndPort(DataChangeParticipant changeSource, juce::IPAddress ipAddress, int port, bool dontSendNotification = false);
	std::pair<juce::IPAddress, int> GetSecondDS100IpAndPort() const;
	void SetSecondDS100IpAndPort(DataChangeParticipant changeSource, juce::IPAddress ipAddress, int port, bool dontSendNotification = false);

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
	juce::String GetDS100DummyProjectData() const;
	void SetDS100DummyProjectData(DataChangeParticipant changeSource, const juce::String& projectDummyData, bool dontSendNotification = false);

	//==========================================================================
	int GetDS100DummyAnimationMode() const;
	void SetDS100DummyAnimationMode(DataChangeParticipant changeSource, const int& animationMode, bool dontSendNotification = false);

	//==========================================================================
	const std::vector<RemoteObject> GetActivatedSoundObjectRemoteObjects();
	const std::vector<RemoteObject> GetActivatedMatrixInputRemoteObjects();
	const std::vector<RemoteObject> GetActivatedMatrixOutputRemoteObjects();

	const std::vector<RemoteObject> GetMutedSoundObjectRemoteObjects(const ProtocolBridgingType& bridgingType);
	const std::vector<RemoteObject> GetMutedMatrixInputRemoteObjects(const ProtocolBridgingType& bridgingType);
	const std::vector<RemoteObject> GetMutedMatrixOutputRemoteObjects(const ProtocolBridgingType& bridgingType);

	//==========================================================================
	ProtocolBridgingType GetActiveProtocolBridging();
	void SetActiveProtocolBridging(ProtocolBridgingType bridgingType);
	int GetActiveProtocolBridgingCount();
	
	bool GetMuteBridgingSoundobjectProcessorId(ProtocolBridgingType bridgingType, SoundobjectProcessorId soundobjectProcessorId);
	bool SetMuteBridgingSoundobjectProcessorId(ProtocolBridgingType bridgingType, SoundobjectProcessorId soundobjectProcessorId, bool mute);
	bool SetMuteBridgingSoundobjectProcessorIds(ProtocolBridgingType bridgingType, const std::vector<SoundobjectProcessorId>& soundobjectProcessorIds, bool mute);

	bool GetMuteBridgingMatrixInputProcessorId(ProtocolBridgingType bridgingType, MatrixInputProcessorId matrixInputProcessorId);
	bool SetMuteBridgingMatrixInputProcessorId(ProtocolBridgingType bridgingType, MatrixInputProcessorId matrixInputProcessorId, bool mute);
	bool SetMuteBridgingMatrixInputProcessorIds(ProtocolBridgingType bridgingType, const std::vector<MatrixInputProcessorId>& matrixInputProcessorIds, bool mute);

	bool GetMuteBridgingMatrixOutputProcessorId(ProtocolBridgingType bridgingType, MatrixOutputProcessorId matrixOutputProcessorId);
	bool SetMuteBridgingMatrixOutputProcessorId(ProtocolBridgingType bridgingType, MatrixOutputProcessorId matrixOutputProcessorId, bool mute);
	bool SetMuteBridgingMatrixOutputProcessorIds(ProtocolBridgingType bridgingType, const std::vector<MatrixOutputProcessorId>& matrixOutputProcessorIds, bool mute);

	juce::IPAddress GetBridgingIpAddress(ProtocolBridgingType bridgingType);
	bool SetBridgingIpAddress(ProtocolBridgingType bridgingType, juce::IPAddress ipAddress, bool dontSendNotification = false);

	int GetBridgingListeningPort(ProtocolBridgingType bridgingType);
	bool SetBridgingListeningPort(ProtocolBridgingType bridgingType, int listeningPort, bool dontSendNotification = false);

	int GetBridgingRemotePort(ProtocolBridgingType bridgingType);
	bool SetBridgingRemotePort(ProtocolBridgingType bridgingType, int remotePort, bool dontSendNotification = false);

	int GetBridgingMappingArea(ProtocolBridgingType bridgingType);
	bool SetBridgingMappingArea(ProtocolBridgingType bridgingType, int mappingAreaId, bool dontSendNotification = false);

	const juce::Point<float> GetBridgingOriginOffset(ProtocolBridgingType bridgingType);
	bool SetBridgingOriginOffset(ProtocolBridgingType bridgingType, const juce::Point<float>& origin, bool dontSendNotification = false);

	const std::pair<juce::Range<float>, juce::Range<float>> GetBridgingMappingRange(ProtocolBridgingType bridgingType);
	bool SetBridgingMappingRange(ProtocolBridgingType bridgingType, const std::pair<juce::Range<float>, juce::Range<float>>& mappingXRange, bool dontSendNotification = false);

	String GetBridgingInputDeviceIdentifier(ProtocolBridgingType bridgingType);
	bool SetBridgingInputDeviceIdentifier(ProtocolBridgingType bridgingType, const String& inputDeviceIdentifier, bool dontSendNotification = false);

	String GetBridgingOutputDeviceIdentifier(ProtocolBridgingType bridgingType);
	bool SetBridgingOutputDeviceIdentifier(ProtocolBridgingType bridgingType, const String& outputDeviceIdentifier, bool dontSendNotification = false);

	JUCEAppBasics::MidiCommandRangeAssignment GetBridgingMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId);
	bool SetBridgingMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId, const JUCEAppBasics::MidiCommandRangeAssignment& assignmentMapping, bool dontSendNotification = false);

	std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> GetBridgingScenesToMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId);
	bool SetBridgingScenesToMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& scenesToMidiAssignmentMapping, bool dontSendNotification = false);

	int GetBridgingXAxisInverted(ProtocolBridgingType bridgingType);
	bool SetBridgingXAxisInverted(ProtocolBridgingType bridgingType, int inverted, bool dontSendNotification = false);

	int GetBridgingYAxisInverted(ProtocolBridgingType bridgingType);
	bool SetBridgingYAxisInverted(ProtocolBridgingType bridgingType, int inverted, bool dontSendNotification = false);

	int GetBridgingXYAxisSwapped(ProtocolBridgingType bridgingType);
	bool SetBridgingXYAxisSwapped(ProtocolBridgingType bridgingType, int swapped, bool dontSendNotification = false);

	bool GetBridgingXYMessageCombined(ProtocolBridgingType bridgingType);
	bool SetBridgingXYMessageCombined(ProtocolBridgingType bridgingType, bool combined, bool dontSendNotification = false);

	int GetBridgingDataSendingDisabled(ProtocolBridgingType bridgingType);
	bool SetBridgingDataSendingDisabled(ProtocolBridgingType bridgingType, int disabled, bool dontSendNotification = false);

	std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>> GetBridgingOscRemapAssignments(ProtocolBridgingType bridgingType);
	bool SetBridgingOscRemapAssignments(ProtocolBridgingType bridgingType, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& oscRemapAssignments, bool dontSendNotification = false);

	std::map<int, ChannelId> GetBridgingChannelRemapAssignments(ProtocolBridgingType bridgingType);
	bool SetBridgingChannelRemapAssignments(ProtocolBridgingType bridgingType, const std::map<int, ChannelId>& oscRemapAssignments, bool dontSendNotification = false);

	const String GetBridgingModuleTypeIdentifier(ProtocolBridgingType bridgingType);
	bool SetBridgingModuleTypeIdentifier(ProtocolBridgingType bridgingType, const String& moduleTypeIdentifier, bool dontSendNotification = false);

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
	void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, const RemoteObjectIdentifier roi, const RemoteObjectMessageData& msgData) override;

	//==========================================================================
	void handleMessage(const Message& message) override;
	bool SendMessageDataDirect(const RemoteObjectIdentifier roi, RemoteObjectMessageData& msgData);

	void EnqueueTickTrigger();
	void StopTickProcessing();
	void ResumeTickProcessing();
	bool IsTickProcessingStopped();
	void PostParameterChanged(DataChangeParticipant changeSource, DataChangeType changeTypes);
private:
	/**
	 * Class StandaloneActiveObjectsPollingHelper
	 * @brief	Controller specific helper class
	 *			that takes over the task of timerbased
	 *			getting of values defined as standalone active in controller.
	 */
	class StandaloneActiveObjectsPollingHelper : private Timer
	{
	public:
		StandaloneActiveObjectsPollingHelper()
		{
			PollOnce();
		}
		explicit StandaloneActiveObjectsPollingHelper(int interval) : StandaloneActiveObjectsPollingHelper()
		{
			SetInterval(interval);
		}
		~StandaloneActiveObjectsPollingHelper() override
		{
		}

		int GetInterval()
		{
			return m_interval;
		}
		void SetInterval(int interval)
		{
			m_interval = interval;

			if (m_running)
				startTimer(interval);
		}

		bool IsRunning()
		{
			return m_running && (getTimerInterval() != 0);
		}
		void SetRunning(bool running)
		{
			// do not restart time (would mess up currently elapsing interval) and no need to set the m_running member to identical value
			if (m_running != running)
			{
				if (running)
					startTimer(GetInterval());
				else
					stopTimer();

				m_running = running;
			}
		}

		void TriggerPollOnce(Controller::StandaloneActiveObjectsListener* listener)
		{
			if (nullptr == listener)
				return PollOnce();
			else if (Controller::GetInstance()->IsOnline())
			{
				auto remoteObjectsToPoll = Controller::GetInstance()->GetStandaloneActiveRemoteObjects(listener);
				for (auto const& remoteObject : remoteObjectsToPoll)
				{
					auto romd = RemoteObjectMessageData(remoteObject._Addr, ROVT_NONE, 0, nullptr, 0);
					Controller::GetInstance()->SendMessageDataDirect(remoteObject._Id, romd);
				}
			}
		}

	private:
		void timerCallback() override
		{
			PollOnce();
		}
		void PollOnce()
		{
			if (Controller::GetInstance()->IsOnline())
			{
				auto remoteObjectsToPoll = Controller::GetInstance()->GetAllStandaloneActiveRemoteObjectsToUse();
				for (auto const& remoteObject : remoteObjectsToPoll)
				{
					auto romd = RemoteObjectMessageData(remoteObject._Addr, ROVT_NONE, 0, nullptr, 0);
					Controller::GetInstance()->SendMessageDataDirect(remoteObject._Id, romd);
				}
			}
		}

		int m_interval{ 0 };
		bool m_running{ false };
	};

	/**
	 * Private message class to act as asynchronous
	 * 'tick'/update trigger via message queue.
	 * To prevent irrelevant processing of multiple queued triggers,
	 * an internal flag is used that signals if a trigger message
	 * is still relevant when dispatched from queue or no longer is relevant
	 * due to an earlier trigger processing already handled things.
	 */
	class TickTrigger : public juce::Message
	{
	public:
		TickTrigger() { s_tickHandled = false; };
		~TickTrigger() {};

		static const bool IsOutdated() { return s_tickHandled; };
		void SetTickHandled() const { s_tickHandled = true; };

	private:
		static bool s_tickHandled;
	};

	/**
	 * Class ParameterChangedMessage
	 * @brief	Private message class to act as asyynchronous
	 *			trigger for SetParameterChanged.
	 */
	class ParameterChangedMessage : public juce::Message
	{
	public:
		ParameterChangedMessage(DataChangeParticipant dcp, DataChangeType dct) { m_dcp = dcp; m_dct = dct; };
		~ParameterChangedMessage() {};

		DataChangeParticipant GetChangeSource() const { return m_dcp; };
		DataChangeType GetChangeTypes() const { return m_dct; };

	private:
		DataChangeParticipant m_dcp;
		DataChangeType m_dct;
	};

private:
	//==========================================================================
	const ProtocolId GetProtocolIdForProtocolType(const ProtocolBridgingType type);
	
	//==========================================================================
	void CleanupMutedObjects();

	/**
	 * @brief Private method to process all parameter related
	 * changes once and if neccessary handle parameter
	 * value changes or parameter cound accordingly.
	 */
	void tick();
	void SetTickWasPostponedWhenPaused();

	//==========================================================================
	static std::unique_ptr<Controller>	s_singleton;				/**< The one and only instance of CController. */
	static bool							s_constructionFinished;		/**< Bool indicator if construction of the singleton is finished (to ensure no recursion stack overflow happens. */

	int								m_refreshInterval;				/**< Interval at which the controller internal update is triggered, in ms. */
	bool							m_onlineState{ false };			/**< State of the protocol bridging/communication. This is only the expected state, not the actual connected state. */

	Array<SoundobjectProcessor*>	m_soundobjectProcessors;		/**< List of registered processor instances. */
	Array<MatrixInputProcessor*>	m_matrixInputProcessors;		/**< List of registered processor instances. */
	Array<MatrixOutputProcessor*>	m_matrixOutputProcessors;		/**< List of registered processor instances. */

	ProtocolBridgingWrapper			m_protocolBridge;				/**< The wrapper for protocol bridging node, allowing to easily interface with it. */

	ProtocolType					m_DS100ProtocolType;			/**< Current protocol type. This has impact on other parameters being available on ui or not. */
	juce::IPAddress					m_DS100IpAddress;				/**< IP Address where OSC messages will be sent to / received from. */
	int								m_DS100Port;					/**< Port on the ds100 device to connect to. */
	ExtensionMode					m_DS100ExtensionMode;			/**< Current extension mode. This has impact on if second DS100 is active or not. */
	ActiveParallelModeDS100			m_DS100ActiveParallelModeDS100;	/**< Currently active DS100 when in extension mode "parallel". */
	juce::IPAddress					m_SecondDS100IpAddress;			/**< IP Address where OSC messages will be sent to / received from. */
	int								m_SecondDS100Port;				/**< Port on a second ds100 device to connect to. */
	juce::String					m_DS100DummyProjectData;		/**< Dummy dbpr project data excerpt as string. */
	int								m_DS100DummyAnimationMode;		/**< Dummy animation mode integer value (0, 1, 2). */

	DataChangeType					m_parametersChanged[DCP_Max];	/**< Keep track of which OSC parameters have changed recently.
																	 * The array has one entry for each application module (see enum DataChangeSource). */

	CriticalSection					m_mutex;						/**< A re-entrant mutex. */

	bool							m_staticProcessorRemoteObjectsPollingEnabled{ false };	/**< Member to define if static processor related objects should be regarded when polling/subscribing (usually object name strings). */

	std::unique_ptr<StandaloneActiveObjectsPollingHelper>								m_pollingHelper;					/**< Polling helper instance for OSC DS100 communation. */
	std::vector<Controller::StandaloneActiveObjectsListener*>							m_standaloneActiveObjectListeners;	/**< The listner objects, for message data handling callback. */
	std::map<Controller::StandaloneActiveObjectsListener*, std::vector<RemoteObject>>	m_standaloneActiveRemoteObjects;	/**< List of remote objects that the controller manages as lowfreq apart from regular hifreq object value subscription/polling. */

	bool m_tickProcessingRunning{ true };		/**< Boolean flag to indicate if the TickTrigger async handling shall be active. */
	bool m_tickWasPostponedWhenPaused{ false };	/**< Boolean flag to indicate if while TickTrigger handling was paused a trigger was dropped and has to be compensated manually. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller)
};


} // namespace SpaConBridge
