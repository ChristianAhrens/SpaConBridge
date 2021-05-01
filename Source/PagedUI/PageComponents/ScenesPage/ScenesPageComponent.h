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
 * Helper class to allow horizontal layouting of child components
 * and use the instance of this class as a single component embedded
 * in other layouts.
 */
class HorizontalComponentLayouter : public Component
{
public:
	void AddComponent(Component* compo)
	{
		addAndMakeVisible(compo);
		m_layoutComponents.push_back(compo);
	}
	bool RemoveComponent(Component* compo)
	{
		auto iter = std::find(m_layoutComponents.begin(), m_layoutComponents.end(), compo);
		if (iter == m_layoutComponents.end())
			return false;

		removeChildComponent(compo);
		m_layoutComponents.erase(iter);
		return true;
	}
	void SetSpacing(int spacing)
	{
		m_spacing = spacing;
	}
	void resized() override
	{
		FlexBox fb;
		fb.flexDirection = FlexBox::Direction::row;
		auto compoCnt = m_layoutComponents.size();
		for (int i = 0; i < compoCnt; i++)
		{
			fb.items.add(FlexItem(*m_layoutComponents.at(i)).withFlex(1));
			if (i < compoCnt - 1)
				fb.items.add(FlexItem().withWidth(static_cast<float>(m_spacing)));
		}
		fb.performLayout(getLocalBounds().toFloat());
	}

	std::vector<Component*>	m_layoutComponents;
	int m_spacing{ 0 };
};

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
