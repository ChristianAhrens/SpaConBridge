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


#include "Controller.h"
#include "WaitingEntertainerComponent.h"
#include "ProcessorSelectionManager.h"

#include "PagedUI/PageComponentManager.h"
#include "PagedUI/PageContainerComponent.h"
#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessor.h"


namespace SpaConBridge
{


static constexpr int PROTOCOL_INTERVAL_MIN			= 20;		//< Minimum supported OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_MAX			= 5000;		//< Maximum supported OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_DEF			= 100;		//< Default OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_STATIC_OBJS	= 2000;		//< Object polling rate for non-flicering static objects in milliseconds


/*
===============================================================================
 Class Controller
===============================================================================
*/

/**
 * The one and only instance of Controller.
 */
std::unique_ptr<Controller> Controller::s_singleton;
bool Controller::s_constructionFinished{ false };
bool Controller::TickTrigger::s_tickHandled{ true };

/**
 * Constructs an Controller object.
 * There can be only one instance of this class, see m_singleton. This is so that network traffic
 * is managed from a central point and only one UDP port is opened for all OSC communication.
 */
Controller::Controller()
{
	jassert(!s_singleton);	// only one instnce allowed!!
	s_singleton = std::unique_ptr<Controller>(this);

	// Clear all changed flags initially
	for (int cs = 0; cs < DCP_Max; cs++)
		m_parametersChanged[cs] = DCT_None;

	// Controller derives from ProcessingEngineNode::Listener
	AddProtocolBridgingWrapperListener(this);

	// Some default value initialization just to be sure
	SetRefreshInterval(DCP_Init, PROTOCOL_INTERVAL_DEF, true);
	SetDS100IpAndPort(DCP_Init, juce::IPAddress(PROTOCOL_DEFAULT_IP), RX_PORT_DS100_DEVICE, true);
	SetSecondDS100IpAndPort(DCP_Init, juce::IPAddress(PROTOCOL_DEFAULT2_IP), RX_PORT_DS100_DEVICE, true);
	SetExtensionMode(DCP_Init, EM_Off, true);
	SetActiveParallelModeDS100(DCP_Init, APM_None, true);

	m_pollingHelper = std::make_unique<Controller::StandaloneActiveObjectsPollingHelper>(PROTOCOL_INTERVAL_STATIC_OBJS);

	s_constructionFinished = true;
}

/**
 * Destroys the Controller.
 */
Controller::~Controller()
{
	s_constructionFinished = false;

	Disconnect();

	const ScopedLock lock(m_mutex);
	m_soundobjectProcessors.clearQuick();
	m_matrixInputProcessors.clearQuick();
	m_matrixOutputProcessors.clearQuick();

	DestroyInstance();
}

/**
 * Returns the one and only instance of Controller. If it doesn't exist yet, it is created.
 * @return The Controller singleton object.
 * @sa m_singleton, Controller
 */
Controller* Controller::GetInstance()
{
	if (!s_singleton)
	{
		new Controller();
		jassert(s_singleton);
	}
	return s_singleton.get();
}

/**
 * Triggers destruction of Controller singleton object
 */
void Controller::DestroyInstance()
{
	s_singleton.reset();
}

/**
 * Method which will be called every time a parameter or property has been changed.
 * @param changeSource	The application module which is causing the property change.
 * @param changeTypes	Defines which parameter or property has been changed.
 */
void Controller::SetParameterChanged(DataChangeParticipant changeSource, DataChangeType changeTypes)
{
	const ScopedLock lock(m_mutex);

	// Set the specified change flag for all DataChangeParticipants, except for the one causing the change.
	for (int cs = 0; cs < DCP_Max; cs++)
	{
		m_parametersChanged[cs] |= changeTypes;
	}

	// Forward the change to all processor instances. This is needed, for example, so that all processor's
	// GUIs update an IP address change.
	for (auto const& processor : m_soundobjectProcessors)
	{
		processor->SetParameterChanged(changeSource, changeTypes);
	}
	for (auto const& processor : m_matrixInputProcessors)
	{
		processor->SetParameterChanged(changeSource, changeTypes);
	}
	for (auto const& processor : m_matrixOutputProcessors)
	{
		processor->SetParameterChanged(changeSource, changeTypes);
	}

	switch (changeTypes)
	{
	case DCT_NumProcessors:
	case DCT_None:
	case DCT_IPAddress:
	case DCT_RefreshInterval:
	case DCT_CommunicationConfig:
	case DCT_SoundobjectID:
	case DCT_MatrixInputID:
	case DCT_MatrixOutputID:
	case DCT_MappingID:
	case DCT_ComsMode:
	case DCT_SoundobjectProcessorConfig:
	case DCT_MatrixInputProcessorConfig:
	case DCT_MatrixOutputProcessorConfig:
	case DCT_NumBridgingModules:
	case DCT_OnlineState:
		if (changeSource != DCP_Init)
			triggerConfigurationUpdate(true);
		break;
	case DCT_BridgingConfig:
	case DCT_MuteState:
		if (changeSource != DCP_Init)
			triggerConfigurationUpdate(false);
		break;
	case DCT_Connected:
	case DCT_SoundobjectPosition:
	case DCT_ReverbSendGain:
	case DCT_SoundobjectSpread:
	case DCT_DelayMode:
	case DCT_SoundobjectParameters:
	case DCT_MatrixInputParameters:
	case DCT_MatrixOutputParameters:
	case DCT_DebugMessage:
	default:
		break;
	}

	EnqueueTickTrigger();
}

/**
 * Get the state of the desired flag (or flags) for the desired change source.
 * @param changeTarget	The application module querying the change flag.
 * @param change		The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value 
 *			since the last time PopParameterChanged() was called.
 */
bool Controller::GetParameterChanged(DataChangeParticipant changeTarget, DataChangeType change)
{
	const ScopedLock lock(m_mutex);
	return ((m_parametersChanged[changeTarget] & change) != 0);
}

/**
 * Reset the state of the desired flag (or flags) for the desired change source.
 * Will return the state of the flag before the resetting.
 * @param changeTarget	The application module querying the change flag.
 * @param change		The desired parameter (or parameters).
 * @return	The state of the flag before the resetting.
 */
bool Controller::PopParameterChanged(DataChangeParticipant changeTarget, DataChangeType change)
{
	const ScopedLock lock(m_mutex);
	bool ret((m_parametersChanged[changeTarget] & change) != 0);
	m_parametersChanged[changeTarget] &= ~change; // Reset flag.
	return ret;
}

/**
 * Helper method to get the next free processor id.
 * The existing processor ids of all types (soundobject, matrixinput, matrixoutput)
 * are taken into account.
 */
juce::int32 Controller::GetNextProcessorId()
{
	juce::int32					newProcessorId = 0;
	std::vector<juce::int32>	processorIds;

	processorIds.reserve(GetSoundobjectProcessorCount() + GetMatrixInputProcessorCount() + GetMatrixOutputProcessorCount());
	for (auto const& sop : m_soundobjectProcessors)
		processorIds.push_back(sop->GetProcessorId());
	for (auto const& mip : m_matrixInputProcessors)
		processorIds.push_back(mip->GetProcessorId());
	for (auto const& mop : m_matrixOutputProcessors)
		processorIds.push_back(mop->GetProcessorId());

	std::sort(processorIds.begin(), processorIds.end());
	for (auto const& processorId : processorIds)
	{
		if (processorId > newProcessorId) // we have found a gap in the list that we can use
			break;
		else
			newProcessorId++;
	}

	return newProcessorId;
}

/**
 * Helper to collect all 'static' remote objects used in the controller dependant parts of the application.
 * 'Static' in this scope has the meaning of a remote object value that is required just once and is
 * updated rarely, like channel or device names.
 * @return	The listing of remote objects.
 */
std::vector<RemoteObject> Controller::GetAllStandaloneActiveRemoteObjectsToUse()
{
	// initially add all RemoteObjects registered as required for standalone app components 
	std::vector<RemoteObject> remoteObjects = GetStandaloneActiveRemoteObjects();

	if (IsStaticProcessorRemoteObjectsPollingEnabled())
	{
		// add all RemoteObjects that are specifically required for soundobject related UI
		auto soProcIds = GetSoundobjectProcessorIds();
		for (auto const& processorId : soProcIds)
		{
			auto processor = GetSoundobjectProcessor(processorId);
			for (auto& roi : SoundobjectProcessor::GetStaticRemoteObjects())
			{
				if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
					jassertfalse;
				else
				{
					auto sosro = RemoteObject(roi, RemoteObjectAddressing(processor->GetSoundobjectId(), INVALID_ADDRESS_VALUE));
					if (std::find(remoteObjects.begin(), remoteObjects.end(), sosro) == remoteObjects.end())
						remoteObjects.push_back(sosro);
				}
			}
		}

		// add all RemoteObjects that are specifically required for matrixinput related UI
		auto miProcIds = GetMatrixInputProcessorIds();
		for (auto const& processorId : miProcIds)
		{
			auto processor = GetMatrixInputProcessor(processorId);
			for (auto& roi : MatrixInputProcessor::GetStaticRemoteObjects())
			{
				if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
					jassertfalse;
				else
				{
					auto misro = RemoteObject(roi, RemoteObjectAddressing(processor->GetMatrixInputId(), INVALID_ADDRESS_VALUE));
					if (std::find(remoteObjects.begin(), remoteObjects.end(), misro) == remoteObjects.end())
						remoteObjects.push_back(misro);
				}
			}
		}

		// add all RemoteObjects that are specifically required for matrixoutput related UI
		auto moProcIds = GetMatrixOutputProcessorIds();
		for (auto const& processorId : moProcIds)
		{
			auto processor = GetMatrixOutputProcessor(processorId);
			for (auto& roi : MatrixOutputProcessor::GetStaticRemoteObjects())
			{
				if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
					jassertfalse;
				else
				{
					auto mosro = RemoteObject(roi, RemoteObjectAddressing(processor->GetMatrixOutputId(), INVALID_ADDRESS_VALUE));
					if (std::find(remoteObjects.begin(), remoteObjects.end(), mosro) == remoteObjects.end())
						remoteObjects.push_back(mosro);
				}
			}
		}
	}

	return remoteObjects;
}

/**
 * Helper to get the running state of internal pollinghelper member for
 * non-flickering objects polling.
 * @return	The running state of pollinghelper or false if object not existing.
 */
bool Controller::IsStaticProcessorRemoteObjectsPollingEnabled()
{
	return m_staticProcessorRemoteObjectsPollingEnabled;
}

/**
 * Helper to set the running state of internal active
 * handling of non-flickering objects.
 * @param	changeSource	
 * @param	enabled			True if processor related non-flickering objects should be monitored, false if to not.

 */
void Controller::SetStaticProcessorRemoteObjectsPollingEnabled(DataChangeParticipant changeSource, bool enabled)
{
	if (m_staticProcessorRemoteObjectsPollingEnabled != enabled)
	{
		m_staticProcessorRemoteObjectsPollingEnabled = enabled;

		SetParameterChanged(changeSource, DCT_RefreshInterval);
	}
}

/**
 * Getter for the list of remote objects that are handled
 * standalone active (read from DS100) as a flat list copy.
 * @return	The internal list of remote objects.
 */
const std::vector<RemoteObject> Controller::GetStandaloneActiveRemoteObjects()
{
	std::vector<RemoteObject> currentStandaloneActiveRemoteObjects;
	for (auto const& objectsPerListener : m_standaloneActiveRemoteObjects)
	{
		for (auto const& ro : objectsPerListener.second)
			currentStandaloneActiveRemoteObjects.push_back(ro);
	}

	return currentStandaloneActiveRemoteObjects;
}

/**
 * Getter for the list of remote objects that are handled
 * standalone active (read from DS100) for a given listener
 * as ref to the internal hash.
 * @param	listener	The listener object to get the currently registered remote objects for
 * @return	The internal list of remote objects.
 */
const std::vector<RemoteObject>& Controller::GetStandaloneActiveRemoteObjects(Controller::StandaloneActiveObjectsListener* listener)
{
	return m_standaloneActiveRemoteObjects[listener];
}

/**
 * Method to check if a given remote object already is contained
 * in the internal list of remote objects being actively handled
 * as standalone.
 * @param	remoteObject	The object to check.
 * @return	True if given object is already contained, false if not.
 */
bool Controller::ContainsStandaloneActiveRemoteObject(Controller::StandaloneActiveObjectsListener* listener, const RemoteObject& remoteObject)
{
	auto objIter = std::find(GetStandaloneActiveRemoteObjects(listener).begin(), GetStandaloneActiveRemoteObjects(listener).end(), remoteObject);
	return (objIter != GetStandaloneActiveRemoteObjects(listener).end());
}

/**
 * Add a given remote object to the list of standalone actively handled objects.
 * If the adding listener instance is not yet registered as listener, it is also added to list of listeners.
 * @param	listener		The listener instance that wants to add an object
 * @param	remoteObject	The object to add
 * @return	True if adding succeeded, false if it already was present in list
 */
bool Controller::AddStandaloneActiveRemoteObject(Controller::StandaloneActiveObjectsListener* listener, const RemoteObject& remoteObject)
{
	// check if listener is already registered and if not add it
	if (std::find(m_standaloneActiveObjectListeners.begin(), m_standaloneActiveObjectListeners.end(), listener) == m_standaloneActiveObjectListeners.end())
		m_standaloneActiveObjectListeners.push_back(listener);

	// if the object is already registered for th given listener, do not add again
	if (ContainsStandaloneActiveRemoteObject(listener, remoteObject))
		return false;
	else
	{
		// if all was ok, add the object for the listener
		m_standaloneActiveRemoteObjects[listener].push_back(remoteObject);
		return true;
	}
}

/**
 * Remove a given remote object from the list of standalone actively handled objects.
 * @param remoteObject	The object to remove
 * @return	True if removing succeeded, false if it was not found in list
 */
bool Controller::RemoveStandaloneActiveRemoteObject(Controller::StandaloneActiveObjectsListener* listener, const RemoteObject& remoteObject)
{
	// something is fishy if the listener that wants to remove an object is not even known yet
	jassert(std::find(m_standaloneActiveObjectListeners.begin(), m_standaloneActiveObjectListeners.end(), listener) != m_standaloneActiveObjectListeners.end());

	// check if the object to remove can be found in the list
	auto objIter = std::find(m_standaloneActiveRemoteObjects[listener].begin(), m_standaloneActiveRemoteObjects[listener].end(), remoteObject);
	if (objIter == m_standaloneActiveRemoteObjects[listener].end())
		return false;
	else
	{
		// if all was ok, remove the object
		m_standaloneActiveRemoteObjects[listener].erase(objIter);
		return true;
	}
}

/**
 * Helper to trigger refreshing object values registered for a given
 * standalone active objects listener.
 * What actually happens to refresh the values depends on 
 * the communication means to DS100 in use - subscription based
 * communication e.g. does not require active confirmation of changes.
 * @param	listener	The listener object instance to refresh the registered object's values for
 */
void Controller::TriggerConfirmStandaloneActiveObjects(Controller::StandaloneActiveObjectsListener* listener)
{
	// todo
	m_pollingHelper->TriggerPollOnce(listener);
}

/**
 * Adds a listener instance to the list of objects to be
 * notified for all incoming bridging layer messages
 * that come from a relevant source (prefiltered by controller).
 * @param	listener	The listener object instance to add
 * @return	True if added successfully, false if already in list.
 */
bool Controller::AddStandaloneActiveObjectsListener(Controller::StandaloneActiveObjectsListener* listener)
{
	if (std::find(m_standaloneActiveObjectListeners.begin(), m_standaloneActiveObjectListeners.end(), listener) == m_standaloneActiveObjectListeners.end())
	{
		m_standaloneActiveObjectListeners.push_back(listener);
		return true;
	}
	else
		return false;
}

/**
 * Removes a listener instance from the internal list.
 * Also removes all active remote objects registered for this listener instance from internal hash.
 * @param	listener	The listener object instance to remove
 * @return	True if removed successfully, false if not found in list.
 */
bool Controller::RemoveStandaloneActiveObjectsListener(Controller::StandaloneActiveObjectsListener* listener)
{
	m_standaloneActiveRemoteObjects.erase(listener);

	auto listenerIter = std::find(m_standaloneActiveObjectListeners.begin(), m_standaloneActiveObjectListeners.end(), listener);
	if (listenerIter != m_standaloneActiveObjectListeners.end())
	{
		m_standaloneActiveObjectListeners.erase(listenerIter);
		return true;
	}
	else
		return false;
}

/**
 * Updates the remote objects that are currently being actively handled.
 * @param dontSendNotification	Flag if the app configuration update should be triggered (fwd. to bridging wrapper).
 */
void Controller::UpdateActiveRemoteObjects(bool dontSendNotification)
{
	// Get currently active objects from controller and split them 
	// into those relevant for first and second DS100
	auto activeSoObjects = GetActivatedSoundObjectRemoteObjects();
	auto activeMiObjects = GetActivatedMatrixInputRemoteObjects();
	auto activeMoObjects = GetActivatedMatrixOutputRemoteObjects();

	auto activeObjects = std::vector<RemoteObject>();
	activeObjects.insert(activeObjects.end(), activeSoObjects.begin(), activeSoObjects.end());
	activeObjects.insert(activeObjects.end(), activeMiObjects.begin(), activeMiObjects.end());
	activeObjects.insert(activeObjects.end(), activeMoObjects.begin(), activeMoObjects.end());
	
	if (!IsPollingDS100ProtocolType())
	{
		auto standaloneActiveObjects = GetAllStandaloneActiveRemoteObjectsToUse();
		activeObjects.insert(activeObjects.end(), standaloneActiveObjects.begin(), standaloneActiveObjects.end());
	}

	m_protocolBridge.UpdateActiveDS100RemoteObjectIds(activeObjects, dontSendNotification);
}

/**
 * Helper method to create a new processor incl. implicit triggering of
 * inserting it into xml config (by setting constructor bool flag to insertToConfig=true)
 */
void Controller::createNewSoundobjectProcessor()
{
	auto processor = std::make_unique<SpaConBridge::SoundobjectProcessor>(true);
	processor.release(); // let go of the instance here, we do not want to destroy it, since it lives as member of CCOntroller when constructed
}

/**
 * Register a processor instance to the local list of processors. 
 * @param changeSource	The application module which is causing the property change.
 * @param p				Pointer to newly crated processor processor object.
 * @return				The ProcessorId of the newly added processor.
 */
SoundobjectProcessorId Controller::AddSoundobjectProcessor(DataChangeParticipant changeSource, SoundobjectProcessor* p)
{
	const ScopedLock lock(m_mutex);

	// Get the highest Input number of all current processors.
	SoundobjectId currentMaxSoundobjectId = 0;
	for (auto const& processor : m_soundobjectProcessors)
	{
		if (processor->GetSoundobjectId() > currentMaxSoundobjectId)
			currentMaxSoundobjectId = processor->GetSoundobjectId();
	}
	SoundobjectId newSoundobjectId = currentMaxSoundobjectId + 1;

	// Get the next free processor id to use (can be one inbetween or the next after the last)
	auto newProcessorId = GetNextProcessorId();

	// add the processor to list now, since we have taken all info we require from the so far untouched list
	m_soundobjectProcessors.add(p);

	// Set the new Processor's id
	p->SetProcessorId(changeSource, newProcessorId);
	
	SetParameterChanged(changeSource, DCT_NumProcessors);

	// Set the new Processor's InputID to the next in sequence.
	p->SetSoundobjectId(changeSource, newSoundobjectId);

	// Set default yellowish colour as an acceptably contrasting startingpoint for both day- and night-lookandfeel ui coloring
	p->SetSoundobjectColour(changeSource, Colour(0xffffc700));

	// Set a default painting siez for the new soundobject
	p->SetSoundobjectSize(changeSource, 0.4f);

	return newProcessorId;
}

/**
 * Remove a Processor instance from the local list of processors.
 * @param p		Pointer to Processor object which should be removed.
 */
void Controller::RemoveSoundobjectProcessor(SoundobjectProcessor* p)
{
	int idx = m_soundobjectProcessors.indexOf(p);
	if (idx >= 0)
	{
		const ScopedLock lock(m_mutex);
		m_soundobjectProcessors.remove(idx);

		// Manually trigger updating active objects, since tick based
		// updating will not catch changes, if no soundobjects are
		// left any more.
		if (m_soundobjectProcessors.isEmpty())
		{
			UpdateActiveRemoteObjects();
			CleanupMutedObjects();
		}

		SetParameterChanged(DCP_Host, DCT_NumProcessors);

		EnqueueTickTrigger();
	}
}

/**
 * Removes the processor ids and deletes the linked Processor instances from the local list of processors.
 * @param sopIds		List of processor ids that shall be removed.
 */
void Controller::RemoveSoundobjectProcessorIds(const std::vector<SoundobjectProcessorId>& sopIds)
{
	if (sopIds.empty())
		return;

	auto config = SpaConBridge::AppConfiguration::getInstance();
	auto configFlushAndUpdateWasDisabled = false;
	if (config)
	{
		configFlushAndUpdateWasDisabled = config->IsFlushAndUpdateDisabled();
		config->SetFlushAndUpdateDisabled();
	}

	// first loop over processor ids to remove is to clean up the internal list, but not yet delete the processor and editor instances themselves
	auto sops = std::vector<SoundobjectProcessor*>();
	for (auto const& processorId : sopIds)
	{
		auto processor = GetSoundobjectProcessor(processorId);
		if (processor != nullptr)
		{
			const ScopedLock lock(m_mutex);
			m_soundobjectProcessors.removeAllInstancesOf(processor);
			sops.push_back(processor);
		}
	}

	// trigger updating page components to ensure the table contents (editor components esp.) don't keep stale obj. pointers
	auto pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
	{
		auto pageContainer = pageMgr->GetPageContainer();
		if (pageContainer)
			pageContainer->UpdateGui(true);
	}

	if (config && !configFlushAndUpdateWasDisabled)
		config->ResetFlushAndUpdateDisabled();

	// second loop over processors to remove is to delete the processor and editor instances themselves
	for (auto const& sop : sops)
	{
		auto processor = std::unique_ptr<SoundobjectProcessor>(sop); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
		if (processor)
		{
			std::unique_ptr<AudioProcessorEditor>(processor->getActiveEditor()).reset();
			processor->releaseResources();
			processor->reset();
		}
	}

	const ScopedLock lock(m_mutex);
	// Manually trigger updating active objects, since tick based
	// updating will not catch changes, if no soundobjects are
	// left any more.
	if (m_soundobjectProcessors.isEmpty())
	{
		UpdateActiveRemoteObjects();
		CleanupMutedObjects();
	}

	SetParameterChanged(DCP_Host, DCT_NumProcessors);

	EnqueueTickTrigger();
}

/**
 * Number of registered Processor instances.
 * @return	Number of registered Processor instances.
 */
int Controller::GetSoundobjectProcessorCount() const
{
	const ScopedLock lock(m_mutex);
	return m_soundobjectProcessors.size();
}

/**
 * Get a pointer to a specified processor.
 * @param processorId	The id of the desired processor.
 * @return	The pointer to the desired processor.
 */
SoundobjectProcessor* Controller::GetSoundobjectProcessor(SoundobjectProcessorId processorId) const
{
	const ScopedLock lock(m_mutex);
	for (const auto&processor : m_soundobjectProcessors)
		if (processor->GetProcessorId() == processorId)
			return processor;

	return nullptr;
}

/**
 * Getter for all currently active processor's processorIds
 * @return	The vector of active processorids
 */
std::vector<SoundobjectProcessorId> Controller::GetSoundobjectProcessorIds() const
{
	std::vector<SoundobjectProcessorId> processorIds;
	processorIds.reserve(m_soundobjectProcessors.size());
	for (auto const& p : m_soundobjectProcessors)
		processorIds.push_back(p->GetProcessorId());
	return processorIds;
}


/**
 * Helper method to create a new processor incl. implicit triggering of
 * inserting it into xml config (by setting constructor bool flag to insertToConfig=true)
 */
void Controller::createNewMatrixInputProcessor()
{
	auto processor = std::make_unique<SpaConBridge::MatrixInputProcessor>(true);
	processor.release(); // let go of the instance here, we do not want to destroy it, since it lives as member of controller when constructed
}

/**
 * Register a processor instance to the local list of processors.
 * @param changeSource	The application module which is causing the property change.
 * @param p				Pointer to newly crated processor processor object.
 * @return				The ProcessorId of the newly added processor.
 */
MatrixInputProcessorId Controller::AddMatrixInputProcessor(DataChangeParticipant changeSource, MatrixInputProcessor* p)
{
	const ScopedLock lock(m_mutex);

	// Get the highest Input number of all current processors.
	MatrixInputId currentMaxMatrixInputId = 0;
	for (auto const& processor : m_matrixInputProcessors)
	{
		if (processor->GetMatrixInputId() > currentMaxMatrixInputId)
			currentMaxMatrixInputId = processor->GetMatrixInputId();
	}

	// Get the next free processor id to use (can be one inbetween or the next after the last)
	auto newProcessorId = GetNextProcessorId();

	// add the processor to list now, since we have taken all info we require from the so far untouched list
	m_matrixInputProcessors.add(p);

	// Set the new Processor's id
	p->SetProcessorId(changeSource, newProcessorId);

	SetParameterChanged(changeSource, DCT_NumProcessors);

	// Set the new Processor's InputID to the next in sequence.
	p->SetMatrixInputId(changeSource, currentMaxMatrixInputId + 1);

	return newProcessorId;
}

/**
 * Remove a Processor instance from the local list of processors.
 * @param p		Pointer to Processor object which should be removed.
 */
void Controller::RemoveMatrixInputProcessor(MatrixInputProcessor* p)
{
	int idx = m_matrixInputProcessors.indexOf(p);
	if (idx >= 0)
	{
		const ScopedLock lock(m_mutex);
		m_matrixInputProcessors.remove(idx);

		// Manually trigger updating active objects, since tick based
		// updating will not catch changes, if no matrix inputs are
		// left any more.
		if (m_matrixInputProcessors.isEmpty())
		{
			UpdateActiveRemoteObjects();
			CleanupMutedObjects();
		}

		SetParameterChanged(DCP_Host, DCT_NumProcessors);

		EnqueueTickTrigger();
	}
}

/**
 * Removes the processor ids and deletes the linked Processor instances from the local list of processors.
 * @param mipIds		List of processor ids that shall be removed.
 */
void Controller::RemoveMatrixInputProcessorIds(const std::vector<MatrixInputProcessorId>& mipIds)
{
	if (mipIds.empty())
		return;

	auto config = SpaConBridge::AppConfiguration::getInstance();
	auto configFlushAndUpdateWasDisabled = false;
	if (config)
	{
		configFlushAndUpdateWasDisabled = config->IsFlushAndUpdateDisabled();
		config->SetFlushAndUpdateDisabled();
	}

	// first loop over processor ids to remove is to clean up the internal list, but not yet delete the processor and editor instances themselves
	auto mips = std::vector<MatrixInputProcessor*>();
	for (auto const& processorId : mipIds)
	{
		auto processor = GetMatrixInputProcessor(processorId);
		if (processor != nullptr)
		{
			const ScopedLock lock(m_mutex);
			m_matrixInputProcessors.removeAllInstancesOf(processor);
			mips.push_back(processor);
		}
	}

	// trigger updating page components to ensure the table contents (editor components esp.) don't keep stale obj. pointers
	auto pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
	{
		auto pageContainer = pageMgr->GetPageContainer();
		if (pageContainer)
			pageContainer->UpdateGui(true);
	}

	if (config && !configFlushAndUpdateWasDisabled)
		config->ResetFlushAndUpdateDisabled();

	// second loop over processors to remove is to delete the processor and editor instances themselves
	for (auto const& mip : mips)
	{
		auto processor = std::unique_ptr<MatrixInputProcessor>(mip); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
		if (processor)
		{
			std::unique_ptr<AudioProcessorEditor>(processor->getActiveEditor()).reset();
			processor->releaseResources();
			processor->reset();
		}
	}

	const ScopedLock lock(m_mutex);
	// Manually trigger updating active objects, since tick based
	// updating will not catch changes, if no matrix inputs are
	// left any more.
	if (m_matrixInputProcessors.isEmpty())
	{
		UpdateActiveRemoteObjects();
		CleanupMutedObjects();
	}

	SetParameterChanged(DCP_Host, DCT_NumProcessors);

	EnqueueTickTrigger();
}

/**
 * Number of registered Processor instances.
 * @return	Number of registered Processor instances.
 */
int Controller::GetMatrixInputProcessorCount() const
{
	const ScopedLock lock(m_mutex);
	return m_matrixInputProcessors.size();
}

/**
 * Get a pointer to a specified processor.
 * @param processorId	The id of the desired processor.
 * @return	The pointer to the desired processor.
 */
MatrixInputProcessor* Controller::GetMatrixInputProcessor(MatrixInputProcessorId processorId) const
{
	const ScopedLock lock(m_mutex);
	for (auto processor : m_matrixInputProcessors)
		if (processor->GetProcessorId() == processorId)
			return processor;

	jassertfalse; // id not existing!
	return nullptr;
}

/**
 * Getter for all currently active processor's processorIds
 * @return	The vector of active processorids
 */
std::vector<MatrixInputProcessorId> Controller::GetMatrixInputProcessorIds() const
{
	std::vector<MatrixInputProcessorId> processorIds;
	processorIds.reserve(m_matrixInputProcessors.size());
	for (auto const& p : m_matrixInputProcessors)
		processorIds.push_back(p->GetProcessorId());
	return processorIds;
}


/**
 * Helper method to create a new processor incl. implicit triggering of
 * inserting it into xml config (by setting constructor bool flag to insertToConfig=true)
 */
void Controller::createNewMatrixOutputProcessor()
{
	auto processor = std::make_unique<SpaConBridge::MatrixOutputProcessor>(true);
	processor.release(); // let go of the instance here, we do not want to destroy it, since it lives as member of controller when constructed
}

/**
 * Register a processor instance to the local list of processors.
 * @param changeSource	The application module which is causing the property change.
 * @param p				Pointer to newly crated processor processor object.
 * @return				The ProcessorId of the newly added processor.
 */
MatrixOutputProcessorId Controller::AddMatrixOutputProcessor(DataChangeParticipant changeSource, MatrixOutputProcessor* p)
{
	const ScopedLock lock(m_mutex);

	// Get the highest Input number of all current processors.
	MatrixOutputId currentMaxMatrixOutputId = 0;
	for (auto const& processor : m_matrixOutputProcessors)
	{
		if (processor->GetMatrixOutputId() > currentMaxMatrixOutputId)
			currentMaxMatrixOutputId = processor->GetMatrixOutputId();
	}

	// Get the next free processor id to use (can be one inbetween or the next after the last)
	auto newProcessorId = GetNextProcessorId();

	// add the processor to list now, since we have taken all info we require from the so far untouched list
	m_matrixOutputProcessors.add(p);

	// Set the new Processor's id
	p->SetProcessorId(changeSource, newProcessorId);

	SetParameterChanged(changeSource, DCT_NumProcessors);

	// Set the new Processor's InputID to the next in sequence.
	p->SetMatrixOutputId(changeSource, currentMaxMatrixOutputId + 1);

	return newProcessorId;
}

/**
 * Remove a Processor instance from the local list of processors.
 * @param p		Pointer to Processor object which should be removed.
 */
void Controller::RemoveMatrixOutputProcessor(MatrixOutputProcessor* p)
{
	int idx = m_matrixOutputProcessors.indexOf(p);
	if (idx >= 0)
	{
		const ScopedLock lock(m_mutex);
		m_matrixOutputProcessors.remove(idx);

		// Manually trigger updating active objects, since tick based
		// updating will not catch changes, if no matrix outputs are
		// left any more.
		if (m_matrixOutputProcessors.isEmpty())
		{
			UpdateActiveRemoteObjects();
			CleanupMutedObjects();
		}

		SetParameterChanged(DCP_Host, DCT_NumProcessors);

		EnqueueTickTrigger();
	}
}

/**
 * Removes the processor ids and deletes the linked Processor instances from the local list of processors.
 * @param mopIds		List of processor ids that shall be removed.
 */
void Controller::RemoveMatrixOutputProcessorIds(const std::vector<MatrixOutputProcessorId>& mopIds)
{
	if (mopIds.empty())
		return;

	auto config = SpaConBridge::AppConfiguration::getInstance();
	auto configFlushAndUpdateWasDisabled = false;
	if (config)
	{
		configFlushAndUpdateWasDisabled = config->IsFlushAndUpdateDisabled();
		config->SetFlushAndUpdateDisabled();
	}

	// first loop over processor ids to remove is to clean up the internal list, but not yet delete the processor and editor instances themselves
	auto mops = std::vector<MatrixOutputProcessor*>();
	for (auto const& processorId : mopIds)
	{
		auto processor = GetMatrixOutputProcessor(processorId);
		if (processor != nullptr)
		{
			const ScopedLock lock(m_mutex);
			m_matrixOutputProcessors.removeAllInstancesOf(processor);
			mops.push_back(processor);
		}
	}

	// trigger updating page components to ensure the table contents (editor components esp.) don't keep stale obj. pointers
	auto pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
	{
		auto pageContainer = pageMgr->GetPageContainer();
		if (pageContainer)
			pageContainer->UpdateGui(true);
	}

	if (config && !configFlushAndUpdateWasDisabled)
		config->ResetFlushAndUpdateDisabled();

	// second loop over processors to remove is to delete the processor and editor instances themselves
	for (auto const& mop : mops)
	{
		auto processor = std::unique_ptr<MatrixOutputProcessor>(mop); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
		if (processor)
		{
			std::unique_ptr<AudioProcessorEditor>(processor->getActiveEditor()).reset();
			processor->releaseResources();
			processor->reset();
		}
	}

	const ScopedLock lock(m_mutex);
	// Manually trigger updating active objects, since tick based
	// updating will not catch changes, if no matrix outputs are
	// left any more.
	if (m_matrixOutputProcessors.isEmpty())
	{
		UpdateActiveRemoteObjects();
		CleanupMutedObjects();
	}

	SetParameterChanged(DCP_Host, DCT_NumProcessors);

	EnqueueTickTrigger();
}

/**
 * Number of registered Processor instances.
 * @return	Number of registered Processor instances.
 */
int Controller::GetMatrixOutputProcessorCount() const
{
	const ScopedLock lock(m_mutex);
	return m_matrixOutputProcessors.size();
}

/**
 * Get a pointer to a specified processor.
 * @param processorId	The id of the desired processor.
 * @return	The pointer to the desired processor.
 */
MatrixOutputProcessor* Controller::GetMatrixOutputProcessor(MatrixOutputProcessorId processorId) const
{
	const ScopedLock lock(m_mutex);
	for (const auto&processor : m_matrixOutputProcessors)
		if (processor->GetProcessorId() == processorId)
			return processor;

	jassertfalse; // id not existing!
	return nullptr;
}

/**
 * Getter for all currently active processor's processorIds
 * @return	The vector of active processorids
 */
std::vector<MatrixOutputProcessorId> Controller::GetMatrixOutputProcessorIds() const
{
	std::vector<MatrixOutputProcessorId> processorIds;
	processorIds.reserve(m_matrixOutputProcessors.size());
	for (auto const& p : m_matrixOutputProcessors)
		processorIds.push_back(p->GetProcessorId());
	return processorIds;
}


/**
 * Getter function for the DS100 protocol type currently used.
 * @return	Current protocol type used for communication.
 */
ProtocolType Controller::GetDS100ProtocolType() const
{
	return m_DS100ProtocolType;
}

/**
 * Setter function for the DS100 protocol type to be used.
 * @param changeSource	The application module which is causing the property change.
 * @param mode		New protocol type.
 * @param dontSendNotification	Flag if the app configuration update should be triggered.
 */
void Controller::SetDS100ProtocolType(DataChangeParticipant changeSource, ProtocolType protocol, bool dontSendNotification)
{
	const ScopedLock lock(m_mutex);

	if (m_DS100ProtocolType != protocol)
	{
		m_DS100ProtocolType = protocol;

		// special case NoProtocol - this requires shutting off any extension mode
		if ((m_DS100ProtocolType == PT_NoProtocol || m_DS100ProtocolType == PT_AURAProtocol) && m_DS100ExtensionMode != EM_Off)
		{
			if (m_protocolBridge.SetDS100ExtensionMode(EM_Off, true))
				m_DS100ExtensionMode = EM_Off;
		}

		m_pollingHelper->SetRunning(IsPollingDS100ProtocolType());

		UpdateActiveRemoteObjects(DCP_Init == changeSource); // update objects but propage update only if it is not during init to avoid unfinished app construction asserts

		m_protocolBridge.SetDS100ProtocolType(protocol, dontSendNotification);

		// IP, port, etc. have likely changed when changing the protocol type (new defaults set)
		m_DS100IpAddress = m_protocolBridge.GetDS100IpAddress();
		m_DS100Port = m_protocolBridge.GetDS100Port();

		m_SecondDS100IpAddress = m_protocolBridge.GetSecondDS100IpAddress();
		m_SecondDS100Port = m_protocolBridge.GetSecondDS100Port();

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, DCT_ProtocolType);
		SetParameterChanged(changeSource, DCT_ExtensionMode);
		SetParameterChanged(changeSource, DCT_IPAddress);
		SetParameterChanged(changeSource, DCT_Connected);

		Reconnect();
	}
}

/***
 * Helper to get the bool info on if the current
 * protocol used for DS100 is using polling for
 * getting updated values.
 * @return	True if polling is used (OSC, No) or if not (OCP1)
 */
bool Controller::IsPollingDS100ProtocolType()
{
	switch (m_DS100ProtocolType)
	{
	case PT_OSCProtocol:
		return true;
	case PT_OCP1Protocol:
	case PT_NoProtocol:
	case PT_AURAProtocol:
	default:
		return false;
		break;
	}
}

/**
 * Getter function for the IP address and port to which we are connected.
 * @return	Current IP address.
 */
std::pair<juce::IPAddress, int> Controller::GetDS100IpAndPort() const
{
	return std::make_pair(m_DS100IpAddress, m_DS100Port);
}

/**
 * Setter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * NOTE: changing ip address will disconnect m_oscSender and m_oscReceiver.
 * @param changeSource	The application module which is causing the property change.
 * @param ipAddress		New IP address.
 * @param port			New port.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetDS100IpAndPort(DataChangeParticipant changeSource, juce::IPAddress ipAddress, int port, bool dontSendNotification)
{
	if (m_DS100IpAddress != ipAddress || m_DS100Port != port)
	{
		const ScopedLock lock(m_mutex);

		m_DS100IpAddress = ipAddress;
		m_protocolBridge.SetDS100IpAddress(ipAddress, dontSendNotification);

		m_DS100Port = port;
		m_protocolBridge.SetDS100Port(port, dontSendNotification);

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, DCT_IPAddress);
		SetParameterChanged(changeSource, DCT_Connected);

		Reconnect();
	}
}

/**
 * Getter function for the IP address and to which we are connected.
 * @return	Current IP address + port.
 */
std::pair<juce::IPAddress, int> Controller::GetSecondDS100IpAndPort() const
{
	return std::make_pair(m_SecondDS100IpAddress, m_SecondDS100Port);
}

/**
 * Setter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * NOTE: changing ip address will disconnect m_oscSender and m_oscReceiver.
 * @param changeSource	The application module which is causing the property change.
 * @param ipAddress		New IP address.
 * @param port			New port.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetSecondDS100IpAndPort(DataChangeParticipant changeSource, juce::IPAddress ipAddress, int port, bool dontSendNotification)
{
	if (m_SecondDS100IpAddress != ipAddress || m_SecondDS100Port != port)
	{
		const ScopedLock lock(m_mutex);

		m_SecondDS100IpAddress = ipAddress;
		m_protocolBridge.SetSecondDS100IpAddress(ipAddress, dontSendNotification);

		m_SecondDS100Port = port;
		m_protocolBridge.SetSecondDS100Port(port, dontSendNotification);

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, DCT_IPAddress);
		SetParameterChanged(changeSource, DCT_Connected);

		Reconnect();
	}
}

/**
 * Getter function for the DS100 bridging communication state.
 * @return		True if all communication channels are connected.
 */
bool Controller::IsConnected() const
{
	switch (GetExtensionMode())
	{
	case ExtensionMode::EM_Off:
		return IsFirstDS100Connected();
	case ExtensionMode::EM_Extend:
	case ExtensionMode::EM_Mirror:
	case ExtensionMode::EM_Parallel:
		return (IsFirstDS100Connected() && IsSecondDS100Connected());
	default:
		return false;
	}
}

/**
 * Getter function for the DS100 bridging communication state.
 * @return		True if communication channel with first DS100 is connected.
 */
bool Controller::IsFirstDS100Connected() const
{
	return ((m_protocolBridge.GetDS100State() & OHS_Protocol_Up) == OHS_Protocol_Up);
}

/**
 * Getter function for the DS100 bridging mirror state.
 * @return		True if first DS100 is currently master in extension modes "parallel" or "mirror".
 */
bool Controller::IsFirstDS100Master() const
{
	if (GetExtensionMode() == EM_Off)
		return true;
	if (GetExtensionMode() == EM_Extend)
		return true;
	if (GetExtensionMode() != EM_Parallel && GetExtensionMode() != EM_Mirror)
		return false;

	return ((m_protocolBridge.GetDS100State() & OHS_Protocol_Master) == OHS_Protocol_Master);
}

/**
 * Getter function for the DS100 bridging communication state.
 * @return		True if communication channel with second DS100 is connected.
 */
bool Controller::IsSecondDS100Connected() const
{
	return ((m_protocolBridge.GetSecondDS100State() & OHS_Protocol_Up) == OHS_Protocol_Up);
}

/**
 * Getter function for the DS100 bridging mirror state.
 * @return		True if second DS100 is currently master in extension modes "parallel" or "mirror".
 */
bool Controller::IsSecondDS100Master() const
{
	if (GetExtensionMode() == EM_Off)
		return false;
	if (GetExtensionMode() == EM_Extend)
		return true;
	if (GetExtensionMode() != EM_Parallel && GetExtensionMode() != EM_Mirror)
		return false;

	return ((m_protocolBridge.GetSecondDS100State() & OHS_Protocol_Master) == OHS_Protocol_Master);
}

/**
 * Setter for the DS100 bridging online state.
 * @param changeSource	The application module which is causing the property change.
 * @param online	The online activated state to be set.
 */
void Controller::SetOnline(DataChangeParticipant changeSource, bool online)
{
	if (online != m_onlineState)
	{
		const ScopedLock lock(m_mutex);

		m_onlineState = online;

		m_protocolBridge.SetOnline(online);

		SetParameterChanged(changeSource, DCT_OnlineState);
	}
}

/**
 * Getter function for the DS100 bridging online state.
 * @return		True if communication channel with DS100 is online (activated, not neccessarily connected).
 */
bool Controller::IsOnline() const
{
	return m_onlineState;
}

/**
 * Getter for the interval at which the controller internal update is triggered.
 * @return	Refresh Interval, in milliseconds.
 */
int Controller::GetRefreshInterval() const
{
	return m_refreshInterval;
}

/**
 * Setter for the rate at which OSC messages are being sent out.
 * @param changeSource	The application module which is causing the property change.
 * @param refreshInterval	New refresh interval, in milliseconds.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetRefreshInterval(DataChangeParticipant changeSource, int refreshInterval, bool dontSendNotification)
{
	if (refreshInterval != m_refreshInterval)
	{
		const ScopedLock lock(m_mutex);

		// Clip rate to the allowed range.
		if(refreshInterval != INVALID_RATE_VALUE)
		{
			DBG(juce::String(__FUNCTION__) << " clipping to range (" << PROTOCOL_INTERVAL_MIN << "," << PROTOCOL_INTERVAL_MAX << "): " << refreshInterval);
			refreshInterval = juce::jlimit(PROTOCOL_INTERVAL_MIN, PROTOCOL_INTERVAL_MAX, refreshInterval);
		}

		m_refreshInterval = refreshInterval;

		m_protocolBridge.SetDS100MsgRate(refreshInterval, dontSendNotification);

		// Signal the change to all Processors.
		SetParameterChanged(changeSource, DCT_RefreshInterval);
	}
}

/**
 * Static methiod which returns the allowed minimum and maximum PROTOCOL message rates.
 * @return	Returns a std::pair<int, int> where the first number is the minimum supported rate, 
 *			and the second number is the maximum.
 */
std::pair<int, int> Controller::GetSupportedRefreshIntervalRange()
{
	return std::pair<int, int>(PROTOCOL_INTERVAL_MIN, PROTOCOL_INTERVAL_MAX);
}

/**
 * Getter function for the DS100 extension mode currently used.
 * @return	Current IP address.
 */
ExtensionMode Controller::GetExtensionMode() const
{
	return m_DS100ExtensionMode;
}

/**
 * Setter function for the DS100 extension mode to be used.
 * @param changeSource	The application module which is causing the property change.
 * @param mode		New extension mode.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetExtensionMode(DataChangeParticipant changeSource, ExtensionMode mode, bool dontSendNotification)
{
	if (m_DS100ExtensionMode != mode)
	{
		const ScopedLock lock(m_mutex);

		m_DS100ExtensionMode = mode;

		m_protocolBridge.SetDS100ExtensionMode(mode, dontSendNotification);

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, DCT_ExtensionMode);
		SetParameterChanged(changeSource, DCT_Connected);

		Reconnect();
	}
}

/**
 * Getter function for the DS100 currently active in extension mode "parallel".
 * @return	Current parallel mode DS100 active.
 */
ActiveParallelModeDS100 Controller::GetActiveParallelModeDS100() const
{
	return m_DS100ActiveParallelModeDS100;
}

/**
 * Setter function for the DS100 to be currently active in extension mode "parallel".
 * @param changeSource	The application module which is causing the property change.
 * @param activeParallelModeDS100		New active DS100 in parallel extension mode.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetActiveParallelModeDS100(DataChangeParticipant changeSource, ActiveParallelModeDS100 activeParallelModeDS100, bool dontSendNotification)
{
	if (m_DS100ActiveParallelModeDS100 != activeParallelModeDS100)
	{
		const ScopedLock lock(m_mutex);

		m_DS100ActiveParallelModeDS100 = activeParallelModeDS100;

		m_protocolBridge.SetActiveParallelModeDS100(activeParallelModeDS100, dontSendNotification);

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, DCT_ExtensionMode);
		SetParameterChanged(changeSource, DCT_Connected);

		Reconnect();
	}
}

/**
 * Getter for the dummy project config data set for DS100 'None' protocol.
 * @return	Current string dummy project config data.
 */
juce::String Controller::GetDS100DummyProjectData() const
{
	return m_DS100DummyProjectData;
}

/**
 * Setter for the dummy project config data set for DS100 'None' protocol.
 * @param changeSource			The application module which is causing the property change.
 * @param projectDummyData		New dummy project config data.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetDS100DummyProjectData(DataChangeParticipant changeSource, const juce::String& projectDummyData, bool dontSendNotification)
{
	if (m_DS100DummyProjectData != projectDummyData)
	{
		const ScopedLock lock(m_mutex);

		m_DS100DummyProjectData = projectDummyData;

		m_protocolBridge.SetDS100dbprData(projectDummyData, dontSendNotification);

		// Signal the change to all Processors.
		SetParameterChanged(changeSource, DCT_Connected | DCT_SpeakerPositionData | DCT_CoordinateMappingSettingsData);

		Reconnect();
	}
}

/**
 * Getter for the dummy animation mode data set for DS100 'None' protocol.
 * @return	Current string dummy dummy animation mode.
 */
int Controller::GetDS100DummyAnimationMode() const
{
	return m_DS100DummyAnimationMode;
}

/**
 * Setter for the dummy animation mode set for DS100 'None' protocol.
 * @param changeSource			The application module which is causing the property change.
 * @param animationMode			New animation mode config.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetDS100DummyAnimationMode(DataChangeParticipant changeSource, const int& animationMode, bool dontSendNotification)
{
	if (m_DS100DummyAnimationMode != animationMode)
	{
		const ScopedLock lock(m_mutex);

		m_DS100DummyAnimationMode = animationMode;

		m_protocolBridge.SetDS100AnimationMode(animationMode, dontSendNotification);

		// Signal the change to all Processors.
		SetParameterChanged(changeSource, DCT_Connected);

		Reconnect();
	}
}

/**
 * Reimplemented callback for bridging wrapper callback to process incoming protocol data.
 * It forwards the message to all registered Processor objects.
 * @param nodeId	The bridging node that the message data was received on (only a single default id node supported currently).
 * @param senderProtocolId	The protocol that the message data was received on and was sent to controller from.
 * @param objectId	The remote object id of the object that was received
 * @param msgData	The actual message data that was received
 */
void Controller::HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData)
{
	jassert(nodeId == DEFAULT_PROCNODE_ID);
	if (nodeId != DEFAULT_PROCNODE_ID)
		return;

	if (ProtocolBridgingWrapper::IsBridgingObjectOnly(objectId))
	{
		// Do not handle any protocol data except what is received
		// from DS100 - any data that was sent by 3rd party devices 
		// is bridged to DS100 and returned by it 
		// so we can handle the data in the end as well
		if (senderProtocolId != DS100_1_PROCESSINGPROTOCOL_ID && senderProtocolId != DS100_2_PROCESSINGPROTOCOL_ID)
			return;
	}

	if (GetExtensionMode() == EM_Parallel)
	{
		// Do neither handle any protocol data from second DS100 if first is set as active one...
		if (GetActiveParallelModeDS100() == APM_1st && senderProtocolId != DS100_1_PROCESSINGPROTOCOL_ID)
			return;
		// ...nor any protocol data from first DS100 if second is set as active one in parallel extension mode.
		if (GetActiveParallelModeDS100() == APM_2nd && senderProtocolId != DS100_2_PROCESSINGPROTOCOL_ID)
			return;
	}

	// notify all listeners that registered for the incoming object
	for (auto const& listener : m_standaloneActiveObjectListeners)
	{
		if (listener)
		{
			auto& objsForListener = GetStandaloneActiveRemoteObjects(listener);
			if (std::find(objsForListener.begin(), objsForListener.end(), RemoteObject(objectId, msgData._addrVal)) != objsForListener.end())
				listener->HandleObjectDataInternal(objectId, msgData);
		}
	}

	const ScopedLock lock(m_mutex);

	SoundobjectParameterIndex sopIdx = SPI_ParamIdx_MaxIndex;
	SoundobjectId soundobjectId = INVALID_ADDRESS_VALUE;

	MatrixInputParameterIndex mipIdx = MII_ParamIdx_MaxIndex;
	MatrixInputId matrixInputId = INVALID_ADDRESS_VALUE;

	MatrixOutputParameterIndex mopIdx = MOI_ParamIdx_MaxIndex;
	MatrixOutputId matrixOutputId = INVALID_ADDRESS_VALUE;
	
	MappingId mappingId = INVALID_ADDRESS_VALUE;

	ProcessorSelectionManager::SoundobjectSelectionId soundobjectSelectionId = INVALID_ADDRESS_VALUE;
	ProcessorSelectionManager::MatrixInputSelectionId matrixInputSelectionId = INVALID_ADDRESS_VALUE;
	ProcessorSelectionManager::MatrixOutputSelectionId matrixOutputSelectionId = INVALID_ADDRESS_VALUE;

	DataChangeType change = DCT_None;

	// Determine which parameter was changed depending on the incoming message's address pattern.
	switch (objectId)
	{
	case RemoteObjectIdentifier::ROI_CoordinateMapping_SourcePosition_XY:
		{
			soundobjectId = msgData._addrVal._first;
			jassert(soundobjectId > 0);
			mappingId = msgData._addrVal._second;
			jassert(mappingId > 0);

			sopIdx = SPI_ParamIdx_X;
			change = DCT_SoundobjectPosition;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixInput_ReverbSendGain:
		{
			// The Source ID
			soundobjectId = msgData._addrVal._first;
			jassert(soundobjectId > 0);

			sopIdx = SPI_ParamIdx_ReverbSendGain;
			change = DCT_ReverbSendGain;
		}
		break;
	case RemoteObjectIdentifier::ROI_Positioning_SourceSpread:
		{
			soundobjectId = msgData._addrVal._first;
			jassert(soundobjectId > 0);

			sopIdx = SPI_ParamIdx_ObjectSpread;
			change = DCT_SoundobjectSpread;
		}
		break;
	case RemoteObjectIdentifier::ROI_Positioning_SourceDelayMode:
		{
			soundobjectId = msgData._addrVal._first;
			jassert(soundobjectId > 0);

			sopIdx = SPI_ParamIdx_DelayMode;
			change = DCT_DelayMode;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixInput_LevelMeterPreMute:
		{
			matrixInputId = msgData._addrVal._first;
			jassert(matrixInputId > 0);

			mipIdx = MII_ParamIdx_LevelMeterPreMute;
			change = DCT_MatrixInputLevelMeter;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixInput_Gain:
		{
			matrixInputId = msgData._addrVal._first;
			jassert(matrixInputId > 0);

			mipIdx = MII_ParamIdx_Gain;
			change = DCT_MatrixInputGain;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixInput_Mute:
		{
			matrixInputId = msgData._addrVal._first;
			jassert(matrixInputId > 0);

			mipIdx = MII_ParamIdx_Mute;
			change = DCT_MatrixInputMute;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixOutput_LevelMeterPostMute:
		{
			matrixOutputId = msgData._addrVal._first;
			jassert(matrixOutputId > 0);

			mopIdx = MOI_ParamIdx_LevelMeterPostMute;
			change = DCT_MatrixOutputLevelMeter;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixOutput_Gain:
		{
			matrixOutputId = msgData._addrVal._first;
			jassert(matrixOutputId > 0);

			mopIdx = MOI_ParamIdx_Gain;
			change = DCT_MatrixOutputGain;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixOutput_Mute:
		{
			matrixOutputId = msgData._addrVal._first;
			jassert(matrixOutputId > 0);

			mopIdx = MOI_ParamIdx_Mute;
			change = DCT_MatrixOutputMute;
		}
		break;
	case RemoteObjectIdentifier::ROI_RemoteProtocolBridge_SoundObjectSelect:
	case RemoteObjectIdentifier::ROI_MatrixInput_Select:
		{
			// The Source ID
			soundobjectId = msgData._addrVal._first;
			jassert(soundobjectId > 0);

			jassert(msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT);

			change = DCT_ProcessorSelection;
		}
		break;
	case RemoteObjectIdentifier::ROI_RemoteProtocolBridge_UIElementIndexSelect:
		{
			jassert(msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT);

			change = DCT_TabPageSelection;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixInput_ChannelName:
		{
			jassert(msgData._valType == RemoteObjectValueType::ROVT_STRING);
			soundobjectId = msgData._addrVal._first;
			jassert(soundobjectId > 0);
			matrixInputId = msgData._addrVal._first;
			jassert(matrixInputId > 0);
			change = DCT_MatrixInputName;
		}
		break;
	case RemoteObjectIdentifier::ROI_MatrixOutput_ChannelName:
		{
			jassert(msgData._valType == RemoteObjectValueType::ROVT_STRING);
			matrixOutputId = msgData._addrVal._first;
			jassert(matrixOutputId > 0);
			change = DCT_MatrixOutputName;
		}
		break;
	case RemoteObjectIdentifier::ROI_RemoteProtocolBridge_SoundObjectGroupSelect:
		{
			// The group No
			soundobjectSelectionId = msgData._addrVal._first;
			jassert(soundobjectSelectionId > 0);

			jassert(msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT);

			change = DCT_ProcessorSelection;
		}
		break;
	case RemoteObjectIdentifier::ROI_RemoteProtocolBridge_MatrixInputGroupSelect:
		{
			// The group No
			matrixInputSelectionId = msgData._addrVal._first;
			jassert(matrixInputSelectionId > 0);

			jassert(msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT);

			change = DCT_ProcessorSelection;
		}
		break;
	case RemoteObjectIdentifier::ROI_RemoteProtocolBridge_MatrixOutputGroupSelect:
		{
			// The group No
			matrixOutputSelectionId = msgData._addrVal._first;
			jassert(matrixOutputSelectionId > 0);

			jassert(msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT);

			change = DCT_ProcessorSelection;
		}
		break;
	default:
		break;
	}

	// If soundobject/matrixInput/matrixOutput id is present, it needs to be checked regarding special DS100 extension mode
	if (soundobjectId > 0 && senderProtocolId == DS100_2_PROCESSINGPROTOCOL_ID && GetExtensionMode() == EM_Extend)
		soundobjectId += DS100_CHANNELCOUNT;
	if (matrixInputId > 0 && senderProtocolId == DS100_2_PROCESSINGPROTOCOL_ID && GetExtensionMode() == EM_Extend)
		matrixInputId += DS100_CHANNELCOUNT;
	if (matrixOutputId > 0 && senderProtocolId == DS100_2_PROCESSINGPROTOCOL_ID && GetExtensionMode() == EM_Extend)
		matrixOutputId += DS100_CHANNELCOUNT;

	// now process what changes were detected to be neccessary to perform
	if (change == DCT_ProcessorSelection)
	{
		auto const selMgr = ProcessorSelectionManager::GetInstance();
		if (selMgr && msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT)
		{
			auto newSelectState = (static_cast<int*>(msgData._payload)[0] == 1);

			if (soundobjectId > 0)
			{
				for (auto const& processorId : GetSoundobjectProcessorIds())
				{
					auto processor = GetSoundobjectProcessor(processorId);
					if (processor->GetSoundobjectId() == soundobjectId && selMgr->IsSoundobjectProcessorIdSelected(processorId) != newSelectState)
					{
						selMgr->SetSoundobjectProcessorIdSelectState(processorId, newSelectState);
						SetParameterChanged(DCP_Protocol, DCT_ProcessorSelection);
					}
				}
			}
			else if (soundobjectSelectionId > 0)
			{
				auto const& existingIds = selMgr->GetSoundobjectProcessorSelectionGroupIds();
				if (std::find(existingIds.begin(), existingIds.end(), soundobjectSelectionId) != existingIds.end())
					selMgr->RecallSoundobjectProcessorSelectionGroup(soundobjectSelectionId);
			}
			else if (matrixInputSelectionId > 0)
			{
				auto const& existingIds = selMgr->GetMatrixInputProcessorSelectionGroupIds();
				if (std::find(existingIds.begin(), existingIds.end(), matrixInputSelectionId) != existingIds.end())
					selMgr->RecallMatrixInputProcessorSelectionGroup(matrixInputSelectionId);
			}
			else if (matrixOutputSelectionId > 0)
			{
				auto const& existingIds = selMgr->GetMatrixOutputProcessorSelectionGroupIds();
				if (std::find(existingIds.begin(), existingIds.end(), matrixOutputSelectionId) != existingIds.end())
					selMgr->RecallMatrixOutputProcessorSelectionGroup(matrixOutputSelectionId);
			}
		}
	}
	else if (change == DCT_TabPageSelection)
	{
		if (msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT)
		{
			auto pageMgr = PageComponentManager::GetInstance();
			if (pageMgr)
			{
				auto pageIndex = static_cast<int*>(msgData._payload)[0];
				if (pageIndex > UPI_InvalidMin && pageIndex < UPI_InvalidMax)
				{
					auto pageId = static_cast<UIPageId>(pageIndex);

					// enshure that the externally selected page is currently visible on UI
					if (std::find(pageMgr->GetEnabledPages().begin(), pageMgr->GetEnabledPages().end(), pageId) != pageMgr->GetEnabledPages().end())
						pageMgr->SetActivePage(pageId, dontSendNotification);
				}
			}
		}
	}
	else if (change == DCT_MatrixInputName)
	{
		for (auto const& processor : m_soundobjectProcessors)
		{
			// Check for matching Input number.
			if (soundobjectId == processor->GetSoundobjectId())
			{
				if (msgData._valType == ROVT_STRING && msgData._payloadSize > 0 && msgData._payload != nullptr)
				{
					auto matrixInputName = std::string(static_cast<char*>(msgData._payload), msgData._payloadSize);
					processor->changeProgramName(processor->getCurrentProgram(), matrixInputName);
				}
			}
		}
		for (auto const& processor : m_matrixInputProcessors)
		{
			// Check for matching Input number.
			if (matrixInputId == processor->GetMatrixInputId())
			{
				if (msgData._valType == ROVT_STRING && msgData._payloadSize > 0 && msgData._payload != nullptr)
				{
					auto matrixInputName = std::string(static_cast<char*>(msgData._payload), msgData._payloadSize);
					processor->changeProgramName(processor->getCurrentProgram(), matrixInputName);
				}
			}
		}
	}
	else if (change == DCT_MatrixOutputName)
	{
		for (auto const& processor : m_matrixOutputProcessors)
		{
			// Check for matching Output number.
			if (matrixOutputId == processor->GetMatrixOutputId())
			{
				if (msgData._valType == ROVT_STRING && msgData._payloadSize > 0 && msgData._payload != nullptr)
				{
					auto matrixOutputName = std::string(static_cast<char*>(msgData._payload), msgData._payloadSize);
					processor->changeProgramName(processor->getCurrentProgram(), matrixOutputName);
				}
			}
		}
	}
	else if (change != DCT_None)
	{
		// update all processors with fresh values
		for (auto const& processor : m_soundobjectProcessors)
		{
			// Check for matching Input number.
			if (soundobjectId == processor->GetSoundobjectId())
			{
				ComsMode mode = processor->GetComsMode();

				bool isReceiveMode = ((mode & CM_Rx) == CM_Rx);

				// Only pass on new positions to processors that are in RX mode.
				if (isReceiveMode)
				{
					// Special handling for X/Y position, since message contains two parameters and MappingID needs to match too.
					if (sopIdx == SPI_ParamIdx_X)
					{
						if (mappingId == processor->GetMappingId())
						{
							jassert(msgData._valCount == 2 && msgData._valType == RemoteObjectValueType::ROVT_FLOAT);
							if (msgData._valCount == 2 && msgData._valType == RemoteObjectValueType::ROVT_FLOAT)
							{
								auto newXValue = static_cast<float*>(msgData._payload)[0];
								auto newYValue = static_cast<float*>(msgData._payload)[1];

								// Set the processor's new position; Only if values have changed within dual decimal digit range (DS100 OSC precision)
								if (static_cast<int>(100.0f * newXValue) != static_cast<int>(100.0f * processor->GetParameterValue(SPI_ParamIdx_X))
									|| static_cast<int>(100.0f * newYValue) != static_cast<int>(100.0f * processor->GetParameterValue(SPI_ParamIdx_Y)))
								{
									processor->SetParameterValue(DCP_Protocol, SPI_ParamIdx_X, newXValue);
									processor->SetParameterValue(DCP_Protocol, SPI_ParamIdx_Y, newYValue);
								}
							}
						}
					}

					// All other automation parameters.
					else
					{
						auto newValue = 0.0f;
						switch (msgData._valType)
						{
						case RemoteObjectValueType::ROVT_INT:
							newValue = static_cast<float>(*(static_cast<int*>(msgData._payload)));
							break;
						case RemoteObjectValueType::ROVT_FLOAT:
							newValue = *(static_cast<float*>(msgData._payload));
							break;
						case RemoteObjectValueType::ROVT_NONE:
						default:
							break;
						}

						if (static_cast<int>(100.0f * newValue) != static_cast<int>(100.0f * processor->GetParameterValue(sopIdx)))
							processor->SetParameterValue(DCP_Protocol, sopIdx, newValue);
					}
				}
			}
		}
		for (auto const& processor : m_matrixInputProcessors)
		{
			// Check for matching Input number.
			if (matrixInputId == processor->GetMatrixInputId())
			{
				ComsMode mode = processor->GetComsMode();

				bool isReceiveMode = ((mode & CM_Rx) == CM_Rx);

				// Only pass on new positions to processors that are in RX mode.
				if (isReceiveMode)
				{
					auto newValue = 0.0f;
					switch (msgData._valType)
					{
					case RemoteObjectValueType::ROVT_INT:
						newValue = static_cast<float>(*(static_cast<int*>(msgData._payload)));
						break;
					case RemoteObjectValueType::ROVT_FLOAT:
						newValue = *(static_cast<float*>(msgData._payload));
						break;
					case RemoteObjectValueType::ROVT_NONE:
					default:
						break;
					}
					
					if (static_cast<int>(100.0f * newValue) != static_cast<int>(100.0f * processor->GetParameterValue(mipIdx)))
						processor->SetParameterValue(DCP_Protocol, mipIdx, newValue);
				}
			}
		}
		for (auto const& processor : m_matrixOutputProcessors)
		{
			// Check for matching Output number.
			if (matrixOutputId == processor->GetMatrixOutputId())
			{
				ComsMode mode = processor->GetComsMode();

				bool isReceiveMode = ((mode & CM_Rx) == CM_Rx);

				// Only pass on new positions to processors that are in RX mode.
				if (isReceiveMode)
				{
					auto newValue = 0.0f;
					switch (msgData._valType)
					{
					case RemoteObjectValueType::ROVT_INT:
						newValue = static_cast<float>(*(static_cast<int*>(msgData._payload)));
						break;
					case RemoteObjectValueType::ROVT_FLOAT:
						newValue = *(static_cast<float*>(msgData._payload));
						break;
					case RemoteObjectValueType::ROVT_NONE:
					default:
						break;
					}

					if (static_cast<int>(100.0f * newValue) != static_cast<int>(100.0f * processor->GetParameterValue(mopIdx)))
						processor->SetParameterValue(DCP_Protocol, mopIdx, newValue);
				}
			}
		}
	}

}

/**
 * Proxy method to allow direct access to bridging module message sending method.
 * @param	roi		The remote object identifier of the message to be sent.
 * @param	msgData	The message data incl. addressing to be sent.
 * @return	True on success, false on sending failure.
 */
void Controller::handleMessage(const Message& message)
{
	if (auto tickTrigger = dynamic_cast<const TickTrigger*>(&message))
	{
		if (!tickTrigger->IsOutdated())
			tick();

		// Mark the tick event as handled here, even though the object will
		// be deleted by JUCE later on, to allow new tick events to be enqueued
		// from here on.
		tickTrigger->SetTickHandled();
	}
	else if(auto parameterChange = dynamic_cast<const ParameterChangedMessage*>(&message))
	{
		SetParameterChanged(parameterChange->GetChangeSource(), parameterChange->GetChangeTypes());
	}
}

bool Controller::SendMessageDataDirect(const RemoteObjectIdentifier roi, RemoteObjectMessageData& msgData)
{
	return m_protocolBridge.SendMessage(roi, msgData);
}

/**
 * Disconnect the active bridging nodes' protocols.
 */
void Controller::EnqueueTickTrigger()
{
	if (TickTrigger::IsOutdated())
		postMessage(new TickTrigger());
}

void Controller::Disconnect()
{
	m_protocolBridge.Disconnect();
}

/**
 * Disconnect and re-connect the OSCSender to a host specified by the current ip settings.
 */
void Controller::Reconnect()
{
	m_protocolBridge.Reconnect();
}

void Controller::tick()
{
	if (IsTickProcessingStopped())
	{
		SetTickWasPostponedWhenPaused();
		return;
	}

	const ScopedLock lock(m_mutex);

	float newDualFloatValue[2];
	int newIntValue;
	RemoteObjectMessageData newMsgData;

	auto cleanupMutedObjectsRequired = PopParameterChanged(DCP_Host, DCT_NumProcessors);
	auto isParameterUpdate = false;

	auto activeSSIdsChanged = false;
	for (auto const& soProcessor : m_soundobjectProcessors)
	{
		auto comsMode = soProcessor->GetComsMode();

		// Check if the processor configuration has changed
		// and need to be updated in the bridging configuration
		if (soProcessor->GetParameterChanged(DCP_Host, DCT_SoundobjectProcessorConfig))
		{
			auto activateSSId = false;
			auto deactivateSSId = false;
			if (soProcessor->PopParameterChanged(DCP_Host, DCT_SoundobjectID))
			{
				// SoundsourceID change means update is only required when
				// remote object is currently activated. 
				activateSSId = ((comsMode & CM_Rx) == CM_Rx);
				cleanupMutedObjectsRequired = true;
			}

			if (soProcessor->PopParameterChanged(DCP_Host, DCT_MappingID))
			{
				// MappingID change means update is only required when
				// remote object is currently activated. 
				activateSSId = ((comsMode & CM_Rx) == CM_Rx);
				cleanupMutedObjectsRequired = true;
			}

			if (soProcessor->PopParameterChanged(DCP_Host, DCT_ComsMode))
			{
				// ComsMode change means toggling polling for the remote object,
				// so one of the two activate/deactivate actions is required
				activateSSId = ((comsMode & CM_Rx) == CM_Rx);
				deactivateSSId = !activateSSId;
			}

			activeSSIdsChanged = activeSSIdsChanged || activateSSId || deactivateSSId;
		}

		// Signal every tick to each processor instance.
		soProcessor->Tick();

		newMsgData._addrVal._first = static_cast<juce::uint16>(soProcessor->GetSoundobjectId());
		newMsgData._addrVal._second = INVALID_ADDRESS_VALUE;

		// Iterate through all automation parameters.
		for (int pIdx = SPI_ParamIdx_X; pIdx < SPI_ParamIdx_MaxIndex; ++pIdx)
		{
			switch (pIdx)
			{
				case SPI_ParamIdx_X:
				{
					// SET command is only sent out while in CM_Tx mode, provided that
					// this parameter has been changed since the last tick.
					if (((comsMode & CM_Tx) == CM_Tx) && soProcessor->GetParameterChanged(DCP_Protocol, DCT_SoundobjectPosition))
					{
						newDualFloatValue[0] = soProcessor->GetParameterValue(SPI_ParamIdx_X);
						newDualFloatValue[1] = soProcessor->GetParameterValue(SPI_ParamIdx_Y);

						newMsgData._addrVal._second = static_cast<juce::uint16>(soProcessor->GetMappingId());

						newMsgData._valCount = 2;
						newMsgData._valType = ROVT_FLOAT;
						newMsgData._payload = &newDualFloatValue;
						newMsgData._payloadSize = 2 * sizeof(float);

						m_protocolBridge.SendMessage(ROI_CoordinateMapping_SourcePosition_XY, newMsgData);
					}
				}
				break;

				case SPI_ParamIdx_Y:
					// Changes to ParamIdx_Y are handled together with ParamIdx_X, so skip it.
					continue;
					break;

				case SPI_ParamIdx_ReverbSendGain:
				{
					// SET command is only sent out while in CM_Tx mode, provided that
					// this parameter has been changed since the last tick.
					if (((comsMode & CM_Tx) == CM_Tx) && soProcessor->GetParameterChanged(DCP_Protocol, DCT_ReverbSendGain))
					{
						newDualFloatValue[0] = soProcessor->GetParameterValue(SPI_ParamIdx_ReverbSendGain);

						newMsgData._valCount = 1;
						newMsgData._valType = ROVT_FLOAT;
						newMsgData._payload = &newDualFloatValue;
						newMsgData._payloadSize = sizeof(float);

						m_protocolBridge.SendMessage(ROI_MatrixInput_ReverbSendGain, newMsgData);
					}
				}
				break;

				case SPI_ParamIdx_ObjectSpread:
				{
					// SET command is only sent out while in CM_Tx mode, provided that
					// this parameter has been changed since the last tick.
					if (((comsMode & CM_Tx) == CM_Tx) && soProcessor->GetParameterChanged(DCP_Protocol, DCT_SoundobjectSpread))
					{
						newDualFloatValue[0] = soProcessor->GetParameterValue(SPI_ParamIdx_ObjectSpread);

						newMsgData._valCount = 1;
						newMsgData._valType = ROVT_FLOAT;
						newMsgData._payload = &newDualFloatValue;
						newMsgData._payloadSize = sizeof(float);

						m_protocolBridge.SendMessage(ROI_Positioning_SourceSpread, newMsgData);
					}
				}
				break;

				case SPI_ParamIdx_DelayMode:
				{
					// SET command is only sent out while in CM_Tx mode, provided that
					// this parameter has been changed since the last tick.
					if (((comsMode & CM_Tx) == CM_Tx) && soProcessor->GetParameterChanged(DCP_Protocol, DCT_DelayMode))
					{
						newIntValue = static_cast<int>(soProcessor->GetParameterValue(SPI_ParamIdx_DelayMode));

						newMsgData._valCount = 1;
						newMsgData._valType = ROVT_INT;
						newMsgData._payload = &newIntValue;
						newMsgData._payloadSize = sizeof(int);

						m_protocolBridge.SendMessage(ROI_Positioning_SourceDelayMode, newMsgData);
					}
				}
				break;

				default:
					jassertfalse;
					break;
			}
		}

		// All changed parameters were sent out, so we can reset their flags now.
		isParameterUpdate = isParameterUpdate || soProcessor->PopParameterChanged(DCP_Protocol, DCT_SoundobjectParameters);
	}
	if (activeSSIdsChanged)
	{
		cleanupMutedObjectsRequired = true;
		UpdateActiveRemoteObjects();
	}

	auto activeMIIdsChanged = false;
	for (auto const& miProcessor : m_matrixInputProcessors)
	{
		auto comsMode = miProcessor->GetComsMode();

		// Check if the processor configuration has changed
		// and need to be updated in the bridging configuration
		if (miProcessor->GetParameterChanged(DCP_Host, DCT_MatrixInputProcessorConfig))
		{
			auto activateMIId = false;
			auto deactivateMIId = false;
			if (miProcessor->PopParameterChanged(DCP_Host, DCT_MatrixInputID))
			{
				// MatrixInputID change means update is only required when
				// remote object is currently activated. 
				activateMIId = ((comsMode & CM_Rx) == CM_Rx);
				cleanupMutedObjectsRequired = true;
			}

			if (miProcessor->PopParameterChanged(DCP_Host, DCT_MappingID))
			{
				// MappingID change means update is only required when
				// remote object is currently activated. 
				activateMIId = ((comsMode & CM_Rx) == CM_Rx);
				cleanupMutedObjectsRequired = true;
			}

			if (miProcessor->PopParameterChanged(DCP_Host, DCT_ComsMode))
			{
				// ComsMode change means toggling polling for the remote object,
				// so one of the two activate/deactivate actions is required
				activateMIId = ((comsMode & CM_Rx) == CM_Rx);
				deactivateMIId = !activateMIId;
			}

			activeMIIdsChanged = activeMIIdsChanged || activateMIId || deactivateMIId;
		}

		// Signal every tick to each processor instance.
		miProcessor->Tick();

		newMsgData._addrVal._first = static_cast<juce::uint16>(miProcessor->GetMatrixInputId());
		newMsgData._addrVal._second = INVALID_ADDRESS_VALUE;

		// Iterate through all automation parameters.
		for (int pIdx = MII_ParamIdx_LevelMeterPreMute; pIdx < MII_ParamIdx_MaxIndex; ++pIdx)
		{
			switch (pIdx)
			{
			case MII_ParamIdx_LevelMeterPreMute:
			{
				// SET command is only sent out while in CM_Tx mode, provided that
				// this parameter has been changed since the last tick.
				if (((comsMode & CM_Tx) == CM_Tx) && miProcessor->GetParameterChanged(DCP_Protocol, DCT_MatrixInputLevelMeter))
				{
					newDualFloatValue[0] = miProcessor->GetParameterValue(MII_ParamIdx_LevelMeterPreMute);

					newMsgData._valCount = 1;
					newMsgData._valType = ROVT_FLOAT;
					newMsgData._payload = &newDualFloatValue;
					newMsgData._payloadSize = sizeof(float);

					m_protocolBridge.SendMessage(ROI_MatrixInput_LevelMeterPreMute, newMsgData);
				}
			}
			break;

			case MII_ParamIdx_Gain:
			{
				// SET command is only sent out while in CM_Tx mode, provided that
				// this parameter has been changed since the last tick.
				if (((comsMode & CM_Tx) == CM_Tx) && miProcessor->GetParameterChanged(DCP_Protocol, DCT_MatrixInputGain))
				{
					newDualFloatValue[0] = miProcessor->GetParameterValue(MII_ParamIdx_Gain);

					newMsgData._valCount = 1;
					newMsgData._valType = ROVT_FLOAT;
					newMsgData._payload = &newDualFloatValue;
					newMsgData._payloadSize = sizeof(float);

					m_protocolBridge.SendMessage(ROI_MatrixInput_Gain, newMsgData);
				}
			}
			break;

			case MII_ParamIdx_Mute:
			{
				// SET command is only sent out while in CM_Tx mode, provided that
				// this parameter has been changed since the last tick.
				if (((comsMode & CM_Tx) == CM_Tx) && miProcessor->GetParameterChanged(DCP_Protocol, DCT_MatrixInputMute))
				{
					newIntValue = static_cast<int>(miProcessor->GetParameterValue(MII_ParamIdx_Mute));

					newMsgData._valCount = 1;
					newMsgData._valType = ROVT_INT;
					newMsgData._payload = &newIntValue;
					newMsgData._payloadSize = sizeof(int);

					m_protocolBridge.SendMessage(ROI_MatrixInput_Mute, newMsgData);
				}
			}
			break;

			default:
				jassertfalse;
				break;
			}
		}

		// All changed parameters were sent out, so we can reset their flags now.
		isParameterUpdate = isParameterUpdate || miProcessor->PopParameterChanged(DCP_Protocol, DCT_MatrixInputParameters);
	}
	if (activeMIIdsChanged)
	{
		cleanupMutedObjectsRequired = true;
		UpdateActiveRemoteObjects();
	}

	auto activeMOIdsChanged = false;
	for (auto const& moProcessor : m_matrixOutputProcessors)
	{
		auto comsMode = moProcessor->GetComsMode();

		// Check if the processor configuration has changed
		// and need to be updated in the bridging configuration
		if (moProcessor->GetParameterChanged(DCP_Host, DCT_MatrixOutputProcessorConfig))
		{
			auto activateMOId = false;
			auto deactivateMOId = false;
			if (moProcessor->PopParameterChanged(DCP_Host, DCT_MatrixOutputID))
			{
				// MatrixOutputID change means update is only required when
				// remote object is currently activated. 
				activateMOId = ((comsMode & CM_Rx) == CM_Rx);
				cleanupMutedObjectsRequired = true;
			}

			if (moProcessor->PopParameterChanged(DCP_Host, DCT_MappingID))
			{
				// MappingID change means update is only required when
				// remote object is currently activated. 
				activateMOId = ((comsMode & CM_Rx) == CM_Rx);
				cleanupMutedObjectsRequired = true;
			}

			if (moProcessor->PopParameterChanged(DCP_Host, DCT_ComsMode))
			{
				// ComsMode change means toggling polling for the remote object,
				// so one of the two activate/deactivate actions is required
				activateMOId = ((comsMode & CM_Rx) == CM_Rx);
				deactivateMOId = !activateMOId;
			}

			activeMOIdsChanged = activeMOIdsChanged || activateMOId || deactivateMOId;
		}

		// Signal every tick to each processor instance.
		moProcessor->Tick();

		newMsgData._addrVal._first = static_cast<juce::uint16>(moProcessor->GetMatrixOutputId());
		newMsgData._addrVal._second = INVALID_ADDRESS_VALUE;

		// Iterate through all automation parameters.
		for (int pIdx = MOI_ParamIdx_LevelMeterPostMute; pIdx < MOI_ParamIdx_MaxIndex; ++pIdx)
		{
			switch (pIdx)
			{
			case MOI_ParamIdx_LevelMeterPostMute:
			{
				// SET command is only sent out while in CM_Tx mode, provided that
				// this parameter has been changed since the last tick.
				if (((comsMode & CM_Tx) == CM_Tx) && moProcessor->GetParameterChanged(DCP_Protocol, DCT_MatrixOutputLevelMeter))
				{
					newDualFloatValue[0] = moProcessor->GetParameterValue(MOI_ParamIdx_LevelMeterPostMute);

					newMsgData._valCount = 1;
					newMsgData._valType = ROVT_FLOAT;
					newMsgData._payload = &newDualFloatValue;
					newMsgData._payloadSize = sizeof(float);

					m_protocolBridge.SendMessage(ROI_MatrixOutput_LevelMeterPostMute, newMsgData);
				}
			}
			break;

			case MOI_ParamIdx_Gain:
			{
				// SET command is only sent out while in CM_Tx mode, provided that
				// this parameter has been changed since the last tick.
				if (((comsMode & CM_Tx) == CM_Tx) && moProcessor->GetParameterChanged(DCP_Protocol, DCT_MatrixOutputGain))
				{
					newDualFloatValue[0] = moProcessor->GetParameterValue(MOI_ParamIdx_Gain);

					newMsgData._valCount = 1;
					newMsgData._valType = ROVT_FLOAT;
					newMsgData._payload = &newDualFloatValue;
					newMsgData._payloadSize = sizeof(float);

					m_protocolBridge.SendMessage(ROI_MatrixOutput_Gain, newMsgData);
				}
			}
			break;

			case MOI_ParamIdx_Mute:
			{
				// SET command is only sent out while in CM_Tx mode, provided that
				// this parameter has been changed since the last tick.
				if (((comsMode & CM_Tx) == CM_Tx) && moProcessor->GetParameterChanged(DCP_Protocol, DCT_MatrixOutputMute))
				{
					newIntValue = static_cast<int>(moProcessor->GetParameterValue(MOI_ParamIdx_Mute));

					newMsgData._valCount = 1;
					newMsgData._valType = ROVT_INT;
					newMsgData._payload = &newIntValue;
					newMsgData._payloadSize = sizeof(int);

					m_protocolBridge.SendMessage(ROI_MatrixOutput_Mute, newMsgData);
				}
			}
			break;

			default:
				jassertfalse;
				break;
			}
		}

		// All changed parameters were sent out, so we can reset their flags now.
		isParameterUpdate = isParameterUpdate || moProcessor->PopParameterChanged(DCP_Protocol, DCT_MatrixOutputParameters);
	}
	if (activeMOIdsChanged)
	{
		cleanupMutedObjectsRequired = true;
		UpdateActiveRemoteObjects();
	}

	if (cleanupMutedObjectsRequired)
		CleanupMutedObjects();

	auto isControllerUpdate = GetParameterChanged(DCP_PageContainer, DCT_AllConfigParameters|DCT_Connected);

	if (isParameterUpdate || isControllerUpdate)
	{
		auto pageMgr = PageComponentManager::GetInstance();
		if (pageMgr)
		{
			auto pageContainer = pageMgr->GetPageContainer();
			if (pageContainer)
				pageContainer->UpdateGui(false);
		}
	}

	// trigger the protocol bridge to update the node
	m_protocolBridge.UpdateNode();
}

void Controller::StopTickProcessing()
{
	m_tickProcessingRunning = false;
	m_tickWasPostponedWhenPaused = false;
}

void Controller::ResumeTickProcessing()
{
	m_tickProcessingRunning = true;
	if (m_tickWasPostponedWhenPaused)
		tick();
}

bool Controller::IsTickProcessingStopped()
{
	return !m_tickProcessingRunning;
}

void Controller::SetTickWasPostponedWhenPaused()
{
	m_tickWasPostponedWhenPaused = true;
}

void Controller::PostParameterChanged(DataChangeParticipant changeSource, DataChangeType changeTypes)
{
	postMessage(new ParameterChangedMessage(changeSource, changeTypes));
}


const ProtocolId Controller::GetProtocolIdForProtocolType(const ProtocolBridgingType type)
{
	switch (type)
	{
	case PBT_DiGiCo:
		return DIGICO_PROCESSINGPROTOCOL_ID;
	case PBT_DAWPlugin:
		return DAWPLUGIN_PROCESSINGPROTOCOL_ID;
	case PBT_GenericOSC:
		return GENERICOSC_PROCESSINGPROTOCOL_ID;
	case PBT_BlacktraxRTTrPM:
		return RTTRPM_PROCESSINGPROTOCOL_ID;
	case PBT_GenericMIDI:
		return GENERICMIDI_PROCESSINGPROTOCOL_ID;
	case PBT_YamahaOSC:
		return YAMAHAOSC_PROCESSINGPROTOCOL_ID;
	case PBT_ADMOSC:
		return ADMOSC_PROCESSINGPROTOCOL_ID;
	case PBT_RemapOSC:
		return REMAPOSC_PROCESSINGPROTOCOL_ID;
	case PBT_DS100:
		// DS100 protocol can either be type 1 or 2, so not supported here
	default:
		jassertfalse;
		return 0;
	}
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to set this objects' settings
 * from a XML element structure that passed as argument.
 * @param stateXml	The XML element containing this objects' configuration data
 * @return	True if the data was read and handled successfuly, false if not.
 */
bool Controller::setStateXml(XmlElement* stateXml)
{
	// sanity check, if the incoming xml does make sense for this method
	if (!stateXml || (stateXml->getTagName() != AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER)))
		return false;

	// To prevent that we end up in a recursive ::setStateXml situation, verify that this setStateXml method is not called by itself
	const ScopedXmlChangeLock lock(IsXmlChangeLocked());
	if (!lock.isLocked())
		return false;

	bool retVal = true;

	// set online state from xml
	auto onlineStateXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ONLINESTATE));
	if (onlineStateXmlElement)
	{
		auto onlineStateTextXmlElement = onlineStateXmlElement->getFirstChildElement();
		if (onlineStateTextXmlElement && onlineStateTextXmlElement->isTextElement())
			SetOnline(DCP_Init, onlineStateTextXmlElement->getAllSubText().getIntValue() == 1);
	}

	// set polling non-flickering objects state from xml
	auto staticObjectsPollingStateXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::STATICOBJECTSPOLLING));
	if (staticObjectsPollingStateXmlElement)
	{
		auto staticObjectsPollingStateTextXmlElement = staticObjectsPollingStateXmlElement->getFirstChildElement();
		if (staticObjectsPollingStateTextXmlElement && staticObjectsPollingStateTextXmlElement->isTextElement())
			SetStaticProcessorRemoteObjectsPollingEnabled(DCP_Init, staticObjectsPollingStateTextXmlElement->getAllSubText().getIntValue() == 1);
	}

	// create soundobject processors from xml
	auto soundobjectProcessorsXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDOBJECTPROCESSORS));
	if (soundobjectProcessorsXmlElement)
	{
		auto oldExistingSOPIds = GetSoundobjectProcessorIds();
		auto newConfigSOPIds = std::vector<SoundobjectProcessorId>();

		auto xmlsForNewProcessorsToCreateKV = std::map<int, XmlElement*>();

		for (auto processorXmlElement : soundobjectProcessorsXmlElement->getChildIterator())
		{
			jassert(processorXmlElement->getTagName().contains(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE)));
			int elementProcessorId = processorXmlElement->getTagName().getTrailingIntValue();
			newConfigSOPIds.push_back(elementProcessorId);

			bool alreadyExists = false;
			for (const auto&processor : m_soundobjectProcessors)
			{
				if (processor->GetProcessorId() == elementProcessorId)
				{
					processor->setStateXml(processorXmlElement);
					alreadyExists = true;
					break;
				}
			}

			if (!alreadyExists)
				xmlsForNewProcessorsToCreateKV.insert(std::make_pair(elementProcessorId, processorXmlElement));
		}

		// clean up no longer used processors first
		auto sopIdsToRemove = std::vector<SoundobjectProcessorId>();
		for (auto const& processorId : oldExistingSOPIds)
		{
			if (std::find(newConfigSOPIds.begin(), newConfigSOPIds.end(), processorId) == newConfigSOPIds.end())
			{
				sopIdsToRemove.push_back(processorId);
			}
		}
		RemoveSoundobjectProcessorIds(sopIdsToRemove);

		// then create the ones that are entirely new in the updated xml config
		for (auto const& newProcessorXmlElement : xmlsForNewProcessorsToCreateKV)
		{
			auto newProcessor = std::make_unique<SoundobjectProcessor>(false);
			newProcessor->SetProcessorId(DCP_Init, newProcessorXmlElement.first);
			auto p = newProcessor.release();
			jassert(m_soundobjectProcessors.contains(p));
			p->setStateXml(newProcessorXmlElement.second);
		}
	}
	else
		retVal = false;

	// create matrixinput processors from xml
	auto matrixInputProcessorsXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXINPUTPROCESSORS));
	if (matrixInputProcessorsXmlElement)
	{
		auto oldExistingMIPIds = GetMatrixInputProcessorIds();
		auto newConfigMIPIds = std::vector<MatrixInputProcessorId>();

		auto xmlsForNewProcessorsToCreateKV = std::map<int, XmlElement*>();

		for (auto processorXmlElement : matrixInputProcessorsXmlElement->getChildIterator())
		{
			jassert(processorXmlElement->getTagName().contains(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE)));
			int elementProcessorId = processorXmlElement->getTagName().getTrailingIntValue();
			newConfigMIPIds.push_back(elementProcessorId);

			bool alreadyExists = false;
			for (const auto&processor : m_matrixInputProcessors)
			{
				if (processor->GetProcessorId() == elementProcessorId)
				{
					processor->setStateXml(processorXmlElement);
					alreadyExists = true;
					break;
				}
			}

			if (!alreadyExists)
				xmlsForNewProcessorsToCreateKV.insert(std::make_pair(elementProcessorId, processorXmlElement));
		}

		// clean up no longer used processors first
		auto mipIdsToRemove = std::vector<MatrixInputProcessorId>();
		for (auto const& processorId : oldExistingMIPIds)
		{
			if (std::find(newConfigMIPIds.begin(), newConfigMIPIds.end(), processorId) == newConfigMIPIds.end())
			{
				mipIdsToRemove.push_back(processorId);
			}
		}
		RemoveMatrixInputProcessorIds(mipIdsToRemove);

		// then create the ones that are entirely new in the updated xml config
		for (auto const& newProcessorXmlElement : xmlsForNewProcessorsToCreateKV)
		{
			auto newProcessor = std::make_unique<MatrixInputProcessor>(false);
			newProcessor->SetProcessorId(DCP_Init, newProcessorXmlElement.first);
			auto p = newProcessor.release();
			jassert(m_matrixInputProcessors.contains(p));
			p->setStateXml(newProcessorXmlElement.second);
		}
	}
	else
		retVal = false;

	// create matrixoutput processors from xml
	auto matrixOutputProcessorsXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXOUTPUTPROCESSORS));
	if (matrixOutputProcessorsXmlElement)
	{
		auto oldExistingMOPIds = GetMatrixOutputProcessorIds();
		auto newConfigMOPIds = std::vector<MatrixOutputProcessorId>();

		auto xmlsForNewProcessorsToCreateKV = std::map<int, XmlElement*>();

		for (auto processorXmlElement : matrixOutputProcessorsXmlElement->getChildIterator())
		{
			jassert(processorXmlElement->getTagName().contains(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE)));
			int elementProcessorId = processorXmlElement->getTagName().getTrailingIntValue();
			newConfigMOPIds.push_back(elementProcessorId);

			bool alreadyExists = false;
			for (const auto&processor : m_matrixOutputProcessors)
			{
				if (processor->GetProcessorId() == elementProcessorId)
				{
					processor->setStateXml(processorXmlElement);
					alreadyExists = true;
					break;
				}
			}

			if (!alreadyExists)
				xmlsForNewProcessorsToCreateKV.insert(std::make_pair(elementProcessorId, processorXmlElement));
		}

		// clean up no longer used processors first
		auto mopIdsToRemove = std::vector<MatrixOutputProcessorId>();
		for (auto const& processorId : oldExistingMOPIds)
		{
			if (std::find(newConfigMOPIds.begin(), newConfigMOPIds.end(), processorId) == newConfigMOPIds.end())
			{
				mopIdsToRemove.push_back(processorId);
			}
		}
		RemoveMatrixOutputProcessorIds(mopIdsToRemove);

		// then create the ones that are entirely new in the updated xml config
		for (auto const& newProcessorXmlElement : xmlsForNewProcessorsToCreateKV)
		{
			auto newProcessor = std::make_unique<MatrixOutputProcessor>(false);
			newProcessor->SetProcessorId(DCP_Init, newProcessorXmlElement.first);
			auto p = newProcessor.release();
			jassert(m_matrixOutputProcessors.contains(p));
			p->setStateXml(newProcessorXmlElement.second);
		}
	}
	else
		retVal = false;

	// Configure the bridging module before the UI is initialized, because it requires certain details in controller
	// config to be set up already to be reflected on UI accordingly
	auto bridgingXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING));
	if (bridgingXmlElement)
	{
		if (m_protocolBridge.setStateXml(bridgingXmlElement))
		{
			SetDS100ProtocolType(DCP_Init, m_protocolBridge.GetDS100ProtocolType(), true);
			SetExtensionMode(DCP_Init, m_protocolBridge.GetDS100ExtensionMode(), true);
			SetDS100IpAndPort(DCP_Init, m_protocolBridge.GetDS100IpAddress(), m_protocolBridge.GetDS100Port(), true);
			SetSecondDS100IpAndPort(DCP_Init, m_protocolBridge.GetSecondDS100IpAddress(), m_protocolBridge.GetSecondDS100Port(), true);
			SetRefreshInterval(DCP_Init, m_protocolBridge.GetDS100MsgRate(), true);
			SetActiveParallelModeDS100(DCP_Init, m_protocolBridge.GetActiveParallelModeDS100(), true);
			SetDS100DummyProjectData(DCP_Init, m_protocolBridge.GetDS100dbprData(), true);
			SetDS100DummyAnimationMode(DCP_Init, m_protocolBridge.GetDS100AnimationMode(), true);
		}
	}

	// trigger UI update once after the processors have been created to clean and update table editors, etc.
	auto pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
	{
		auto pageContainer = pageMgr->GetPageContainer();
		if (pageContainer)
			pageContainer->UpdateGui(true);
	}

	return retVal;
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to dump this objects' settings
 * to a XML element structure that is returned and written to config file by the
 * singleton AppConfiguration class implementation.
 * @return	The XML element data that was created.
 */
std::unique_ptr<XmlElement> Controller::createStateXml()
{
	auto controllerXmlElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));

	auto onlineStateXmlElement = controllerXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ONLINESTATE));
	if (!onlineStateXmlElement)
		onlineStateXmlElement = controllerXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::ONLINESTATE));
	auto onlineStateTextXmlElement = onlineStateXmlElement->getFirstChildElement();
	if (onlineStateTextXmlElement && onlineStateTextXmlElement->isTextElement())
		onlineStateTextXmlElement->setText(String(IsOnline() ? 1 : 0));
	else
		onlineStateXmlElement->addTextElement(String(IsOnline() ? 1 : 0));

	auto staticObjectsPollingStateXmlElement = controllerXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::STATICOBJECTSPOLLING));
	if (!staticObjectsPollingStateXmlElement)
		staticObjectsPollingStateXmlElement = controllerXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::STATICOBJECTSPOLLING));
	auto staticObjectsPollingStateTextXmlElement = staticObjectsPollingStateXmlElement->getFirstChildElement();
	if (staticObjectsPollingStateTextXmlElement && staticObjectsPollingStateTextXmlElement->isTextElement())
		staticObjectsPollingStateTextXmlElement->setText(String(IsStaticProcessorRemoteObjectsPollingEnabled() ? 1 : 0));
	else
		staticObjectsPollingStateXmlElement->addTextElement(String(IsStaticProcessorRemoteObjectsPollingEnabled() ? 1 : 0));

	// create xml from soundobject processors
	auto soundobjectProcessorsXmlElement = controllerXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDOBJECTPROCESSORS));
	if (soundobjectProcessorsXmlElement)
	{
		for (const auto&processor : m_soundobjectProcessors)
		{
			jassert(processor->GetProcessorId() != -1);
			soundobjectProcessorsXmlElement->addChildElement(processor->createStateXml().release());
		}
	}

	// create xml from matrix input processors
	auto matrixInputProcessorsXmlElement = controllerXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXINPUTPROCESSORS));
	if (matrixInputProcessorsXmlElement)
	{
		for (const auto&processor : m_matrixInputProcessors)
		{
			jassert(processor->GetProcessorId() != -1);
			matrixInputProcessorsXmlElement->addChildElement(processor->createStateXml().release());
		}
	}

	// create xml from matrix output processors
	auto matrixOutputProcessorsXmlElement = controllerXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXOUTPUTPROCESSORS));
	if (matrixOutputProcessorsXmlElement)
	{
		for (const auto&processor : m_matrixOutputProcessors)
		{
			jassert(processor->GetProcessorId() != -1);
			matrixOutputProcessorsXmlElement->addChildElement(processor->createStateXml().release());
		}
	}

	// create xml from bridging module
	auto bridgingXmlElement = m_protocolBridge.createStateXml();
	if (bridgingXmlElement)
		controllerXmlElement->addChildElement(bridgingXmlElement.release());

    return controllerXmlElement;
}

/**
 * Helper method to get a list of currently active remote objects.
 * This is generated by dumping all active processor properties and their objects to a list.
 * @return	The list of currently active remote objects.
 */
const std::vector<RemoteObject> Controller::GetActivatedSoundObjectRemoteObjects()
{
	std::vector<RemoteObject> activeRemoteObjects;
	for (auto const& processor : m_soundobjectProcessors)
	{
		if ((processor->GetComsMode() & CM_Rx) == CM_Rx)
		{
			for (auto const& roi : SoundobjectProcessor::GetUsedRemoteObjects())
			{
				auto sourceId = processor->GetSoundobjectId();
				auto mappingId = processor->GetMappingId();
				if (sourceId != INVALID_ADDRESS_VALUE)
				{
					if (ProcessingEngineConfig::IsRecordAddressingObject(roi) && mappingId != INVALID_ADDRESS_VALUE)
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, mappingId)));
					else if (!ProcessingEngineConfig::IsRecordAddressingObject(roi))
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
				}
			}
		}
	}
	return activeRemoteObjects;
}

/**
 * Helper method to get a list of currently muted remote objects.
 * This is generated by dumping all muted processor properties and their objects to a list.
 * @param	bridgingType	The 3rd party protocol type to get the muted objects for
 * @return	The list of currently muted remote objects.
 */
const std::vector<RemoteObject> Controller::GetMutedSoundObjectRemoteObjects(const ProtocolBridgingType& bridgingType)
{
	std::vector<RemoteObject> mutedRemoteObjects;
	for (auto const& processor : m_soundobjectProcessors)
	{
		if (GetMuteBridgingSoundobjectProcessorId(bridgingType, processor->GetProcessorId()))
		{
			for (auto const& roi : SoundobjectProcessor::GetUsedRemoteObjects())
			{
				auto sourceId = processor->GetSoundobjectId();
				auto mappingId = processor->GetMappingId();
				if (sourceId != INVALID_ADDRESS_VALUE)
				{
					if (ProcessingEngineConfig::IsRecordAddressingObject(roi) && mappingId != INVALID_ADDRESS_VALUE)
						mutedRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, mappingId)));
					else if (!ProcessingEngineConfig::IsRecordAddressingObject(roi))
						mutedRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
				}
			}
		}
	}
	return mutedRemoteObjects;
}

/**
 * Helper method to collect all remote objects that are used by a soundobject processor.
 * @param soundobjectProcessorId		The id of the sound object processor to get the used remote objects for.
 * @return		The list of used remote objects.
 */
std::vector<RemoteObject> Controller::GetSoundobjectProcessorRemoteObjects(SoundobjectProcessorId soundobjectProcessorId)
{
	auto remoteObjects = std::vector<RemoteObject>();
	auto processor = GetSoundobjectProcessor(soundobjectProcessorId);
	for (auto& roi : SoundobjectProcessor::GetUsedRemoteObjects())
	{
		if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
			remoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(processor->GetSoundobjectId(), processor->GetMappingId())));
		else
			remoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(processor->GetSoundobjectId(), INVALID_ADDRESS_VALUE)));
	}

	return remoteObjects;
}

/**
 * Helper method to get a list of currently active remote objects.
 * This is generated by dumping all active processor properties and their objects to a list.
 * @return	The list of currently active remote objects.
 */
const std::vector<RemoteObject> Controller::GetActivatedMatrixInputRemoteObjects()
{
	std::vector<RemoteObject> activeRemoteObjects;
	for (auto const& processor : m_matrixInputProcessors)
	{
		if ((processor->GetComsMode() & CM_Rx) == CM_Rx)
		{
			for (auto const& roi : MatrixInputProcessor::GetUsedRemoteObjects())
			{
				auto sourceId = processor->GetMatrixInputId();
				if (sourceId != INVALID_ADDRESS_VALUE)
				{
					if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
					else if (!ProcessingEngineConfig::IsRecordAddressingObject(roi))
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
				}
			}
		}
	}
	return activeRemoteObjects;
}

/**
 * Helper method to get a list of currently muted remote objects.
 * This is generated by dumping all muted processor properties and their objects to a list.
 * @param	bridgingType	The 3rd party protocol type to get the muted objects for
 * @return	The list of currently muted remote objects.
 */
const std::vector<RemoteObject> Controller::GetMutedMatrixInputRemoteObjects(const ProtocolBridgingType& bridgingType)
{
	std::vector<RemoteObject> mutedRemoteObjects;
	for (auto const& processor : m_matrixInputProcessors)
	{
		if (GetMuteBridgingMatrixInputProcessorId(bridgingType, processor->GetProcessorId()))
		{
			for (auto const& roi : MatrixInputProcessor::GetUsedRemoteObjects())
			{
				auto matrixInputId = processor->GetMatrixInputId();
				if (matrixInputId != INVALID_ADDRESS_VALUE)
					mutedRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(matrixInputId, INVALID_ADDRESS_VALUE)));
			}
		}
	}
	return mutedRemoteObjects;
}

/**
 * Cleanup the soundobject, matrix inputs and outputs mute states in config
 * esp. checking if mute states are present, for which
 * no so/mi/mo is handled any more is of importance here.
 */
void Controller::CleanupMutedObjects()
{
	auto activeBridgingBitmask = GetActiveProtocolBridging();
	for (auto const& bridgingType : ProtocolBridgingTypes)
	{
		if ((activeBridgingBitmask & bridgingType) == bridgingType)
		{
			auto mutedObjects = GetMutedSoundObjectRemoteObjects(bridgingType);
			auto mutedMIObjects = GetMutedMatrixInputRemoteObjects(bridgingType);
			mutedObjects.insert(mutedObjects.end(), mutedMIObjects.begin(), mutedMIObjects.end());
			auto mutedMOObjects = GetMutedMatrixOutputRemoteObjects(bridgingType);
			mutedObjects.insert(mutedObjects.end(), mutedMOObjects.begin(), mutedMOObjects.end());

			m_protocolBridge.SetProtocolRemoteObjectsStateMuted(Controller::GetProtocolIdForProtocolType(bridgingType), mutedObjects);
		}
	}
}

/**
 * Helper method to collect all remote objects that are used by a matrix input processor.
 * @param matrixInputProcessorId		The id of the matrix input processor to get the used remote objects for.
 * @return		The list of used remote objects.
 */
std::vector<RemoteObject> Controller::GetMatrixInputProcessorRemoteObjects(MatrixInputProcessorId matrixInputProcessorId)
{
	auto remoteObjects = std::vector<RemoteObject>();
	auto processor = GetMatrixInputProcessor(matrixInputProcessorId);
	for (auto& roi : MatrixInputProcessor::GetUsedRemoteObjects())
	{
		if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
			jassertfalse; // Matrix Channel processors cannot handle record addressing!
		else
			remoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(processor->GetMatrixInputId(), INVALID_ADDRESS_VALUE)));
	}

	return remoteObjects;
}

/**
 * Helper method to get a list of currently active remote objects.
 * This is generated by dumping all active processor properties and their objects to a list.
 * @return	The list of currently active remote objects.
 */
const std::vector<RemoteObject> Controller::GetActivatedMatrixOutputRemoteObjects()
{
	std::vector<RemoteObject> activeRemoteObjects;
	for (auto const& processor : m_matrixOutputProcessors)
	{
		if ((processor->GetComsMode() & CM_Rx) == CM_Rx)
		{
			for (auto const& roi : MatrixOutputProcessor::GetUsedRemoteObjects())
			{
				auto sourceId = processor->GetMatrixOutputId();
				if (sourceId != INVALID_ADDRESS_VALUE)
				{
					if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
					else if (!ProcessingEngineConfig::IsRecordAddressingObject(roi))
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
				}
			}
		}
	}
	return activeRemoteObjects;
}

/**
 * Helper method to get a list of currently muted remote objects.
 * This is generated by dumping all muted processor properties and their objects to a list.
 * @param	bridgingType	The 3rd party protocol type to get the muted objects for
 * @return	The list of currently muted remote objects.
 */
const std::vector<RemoteObject> Controller::GetMutedMatrixOutputRemoteObjects(const ProtocolBridgingType& bridgingType)
{
	std::vector<RemoteObject> mutedRemoteObjects;
	for (auto const& processor : m_matrixOutputProcessors)
	{
		if (GetMuteBridgingMatrixOutputProcessorId(bridgingType, processor->GetProcessorId()))
		{
			for (auto const& roi : MatrixOutputProcessor::GetUsedRemoteObjects())
			{
				auto matrixOutputId = processor->GetMatrixOutputId();
				if (matrixOutputId != INVALID_ADDRESS_VALUE)
					mutedRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(matrixOutputId, INVALID_ADDRESS_VALUE)));
			}
		}
	}
	return mutedRemoteObjects;
}

/**
 * Helper method to collect all remote objects that are used by a matrix output processor.
 * @param matrixOutputProcessorId		The id of the matrix output processor to get the used remote objects for.
 * @return		The list of used remote objects.
 */
std::vector<RemoteObject> Controller::GetMatrixOutputProcessorRemoteObjects(MatrixOutputProcessorId matrixOutputProcessorId)
{
	auto remoteObjects = std::vector<RemoteObject>();
	auto processor = GetMatrixOutputProcessor(matrixOutputProcessorId);
	for (auto& roi : MatrixOutputProcessor::GetUsedRemoteObjects())
	{
		if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
			jassertfalse; // Matrix Channel processors cannot handle record addressing!
		else
			remoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(processor->GetMatrixOutputId(), INVALID_ADDRESS_VALUE)));
	}

	return remoteObjects;
}

/**
 * Adds a given listener object to this controller instance's bridging wrapper object.
 * @param listener	The listener object to add to bridging wrapper.
 */
void Controller::AddProtocolBridgingWrapperListener(ProtocolBridgingWrapper::Listener* listener)
{
	m_protocolBridge.AddListener(listener);
}

/**
 * Getter for the active protocol bridging types (active protocols RoleB - those are used for bridging to DS100 running as RoleA, see RemoteProtocolBridge for details)
 * @return The bitfield containing all active bridging types
 */
ProtocolBridgingType Controller::GetActiveProtocolBridging()
{
	return m_protocolBridge.GetActiveBridgingProtocols();
}

/**
 * Getter for currently active bridging protocols count.
 * @return The number of currently active bridging protocols.
 */
int Controller::GetActiveProtocolBridgingCount()
{
	auto activeProtocolBridgingCount = 0;
	auto activeBridging = GetActiveProtocolBridging();

	if ((activeBridging & PBT_DiGiCo) == PBT_DiGiCo)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_DAWPlugin) == PBT_DAWPlugin)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_GenericOSC) == PBT_GenericOSC)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_BlacktraxRTTrPM) == PBT_BlacktraxRTTrPM)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_GenericMIDI) == PBT_GenericMIDI)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_DS100) == PBT_DS100)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_YamahaOSC) == PBT_YamahaOSC)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_ADMOSC) == PBT_ADMOSC)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_RemapOSC) == PBT_RemapOSC)
		activeProtocolBridgingCount++;

	return activeProtocolBridgingCount;
}

/**
 * Setter for protocol bridging types that shall be active.
 * @param	bridgingTypes	Bitfield containing all types that are to be active.
 */
void Controller::SetActiveProtocolBridging(ProtocolBridgingType bridgingTypes)
{
	m_protocolBridge.SetActiveBridgingProtocols(bridgingTypes);
}

/**
 * Gets the mute state of the given source via proxy bridge object
 * @param soundobjectProcessorId The id of the soundobject processor for which the mute state shall be returned
 * @return The mute state
 */
bool Controller::GetMuteBridgingSoundobjectProcessorId(ProtocolBridgingType bridgingType, SoundobjectProcessorId soundobjectProcessorId)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	return m_protocolBridge.GetMuteProtocolSoundobjectProcessorId(protocolId, soundobjectProcessorId);
}

/**
 * Sets the given source to be (un-)muted in DiGiCo protocol via proxy bridge object
 * @param soundobjectProcessorId The id of the soundobject processor that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool Controller::SetMuteBridgingSoundobjectProcessorId(ProtocolBridgingType bridgingType, SoundobjectId soundobjectProcessorId, bool mute)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	if (mute)
		return m_protocolBridge.SetMuteProtocolSoundobjectProcessorId(protocolId, soundobjectProcessorId);
	else
		return m_protocolBridge.SetUnmuteProtocolSoundobjectProcessorId(protocolId, soundobjectProcessorId);
}

/**
 * Sets the given sources to be (un-)muted in DiGiCo protocol via proxy bridge object
 * @param soundobjectProcessorId The ids of the soundobject processors that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool Controller::SetMuteBridgingSoundobjectProcessorIds(ProtocolBridgingType bridgingType, const std::vector<SoundobjectProcessorId>& soundobjectProcessorIds, bool mute)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	if (mute)
		return m_protocolBridge.SetMuteProtocolSoundobjectProcessorIds(protocolId, soundobjectProcessorIds);
	else
		return m_protocolBridge.SetUnmuteProtocolSoundobjectProcessorIds(protocolId, soundobjectProcessorIds);
}

/**
 * Gets the mute state of the given matrix input via proxy bridge object
 * @param matrixInputProcessorId The id of the matrix input processor for which the mute state shall be returned
 * @return The mute state
 */
bool Controller::GetMuteBridgingMatrixInputProcessorId(ProtocolBridgingType bridgingType, MatrixInputId matrixInputProcessorId)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	return m_protocolBridge.GetMuteProtocolMatrixInputProcessorId(protocolId, matrixInputProcessorId);
}

/**
 * Sets the given source to be (un-)muted in DiGiCo protocol via proxy bridge object
 * @param matrixInputId The id of the matrix input that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool Controller::SetMuteBridgingMatrixInputProcessorId(ProtocolBridgingType bridgingType, MatrixInputId matrixInputProcessorId, bool mute)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	if (mute)
		return m_protocolBridge.SetMuteProtocolMatrixInputProcessorId(protocolId, matrixInputProcessorId);
	else
		return m_protocolBridge.SetUnmuteProtocolMatrixInputProcessorId(protocolId, matrixInputProcessorId);
}

/**
 * Sets the given sources to be (un-)muted in DiGiCo protocol via proxy bridge object
 * @param matrixInputProcessorIds The ids of the matrix Inputs that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool Controller::SetMuteBridgingMatrixInputProcessorIds(ProtocolBridgingType bridgingType, const std::vector<MatrixInputProcessorId>& matrixInputProcessorIds, bool mute)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	if (mute)
		return m_protocolBridge.SetMuteProtocolMatrixInputProcessorIds(protocolId, matrixInputProcessorIds);
	else
		return m_protocolBridge.SetUnmuteProtocolMatrixInputProcessorIds(protocolId, matrixInputProcessorIds);
}

/**
 * Gets the mute state of the given source via proxy bridge object
 * @param matrixOutputProcessorId The id of the matrix output processor for which the mute state shall be returned
 * @return The mute state
 */
bool Controller::GetMuteBridgingMatrixOutputProcessorId(ProtocolBridgingType bridgingType, MatrixOutputId matrixOutputProcessorId)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	return m_protocolBridge.GetMuteProtocolMatrixOutputProcessorId(protocolId, matrixOutputProcessorId);
}

/**
 * Sets the given source to be (un-)muted in DiGiCo protocol via proxy bridge object
 * @param matrixOutputProcessorId The id of the matrix output procssor that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool Controller::SetMuteBridgingMatrixOutputProcessorId(ProtocolBridgingType bridgingType, MatrixOutputId matrixOutputProcessorId, bool mute)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	if (mute)
		return m_protocolBridge.SetMuteProtocolMatrixOutputProcessorId(protocolId, matrixOutputProcessorId);
	else
		return m_protocolBridge.SetUnmuteProtocolMatrixOutputProcessorId(protocolId, matrixOutputProcessorId);
}

/**
 * Sets the given sources to be (un-)muted in DiGiCo protocol via proxy bridge object
 * @param matrixOutputProcessorIds The ids of the matrix output processors that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool Controller::SetMuteBridgingMatrixOutputProcessorIds(ProtocolBridgingType bridgingType, const std::vector<MatrixOutputProcessorId>& matrixOutputProcessorIds, bool mute)
{
	auto protocolId = GetProtocolIdForProtocolType(bridgingType);

	if (mute)
		return m_protocolBridge.SetMuteProtocolMatrixOutputProcessorIds(protocolId, matrixOutputProcessorIds);
	else
		return m_protocolBridge.SetUnmuteProtocolMatrixOutputProcessorIds(protocolId, matrixOutputProcessorIds);
}

juce::IPAddress Controller::GetBridgingIpAddress(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolIpAddress(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingIpAddress(ProtocolBridgingType bridgingType, juce::IPAddress ipAddress, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolIpAddress(GetProtocolIdForProtocolType(bridgingType), ipAddress, dontSendNotification);
}

int Controller::GetBridgingListeningPort(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolListeningPort(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingListeningPort(ProtocolBridgingType bridgingType, int listeningPort, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolListeningPort(GetProtocolIdForProtocolType(bridgingType), listeningPort, dontSendNotification);
}

int Controller::GetBridgingRemotePort(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolRemotePort(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingRemotePort(ProtocolBridgingType bridgingType, int remotePort, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolRemotePort(GetProtocolIdForProtocolType(bridgingType), remotePort, dontSendNotification);
}

int Controller::GetBridgingMappingArea(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolMappingArea(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingMappingArea(ProtocolBridgingType bridgingType, int mappingAreaId, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolMappingArea(GetProtocolIdForProtocolType(bridgingType), mappingAreaId, dontSendNotification);
}

const juce::Point<float> Controller::GetBridgingOriginOffset(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolOriginOffset(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingOriginOffset(ProtocolBridgingType bridgingType, const juce::Point<float>& origin, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolOriginOffset(GetProtocolIdForProtocolType(bridgingType), origin, dontSendNotification);
}

const std::pair<juce::Range<float>, juce::Range<float>> Controller::GetBridgingMappingRange(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolMappingRange(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingMappingRange(ProtocolBridgingType bridgingType, const std::pair<juce::Range<float>, juce::Range<float>>& mappingRange, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolMappingRange(GetProtocolIdForProtocolType(bridgingType), mappingRange, dontSendNotification);
}

String Controller::GetBridgingInputDeviceIdentifier(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolInputDeviceIdentifier(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingInputDeviceIdentifier(ProtocolBridgingType bridgingType, const String& inputDeviceIdentifier, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolInputDeviceIdentifier(GetProtocolIdForProtocolType(bridgingType), inputDeviceIdentifier, dontSendNotification);
}

String Controller::GetBridgingOutputDeviceIdentifier(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolOutputDeviceIdentifier(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingOutputDeviceIdentifier(ProtocolBridgingType bridgingType, const String& outputDeviceIdentifier, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolOutputDeviceIdentifier(GetProtocolIdForProtocolType(bridgingType), outputDeviceIdentifier, dontSendNotification);
}

JUCEAppBasics::MidiCommandRangeAssignment Controller::GetBridgingMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId)
{
	return m_protocolBridge.GetMidiAssignmentMapping(GetProtocolIdForProtocolType(bridgingType), remoteObjectId);
}

bool Controller::SetBridgingMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId, const JUCEAppBasics::MidiCommandRangeAssignment& assignmentMapping, bool dontSendNotification)
{
	return m_protocolBridge.SetMidiAssignmentMapping(GetProtocolIdForProtocolType(bridgingType), remoteObjectId, assignmentMapping, dontSendNotification);
}

std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> Controller::GetBridgingScenesToMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId)
{
	return m_protocolBridge.GetMidiScenesAssignmentMapping(GetProtocolIdForProtocolType(bridgingType), remoteObjectId);
}

bool Controller::SetBridgingScenesToMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& scenesToMidiAssignmentMapping, bool dontSendNotification)
{
	return m_protocolBridge.SetMidiScenesAssignmentMapping(GetProtocolIdForProtocolType(bridgingType), remoteObjectId, scenesToMidiAssignmentMapping, dontSendNotification);
}

int Controller::GetBridgingXAxisInverted(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolXAxisInverted(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingXAxisInverted(ProtocolBridgingType bridgingType, int inverted, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolXAxisInverted(GetProtocolIdForProtocolType(bridgingType), inverted, dontSendNotification);
}

int Controller::GetBridgingYAxisInverted(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolYAxisInverted(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingYAxisInverted(ProtocolBridgingType bridgingType, int inverted, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolYAxisInverted(GetProtocolIdForProtocolType(bridgingType), inverted, dontSendNotification);
}

int Controller::GetBridgingXYAxisSwapped(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolXYAxisSwapped(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingXYAxisSwapped(ProtocolBridgingType bridgingType, int swapped, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolXYAxisSwapped(GetProtocolIdForProtocolType(bridgingType), swapped, dontSendNotification);
}

bool Controller::GetBridgingXYMessageCombined(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolBridgingXYMessageCombined(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingXYMessageCombined(ProtocolBridgingType bridgingType, bool combined, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolBridgingXYMessageCombined(GetProtocolIdForProtocolType(bridgingType), combined, dontSendNotification);
}

int Controller::GetBridgingDataSendingDisabled(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolDataSendingDisabled(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingDataSendingDisabled(ProtocolBridgingType bridgingType, int disabled, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolDataSendingDisabled(GetProtocolIdForProtocolType(bridgingType), disabled, dontSendNotification);
}

std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>> Controller::GetBridgingOscRemapAssignments(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolOscRemapAssignments(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingOscRemapAssignments(ProtocolBridgingType bridgingType, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& oscRemapAssignments, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolOscRemapAssignments(GetProtocolIdForProtocolType(bridgingType), oscRemapAssignments, dontSendNotification);
}

std::map<int, ChannelId> Controller::GetBridgingChannelRemapAssignments(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolChannelRemapAssignments(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingChannelRemapAssignments(ProtocolBridgingType bridgingType, const std::map<int, ChannelId>& channelRemapAssignments, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolChannelRemapAssignments(GetProtocolIdForProtocolType(bridgingType), channelRemapAssignments, dontSendNotification);
}

const String Controller::GetBridgingModuleTypeIdentifier(ProtocolBridgingType bridgingType)
{
	return m_protocolBridge.GetProtocolModuleTypeIdentifier(GetProtocolIdForProtocolType(bridgingType));
}

bool Controller::SetBridgingModuleTypeIdentifier(ProtocolBridgingType bridgingType, const String& moduleTypeIdentifier, bool dontSendNotification)
{
	return m_protocolBridge.SetProtocolModuleTypeIdentifier(GetProtocolIdForProtocolType(bridgingType), moduleTypeIdentifier, dontSendNotification);
}

/**
 * Method to load a given input file as the new application configuration.
 * This tries to handle possible errors and shows a popup to the user in case an error was detected.
 * @param fileToLoadFrom	The input file to load the new app config from.
 * @return	True on succes, false on failure
 */
bool Controller::LoadConfigurationFile(const File& fileToLoadFrom)
{
    if (!fileToLoadFrom.existsAsFile() || !fileToLoadFrom.hasReadAccess())
    {
		ShowUserErrorNotification(SEC_LoadConfig_CannotAccess);
        return false;
    }
    
	auto config = SpaConBridge::AppConfiguration::getInstance();
	if (!config)
	{
		ShowUserErrorNotification(SEC_LoadConfig_InternalError);
		return false;
	}

	auto xmlConfig = juce::parseXML(fileToLoadFrom);
	if (!xmlConfig)
	{
		ShowUserErrorNotification(SEC_LoadConfig_InvalidFile);
		return false;
	}

	if (!SpaConBridge::AppConfiguration::isValid(xmlConfig))
	{
		ShowUserErrorNotification(SEC_LoadConfig_InvalidConfig);
		return false;
	}

	config->SetFlushAndUpdateDisabled();
	if (!config->resetConfigState(std::move(xmlConfig)))
	{
		ShowUserErrorNotification(SEC_LoadConfig_ConfigInit);
		config->ResetFlushAndUpdateDisabled();
		return false;
	}
	config->ResetFlushAndUpdateDisabled();

	SetParameterChanged(DCP_Init, DCT_AllConfigParameters);
	return true;
}

/**
 * Method to save the current application configuration to a given input file.
 * This tries to handle possible errors and shows a popup to the user in case an error was detected.
 * @param fileToSaveTo	The output file to save the current config to.
 * @return	True on succes, false on failure
 */
bool Controller::SaveConfigurationFile(const File& fileToSaveTo)
{
    if (!fileToSaveTo.hasWriteAccess())
    {
		ShowUserErrorNotification(SEC_SaveConfig_CannotAccess);
        return false;
    }
    
	auto config = SpaConBridge::AppConfiguration::getInstance();
	auto xmlConfig = config->getConfigState();

	if (!config)
		ShowUserErrorNotification(SEC_SaveConfig_InternalError);
	else if (!xmlConfig)
		ShowUserErrorNotification(SEC_SaveConfig_InvalidInternalConfig);
	else if (!xmlConfig->writeTo(fileToSaveTo))
		ShowUserErrorNotification(SEC_SaveConfig_CannotWrite);
	else
		return true;

	return false;
}

} // namespace SpaConBridge
