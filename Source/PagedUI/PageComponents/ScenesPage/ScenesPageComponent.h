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


#pragma once

#include <JuceHeader.h>
#include <TextWithImageButton.h>

#include "../StandaloneActiveObjectsPageComponentBase.h"

 /**
  * Fwd. decls.
  */
namespace JUCEAppBasics {
	class FixedFontTextEditor;
}

namespace SpaConBridge
{


/**
 * class ScenesPageComponent provides control for DS100 scene transport.
 */
class ScenesPageComponent : public StandaloneActiveObjectsPageComponentBase,
							public TextButton::Listener,
							public TextEditor::Listener
{
public:
	explicit ScenesPageComponent();
	~ScenesPageComponent() override;

	std::pair<int, int> GetCurrentSceneIndex();

	//==========================================================================
	std::vector<std::pair<std::pair<int, int>, std::string>> GetPinnedScenes();
	void SetPinnedScenes(const std::vector<std::pair<std::pair<int, int>, std::string>>& pinnedScenes);
	void ClearPinnedScenes();

	//==========================================================================
	void resized() override;

	//==========================================================================
	void buttonClicked(Button* button) override;

	//==========================================================================
	void textEditorTextChanged(TextEditor& textEdit) override;
	void textEditorReturnKeyPressed(TextEditor& textEdit) override;
	void textEditorEscapeKeyPressed(TextEditor& textEdit) override;
    void textEditorFocusLost (TextEditor& textEdit) override;

	//==========================================================================
	void lookAndFeelChanged() override;

protected:
	void HandleObjectDataInternal(const RemoteObjectIdentifier& roi, const RemoteObjectMessageData& msgData) override;

private:
	void PinSceneRecall(const std::pair<int, int>& sceneIndex);
	void UnpinSceneRecall(const std::pair<int, int>& sceneIndex);
	bool SendRecallSceneIndex(const std::pair<int, int>& sceneIndex);

	std::unique_ptr<HorizontalLayouterComponent>		m_prevNextLayoutContainer;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_previousButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_nextButton;
	
	std::unique_ptr<HorizontalLayouterComponent>				m_recallIdxLayoutContainer;
	std::unique_ptr<HorizontalLayouterComponent>				m_recallIdxSubLayoutContainer;
	std::unique_ptr<TextButton>									m_recallButton;
	std::unique_ptr<DrawableButton>								m_pinSceneIdxRecallButton;
	std::unique_ptr<Label>										m_sceneIdxLabel;
	std::unique_ptr<TextEditor::LengthAndCharacterRestriction>	m_sceneIdxFilter;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_sceneIdxEdit;
	
	std::unique_ptr<Label>								m_sceneNameLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>	m_sceneNameEdit;
	std::unique_ptr<Label>								m_sceneCommentLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>	m_sceneCommentEdit;

	std::unique_ptr<Label>														m_pinnedSceneIdxRecallLabel;
	std::map<std::pair<int, int>, std::unique_ptr<HorizontalLayouterComponent>>	m_pinnedSceneIdxRecallLayoutContainer;
	std::map<std::pair<int, int>, std::unique_ptr<TextButton>>					m_pinnedSceneIdxRecallButtons;
	std::map<std::pair<int, int>, std::unique_ptr<DrawableButton>>				m_unpinSceneIdxRecallButtons;

	bool				m_sceneIndexChangePending{ false };
	std::pair<int, int> m_sceneIndexChange{ 0, 0 };
    
    bool    m_sceneIdxEditTextChanged{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScenesPageComponent)
};


} // namespace SpaConBridge
