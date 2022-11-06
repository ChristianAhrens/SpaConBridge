/* Copyright (c) 2022, Christian Ahrens
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

#include "ProcessorSelectionManager.h"

#include "Controller.h"

#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessor.h"


namespace SpaConBridge
{

/*
===============================================================================
 Class ProcessorSelectionManager
===============================================================================
*/

/**
 * The one and only instance of ProcessorSelectionManager.
 */
std::unique_ptr<ProcessorSelectionManager> ProcessorSelectionManager::s_singleton;

/**
 * Constructs an ProcessorSelectionManager object.
 */
ProcessorSelectionManager::ProcessorSelectionManager()
{
	jassert(!s_singleton);	// only one instnce allowed!!
	s_singleton = std::unique_ptr<ProcessorSelectionManager>(this);
}

/**
 * Destroys the ProcessorSelectionManager.
 */
ProcessorSelectionManager::~ProcessorSelectionManager()
{
	DestroyInstance();
}

/**
 * Returns the one and only instance of ProcessorSelectionManager. If it doesn't exist yet, it is created.
 * @return The ProcessorSelectionManager singleton object.
 * @sa m_singleton, ProcessorSelectionManager
 */
ProcessorSelectionManager* ProcessorSelectionManager::GetInstance()
{
	if (!s_singleton)
	{
		new ProcessorSelectionManager();
		jassert(s_singleton);
	}
	return s_singleton.get();
}

/**
 * Triggers destruction of ProcessorSelectionManager singleton object
 */
void ProcessorSelectionManager::DestroyInstance()
{
	s_singleton.reset();
}

/**
 * Adds a listener object to the internal list of objects to be notified of changes.
 * @param	listener	The object to be added to internal list of listeners.
 * @return	True on successful adding, false if already present in internal list.
 */
bool ProcessorSelectionManager::AddListener(ProcessorSelectionManager::Listener* listener)
{
	if (std::find(m_listeners.begin(), m_listeners.end(), listener) == m_listeners.end())
		m_listeners.push_back(listener);
	else
		return false;

	return true;
}

/**
 * Removes a listener object from the internal list of objects to be notified of changes.
 * @param	listener	The object to be removed from internal list of listeners.
 * @return	True on successful removal, false if not found in internal list.
 */
bool ProcessorSelectionManager::RemoveListener(ProcessorSelectionManager::Listener* listener)
{
	auto listenerIter = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if (listenerIter == m_listeners.end())
		return false;
	else
		m_listeners.erase(listenerIter);

	return true;
}

/**
 * Method to set a list of soundsource ids to be selected, based on given list of processorIds.
 * This affects the internal map of soundsource select states and triggers setting/updating table/multislider pages.
 * The additional bool is used to indicate if the current selection shall be extended or cleared and be replaced by new selection.
 * @param processorIds	The list of processorIds to use to set internal map of soundsourceids selected state
 * @param clearPrevSelection	Use to indicate if previously active selection shall be replaced or extended.
 */
void ProcessorSelectionManager::SetSelectedSoundobjectProcessorIds(const std::vector<SoundobjectProcessorId>& processorIds, bool clearPrevSelection)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (clearPrevSelection)
	{
		// clear all selected soundobject ids
		m_currentSoundobjectProcessorSelection.clear();

		// iterate through all processors and set each selected state based on given selection list
		for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
		{
			SetSoundobjectProcessorIdSelectState(processorId, std::find(processorIds.begin(), processorIds.end(), processorId) != processorIds.end());
		}
	}
	else
	{
		// iterate through selection list and set all contained processor ids to selected
		for (auto const& processorId : processorIds)
		{
			SetSoundobjectProcessorIdSelectState(processorId, true);
		}
	}
}

/**
 * Method to get the list of currently selected processors.
 * This internally accesses the list of processors and selected soundsourceids and combines the info in new list.
 * @return The list of currently selected processors.
 */
const std::vector<SoundobjectProcessorId> ProcessorSelectionManager::GetSelectedSoundobjectProcessorIds()
{
	std::vector<SoundobjectProcessorId> processorIds;

	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return processorIds;

	processorIds.reserve(m_currentSoundobjectProcessorSelection.size());
	for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
	{
		auto const& processor = ctrl->GetSoundobjectProcessor(processorId);
		auto soundobjectProcessorId = processor->GetProcessorId();
		if ((m_currentSoundobjectProcessorSelection.count(soundobjectProcessorId) > 0) && m_currentSoundobjectProcessorSelection.at(soundobjectProcessorId))
			processorIds.push_back(soundobjectProcessorId);
	}

	return processorIds;
}

/**
 * Method to set a soundsource to be selected. This affects the internal map of soundsource select states
 * and triggers setting/updating table/multislider pages.
 * @param soundobjectProcessorId	The soundobjectProcessorId to modify regarding selected state
 * @param selected	The selected state to set.
 */
void ProcessorSelectionManager::SetSoundobjectProcessorIdSelectState(SoundobjectProcessorId soundobjectProcessorId, bool selected)
{
	m_currentSoundobjectProcessorSelection[soundobjectProcessorId] = selected;
}

/**
 * Method to get a soundsource id selected state.
 * @param soundobjectProcessorId	The sourceId to modify regarding selected state
 * @param selected	The selected state to set.
 */
bool ProcessorSelectionManager::IsSoundobjectProcessorIdSelected(SoundobjectProcessorId soundobjectProcessorId)
{
	if (m_currentSoundobjectProcessorSelection.count(soundobjectProcessorId) > 0)
		return m_currentSoundobjectProcessorSelection.at(soundobjectProcessorId);
	else
		return false;
}

/**
 * Method to create a new SO selection group from the currently selected processors with the given name.
 * If the given name is empty, a default containing the group number is created.
 * @param	groupName	The string to use as name for the new group.
 * @return	The id of the newly created selection group.
 */
const ProcessorSelectionManager::SoundobjectSelectionId ProcessorSelectionManager::CreateSoundobjectProcessorSelectionGroup(std::string groupName)
{
	if (groupName.empty())
		groupName = "SO Selection " + std::to_string(m_soundobjectProcessorSelectionGroups.size() + 1);

	auto newId = SoundobjectSelectionId(1);
	for (auto const& soSelGrp : m_soundobjectProcessorSelectionGroups)
		if (soSelGrp.first >= newId)
			newId = soSelGrp.first + 1;

	m_soundobjectProcessorSelectionGroups.insert(std::make_pair(newId, m_currentSoundobjectProcessorSelection));
	m_soundobjectProcessorSelectionGroupNames.insert(std::make_pair(newId, groupName));

	for (auto const& listener : m_listeners)
		listener->SoundobjectSelectionGroupsChanged();

	return newId;
}

/**
 * Method to recall a given soundobject selection group.
 * @param	selectionId		The id of the selection group to recall.
 * @return	True on success, false if given id is invalid.
 */
bool ProcessorSelectionManager::RecallSoundobjectProcessorSelectionGroup(ProcessorSelectionManager::SoundobjectSelectionId selectionId)
{
	if (m_soundobjectProcessorSelectionGroups.count(selectionId) == 0)
		return false;

	for (auto const& procSelState : m_soundobjectProcessorSelectionGroups.at(selectionId))
		SetSoundobjectProcessorIdSelectState(procSelState.first, procSelState.second);

	auto ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetParameterChanged(DCP_SoundobjectProcessor, DCT_ProcessorSelection);

	return true;
}

/**
 * Method to get the name of a given soundobject selection group.
 * @param	selectionId		The id of the selection group to get the name for.
 * @return	The name of the group or empty string if not found.
 */
const std::string ProcessorSelectionManager::GetSoundobjectProcessorSelectionGroupName(ProcessorSelectionManager::SoundobjectSelectionId selectionId)
{
	if (m_soundobjectProcessorSelectionGroupNames.count(selectionId) == 0)
		return std::string();
	else
		return m_soundobjectProcessorSelectionGroupNames.at(selectionId);
}

/**
 * Method to get a list of currently used soundobject selection group ids.
 * @return	The list of currently used selection group ids.
 */
const std::vector<ProcessorSelectionManager::SoundobjectSelectionId> ProcessorSelectionManager::GetSoundobjectProcessorSelectionGroupIds()
{
	auto selGrpIds = std::vector<SoundobjectSelectionId>();
	for (auto const& selectionGroups : m_soundobjectProcessorSelectionGroups)
		selGrpIds.push_back(selectionGroups.first);

	return selGrpIds;
}

/**
 * Method to set a list of soundsource ids to be selected, based on given list of processorIds.
 * This affects the internal map of soundsource select states and triggers setting/updating table/multislider pages.
 * The additional bool is used to indicate if the current selection shall be extended or cleared and be replaced by new selection.
 * @param processorIds	The list of processorIds to use to set internal map of matrixInput selected state
 * @param clearPrevSelection	Use to indicate if previously active selection shall be replaced or extended.
 */
void ProcessorSelectionManager::SetSelectedMatrixInputProcessorIds(const std::vector<MatrixInputProcessorId>& processorIds, bool clearPrevSelection)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (clearPrevSelection)
	{
		// clear all selected soundobject ids
		m_currentMatrixInputProcessorSelection.clear();

		// iterate through all processors and set each selected state based on given selection list
		for (auto const& processorId : ctrl->GetMatrixInputProcessorIds())
		{
			SetMatrixInputProcessorIdSelectState(processorId, std::find(processorIds.begin(), processorIds.end(), processorId) != processorIds.end());
		}
	}
	else
	{
		// iterate through selection list and set all contained processor ids to selected
		for (auto const& processorId : processorIds)
		{
			SetMatrixInputProcessorIdSelectState(processorId, true);
		}
	}
}

/**
 * Method to get the list of currently selected processors.
 * This internally accesses the list of processors and selected MatrixChannelids and combines the info in new list.
 * @return The list of currently selected processors.
 */
const std::vector<MatrixInputProcessorId> ProcessorSelectionManager::GetSelectedMatrixInputProcessorIds()
{
	std::vector<MatrixInputProcessorId> processorIds;
	
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return processorIds;

	processorIds.reserve(m_currentMatrixInputProcessorSelection.size());
	for (auto const& processorId : ctrl->GetMatrixInputProcessorIds())
	{
		auto const& processor = ctrl->GetMatrixInputProcessor(processorId);
		auto sourceId = processor->GetMatrixInputId();
		if ((m_currentMatrixInputProcessorSelection.count(sourceId) > 0) && m_currentMatrixInputProcessorSelection.at(sourceId))
			processorIds.push_back(processor->GetProcessorId());
	}

	return processorIds;
}

/**
 * Method to set a MatrixChannel to be selected. This affects the internal map of MatrixChannel select states
 * and triggers setting/updating MatrixChannel pages.
 * @param sourceId	The sourceId to modify regarding selected state
 * @param selected	The selected state to set.
 */
void ProcessorSelectionManager::SetMatrixInputProcessorIdSelectState(MatrixInputProcessorId matrixInputProcessorId, bool selected)
{
	m_currentMatrixInputProcessorSelection[matrixInputProcessorId] = selected;
}

/**
 * Method to get a MatrixInput id selected state.
 * @param matrixInputProcessorId	The id to modify regarding selected state
 * @param selected	The selected state to set.
 */
bool ProcessorSelectionManager::IsMatrixInputProcessorIdSelected(MatrixInputProcessorId matrixInputProcessorId)
{
	if (m_currentMatrixInputProcessorSelection.count(matrixInputProcessorId) > 0)
		return m_currentMatrixInputProcessorSelection.at(matrixInputProcessorId);
	else
		return false;
}

/**
 * Method to create a new MI selection group from the currently selected processors with the given name.
 * If the given name is empty, a default containing the group number is created.
 * @param	groupName	The string to use as name for the new group.
 * @return	The id of the newly created selection group.
 */
const ProcessorSelectionManager::MatrixInputSelectionId ProcessorSelectionManager::CreateMatrixInputProcessorSelectionGroup(std::string groupName)
{
	if (groupName.empty())
		groupName = "MI Selection " + std::to_string(m_matrixInputProcessorSelectionGroups.size() + 1);

	auto newId = MatrixInputSelectionId(1);
	for (auto const& miSelGrp : m_matrixInputProcessorSelectionGroups)
		if (miSelGrp.first >= newId)
			newId = miSelGrp.first + 1;

	m_matrixInputProcessorSelectionGroups.insert(std::make_pair(newId, m_currentMatrixInputProcessorSelection));
	m_matrixInputProcessorSelectionGroupNames.insert(std::make_pair(newId, groupName));

	for (auto const& listener : m_listeners)
		listener->MatrixInputSelectionGroupsChanged();

	return newId;
}

/**
 * Method to recall a given matrix input selection group.
 * @param	selectionId		The id of the selection group to recall.
 * @return	True on success, false if given id is invalid.
 */
bool ProcessorSelectionManager::RecallMatrixInputProcessorSelectionGroup(ProcessorSelectionManager::MatrixInputSelectionId selectionId)
{
	if (m_matrixInputProcessorSelectionGroups.count(selectionId) == 0)
		return false;

	for (auto const& procSelState : m_matrixInputProcessorSelectionGroups.at(selectionId))
		SetMatrixInputProcessorIdSelectState(procSelState.first, procSelState.second);

	auto ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetParameterChanged(DCP_MatrixInputProcessor, DCT_ProcessorSelection);

	return true;
}

/**
 * Method to get the name of a given matrix input selection group.
 * @param	selectionId		The id of the selection group to get the name for.
 * @return	The name of the group or empty string if not found.
 */
const std::string ProcessorSelectionManager::GetMatrixInputProcessorSelectionGroupName(ProcessorSelectionManager::MatrixInputSelectionId selectionId)
{
	if (m_matrixInputProcessorSelectionGroupNames.count(selectionId) == 0)
		return std::string();
	else
		return m_matrixInputProcessorSelectionGroupNames.at(selectionId);
}

/**
 * Method to get a list of currently used matrix input selection group ids.
 * @return	The list of currently used selection group ids.
 */
const std::vector<ProcessorSelectionManager::MatrixInputSelectionId> ProcessorSelectionManager::GetMatrixInputProcessorSelectionGroupIds()
{
	auto selGrpIds = std::vector<MatrixInputSelectionId>();
	for (auto const& selectionGroups : m_matrixInputProcessorSelectionGroupNames)
		selGrpIds.push_back(selectionGroups.first);

	return selGrpIds;
}

/**
 * Method to set a list of soundsource ids to be selected, based on given list of processorIds.
 * This affects the internal map of soundsource select states and triggers setting/updating table/multislider pages.
 * The additional bool is used to indicate if the current selection shall be extended or cleared and be replaced by new selection.
 * @param processorIds	The list of processorIds to use to set internal map of soundsourceids selected state
 * @param clearPrevSelection	Use to indicate if previously active selection shall be replaced or extended.
 */
void ProcessorSelectionManager::SetSelectedMatrixOutputProcessorIds(const std::vector<MatrixOutputProcessorId>& processorIds, bool clearPrevSelection)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (clearPrevSelection)
	{
		// clear all selected soundobject ids
		m_currentMatrixOutputProcessorSelection.clear();

		// iterate through all processors and set each selected state based on given selection list
		for (auto const& processorId : ctrl->GetMatrixOutputProcessorIds())
		{
			SetMatrixOutputProcessorIdSelectState(processorId, std::find(processorIds.begin(), processorIds.end(), processorId) != processorIds.end());
		}
	}
	else
	{
		// iterate through selection list and set all contained processor ids to selected
		for (auto const& processorId : processorIds)
		{
			SetMatrixOutputProcessorIdSelectState(processorId, true);
		}
	}
}

/**
 * Method to get the list of currently selected processors.
 * This internally accesses the list of processors and selected MatrixChannelids and combines the info in new list.
 * @return The list of currently selected processors.
 */
const std::vector<MatrixOutputProcessorId> ProcessorSelectionManager::GetSelectedMatrixOutputProcessorIds()
{
	std::vector<MatrixOutputProcessorId> processorIds;
	
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return processorIds;

	processorIds.reserve(m_currentMatrixOutputProcessorSelection.size());
	for (auto const& processorId : ctrl->GetMatrixOutputProcessorIds())
	{
		auto const& processor = ctrl->GetMatrixOutputProcessor(processorId);
		auto matrixOutputProcessorId = processor->GetProcessorId();
		if ((m_currentMatrixOutputProcessorSelection.count(matrixOutputProcessorId) > 0) && m_currentMatrixOutputProcessorSelection.at(matrixOutputProcessorId))
			processorIds.push_back(matrixOutputProcessorId);
	}

	return processorIds;
}

/**
 * Method to set a MatrixChannel to be selected. This affects the internal map of MatrixChannel select states
 * and triggers setting/updating MatrixChannel pages.
 * @param matrixOutputProcessorId	The sourceId to modify regarding selected state
 * @param selected	The selected state to set.
 */
void ProcessorSelectionManager::SetMatrixOutputProcessorIdSelectState(MatrixOutputProcessorId matrixOutputProcessorId, bool selected)
{
	m_currentMatrixOutputProcessorSelection[matrixOutputProcessorId] = selected;
}

/**
 * Method to get a MatrixChannel id selected state.
 * @param matrixOutputProcessorId	The sourceId to modify regarding selected state
 * @param selected	The selected state to set.
 */
bool ProcessorSelectionManager::IsMatrixOutputProcessorIdSelected(MatrixOutputProcessorId matrixOutputProcessorId)
{
	if (m_currentMatrixOutputProcessorSelection.count(matrixOutputProcessorId) > 0)
		return m_currentMatrixOutputProcessorSelection.at(matrixOutputProcessorId);
	else
		return false;
}

/**
 * Method to create a new MO selection group from the currently selected processors with the given name.
 * If the given name is empty, a default containing the group number is created.
 * @param	groupName	The string to use as name for the new group.
 * @return	The id of the newly created selection group.
 */
const ProcessorSelectionManager::MatrixOutputSelectionId ProcessorSelectionManager::CreateMatrixOutputProcessorSelectionGroup(std::string groupName)
{
	if (groupName.empty())
		groupName = "MO Selection " + std::to_string(m_matrixOutputProcessorSelectionGroups.size() + 1);

	auto newId = MatrixOutputSelectionId(1);
	for (auto const& moSelGrp : m_matrixOutputProcessorSelectionGroups)
		if (moSelGrp.first >= newId)
			newId = moSelGrp.first + 1;

	m_matrixOutputProcessorSelectionGroups.insert(std::make_pair(newId, m_currentMatrixOutputProcessorSelection));
	m_matrixOutputProcessorSelectionGroupNames.insert(std::make_pair(newId, groupName));

	for (auto const& listener : m_listeners)
		listener->MatrixOutputSelectionGroupsChanged();

	return newId;
}

/**
 * Method to recall a given matrix output selection group.
 * @param	selectionId		The id of the selection group to recall.
 * @return	True on success, false if given id is invalid.
 */
bool ProcessorSelectionManager::RecallMatrixOutputProcessorSelectionGroup(ProcessorSelectionManager::MatrixOutputSelectionId selectionId)
{
	if (m_matrixOutputProcessorSelectionGroups.count(selectionId) == 0)
		return false;

	for (auto const& procSelState : m_matrixOutputProcessorSelectionGroups.at(selectionId))
		SetMatrixOutputProcessorIdSelectState(procSelState.first, procSelState.second);

	auto ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetParameterChanged(DCP_MatrixOutputProcessor, DCT_ProcessorSelection);

	return true;
}

/**
 * Method to get the name of a given matrix output selection group.
 * @param	selectionId		The id of the selection group to get the name for.
 * @return	The name of the group or empty string if not found.
 */
const std::string ProcessorSelectionManager::GetMatrixOutputProcessorSelectionGroupName(ProcessorSelectionManager::MatrixOutputSelectionId selectionId)
{
	if (m_matrixOutputProcessorSelectionGroupNames.count(selectionId) == 0)
		return std::string();
	else
		return m_matrixOutputProcessorSelectionGroupNames.at(selectionId);
}

/**
 * Method to get a list of currently used matrix output selection group ids.
 * @return	The list of currently used selection group ids.
 */
const std::vector<ProcessorSelectionManager::MatrixOutputSelectionId> ProcessorSelectionManager::GetMatrixOutputProcessorSelectionGroupIds()
{
	auto selGrpIds = std::vector<MatrixOutputSelectionId>();
	for (auto const& selectionGroups : m_matrixOutputProcessorSelectionGroupNames)
		selGrpIds.push_back(selectionGroups.first);

	return selGrpIds;
}


} // namespace SpaConBridge
