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
#pragma once

#include "SpaConBridgeCommon.h"

#include "AppConfiguration.h"


namespace SpaConBridge
{


/**
 * Helper class to encapsulate all processor related selection handling.
 * This is a singleton class, so only one instance can exist at a time and can be accessed
 * from throughout the app by calling GetInstance().
 */
class ProcessorSelectionManager :
	public AppConfiguration::XmlConfigurableElement
{
public:
	typedef int SoundobjectSelectionId;
	typedef int MatrixInputSelectionId;
	typedef int MatrixOutputSelectionId;

	class Listener
	{
	public:
		//==============================================================================
		virtual ~Listener() = default;

		//==============================================================================
		virtual void SoundobjectSelectionChanged(SoundobjectSelectionId selectionId) = 0; /**> Called when a soundobject selection group is activated. */
		virtual void MatrixInputSelectionChanged(MatrixInputSelectionId selectionId) = 0; /**> Called when a matrix input selection group is activated. */
		virtual void MatrixOutputSelectionChanged(MatrixOutputSelectionId selectionId) = 0; /**> Called when a matrix output selection group is activated. */

		//==============================================================================
		virtual void SoundobjectSelectionGroupsChanged() = 0; /**> Called when soundobject selection groups are changed, e.g. one is added. */
		virtual void MatrixInputSelectionGroupsChanged() = 0; /**> Called when matrix input selection groups are changed, e.g. one is added. */
		virtual void MatrixOutputSelectionGroupsChanged() = 0; /**> Called when matrix output selection groups are changed, e.g. one is added. */
	};

public:
	ProcessorSelectionManager();
	~ProcessorSelectionManager();
	static ProcessorSelectionManager* GetInstance();
	void DestroyInstance();

	//==========================================================================
	bool AddListener(ProcessorSelectionManager::Listener* listener);
	bool RemoveListener(ProcessorSelectionManager::Listener* listener);

	//==========================================================================
	void SetSelectedSoundobjectProcessorIds(const std::vector<SoundobjectProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<SoundobjectProcessorId> GetSelectedSoundobjectProcessorIds();
	void SetSoundobjectProcessorIdSelectState(SoundobjectProcessorId soundobjectProcessorId, bool selected);
	bool IsSoundobjectProcessorIdSelected(SoundobjectProcessorId soundobjectProcessorId);

	const SoundobjectSelectionId CreateSoundobjectProcessorSelectionGroup(std::string groupName = std::string());
	bool RecallSoundobjectProcessorSelectionGroup(SoundobjectSelectionId selectionId);
	const std::string GetSoundobjectProcessorSelectionGroupName(SoundobjectSelectionId selectionId);
	const std::vector<SoundobjectSelectionId> GetSoundobjectProcessorSelectionGroupIds();
	void ClearSoundobjectProcessorSelectionGroups();

	//==========================================================================
	void SetSelectedMatrixInputProcessorIds(const std::vector<MatrixInputProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<MatrixInputProcessorId> GetSelectedMatrixInputProcessorIds();
	void SetMatrixInputProcessorIdSelectState(MatrixInputProcessorId matrixInputProcessorId, bool selected);
	bool IsMatrixInputProcessorIdSelected(MatrixInputProcessorId matrixInputProcessorId);

	const MatrixInputSelectionId CreateMatrixInputProcessorSelectionGroup(std::string groupName = std::string());
	bool RecallMatrixInputProcessorSelectionGroup(MatrixInputSelectionId selectionId);
	const std::string GetMatrixInputProcessorSelectionGroupName(MatrixInputSelectionId selectionId);
	const std::vector<MatrixInputSelectionId> GetMatrixInputProcessorSelectionGroupIds();
	void ClearMatrixInputProcessorSelectionGroups();

	//==========================================================================
	void SetSelectedMatrixOutputProcessorIds(const std::vector<MatrixOutputProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<MatrixOutputProcessorId> GetSelectedMatrixOutputProcessorIds();
	void SetMatrixOutputProcessorIdSelectState(MatrixOutputProcessorId matrixOutputProcessorId, bool selected);
	bool IsMatrixOutputProcessorIdSelected(MatrixOutputProcessorId matrixOutputProcessorId);

	const MatrixOutputSelectionId CreateMatrixOutputProcessorSelectionGroup(std::string groupName = std::string());
	bool RecallMatrixOutputProcessorSelectionGroup(MatrixOutputSelectionId selectionId);
	const std::string GetMatrixOutputProcessorSelectionGroupName(MatrixOutputSelectionId selectionId);
	const std::vector<MatrixOutputSelectionId> GetMatrixOutputProcessorSelectionGroupIds();
	void ClearMatrixOutputProcessorSelectionGroups();

	//==========================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

protected:
	static std::unique_ptr<ProcessorSelectionManager>	s_singleton;	/**< The one and only instance of ProcessorSelectionManager. */

private:
	std::vector<ProcessorSelectionManager::Listener*>	m_listeners;	/**< The listener objects currently active and to be notified of changes. */

	std::map<SoundobjectProcessorId, bool>	m_currentSoundobjectProcessorSelection;		/**< The current select state of sound objects. */
	std::map<MatrixInputProcessorId, bool>	m_currentMatrixInputProcessorSelection;		/**< The current select state of matrix inputs. */
	std::map<MatrixOutputProcessorId, bool>	m_currentMatrixOutputProcessorSelection;	/**< The current select state of matrix outputs. */

	std::map<SoundobjectSelectionId, std::map<SoundobjectProcessorId, bool>> m_soundobjectProcessorSelectionGroups;
	std::map<MatrixInputSelectionId, std::map<MatrixInputProcessorId, bool>> m_matrixInputProcessorSelectionGroups;
	std::map<MatrixOutputSelectionId, std::map<MatrixOutputProcessorId, bool>> m_matrixOutputProcessorSelectionGroups;
	std::map<SoundobjectSelectionId, std::string> m_soundobjectProcessorSelectionGroupNames;
	std::map<MatrixInputSelectionId, std::string> m_matrixInputProcessorSelectionGroupNames;
	std::map<MatrixOutputSelectionId, std::string> m_matrixOutputProcessorSelectionGroupNames;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorSelectionManager)
};


} // namespace SpaConBridge
