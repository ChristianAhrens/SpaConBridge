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
	Class AboutPageContentComponent
===============================================================================
*/

/**
 * Class constructor.
 */
AboutPageContentComponent::AboutPageContentComponent()
{

	// JUCE icon drawable
	m_juceIconDrawable = Drawable::createFromImageData(BinaryData::logo_juce_svg, BinaryData::logo_juce_svgSize);
	addAndMakeVisible(m_juceIconDrawable.get());
	// JUCE copyright label
	String juceLabelString = String("Made with JUCE.\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2020 - ROLI Ltd.");
	m_juceLabel = std::make_unique<Label>("JuceLabel", juceLabelString);
	m_juceLabel->setJustificationType(Justification::topRight);
	m_juceLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_juceLabel.get());

	// App icon drawable
	m_appIconDrawable = Drawable::createFromImageData(BinaryData::SoundscapeBridgeApp_png, BinaryData::SoundscapeBridgeApp_pngSize);
	addAndMakeVisible(m_appIconDrawable.get());
	// App info label
	String infoString = String("SoundscapeBridgeApp V") + String(JUCE_STRINGIFY(JUCE_APP_VERSION));
	infoString += String("\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2020\nChristian Ahrens,\nall rights reserved.");
	m_appInfoLabel = std::make_unique<Label>("Version", infoString);
	m_appInfoLabel->setJustificationType(Justification::topLeft);
	m_appInfoLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_appInfoLabel.get());

	// Hyperlink to dbaudio.com
	m_githubLink = std::make_unique<HyperlinkButton>(JUCEApplication::getInstance()->getApplicationName() + String(" home on GitHub"), URL("https://www.github.com/ChristianAhrens/SoundscapeBridgeApp"));
	m_githubLink->setFont(Font(13.0, Font::plain), false /* do not resize */);
	addAndMakeVisible(m_githubLink.get());

	// Enduser License Agreement
	String eula(BinaryData::EULA, BinaryData::EULASize);
	m_eulaField = std::make_unique<TextEditor>("eula");
	m_eulaField->setReadOnly(true);
	m_eulaField->setFont(Font(13.0, Font::plain));
	m_eulaField->setCaretVisible(false);
	m_eulaField->setMultiLine(true, false /* no wrapping */);
	m_eulaField->setScrollbarsShown(true);
	m_eulaField->setText(eula, false);
	addAndMakeVisible(m_eulaField.get());

	// GPLv3
	String GPLv3(BinaryData::LICENSE, BinaryData::LICENSESize);
	m_gplField = std::make_unique<TextEditor>("GPLv3");
	m_gplField->setReadOnly(true);
	m_gplField->setFont(Font(13.0, Font::plain));
	m_gplField->setCaretVisible(false);
	m_gplField->setMultiLine(true, false /* no wrapping */);
	m_gplField->setScrollbarsShown(true);
	m_gplField->setText(GPLv3, false);
	addAndMakeVisible(m_gplField.get());
}

/**
 * Class destructor.
 */
AboutPageContentComponent::~AboutPageContentComponent()
{
}

/**
 * Reimplemented to resize and re-postion controls & labels.
 */
void AboutPageContentComponent::resized()
{
	auto juceInfoHeight = 70;
	auto appInfoHeight = 55;
	auto gitHubLinkHeight = 18;
	auto eulaHeight = 305;
	auto gplHeight = 8850;
	auto totalHeight = juceInfoHeight + appInfoHeight + gitHubLinkHeight + 15 + eulaHeight + 15 + gplHeight;
	setBounds(Rectangle<int>(getLocalBounds().getWidth(), totalHeight));

	auto bounds = getLocalBounds();

	// juce copyright text under the logo
	auto juceInfoBounds = bounds.removeFromTop(juceInfoHeight);
	auto juceDrawableBounds = juceInfoBounds;
	juceDrawableBounds = juceDrawableBounds.removeFromRight(175).removeFromLeft(100).removeFromTop(35);
	m_juceIconDrawable->setTransformToFit(juceDrawableBounds.toFloat(), RectanglePlacement(RectanglePlacement::stretchToFit));
	auto juceLabelBounds = juceInfoBounds.removeFromBottom(40).removeFromRight(210).removeFromLeft(200);
	m_juceLabel->setBounds(juceLabelBounds);

	// app info text right of app logo
	auto appInfoBounds = bounds.removeFromTop(appInfoHeight);
	auto appDrawableBounds = appInfoBounds.removeFromLeft(70).removeFromRight(55);
	m_appIconDrawable->setTransformToFit(appDrawableBounds.toFloat(), RectanglePlacement(RectanglePlacement::stretchToFit));
	auto infoLabelBounds = appInfoBounds;
	m_appInfoLabel->setBounds(infoLabelBounds);

	// github link below app info
	auto githubLinkBounds = bounds.removeFromTop(gitHubLinkHeight).removeFromLeft(270);
	m_githubLink->setBounds(githubLinkBounds);

	auto textBoxesBounds = bounds.reduced(15);

	auto eulaBounds = textBoxesBounds.removeFromTop(eulaHeight);
	m_eulaField->setBounds(eulaBounds);

	textBoxesBounds.removeFromTop(15);

	auto gplBounds = textBoxesBounds;
	m_gplField->setBounds(gplBounds);
}


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

	m_aboutContents = std::make_unique<AboutPageContentComponent>();
	m_aboutViewport = std::make_unique<Viewport>();
	m_aboutViewport->setViewedComponent(m_aboutContents.get(), false);
	addAndMakeVisible(m_aboutViewport.get());
    
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

	bounds.reduce(2, 2);
	m_aboutViewport->setBounds(bounds);

	bounds.reduce(4, 0);
	m_aboutContents->setBounds(bounds);

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
