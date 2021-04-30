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
	void resized() override
	{
		FlexBox fb;
		fb.flexDirection = FlexBox::Direction::row;
		for (auto const& c : m_layoutComponents)
		{
			fb.items.add(FlexItem(*c).withFlex(1));
		}
		fb.performLayout(getLocalBounds().toFloat());
	}

	std::vector<Component*>	m_layoutComponents;
};

/**
 * class ScenesPageComponent provides control for DS100 scene transport.
 */
class ScenesPageComponent : public StandalonePollingPageComponentBase,
							public TextButton::Listener
{
public:
	explicit ScenesPageComponent();
	~ScenesPageComponent() override;

	//==========================================================================
	void buttonClicked(Button* button) override;

	//==========================================================================
	void lookAndFeelChanged() override;

protected:
	void HandleObjectDataInternal(RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData) override;

private:
	std::unique_ptr<HorizontalComponentLayouter>		m_transportControls;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_previousButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_nextButton;
	std::unique_ptr<TextEditor>							m_sceneIndexEdit;
	std::unique_ptr<TextEditor>							m_sceneNameEdit;
	std::unique_ptr<TextEditor>							m_sceneCommentEdit;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScenesPageComponent)
};


} // namespace SpaConBridge
