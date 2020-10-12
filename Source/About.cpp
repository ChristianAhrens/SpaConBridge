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


#include "About.h"
#include "SoundscapeBridgeAppCommon.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class AboutOverlay
===============================================================================
*/

/**
 * Class constructor.
 */
AboutOverlay::AboutOverlay()
	: OverlayBase(OT_About)
{
	// Plugin version label
	String versionString = String("Soundscape Plug-in V") + String(JUCE_STRINGIFY(JUCE_APP_VERSION));
	versionString += String("\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2017-2019\nd&b audiotechnik GmbH & Co. KG,\nall rights reserved.");
	m_versionLabel = std::make_unique<Label>("PluginVersion", versionString);
	m_versionLabel->setJustificationType(Justification::topLeft);
	m_versionLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_versionLabel.get());

	// Hyperlink to dbaudio.com
	m_dbLink = std::make_unique<HyperlinkButton>(String("www.dbaudio.com"), URL("www.dbaudio.com"));
	m_dbLink->setFont(Font(13.0, Font::plain), false /* do not resize */);
	addAndMakeVisible(m_dbLink.get());

	String eula("End-User License Agreement (\"Agreement\") for d&b Soundscape DAW Plug-in (\"Software\")\n" \
		"======================================================================\n" \
		"This is a legal Agreement between the end user (\"you\") and d&b audiotechnik GmbH & Co. KG, Eugen-Adolff-Strasse 134, 71522 Backnang, Germany (\"d&b audiotechnik\"). \n" \
		"1.  By downloading, installing or using the Software you agree to the terms of this Agreement. If you do not agree to the terms of this Agreement you must cease and desist from down-loading, installing and/or using of the Software.\n" \
		"2.  The Software is intended solely for use by Entrepreneurs. An \"Entrepreneur\" is every natural person or legal entity acting in his/her or its professional or self-employed capacity when entering into this Agreement. If you are not an Entrepreneur and still wish to use the Software, please contact d&b audiotechnik directly.\n" \
		"3.  Please note, that the Software is not a stand-alone executable software. To use the Soft-ware third party software is necessary which is not part of the Software and which is subject to its own license terms and has to be provided for by you on your own expenses and responsibility.\n" \
		"4.  d&b audiotechnik grants you for the duration of the protection of the Software a non-exclusive, non-sublicensable right to use the Software for your own purposes subject to the terms and conditions of this Agreement. All rights to the Software are owned by d&b audiotechnik or its respective licensors. You may NOT copy the documentation accompanying the Software.\n" \
		"5.  Any such right to use does only apply to the object code of the Software, which means the Software in a form readable solely by machines. You do not have a claim to being provided with the source code or parts of the source code and will not receive any rights to use or otherwise exploit the source code. In this regard, source code means the Software's source text, written in a programming language in a human readable form.\n" \
		"6.  Subject to the mandatory limitations according to applicable copyright law, you may NOT (i) reverse engineer, disassemble, decompile or otherwise reduce the Software to a human perceivable version, nor shall you permit others to do so, except and only to the ex-tent that such activity is expressly permitted by applicable law notwithstanding this limitation, (ii) modify, adapt, rent, lease, resell, distribute, network or create derivative works based upon the Software or any part thereof.\n" \
		"7.  This Agreement is immediately terminated if you violate the terms and conditions hereof. You agree upon such termination to cease and desist from using the Software and to destroy the Software together with all copies.\n" \
		"8.  Limitations of Liability:\n" \
		" a. d&b audiotechnik shall bear liability for material defects and defects in title in the Software and its content and information (warranty for defects) only if d&b audiotechnik has fraudulently concealed a defect and/or has assumed a guarantee.\n" \
		" b. Outside of the warranty for defects, d&b audiotechnik shall be liable only in cases of intent (Vorsatz) and gross negligence (grobe Fahrlaessigkeit), pursuant to the provisions of the Product Liability Act (Produkthaftungsgesetz) and in all other cases subject to statutory mandatory liability, in each case according to the statutory provisions.\n" \
		" c. Otherwise, d&b audiotechnik's liability is hereby excluded.\n" \
		" d. Where d&b audiotechnik's liability is restricted or excluded according to the provisions above, this shall also apply to the personal liability of the statutory representatives, employees and vicarious agents of d&b audiotechnik, as well as for indirect damages and consequential damages (e.g. loss of data, damage to your hardware or software, disruption of operations, stoppages in production, loss of profit).\n" \
		" e. You bear sole responsibility for accuracy of the data and information entered for use of the Software, including interpretation of the results delivered by the Software.\n" \
		"9.  You are entitled to provide a third party with the original version of the Software together with a copy of this Agreement if this third party is an Entrepreneur and expressly consents in writing to the application of this Agreement for any use of the Software. As soon as you pass on the Software to the third party you should immediately notify d&b audiotechnik. Notification should, at least, include the date of transfer of the Software and the contact details of the new user. When passing on the Software, you shall promptly and completely delete or otherwise destroy all of your other copies of the Software.\n" \
		"10. This Agreement shall be governed by the laws of Germany.\n" \
		"If you have any questions concerning this Agreement, please contact d&b audiotechnik's support.");
	m_eulaField = std::make_unique<TextEditor>("eula");
	//m_eulaField->setColour(TextEditor::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));
	//m_eulaField->setColour(TextEditor::textColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));
	//m_eulaField->setColour(TextEditor::outlineColourId, CDbStyle::GetDbColor(CDbStyle::ButtonColor));
	m_eulaField->setReadOnly(true);
	m_eulaField->setFont(Font(13.0, Font::plain));
	m_eulaField->setCaretVisible(false);
	m_eulaField->setMultiLine(true, false /* no wrapping */);
	m_eulaField->setScrollbarsShown(true);
	m_eulaField->setText(eula, false);
	addAndMakeVisible(m_eulaField.get());

	// JUCE copyright label
	String juceLabelString = String("Made with JUCE.\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2017 - ROLI Ltd.");
	m_juceLabel = std::make_unique<Label>("JuceLabel", juceLabelString);
	m_juceLabel->setJustificationType(Justification::topRight);
	m_juceLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_juceLabel.get());
}

/**
 * Class destructor.
 */
AboutOverlay::~AboutOverlay()
{
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * NOTE: this reimplementation does nothing, since these pages are static.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void AboutOverlay::UpdateGui(bool init)
{
	ignoreUnused(init);
}

/**
 * Reimplemented to paint the overlay's background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void AboutOverlay::paint(Graphics& g)
{
	int w = getLocalBounds().getWidth();
	int h = getLocalBounds().getHeight();

	// Background
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker()/*CDbStyle::GetDbColor(CDbStyle::DarkColor)*/);
	g.fillRect(Rectangle<int>(8, 8, w - 16, h - 16));

	// JUCE logo
	std::unique_ptr<Drawable> formatLogo = Drawable::createFromImageData(BinaryData::logo_juce_svg, BinaryData::logo_juce_svgSize);
	formatLogo->drawWithin(g, Rectangle<float>(w - 120.0f, 10.0f, 100.0f, 35.0f), RectanglePlacement::stretchToFit, 1.0f);
}

/**
 * Reimplemented to resize and re-postion controls & labels.
 */
void AboutOverlay::resized()
{
	int eulaVPos = 170;
	int eulaHeight = jmin((getLocalBounds().getHeight() - (eulaVPos + 20)), 270);

	m_versionLabel->setBounds(60, 12, 300, 55);
	m_eulaField->setBounds(20, (eulaVPos), getLocalBounds().getWidth() - 40, eulaHeight);
	m_dbLink->setBounds(60, 65, 110, 18);
	m_juceLabel->setBounds(getLocalBounds().getWidth() - 210, 48, 200, 50);
}


/*
===============================================================================
 Class CAboutOverlayGeneric
===============================================================================
*/

/**
 * Class constructor.
 */
CAboutOverlayGeneric::CAboutOverlayGeneric()
{
	// Plugin version label
	String formatString = String("Audio Units (AU) Plug-in format. \nThe Audio Units logo is a trademark of Apple Computer, Inc. \nCopyright ") +
		String(CharPointer_UTF8("\xc2\xa9")) + String(" 2005 Apple Computer, Inc. All rights reserved.");
	m_formatInfoLabel = std::make_unique<Label>("FormatInfo", formatString);
	m_formatInfoLabel->setJustificationType(Justification::topLeft);
	m_formatInfoLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_formatInfoLabel.get());
}

/**
 * Class destructor.
 */
CAboutOverlayGeneric::~CAboutOverlayGeneric()
{
}

/**
 * Reimplemented to paint the overlay's background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void CAboutOverlayGeneric::paint(Graphics& g)
{
	// First paint base class
	AboutOverlay::paint(g);

}

/**
 * Reimplemented to resize and re-postion controls & labels.
 */
void CAboutOverlayGeneric::resized()
{
	// First resize base class components
	AboutOverlay::resized();

	m_formatInfoLabel->setBounds(95, 105, getLocalBounds().getWidth() - 135, 70);
}


} // namespace SoundscapeBridgeApp
