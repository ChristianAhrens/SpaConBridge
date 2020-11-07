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


#include "AboutPageComponent.h"

#include "../../LookAndFeel.h"
#include "../../SoundscapeBridgeAppCommon.h"

#include "../submodules/JUCE-AppBasics/Source/Image_utils.hpp"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class AboutPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
AboutPageComponent::AboutPageComponent()
	: PageComponentBase(PCT_About)
{
	// Close button
	m_closeButton = std::make_unique<DrawableButton>("Close", DrawableButton::ButtonStyle::ImageFitted);
	m_closeButton->onClick = [=] { onCloseClick(); };
	addAndMakeVisible(m_closeButton.get());
	lookAndFeelChanged();

	// Plugin version label
	String versionString = String("SoundscapeBridgeApp V") + String(JUCE_STRINGIFY(JUCE_APP_VERSION));
	versionString += String("\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2020\nChristian Ahrens,\nall rights reserved.");
	m_versionLabel = std::make_unique<Label>("Version", versionString);
	m_versionLabel->setJustificationType(Justification::topLeft);
	m_versionLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_versionLabel.get());

	// Hyperlink to dbaudio.com
	m_githubLink = std::make_unique<HyperlinkButton>(JUCEApplication::getInstance()->getApplicationName() + String(" home on GitHub"), URL("www.github.com/ChristianAhrens/SoundscapeBridgeApp"));
	m_githubLink->setFont(Font(13.0, Font::plain), false /* do not resize */);
	addAndMakeVisible(m_githubLink.get());

	String eula(
		//"End-User License Agreement (\"Agreement\") for d&b Soundscape DAW Plug-in (\"Software\")\n" \
		//"======================================================================\n" \
		//"This is a legal Agreement between the end user (\"you\") and d&b audiotechnik GmbH & Co. KG, Eugen-Adolff-Strasse 134, 71522 Backnang, Germany (\"d&b audiotechnik\"). \n" \
		//"1.  By downloading, installing or using the Software you agree to the terms of this Agreement. If you do not agree to the terms of this Agreement you must cease and desist from down-loading, installing and/or using of the Software.\n" \
		//"2.  The Software is intended solely for use by Entrepreneurs. An \"Entrepreneur\" is every natural person or legal entity acting in his/her or its professional or self-employed capacity when entering into this Agreement. If you are not an Entrepreneur and still wish to use the Software, please contact d&b audiotechnik directly.\n" \
		//"3.  Please note, that the Software is not a stand-alone executable software. To use the Soft-ware third party software is necessary which is not part of the Software and which is subject to its own license terms and has to be provided for by you on your own expenses and responsibility.\n" \
		//"4.  d&b audiotechnik grants you for the duration of the protection of the Software a non-exclusive, non-sublicensable right to use the Software for your own purposes subject to the terms and conditions of this Agreement. All rights to the Software are owned by d&b audiotechnik or its respective licensors. You may NOT copy the documentation accompanying the Software.\n" \
		//"5.  Any such right to use does only apply to the object code of the Software, which means the Software in a form readable solely by machines. You do not have a claim to being provided with the source code or parts of the source code and will not receive any rights to use or otherwise exploit the source code. In this regard, source code means the Software's source text, written in a programming language in a human readable form.\n" \
		//"6.  Subject to the mandatory limitations according to applicable copyright law, you may NOT (i) reverse engineer, disassemble, decompile or otherwise reduce the Software to a human perceivable version, nor shall you permit others to do so, except and only to the ex-tent that such activity is expressly permitted by applicable law notwithstanding this limitation, (ii) modify, adapt, rent, lease, resell, distribute, network or create derivative works based upon the Software or any part thereof.\n" \
		//"7.  This Agreement is immediately terminated if you violate the terms and conditions hereof. You agree upon such termination to cease and desist from using the Software and to destroy the Software together with all copies.\n" \
		//"8.  Limitations of Liability:\n" \
		//" a. d&b audiotechnik shall bear liability for material defects and defects in title in the Software and its content and information (warranty for defects) only if d&b audiotechnik has fraudulently concealed a defect and/or has assumed a guarantee.\n" \
		//" b. Outside of the warranty for defects, d&b audiotechnik shall be liable only in cases of intent (Vorsatz) and gross negligence (grobe Fahrlaessigkeit), pursuant to the provisions of the Product Liability Act (Produkthaftungsgesetz) and in all other cases subject to statutory mandatory liability, in each case according to the statutory provisions.\n" \
		//" c. Otherwise, d&b audiotechnik's liability is hereby excluded.\n" \
		//" d. Where d&b audiotechnik's liability is restricted or excluded according to the provisions above, this shall also apply to the personal liability of the statutory representatives, employees and vicarious agents of d&b audiotechnik, as well as for indirect damages and consequential damages (e.g. loss of data, damage to your hardware or software, disruption of operations, stoppages in production, loss of profit).\n" \
		//" e. You bear sole responsibility for accuracy of the data and information entered for use of the Software, including interpretation of the results delivered by the Software.\n" \
		//"9.  You are entitled to provide a third party with the original version of the Software together with a copy of this Agreement if this third party is an Entrepreneur and expressly consents in writing to the application of this Agreement for any use of the Software. As soon as you pass on the Software to the third party you should immediately notify d&b audiotechnik. Notification should, at least, include the date of transfer of the Software and the contact details of the new user. When passing on the Software, you shall promptly and completely delete or otherwise destroy all of your other copies of the Software.\n" \
		//"10. This Agreement shall be governed by the laws of Germany.\n" \
		//"If you have any questions concerning this Agreement, please contact d&b audiotechnik's support.");
		"Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.  \n" \
		"At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, \n" \
		"consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo  \n" \
		"duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, \n" \
		"sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum.  \n" \
		"Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.    \n" \
		"\n" \
		"Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan  \n" \
		"et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.Lorem ipsum dolor sit amet, consectetuer adipiscing elit, \n" \
		"sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. \n" \
		"\n" \
		"Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. \n" \
		"Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis  \n" \
		"at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi." \
		"\n" \
		"Nam liber tempor cum soluta nobis eleifend option congue nihil imperdiet doming id quod mazim placerat facer possim assum.Lorem ipsum dolor sit amet,  \n" \
		"consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat.Ut wisi enim ad minim veniam, \n" \
		"quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. \n" \
		"\n" \
		"Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis. \n" \
		"\n" \
		"At vero eos et accusam et justo duo dolores et ea rebum.Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur" \
	);
	m_eulaField = std::make_unique<TextEditor>("eula");
	m_eulaField->setReadOnly(true);
	m_eulaField->setFont(Font(13.0, Font::plain));
	m_eulaField->setCaretVisible(false);
	m_eulaField->setMultiLine(true, false /* no wrapping */);
	m_eulaField->setScrollbarsShown(true);
	m_eulaField->setText(eula, false);
	addAndMakeVisible(m_eulaField.get());

	// JUCE copyright label
	String juceLabelString = String("Made with JUCE.\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2020 - ROLI Ltd.");
	m_juceLabel = std::make_unique<Label>("JuceLabel", juceLabelString);
	m_juceLabel->setJustificationType(Justification::topRight);
	m_juceLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_juceLabel.get());
}

/**
 * Class destructor.
 */
AboutPageComponent::~AboutPageComponent()
{
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * NOTE: this reimplementation does nothing, since these pages are static.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void AboutPageComponent::UpdateGui(bool init)
{
	ignoreUnused(init);
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the close buttons' svg images are colored correctly.
 */
void AboutPageComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	PageComponentBase::lookAndFeelChanged();

	// create the required button drawable images based on lookandfeel colours
	String imageName = BinaryData::cancel24px_svg;
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	auto lookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (lookAndFeel)
	{
		JUCEAppBasics::Image_utils::getDrawableButtonImages(imageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_closeButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}
}

/**
 * Reimplemented to paint the overlay's background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void AboutPageComponent::paint(Graphics& g)
{
	// Transparent background overlay
	g.setColour(Colours::black);
	g.setOpacity(0.5f);
	g.fillRect(getLocalBounds());
	g.setOpacity(1.0f);

	auto bounds = getLocalBounds().reduced(25);

	// Background
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
	g.setColour(getLookAndFeel().findColour(TextEditor::outlineColourId));
	g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 3.0f);

	// App logo
	auto appLogoBounds = bounds.removeFromLeft(75).removeFromRight(60).removeFromTop(155).removeFromBottom(60);
	Image appLogo = ImageCache::getFromMemory(BinaryData::SoundscapeBridgeApp_png, BinaryData::SoundscapeBridgeApp_pngSize);
	g.drawImage(appLogo, appLogoBounds.toFloat());

	// JUCE logo
	auto juceLogoBounds = bounds.removeFromRight(180).removeFromLeft(100).removeFromTop(55).removeFromBottom(35);
	std::unique_ptr<Drawable> formatLogo = Drawable::createFromImageData(BinaryData::logo_juce_svg, BinaryData::logo_juce_svgSize);
	formatLogo->drawWithin(g, juceLogoBounds.toFloat(), RectanglePlacement::stretchToFit, 1.0f);
}

/**
 * Reimplemented to resize and re-postion controls & labels.
 */
void AboutPageComponent::resized()
{
	auto bounds = getLocalBounds().reduced(25);

	// close button in upper right corner
	auto closeButtonBounds = bounds.removeFromTop(30).removeFromBottom(25).removeFromRight(30).removeFromLeft(25);
	m_closeButton->setBounds(closeButtonBounds);

	// juce copyright text under the logo that is drawn in ::paint
	auto juceLabelBounds = bounds.removeFromTop(60).removeFromBottom(40).removeFromRight(210).removeFromLeft(200);
	m_juceLabel->setBounds(juceLabelBounds);

	// app info text right of app logo that is drawn in ::paint as well
	auto versionLabelBounds = bounds.removeFromTop(55).removeFromLeft(290).removeFromRight(220);
	m_versionLabel->setBounds(versionLabelBounds);
	// github link below app info
	auto githubLinkBounds = bounds.removeFromTop(18).removeFromLeft(290).removeFromRight(220);
	m_githubLink->setBounds(githubLinkBounds);

	auto eulaBounds = bounds.reduced(15);
	m_eulaField->setBounds(eulaBounds);
}

/**
 * Called when the mouse button is released.
 * Reimplemented just to call EndGuiGesture() to inform the host.
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void AboutPageComponent::mouseUp(const MouseEvent& e)
{
	auto clickPos = e.getMouseDownPosition();
	auto bounds = getLocalBounds().reduced(35);

	if (!bounds.contains(clickPos) && onCloseClick)
	{
		onCloseClick();
	}
}


} // namespace SoundscapeBridgeApp
