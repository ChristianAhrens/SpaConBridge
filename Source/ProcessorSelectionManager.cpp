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
 */
void ProcessorSelectionManager::CreateSoundobjectProcessorSelectionGroup(std::string groupName)
{
	if (groupName.empty())
		groupName = "SO Selection " + std::to_string(m_soundobjectProcessorSelectionGroups.size() + 1);

	m_soundobjectProcessorSelectionGroups.insert(std::make_pair(groupName, m_currentSoundobjectProcessorSelection));
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
 */
void ProcessorSelectionManager::CreateMatrixInputProcessorSelectionGroup(std::string groupName)
{
	if (groupName.empty())
		groupName = "MI Selection " + std::to_string(m_matrixInputProcessorSelectionGroups.size() + 1);

	m_matrixInputProcessorSelectionGroups.insert(std::make_pair(groupName, m_currentMatrixInputProcessorSelection));
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
 */
void ProcessorSelectionManager::CreateMatrixOutputProcessorSelectionGroup(std::string groupName)
{
	if (groupName.empty())
		groupName = "MO Selection " + std::to_string(m_matrixOutputProcessorSelectionGroups.size() + 1);

	m_matrixOutputProcessorSelectionGroups.insert(std::make_pair(groupName, m_currentMatrixOutputProcessorSelection));
}

} // namespace SpaConBridge
