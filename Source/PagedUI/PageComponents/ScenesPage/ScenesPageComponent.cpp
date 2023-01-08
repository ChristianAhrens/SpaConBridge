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


#include "ScenesPageComponent.h"

#include "../../../Controller.h"
#include "../../../LookAndFeel.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class ScenesPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
ScenesPageComponent::ScenesPageComponent()
	: StandalonePollingPageComponentBase(UIPageId::UPI_Scenes)
{
	AddStandalonePollingObject(ROI_Scene_SceneIndex, RemoteObjectAddressing());
	AddStandalonePollingObject(ROI_Scene_SceneName, RemoteObjectAddressing());
	AddStandalonePollingObject(ROI_Scene_SceneComment, RemoteObjectAddressing());

	if (GetElementsContainer())
		GetElementsContainer()->setHeaderText("Scenes");

	// Previous / Next buttons wrapped in horizontal layouting container
	m_prevNextLayoutContainer = std::make_unique<HorizontalLayouterComponent>();
	m_prevNextLayoutContainer->SetSpacing(5);
	if (GetElementsContainer())
		GetElementsContainer()->addComponent(m_prevNextLayoutContainer.get(), true, false);
	
	m_previousButton = std::make_unique<JUCEAppBasics::TextWithImageButton>();
	m_previousButton->setButtonText("Previous");
	m_previousButton->setTooltip("Recall Previous Scene");
	m_previousButton->setImagePosition(Justification::centredLeft);
	m_previousButton->addListener(this);
	m_prevNextLayoutContainer->AddComponent(m_previousButton.get());
	m_nextButton = std::make_unique<JUCEAppBasics::TextWithImageButton>();
	m_nextButton->setButtonText("Next");
	m_nextButton->setTooltip("Recall Next Scene");
	m_nextButton->setImagePosition(Justification::centredLeft);
	m_nextButton->addListener(this);
	m_prevNextLayoutContainer->AddComponent(m_nextButton.get());

	// scene index editor and recall button wrapped in horizontal layouting container
	m_recallIdxLayoutContainer = std::make_unique<HorizontalLayouterComponent>();
	m_recallIdxLayoutContainer->SetSpacing(5);
	if (GetElementsContainer())
		GetElementsContainer()->addComponent(m_recallIdxLayoutContainer.get(), true, false);
	m_recallIdxSubLayoutContainer = std::make_unique<HorizontalLayouterComponent>();
	m_recallIdxSubLayoutContainer->SetSpacing(5);

	m_sceneIdxFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(6, "1234567890.");
	m_sceneIdxEdit = std::make_unique<TextEditor>();
	m_sceneIdxEdit->addListener(this);
	m_sceneIdxEdit->setInputFilter(m_sceneIdxFilter.get(), false);
	m_recallIdxLayoutContainer->AddComponent(m_sceneIdxEdit.get(), 1);
	m_recallButton = std::make_unique<TextButton>();
	m_recallButton->setButtonText("Recall");
	m_recallButton->addListener(this);
	m_recallButton->setTooltip("Recall Scene Index");
	m_recallIdxSubLayoutContainer->AddComponent(m_recallButton.get(), 3);
	m_pinSceneIdxRecallButton = std::make_unique<DrawableButton>("pin scene", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_pinSceneIdxRecallButton->setClickingTogglesState(false);
	m_pinSceneIdxRecallButton->addListener(this);
	m_pinSceneIdxRecallButton->setTooltip("Pin Scene Index");
	m_recallIdxSubLayoutContainer->AddComponent(m_pinSceneIdxRecallButton.get(), 1);
	m_recallIdxLayoutContainer->AddComponent(m_recallIdxSubLayoutContainer.get(), 1);

	// scene name and comment as full-width elements, comment with special height
	m_sceneNameEdit = std::make_unique<TextEditor>();
	m_sceneNameEdit->setReadOnly(true);
	m_sceneNameLabel = std::make_unique<Label>();
	m_sceneNameLabel->setJustificationType(Justification::centred);
	m_sceneNameLabel->setText("Name", dontSendNotification);
	m_sceneNameLabel->attachToComponent(m_sceneNameEdit.get(), true);
	if (GetElementsContainer())
	{
		GetElementsContainer()->addComponent(m_sceneNameLabel.get(), false, false);
		GetElementsContainer()->addComponent(m_sceneNameEdit.get(), true, false);
	}
	m_sceneCommentEdit = std::make_unique<TextEditor>();
	m_sceneCommentEdit->setReadOnly(true);
	m_sceneCommentEdit->setMultiLine(true, true);
	m_sceneCommentLabel = std::make_unique<Label>();
	m_sceneCommentLabel->setJustificationType(Justification::centredTop);
	m_sceneCommentLabel->setText("Comment", dontSendNotification);
	m_sceneCommentLabel->attachToComponent(m_sceneCommentEdit.get(), true);
	if (GetElementsContainer())
	{
		GetElementsContainer()->addComponent(m_sceneCommentLabel.get(), false, false);
		GetElementsContainer()->addComponent(m_sceneCommentEdit.get(), true, false, 3);
	}

	m_pinnedSceneIdxRecallLabel = std::make_unique<Label>();
	m_pinnedSceneIdxRecallLabel->setJustificationType(Justification::centred);
	m_pinnedSceneIdxRecallLabel->setText("Pinned Scenes", dontSendNotification);
	if (GetElementsContainer())
		GetElementsContainer()->addComponent(m_pinnedSceneIdxRecallLabel.get(), false, false);

	lookAndFeelChanged();

	resized();
}

/**
 * Class destructor.
 */
ScenesPageComponent::~ScenesPageComponent()
{
}

/**
 * Reimplemented to resize elements container component.
 */
void ScenesPageComponent::resized()
{
	// update the sizing of the embedded viewport contents
	if (GetElementsContainer())
		GetElementsContainer()->resized();

	StandalonePollingPageComponentBase::resized();
}

/**
 * Helper method to get the current scene index as is set as text in idx editor.
 * @return	The scene index major, minor.
 */
std::pair<int, int> ScenesPageComponent::GetCurrentSceneIndex()
{
	auto sceneIndexFloat = m_sceneIdxEdit->getText().getFloatValue();
	auto sceneIndexCent = static_cast<int>(sceneIndexFloat * 100);
	auto sceneIndexMajor = sceneIndexCent / 100;
	auto sceneIndexMinor = sceneIndexCent - (sceneIndexMajor * 100);

	return std::make_pair(sceneIndexMajor, sceneIndexMinor);
}

/**
 * Method to get the list of pinned scenes from internal hashes.
 * @return	The list of scenes that shall be set as new pinned scenes.
 */
std::vector<std::pair<std::pair<int, int>, std::string>> ScenesPageComponent::GetPinnedScenes()
{
	auto pinnedScenes = std::vector<std::pair<std::pair<int, int>, std::string>>();

	for (auto const& pinnedButton : m_pinnedSceneIdxRecallButtons)
	{
		auto& sceneIndex = pinnedButton.first;

		auto recallButtonTextHypothesis = String(sceneIndex.first) + "." + String(sceneIndex.second).paddedLeft('0', 2) + " "; // This is how the button text was created, so we try to disassemble it the same way
		auto sceneName = pinnedButton.second->getButtonText().substring(recallButtonTextHypothesis.length());;

		pinnedScenes.push_back(std::make_pair(pinnedButton.first, sceneName.toStdString()));
	}

	return pinnedScenes;
}

/**
 * Method to set the list of pinned scenes and refresh the UI accordingly.
 * This also does clear any existing pinned scenes.
 * @param	pinnedScenes	The list of scenes that shall be set as new pinned scenes.
 */
void ScenesPageComponent::SetPinnedScenes(const std::vector<std::pair<std::pair<int, int>, std::string>>& pinnedScenes)
{
	ClearPinnedScenes();

	for (auto const& sceneIdxNameKV : pinnedScenes)
	{
		auto& sceneIndex = sceneIdxNameKV.first;
		auto& sceneName = sceneIdxNameKV.second;

		m_pinnedSceneIdxRecallLayoutContainer.insert(std::make_pair(sceneIndex, std::make_unique<HorizontalLayouterComponent>()));
		m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex)->SetSpacing(5);
		if (GetElementsContainer())
			GetElementsContainer()->addComponent(m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex).get(), true, false);

		auto recallButtonText = String(sceneIndex.first) + "." + String(sceneIndex.second).paddedLeft('0', 2);
		if (!sceneName.empty())
			recallButtonText += " " + String(sceneName);
		m_pinnedSceneIdxRecallButtons.insert(std::make_pair(sceneIndex, std::make_unique<TextButton>()));
		m_pinnedSceneIdxRecallButtons.at(sceneIndex)->setButtonText(recallButtonText);
		m_pinnedSceneIdxRecallButtons.at(sceneIndex)->addListener(this);
		m_pinnedSceneIdxRecallButtons.at(sceneIndex)->setTooltip("Recall Scene");
		m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex)->AddComponent(m_pinnedSceneIdxRecallButtons.at(sceneIndex).get(), 7);

		m_unpinSceneIdxRecallButtons.insert(std::make_pair(sceneIndex, std::make_unique<DrawableButton>("unpin scene index", DrawableButton::ButtonStyle::ImageOnButtonBackground)));
		m_unpinSceneIdxRecallButtons.at(sceneIndex)->setClickingTogglesState(false);
		m_unpinSceneIdxRecallButtons.at(sceneIndex)->addListener(this);
		m_unpinSceneIdxRecallButtons.at(sceneIndex)->setTooltip("Unpin Scene Index");
		m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex)->AddComponent(m_unpinSceneIdxRecallButtons.at(sceneIndex).get(), 1);
	}

	// attach the pinned scenes label to first of the recall trigger buttons
	if (!pinnedScenes.empty() &&m_pinnedSceneIdxRecallLayoutContainer.count(pinnedScenes.front().first) > 0 && m_pinnedSceneIdxRecallLayoutContainer.at(pinnedScenes.front().first))
		m_pinnedSceneIdxRecallLabel->attachToComponent(m_pinnedSceneIdxRecallLayoutContainer.at(pinnedScenes.front().first).get(), true);

	// set the correct icons to the newly created buttons
	lookAndFeelChanged();

	// update the sizing of the embedded viewport contents
	resized();
}

/**
 * Method to clear internal hashes containing elements related to pinned scenes on ui
 */
void ScenesPageComponent::ClearPinnedScenes()
{
	for (auto const& buttonKV : m_pinnedSceneIdxRecallButtons)
		if (m_pinnedSceneIdxRecallLayoutContainer.count(buttonKV.first) > 0)
			m_pinnedSceneIdxRecallLayoutContainer.at(buttonKV.first)->RemoveComponent(buttonKV.second.get());
	m_pinnedSceneIdxRecallButtons.clear();

	for (auto const& buttonKV : m_unpinSceneIdxRecallButtons)
		if (m_pinnedSceneIdxRecallLayoutContainer.count(buttonKV.first) > 0)
			m_pinnedSceneIdxRecallLayoutContainer.at(buttonKV.first)->RemoveComponent(buttonKV.second.get());
	m_unpinSceneIdxRecallButtons.clear();

	for (auto const& container : m_pinnedSceneIdxRecallLayoutContainer)
		if (GetElementsContainer())
			GetElementsContainer()->removeComponent(container.second.get());
	m_pinnedSceneIdxRecallLayoutContainer.clear();
}

/**
 * Reimplemented to handle button member clicks.
 * @param	button	The button that has been clicked.
 */
void ScenesPageComponent::buttonClicked(Button* button)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (m_nextButton && m_nextButton.get() == button)
	{
		RemoteObjectMessageData romd;
		ctrl->SendMessageDataDirect(ROI_Scene_Next, romd);
        
        // clear the text changed indicator, to not block the index change updating
        m_sceneIdxEditTextChanged = false;
        
        // after the recall command was sent, refresh the object values relevant for UI components
        Timer::callAfterDelay(100, std::bind(&ScenesPageComponent::triggerPollOnce, this));
	}
	else if (m_previousButton && m_previousButton.get() == button)
	{
		RemoteObjectMessageData romd;
		ctrl->SendMessageDataDirect(ROI_Scene_Previous, romd);
        
        // clear the text changed indicator, to not block the index change updating
        m_sceneIdxEditTextChanged = false;
        
        // after the recall command was sent, refresh the object values relevant for UI components
        Timer::callAfterDelay(100, std::bind(&ScenesPageComponent::triggerPollOnce, this));
	}
	else if (m_recallButton && m_recallButton.get() == button)
	{
		SendRecallSceneIndex(GetCurrentSceneIndex());
        
        // clear the text changed indicator, to not block the index change updating
        m_sceneIdxEditTextChanged = false;
        
        // after the recall command was sent, refresh the object values relevant for UI components
        Timer::callAfterDelay(100, std::bind(&ScenesPageComponent::triggerPollOnce, this));
	}
	else if (m_pinSceneIdxRecallButton && m_pinSceneIdxRecallButton.get() == button)
	{
		PinSceneRecall(GetCurrentSceneIndex());
	}
	else
	{
		for (auto const& sceneIdxRecallButton : m_pinnedSceneIdxRecallButtons)
		{
			if (sceneIdxRecallButton.second && sceneIdxRecallButton.second.get() == button)
			{
				SendRecallSceneIndex(sceneIdxRecallButton.first);
                
                // clear the text changed indicator, to not block the index change updating
                m_sceneIdxEditTextChanged = false;
                
                // after the recall command was sent, refresh the object values relevant for UI components
                Timer::callAfterDelay(100, std::bind(&ScenesPageComponent::triggerPollOnce, this));
                
				return;
			}
		}

		for (auto const& unpinSceneIdxButton : m_unpinSceneIdxRecallButtons)
		{
			if (unpinSceneIdxButton.second && unpinSceneIdxButton.second.get() == button)
			{
				auto sceneIndex = unpinSceneIdxButton.first; // make a copy of the index since the map entry will be erased in the following function call!
				UnpinSceneRecall(sceneIndex);
				return;
			}
		}
	}
}

/**
 * Reimplemented to handle text editor changes.
 * @param	textEdit	The text editor whose text was changed.
 */
void ScenesPageComponent::textEditorTextChanged(TextEditor& textEdit)
{
	if (m_sceneIdxEdit && m_sceneIdxEdit.get() == &textEdit)
	{
        m_sceneIdxEditTextChanged = true;
	}
}

/**
 * Reimplemented to handle text editor return key presses.
 * @param	textEdit	The text editor that the return key was pressed on.
 */
void ScenesPageComponent::textEditorReturnKeyPressed(TextEditor& textEdit)
{
	if (m_sceneIdxEdit && m_sceneIdxEdit.get() == &textEdit)
	{
		SendRecallSceneIndex(GetCurrentSceneIndex());
        m_sceneIdxEditTextChanged = false;
	}
}

/**
 * Reimplemented to handle text editor escape key presses.
 * @param	textEdit	The text editor that the excape key was pressed on.
 */
void ScenesPageComponent::textEditorEscapeKeyPressed(TextEditor& textEdit)
{
	if (m_sceneIdxEdit && m_sceneIdxEdit.get() == &textEdit)
	{
        m_sceneIdxEditTextChanged = false;
	}
}

/**
 * Reimplemented to handle text editor focus loosing.
 * @param    textEdit    The text editor that lost the focus.
 */
void ScenesPageComponent::textEditorFocusLost(TextEditor& textEdit)
{
    if (m_sceneIdxEdit && m_sceneIdxEdit.get() == &textEdit)
    {
        m_sceneIdxEditTextChanged = false;
    }
}

/**
 * Reimplemented method to handle updated object data for objects that have been added for standalone polling.
 * @param	objectId	The remote object identifier of the object that shall be handled.
 * @param	msgData		The remote object message data that was received and shall be handled.
 */
void ScenesPageComponent::HandleObjectDataInternal(RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData)
{
	// all remote objects that are read here are of type string and must meet these common criteria
	if (msgData._valType == ROVT_STRING
		&& msgData._valCount * sizeof(char) == msgData._payloadSize
		&& msgData._payload != nullptr)
	{
		auto remoteObjectContentString = String(static_cast<char*>(msgData._payload), msgData._payloadSize);

		switch (objectId)
		{
		case ROI_Scene_SceneIndex:
			{
			auto sceneIndexFloat = remoteObjectContentString.getFloatValue();
			auto sceneIndexCent = static_cast<int>(sceneIndexFloat * 100);
			auto sceneIndexMajor = sceneIndexCent / 100;
			auto sceneIndexMinor = sceneIndexCent - (sceneIndexMajor * 100);

			if (m_sceneIndexChangePending)
			{
				if (m_sceneIndexChange.first == sceneIndexMajor && m_sceneIndexChange.second == sceneIndexMinor)
					m_sceneIndexChangePending = false;
			}

			if (m_sceneIdxEdit && !m_sceneIdxEditTextChanged)
				m_sceneIdxEdit->setText(remoteObjectContentString, dontSendNotification);
			break;
			}
		case ROI_Scene_SceneName:
			if (m_sceneNameEdit)
				m_sceneNameEdit->setText(remoteObjectContentString);
			break;
		case ROI_Scene_SceneComment:
			if (m_sceneCommentEdit)
				m_sceneCommentEdit->setText(remoteObjectContentString);
			break;
		default:
			break;
		}
	}
	else
		jassertfalse;
}

/**
 * Method to add a direct recall trigger button for the given scene index.
 * @param	sceneIndex		The scene index to add.
 */
void ScenesPageComponent::PinSceneRecall(const std::pair<int, int>& sceneIndex)
{
    if (m_pinnedSceneIdxRecallButtons.count(sceneIndex) != 0)
        return;
    
	m_pinnedSceneIdxRecallLayoutContainer.insert(std::make_pair(sceneIndex, std::make_unique<HorizontalLayouterComponent>()));
	m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex)->SetSpacing(5);
	if (GetElementsContainer())
		GetElementsContainer()->addComponent(m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex).get(), true, false);

	if (m_pinnedSceneIdxRecallLayoutContainer.size() == 1)
		m_pinnedSceneIdxRecallLabel->attachToComponent(m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex).get(), true);

	auto recallButtonText = String(sceneIndex.first) + "." + String(sceneIndex.second).paddedLeft('0', 2);
	if (m_sceneNameEdit && m_sceneNameEdit->getText().isNotEmpty())
		recallButtonText += " " + m_sceneNameEdit->getText();
	m_pinnedSceneIdxRecallButtons.insert(std::make_pair(sceneIndex, std::make_unique<TextButton>()));
	m_pinnedSceneIdxRecallButtons.at(sceneIndex)->setButtonText(recallButtonText);
	m_pinnedSceneIdxRecallButtons.at(sceneIndex)->addListener(this);
	m_pinnedSceneIdxRecallButtons.at(sceneIndex)->setTooltip("Recall Scene");
	m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex)->AddComponent(m_pinnedSceneIdxRecallButtons.at(sceneIndex).get(), 7);

	m_unpinSceneIdxRecallButtons.insert(std::make_pair(sceneIndex, std::make_unique<DrawableButton>("unpin scene index", DrawableButton::ButtonStyle::ImageOnButtonBackground)));
	m_unpinSceneIdxRecallButtons.at(sceneIndex)->setClickingTogglesState(false);
	m_unpinSceneIdxRecallButtons.at(sceneIndex)->addListener(this);
	m_unpinSceneIdxRecallButtons.at(sceneIndex)->setTooltip("Unpin Scene Index");
	m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex)->AddComponent(m_unpinSceneIdxRecallButtons.at(sceneIndex).get(), 1);

	// set the correct icons to the newly created buttons
	lookAndFeelChanged();

	// update the sizing of the embedded viewport contents
	resized();

	// finally trigger refreshing the config file
	auto config = SpaConBridge::AppConfiguration::getInstance();
	if (config)
		config->triggerConfigurationDump(false);
}

/**
 * Method to remove the direct recall trigger button for the given scene index.
 * @param	sceneIndex		The scene index to remove.
 */
void ScenesPageComponent::UnpinSceneRecall(const std::pair<int, int>& sceneIndex)
{
	if (m_pinnedSceneIdxRecallLayoutContainer.find(sceneIndex) != m_pinnedSceneIdxRecallLayoutContainer.end())
	{
		if (m_pinnedSceneIdxRecallButtons.find(sceneIndex) != m_pinnedSceneIdxRecallButtons.end())
		{		
			m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex)->RemoveComponent(m_pinnedSceneIdxRecallButtons.at(sceneIndex).get());
			m_pinnedSceneIdxRecallButtons.erase(sceneIndex);
		}
		if (m_unpinSceneIdxRecallButtons.find(sceneIndex) != m_unpinSceneIdxRecallButtons.end())
		{	
			m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex)->RemoveComponent(m_unpinSceneIdxRecallButtons.at(sceneIndex).get());
			m_unpinSceneIdxRecallButtons.erase(sceneIndex);
		}

		if (GetElementsContainer())
		{
			GetElementsContainer()->removeComponent(m_pinnedSceneIdxRecallLayoutContainer.at(sceneIndex).get());
			if (m_pinnedSceneIdxRecallLayoutContainer.size() == 1)
				GetElementsContainer()->removeComponent(m_pinnedSceneIdxRecallLabel.get());
		}

		m_pinnedSceneIdxRecallLayoutContainer.erase(sceneIndex);
	}

	// update the sizing of the embedded viewport contents
	resized();

	// finally trigger refreshing the config file
	auto config = SpaConBridge::AppConfiguration::getInstance();
	if (config)
		config->triggerConfigurationDump(false);
}

/**
 * Method to trigger sending a recall message for the given scene index.
 * @param	sceneIndex		The scene index to recall.
 * @return	True if sending the scene index recall message succeeded.
 */
bool ScenesPageComponent::SendRecallSceneIndex(const std::pair<int, int>& sceneIndex)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	m_sceneIndexChange = sceneIndex;

	int dualIntValue[2];
	dualIntValue[0] = sceneIndex.first;
	dualIntValue[1] = sceneIndex.second;

	RemoteObjectMessageData romd;
	romd._valType = ROVT_INT;
	romd._valCount = 2;
	romd._payloadOwned = false;
	romd._payloadSize = 2 * sizeof(int);
	romd._payload = &dualIntValue;
	auto sendSuccess = ctrl->SendMessageDataDirect(ROI_Scene_Recall, romd);

	m_sceneIndexChangePending = sendSuccess;

	return sendSuccess;
}

/**
 * Reimplemented from component to change drawablebutton icon data.
 */
void ScenesPageComponent::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_nextButton, BinaryData::skip_next24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_previousButton, BinaryData::skip_previous24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_pinSceneIdxRecallButton, BinaryData::push_pin_black_24dp_svg, &getLookAndFeel());

	for (auto const& unpinRecallButton : m_unpinSceneIdxRecallButtons)
		UpdateDrawableButtonImages(unpinRecallButton.second, BinaryData::clear_black_24dp_svg, &getLookAndFeel());
}


} // namespace SpaConBridge
