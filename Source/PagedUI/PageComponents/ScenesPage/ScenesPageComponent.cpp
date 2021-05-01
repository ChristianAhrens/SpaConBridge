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
	m_prevNextLayoutContainer = std::make_unique<HorizontalComponentLayouter>();
	m_prevNextLayoutContainer->SetSpacing(5);
	if (GetElementsContainer())
		GetElementsContainer()->addComponent(m_prevNextLayoutContainer.get(), true, false);
	
	m_previousButton = std::make_unique<JUCEAppBasics::TextWithImageButton>();
	m_previousButton->setButtonText("Previous");
	m_previousButton->setImagePosition(Justification::centredLeft);
	m_previousButton->addListener(this);
	m_prevNextLayoutContainer->AddComponent(m_previousButton.get());
	m_nextButton = std::make_unique<JUCEAppBasics::TextWithImageButton>();
	m_nextButton->setButtonText("Next");
	m_nextButton->setImagePosition(Justification::centredLeft);
	m_nextButton->addListener(this);
	m_prevNextLayoutContainer->AddComponent(m_nextButton.get());

	// scene index editor and recall button wrapped in horizontal layouting container
	m_recallIdxLayoutContainer = std::make_unique<HorizontalComponentLayouter>();
	m_recallIdxLayoutContainer->SetSpacing(5);
	if (GetElementsContainer())
		GetElementsContainer()->addComponent(m_recallIdxLayoutContainer.get(), true, false);

	m_sceneIndexFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(6, "1234567890,");
	m_sceneIndexEdit = std::make_unique<TextEditor>();
	m_sceneIndexEdit->addListener(this);
	m_sceneIndexEdit->setInputFilter(m_sceneIndexFilter.get(), false);
	m_recallIdxLayoutContainer->AddComponent(m_sceneIndexEdit.get());
	m_recallButton = std::make_unique<TextButton>();
	m_recallButton->setButtonText("Recall");
	m_recallButton->addListener(this);
	m_recallIdxLayoutContainer->AddComponent(m_recallButton.get());

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

	lookAndFeelChanged();
}

/**
 * Class destructor.
 */
ScenesPageComponent::~ScenesPageComponent()
{
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
		auto sceneIndexFloat = m_sceneIndexEdit->getText().getFloatValue();
		auto sceneIndexCent = static_cast<int>(sceneIndexFloat * 100);
		auto sceneIndexMajor = sceneIndexCent / 100;
		auto sceneIndexMinor = sceneIndexCent - (sceneIndexMajor * 100);

		int dualIntValue[2];
		dualIntValue[0] = sceneIndexMajor;
		dualIntValue[1] = sceneIndexMinor;

		RemoteObjectMessageData romd;
		romd._valType = ROVT_INT;
		romd._valCount = 2;
		romd._payloadOwned = false;
		romd._payloadSize = 2 * sizeof(int);
		romd._payload = &dualIntValue;
		ctrl->SendMessageDataDirect(ROI_Scene_Recall, romd);

		m_sceneIndexChangePending = false;
	}
}

/**
 * Reimplemented to handle text editor changes.
 * @param	textEdit	The text editor whose text was changed.
 */
void ScenesPageComponent::textEditorTextChanged(TextEditor& textEdit)
{
	if (m_sceneIndexEdit && m_sceneIndexEdit.get() == &textEdit)
	{
		DBG(String(__FUNCTION__));
		m_sceneIndexChangePending = true;
	}
}

/**
 * Reimplemented to handle text editor return key presses.
 * @param	textEdit	The text editor that the return key was pressed on.
 */
void ScenesPageComponent::textEditorReturnKeyPressed(TextEditor& textEdit)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (m_sceneIndexEdit && m_sceneIndexEdit.get() == &textEdit)
	{
		auto sceneIndexFloat = m_sceneIndexEdit->getText().getFloatValue();
		auto sceneIndexCent = static_cast<int>(sceneIndexFloat * 100);
		auto sceneIndexMajor = sceneIndexCent / 100;
		auto sceneIndexMinor = sceneIndexCent - (sceneIndexMajor * 100);

		int dualIntValue[2];
		dualIntValue[0] = sceneIndexMajor;
		dualIntValue[1] = sceneIndexMinor;

		RemoteObjectMessageData romd;
		romd._valType = ROVT_INT;
		romd._valCount = 2;
		romd._payloadOwned = false;
		romd._payloadSize = 2 * sizeof(int);
		romd._payload = &dualIntValue;
		ctrl->SendMessageDataDirect(ROI_Scene_Recall, romd);
		
		m_sceneIndexChangePending = false;
	}
}

/**
 * Reimplemented to handle text editor escape key presses.
 * @param	textEdit	The text editor that the excape key was pressed on.
 */
void ScenesPageComponent::textEditorEscapeKeyPressed(TextEditor& textEdit)
{
	if (m_sceneIndexEdit && m_sceneIndexEdit.get() == &textEdit)
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
			if (m_sceneIndexEdit && !m_sceneIndexChangePending)
				m_sceneIndexEdit->setText(remoteObjectContentString, dontSendNotification);
			break;
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
}


} // namespace SpaConBridge
