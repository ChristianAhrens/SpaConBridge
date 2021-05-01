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


#pragma once

#include <JuceHeader.h>
#include <TextWithImageButton.h>

#include "../StandalonePollingPageComponentBase.h"


namespace SpaConBridge
{


/**
 * class ScenesPageComponent provides control for DS100 scene transport.
 */
class ScenesPageComponent : public StandalonePollingPageComponentBase,
							public TextButton::Listener,
							public TextEditor::Listener
{
public:
	explicit ScenesPageComponent();
	~ScenesPageComponent() override;

	//==========================================================================
	void buttonClicked(Button* button) override;

	//==========================================================================
	void textEditorTextChanged(TextEditor& textEdit) override;
	void textEditorReturnKeyPressed(TextEditor& textEdit) override;
	void textEditorEscapeKeyPressed(TextEditor& textEdit) override;

	//==========================================================================
	void lookAndFeelChanged() override;

protected:
	void HandleObjectDataInternal(RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData) override;

private:
	std::unique_ptr<HorizontalComponentLayouter>		m_prevNextLayoutContainer;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_previousButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_nextButton;
	
	std::unique_ptr<HorizontalComponentLayouter>				m_recallIdxLayoutContainer;
	std::unique_ptr<TextButton>									m_recallButton;
	std::unique_ptr<Label>										m_sceneIndexLabel;
	std::unique_ptr<TextEditor::LengthAndCharacterRestriction>	m_sceneIndexFilter;
	std::unique_ptr<TextEditor>									m_sceneIndexEdit;
	
	std::unique_ptr<Label>		m_sceneNameLabel;
	std::unique_ptr<TextEditor>	m_sceneNameEdit;
	std::unique_ptr<Label>		m_sceneCommentLabel;
	std::unique_ptr<TextEditor>	m_sceneCommentEdit;

	bool m_sceneIndexChangePending{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScenesPageComponent)
};


} // namespace SpaConBridge
