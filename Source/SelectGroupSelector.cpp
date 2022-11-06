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


#include "SelectGroupSelector.h"

#include "LookAndFeel.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class SelectGroupSelector
===============================================================================
*/

/**
 * Object constructor.
 */
SelectGroupSelector::SelectGroupSelector(const String& componentName)
	: ComboBox(componentName)
{
	addSeparator();
	addItem("Store current selection", s_storeNewGroupId);

	setTooltip("Recall or store a selection");
	setTextWhenNothingSelected("Recall selection");

	lookAndFeelChanged();

	onChange = [this]() {
		auto selId = getSelectedId();
		if (selId == s_storeNewGroupId)
			TriggerStoreCurrentSelection();
		else if (selId != 0)
			TriggerRecallSelectionId(selId);
	};

	auto selMgr = ProcessorSelectionManager::GetInstance();
	if (selMgr)
		selMgr->AddListener(this);
}

/**
 * Object destructor.
 */
SelectGroupSelector::~SelectGroupSelector()
{
	auto selMgr = ProcessorSelectionManager::GetInstance();
	if (selMgr)
		selMgr->RemoveListener(this);
}

/**
 * Setter for the operation mode (SO, MI, MO)
 * @param	mode The mode to set active.
 */
void SelectGroupSelector::SetMode(SelectGroupSelector::Mode mode)
{
	if (m_mode != mode)
	{
		m_mode = mode;

		switch (m_mode)
		{
		case SoundobjectSelections:
			RepopulateWithSoundobjectSelectionGroups();
			break;
		case MatrixInputSelections:
			RepopulateWithMatrixInputSelectionGroups();
			break;
		case MatrixOutputSelections:
			RepopulateWithMatrixOutputSelectionGroups();
			break;
		case Invalid:
		default:
			jassertfalse;
			break;
		}
	}
}

/**
 * Reimplemented from ProcessorSelectionManager::Listener to process
 * changed soundobject selections.
 * @param	selectionId	The id of the newly selected selection group.
 */
void SelectGroupSelector::SoundobjectSelectionChanged(ProcessorSelectionManager::SoundobjectSelectionId selectionId)
{
	ignoreUnused(selectionId);

	if (m_mode != SelectGroupSelector::Mode::SoundobjectSelections)
		return;
}

/**
 * Reimplemented from ProcessorSelectionManager::Listener to process
 * changed matrix input selections.
 * @param	selectionId	The id of the newly selected selection group.
 */
void SelectGroupSelector::MatrixInputSelectionChanged(ProcessorSelectionManager::MatrixInputSelectionId selectionId)
{
	ignoreUnused(selectionId);

	if (m_mode != SelectGroupSelector::Mode::MatrixInputSelections)
		return;
}

/**
 * Reimplemented from ProcessorSelectionManager::Listener to process
 * changed matrix output selections.
 * @param	selectionId	The id of the newly selected selection group.
 */
void SelectGroupSelector::MatrixOutputSelectionChanged(ProcessorSelectionManager::MatrixOutputSelectionId selectionId)
{
	ignoreUnused(selectionId);

	if (m_mode != SelectGroupSelector::Mode::MatrixOutputSelections)
		return;
}

/**
 * Reimplemented from ProcessorSelectionManager::Listener to process
 * changed soundobject selection groups.
 */
void SelectGroupSelector::SoundobjectSelectionGroupsChanged()
{
	if (m_mode != SelectGroupSelector::Mode::SoundobjectSelections)
		return;

	RepopulateWithSoundobjectSelectionGroups();
}

/**
 * Reimplemented from ProcessorSelectionManager::Listener to process
 * changed matrix input selection groups.
 */
void SelectGroupSelector::MatrixInputSelectionGroupsChanged()
{
	if (m_mode != SelectGroupSelector::Mode::MatrixInputSelections)
		return;

	RepopulateWithMatrixInputSelectionGroups();
}

/**
 * Reimplemented from ProcessorSelectionManager::Listener to process
 * changed matrix output selection groups.
 */
void SelectGroupSelector::MatrixOutputSelectionGroupsChanged()
{
	if (m_mode != SelectGroupSelector::Mode::MatrixOutputSelections)
		return;

	RepopulateWithMatrixOutputSelectionGroups();
}

/**
 * Helper method that is called when the 'store' dropdown menu item is selected.
 */
void SelectGroupSelector::TriggerStoreCurrentSelection()
{
	auto w = std::make_unique<AlertWindow>("Selection Group", "Choose a name for the current selection", MessageBoxIconType::NoIcon).release();
	w->addTextEditor("selGroupName", "");
	w->getTextEditor("selGroupName")->setKeyboardType(TextInputTarget::VirtualKeyboardType::textKeyboard);
	w->addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
	w->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

	// lambda to be called with the result of the modal processor count choise dialog
	auto nameChoiceCallbackFunctionBody = ([w, this](int result)
		{
			if (!w)
				return;

			if (1 == result)
			{
				auto newSelectionGroupName = w->getTextEditorContents("selGroupName").toStdString();

				auto selMgr = ProcessorSelectionManager::GetInstance();
				if (selMgr)
				{
					switch (m_mode)
					{
					case SoundobjectSelections:
						selMgr->CreateSoundobjectProcessorSelectionGroup(newSelectionGroupName);
						break;
					case MatrixInputSelections:
						selMgr->CreateMatrixInputProcessorSelectionGroup(newSelectionGroupName);
						break;
					case MatrixOutputSelections:
						selMgr->CreateMatrixOutputProcessorSelectionGroup(newSelectionGroupName);
						break;
					case Invalid:
					default:
						jassertfalse;
						break;
					}
				}
			}
			else if (0 == result)
			{
				setSelectedId(0);
			}
		});
	auto modalCallback = juce::ModalCallbackFunction::create(nameChoiceCallbackFunctionBody);

	// Run asynchronously
	w->enterModalState(true, modalCallback, true);
}

/**
 * Helper method that is called when a dropdown menu item
 * corresponding to a selection group is selected.
 */
void SelectGroupSelector::TriggerRecallSelectionId(int id)
{
	auto selMgr = ProcessorSelectionManager::GetInstance();
	if (selMgr)
	{
		switch (m_mode)
		{
		case SoundobjectSelections:
			selMgr->RecallSoundobjectProcessorSelectionGroup(ProcessorSelectionManager::SoundobjectSelectionId(id));
			break;
		case MatrixInputSelections:
			selMgr->RecallMatrixInputProcessorSelectionGroup(ProcessorSelectionManager::MatrixInputSelectionId(id));
			break;
		case MatrixOutputSelections:
			selMgr->RecallMatrixOutputProcessorSelectionGroup(ProcessorSelectionManager::MatrixOutputSelectionId(id));
			break;
		case Invalid:
		default:
			jassertfalse;
			break;
		}
	}

	setSelectedId(0);
}

/**
 * Helper method to reset contents and refill with current soundobject select group contents.
 */
void SelectGroupSelector::RepopulateWithSoundobjectSelectionGroups()
{
	jassert(m_mode == SelectGroupSelector::Mode::SoundobjectSelections);

	auto selMgr = ProcessorSelectionManager::GetInstance();
	if (!selMgr)
		return;

	clear();

	auto ids = selMgr->GetSoundobjectProcessorSelectionGroupIds();
	for (auto const& id : ids)
	{
		jassert(0 != id); // zero is not allowed
		jassert(s_storeNewGroupId != id); // new group id is reserved

		auto grpName = selMgr->GetSoundobjectProcessorSelectionGroupName(id);
		if (grpName.empty())
			grpName = "SO Selection Id" + std::to_string(id);

		addItem(grpName, id);
	}

	addSeparator();
	addItem("Store current selection", s_storeNewGroupId);
}

/**
 * Helper method to reset contents and refill with current matrixinput select group contents.
 */
void SelectGroupSelector::RepopulateWithMatrixInputSelectionGroups()
{
	jassert(m_mode == SelectGroupSelector::Mode::MatrixInputSelections);

	auto selMgr = ProcessorSelectionManager::GetInstance();
	if (!selMgr)
		return;

	clear();

	auto ids = selMgr->GetMatrixInputProcessorSelectionGroupIds();
	for (auto const& id : ids)
	{
		jassert(0 != id); // zero is not allowed
		jassert(s_storeNewGroupId != id); // new group id is reserved

		auto grpName = selMgr->GetMatrixInputProcessorSelectionGroupName(id);
		if (grpName.empty())
			grpName = "MI Selection Id" + std::to_string(id);

		addItem(grpName, id);
	}

	addSeparator();
	addItem("Store current selection", s_storeNewGroupId);
}

/**
 * Helper method to reset contents and refill with current matrixoutput select group contents.
 */
void SelectGroupSelector::RepopulateWithMatrixOutputSelectionGroups()
{
	jassert(m_mode == SelectGroupSelector::Mode::MatrixOutputSelections);

	auto selMgr = ProcessorSelectionManager::GetInstance();
	if (!selMgr)
		return;

	clear();

	auto ids = selMgr->GetMatrixOutputProcessorSelectionGroupIds();
	for (auto const& id : ids)
	{
		jassert(0 != id); // zero is not allowed
		jassert(s_storeNewGroupId != id); // new group id is reserved

		auto grpName = selMgr->GetMatrixOutputProcessorSelectionGroupName(id);
		if (grpName.empty())
			grpName = "MO Selection Id" + std::to_string(id);

		addItem(grpName, id);
	}

	addSeparator();
	addItem("Store current selection", s_storeNewGroupId);
}


} // namespace SpaConBridge
