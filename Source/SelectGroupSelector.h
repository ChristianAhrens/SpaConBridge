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

#include <JuceHeader.h>
#include <climits>

#include "ProcessorSelectionManager.h"

namespace SpaConBridge
{


/**
 * SelectGroupSelector class provides a dropdown ui component that allows
 * selection of object select groups, managed by SelectionManager.
 */
class SelectGroupSelector  : public ComboBox, ProcessorSelectionManager::Listener
{
public:
	enum Mode
	{
		SoundobjectSelections,
		MatrixInputSelections,
		MatrixOutputSelections,
		Invalid
	};

public:
	SelectGroupSelector(const String& componentName);
	~SelectGroupSelector() override;

	void SetMode(SelectGroupSelector::Mode mode);

	//==============================================================================
	void SoundobjectSelectionChanged(ProcessorSelectionManager::SoundobjectSelectionId selectionId) override;
	void MatrixInputSelectionChanged(ProcessorSelectionManager::MatrixInputSelectionId selectionId) override;
	void MatrixOutputSelectionChanged(ProcessorSelectionManager::MatrixOutputSelectionId selectionId) override;
	void SoundobjectSelectionGroupsChanged() override;
	void MatrixInputSelectionGroupsChanged() override;
	void MatrixOutputSelectionGroupsChanged() override;

private:
	void TriggerStoreCurrentSelection();
	void TriggerClearAllSelections();
	void TriggerRecallSelectionId(int id);

	void RepopulateWithSoundobjectSelectionGroups();
	void RepopulateWithMatrixInputSelectionGroups();
	void RepopulateWithMatrixOutputSelectionGroups();

	//==============================================================================
	static constexpr int s_storeNewGroupId = INT_MAX - 1;
	static constexpr int s_clearAllGroupsId = INT_MAX;

	//==============================================================================
	SelectGroupSelector::Mode m_mode = SelectGroupSelector::Mode::Invalid;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelectGroupSelector)
};


} // namespace SpaConBridge
