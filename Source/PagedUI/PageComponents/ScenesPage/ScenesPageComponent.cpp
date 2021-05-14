/* Copyright (c) 2020-2021, Christian Ahrens
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
	: StandalonePollingPageComponentBase(PCT_Scenes)
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
	}
	else if (m_previousButton && m_previousButton.get() == button)
	{
		RemoteObjectMessageData romd;
		ctrl->SendMessageDataDirect(ROI_Scene_Previous, romd);
	}
	else if (m_recallButton && m_recallButton.get() == button)
	{
		SendRecallSceneIndex(GetCurrentSceneIndex());
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
		m_sceneIndexChangePending = true;
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
		m_sceneIndexChangePending = false;
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
				break;
			}

			if (m_sceneIdxEdit)
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

	lookAndFeelChanged();

	if (GetElementsContainer())
		GetElementsContainer()->resized();
	resized();
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

	if (GetElementsContainer())
		GetElementsContainer()->resized();
	resized();
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

	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (!dblookAndFeel)
		return;

	if (m_nextButton)
	{
		std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
		JUCEAppBasics::Image_utils::getDrawableButtonImages(String(BinaryData::skip_next24px_svg), NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));
		m_nextButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}

	if (m_previousButton)
	{
		std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
		JUCEAppBasics::Image_utils::getDrawableButtonImages(String(BinaryData::skip_previous24px_svg), NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));
		m_previousButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}

	if (m_pinSceneIdxRecallButton)
	{
		std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
		JUCEAppBasics::Image_utils::getDrawableButtonImages(String(BinaryData::push_pin_black_24dp_svg), NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));
		m_pinSceneIdxRecallButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}

	for (auto const& unpinRecallButton : m_unpinSceneIdxRecallButtons)
	{
		std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
		JUCEAppBasics::Image_utils::getDrawableButtonImages(String(BinaryData::clear_black_24dp_svg), NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));
		unpinRecallButton.second->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}
}


} // namespace SpaConBridge
