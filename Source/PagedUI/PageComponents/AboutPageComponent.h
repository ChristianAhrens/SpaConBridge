/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#pragma once

#include "PageComponentBase.h"	//<USE PageComponentBase


namespace SoundscapeBridgeApp
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
	std::unique_ptr<HyperlinkButton>	m_githubLink;			/**> Hyperlink to app home on github. */

	std::unique_ptr<Drawable>           m_juceIconDrawable;		/**> JUCE icon drawable. */
	std::unique_ptr<Label>				m_juceLabel;			/**> JUCE copyright label. */
	std::unique_ptr<HyperlinkButton>	m_juceLink;				/**> Hyperlink to juce. */

	std::unique_ptr<Drawable>           m_materialIconDrawable;	/**> MATERIAL.IO icon drawable. */
	std::unique_ptr<Label>				m_materialLabel;		/**> MATERIAL.IO copyright label. */
	std::unique_ptr<HyperlinkButton>	m_materialLink;			/**> Hyperlink to material.io. */

	std::unique_ptr<Drawable>           m_servusIconDrawable;	/**> HBP (Servus) icon drawable. */
	std::unique_ptr<Label>				m_servusLabel;			/**> HBP (Servus) copyright label. */
	std::unique_ptr<HyperlinkButton>	m_servusLink;			/**> Hyperlink to HBP Servus on github. */

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


} // namespace SoundscapeBridgeApp
