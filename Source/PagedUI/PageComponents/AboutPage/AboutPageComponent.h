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

#include "../PageComponentBase.h"	//<USE PageComponentBase


namespace SpaConBridge
{


/**
 * Class AboutPageContentComponent provides copyright and licensing info.
 */
class AboutPageContentComponent : public Component
{
public:
	AboutPageContentComponent();
	~AboutPageContentComponent() override;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	std::unique_ptr<Drawable>           m_appIconDrawable;		/**> App icon drawable. */
	std::unique_ptr<Label>				m_appInfoLabel;			/**> App version label. */
	std::unique_ptr<HyperlinkButton>	m_appInfoLink;			/**> Hyperlink to app home on github. */

	std::unique_ptr<Drawable>           m_juceIconDrawable;		/**> JUCE icon drawable. */
	std::unique_ptr<Label>				m_juceLabel;			/**> JUCE copyright label. */
	std::unique_ptr<HyperlinkButton>	m_juceLink;				/**> Hyperlink to juce. */

	std::unique_ptr<Drawable>           m_materialIconDrawable;	/**> MATERIAL.IO icon drawable. */
	std::unique_ptr<Label>				m_materialLabel;		/**> MATERIAL.IO copyright label. */
	std::unique_ptr<HyperlinkButton>	m_materialLink;			/**> Hyperlink to material.io. */

	std::unique_ptr<TextEditor>         m_licenseGPLv3Field;	/**> Text field containing Licensing info. */
	std::unique_ptr<TextEditor>         m_licenseAPACHEv2Field;	/**> Text field containing Licensing info. */
	std::unique_ptr<TextEditor>         m_licenseLGPLv3Field;	/**> Text field containing Licensing info. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutPageContentComponent)
};

/**
 * Class AboutPageComponent provides copyright and licensing info.
 */
class AboutPageComponent : public PageComponentBase
{
public:
	AboutPageComponent();
	~AboutPageComponent() override;

	//==============================================================================
	void UpdateGui(bool init) override;

	void lookAndFeelChanged() override;

	//==============================================================================
	std::function<void()> onCloseClick;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==============================================================================
	void mouseUp(const MouseEvent& e) override;

private:
	std::unique_ptr<DrawableButton>				m_closeButton;		/**< Button to close the about page. */
	std::unique_ptr<AboutPageContentComponent>	m_aboutContents;	/**< Component that holds actual contents. */
	std::unique_ptr<Viewport>					m_aboutViewport;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutPageComponent)
};


} // namespace SpaConBridge
