/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
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


#include "AboutPageComponent.h"

#include "../../../LookAndFeel.h"
#include "../../../SoundscapeBridgeAppCommon.h"

#include <Image_utils.h>


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
	// App icon drawable
	m_appIconDrawable = Drawable::createFromImageData(BinaryData::SoundscapeBridgeApp_png, BinaryData::SoundscapeBridgeApp_pngSize);
	addAndMakeVisible(m_appIconDrawable.get());
	// App info label
	String infoString = JUCEApplication::getInstance()->getApplicationName() + String(" V") + String(JUCE_STRINGIFY(JUCE_APP_VERSION)) + String("\n")
                    + String("Copyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2021 - Christian Ahrens,\n")
                    + JUCEApplication::getInstance()->getApplicationName() + String(" uses GPLv3");
	m_appInfoLabel = std::make_unique<Label>("Version", infoString);
	m_appInfoLabel->setJustificationType(Justification::topLeft);
	m_appInfoLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_appInfoLabel.get());

	// Hyperlink to dbaudio.com
	m_appInfoLink = std::make_unique<HyperlinkButton>(JUCEApplication::getInstance()->getApplicationName() + String(" on GitHub"), URL("https://www.github.com/ChristianAhrens/SoundscapeBridgeApp"));
	m_appInfoLink->setFont(Font(13.0, Font::plain), false /* do not resize */);
    m_appInfoLink->setJustificationType(Justification::centredLeft);
	addAndMakeVisible(m_appInfoLink.get());

	// JUCE icon drawable
	m_juceIconDrawable = Drawable::createFromImageData(BinaryData::logo_juce_svg, BinaryData::logo_juce_svgSize);
	addAndMakeVisible(m_juceIconDrawable.get());
	// JUCE copyright label
	String juceLabelString = String("Made with JUCE.\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2020 - ROLI Ltd.\nJUCE uses GPLv3");
	m_juceLabel = std::make_unique<Label>("JuceLabel", juceLabelString);
	m_juceLabel->setJustificationType(Justification::topRight);
	m_juceLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_juceLabel.get());
	// JUCE link
	m_juceLink = std::make_unique<HyperlinkButton>("JUCE.com", URL("https://juce.com/"));
	m_juceLink->setFont(Font(13.0, Font::plain), false /* do not resize */);
	m_juceLink->setJustificationType(Justification::centredLeft);
	addAndMakeVisible(m_juceLink.get());

	// MATERIAL.IO icon drawable
	m_materialIconDrawable = Drawable::createFromImageData(BinaryData::MaterialDesignLogo_png, BinaryData::MaterialDesignLogo_pngSize);
	addAndMakeVisible(m_materialIconDrawable.get());
	// MATERIAL.IO copyright label
	String materialLabelString = String("Material.io Icon Theme.\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2020 - Google, Inc.\nMaterial Icons uses Apache License v2.0");
	m_materialLabel = std::make_unique<Label>("MaterialLabel", materialLabelString);
	m_materialLabel->setJustificationType(Justification::topRight);
	m_materialLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_materialLabel.get());
	// MATERIAL.IO link
	m_materialLink = std::make_unique<HyperlinkButton>("material.io", URL("https://material.io"));
	m_materialLink->setFont(Font(13.0, Font::plain), false /* do not resize */);
	m_materialLink->setJustificationType(Justification::centredLeft);
	addAndMakeVisible(m_materialLink.get());

	// HBP (Servus) icon drawable
	m_servusIconDrawable = Drawable::createFromImageData(BinaryData::HumanBrainProjectLogo_png, BinaryData::HumanBrainProjectLogo_pngSize);
	addAndMakeVisible(m_servusIconDrawable.get());
	// HBP (Servus) copyright label
	String servusLabelString = String("Servus Zeroconf API.\nCopyright ") + String(CharPointer_UTF8("\xc2\xa9")) + String(" 2014 - 2015, Human Brain Project\nServus uses LGPLv3");
	m_servusLabel = std::make_unique<Label>("ServusLabel", servusLabelString);
	m_servusLabel->setJustificationType(Justification::topRight);
	m_servusLabel->setFont(Font(13.0, Font::plain));
	addAndMakeVisible(m_servusLabel.get());
	// HBP (Servus) github link
	m_servusLink = std::make_unique<HyperlinkButton>("Servus on GitHub", URL("https://github.com/HBPVIS/Servus"));
	m_servusLink->setFont(Font(13.0, Font::plain), false /* do not resize */);
	m_servusLink->setJustificationType(Justification::centredLeft);
	addAndMakeVisible(m_servusLink.get());

	// GPLv3 License
	String LicenseGPLv3(BinaryData::COPYING, BinaryData::COPYINGSize);
	m_licenseGPLv3Field = std::make_unique<TextEditor>("GPLv3License");
	m_licenseGPLv3Field->setReadOnly(true);
	m_licenseGPLv3Field->setPopupMenuEnabled(false);
	m_licenseGPLv3Field->setFont(Font(13.0, Font::plain));
	m_licenseGPLv3Field->setCaretVisible(false);
	m_licenseGPLv3Field->setMultiLine(true, false /* no wrapping */);
	m_licenseGPLv3Field->setScrollbarsShown(true);
	m_licenseGPLv3Field->setText(LicenseGPLv3, false);
	addAndMakeVisible(m_licenseGPLv3Field.get());

	// Apache License v2
	String LicenseApachev2(BinaryData::COPYING_apachev2, BinaryData::COPYING_apachev2Size);
	m_licenseAPACHEv2Field = std::make_unique<TextEditor>("APACHEv2License");
	m_licenseAPACHEv2Field->setReadOnly(true);
	m_licenseAPACHEv2Field->setPopupMenuEnabled(false);
	m_licenseAPACHEv2Field->setFont(Font(13.0, Font::plain));
	m_licenseAPACHEv2Field->setCaretVisible(false);
	m_licenseAPACHEv2Field->setMultiLine(true, false /* no wrapping */);
	m_licenseAPACHEv2Field->setScrollbarsShown(true);
	m_licenseAPACHEv2Field->setText(LicenseApachev2, false);
	addAndMakeVisible(m_licenseAPACHEv2Field.get());

	// LGPLv3 License
	String LicenseLGPLv3(BinaryData::COPYING_LESSER, BinaryData::COPYING_LESSERSize);
	m_licenseLGPLv3Field = std::make_unique<TextEditor>("LGPLv3License");
	m_licenseLGPLv3Field->setReadOnly(true);
	m_licenseLGPLv3Field->setPopupMenuEnabled(false);
	m_licenseLGPLv3Field->setFont(Font(13.0, Font::plain));
	m_licenseLGPLv3Field->setCaretVisible(false);
	m_licenseLGPLv3Field->setMultiLine(true, false /* no wrapping */);
	m_licenseLGPLv3Field->setScrollbarsShown(true);
	m_licenseLGPLv3Field->setText(LicenseLGPLv3, false);
	addAndMakeVisible(m_licenseLGPLv3Field.get());
}

/**
 * Class destructor.
 */
AboutPageContentComponent::~AboutPageContentComponent()
{
}

/**
 * Reimplemented to draw horizontal layout lines
 */
void AboutPageContentComponent::paint(Graphics& g)
{
	auto headlineLineOffset = static_cast<float>(55 + 18 + 15);

	g.setColour(getLookAndFeel().findColour(TextEditor::outlineColourId));
	g.drawLine(Line<float>(15.0f, headlineLineOffset, getWidth() - 15.0f, headlineLineOffset));
}

/**
 * Reimplemented to resize and re-postion controls & labels.
 */
void AboutPageContentComponent::resized()
{
    auto appInfoWidth = 295;
	auto appInfoHeight = 55;
	auto appInfoLinkHeight = 18;

	auto spacing = 15;
	auto infoSpacing = spacing + 5;
	auto juceInfoHeight = 70;
	auto juceDrawableHeight = 35;
	auto materialInfoHeight = 80;
	auto materialDrawableHeight = 52;
	auto servusInfoHeight = 80;
	auto servusDrawableHeight = 48;

	auto GPLv3LicenseHeight = 8810;
	auto APACHEv2LicenseHeight = 2680;
	auto LGPLv3LicenseHeight = 2200;

	auto totalHeight =  appInfoHeight + appInfoLinkHeight + spacing + juceInfoHeight + materialInfoHeight + servusInfoHeight + infoSpacing + GPLv3LicenseHeight + spacing + APACHEv2LicenseHeight + spacing + LGPLv3LicenseHeight;
	setBounds(Rectangle<int>(getLocalBounds().getWidth(), totalHeight));

	auto bounds = getLocalBounds().reduced(spacing, spacing);

	// app info text right of app logo
    auto appInfoBounds = bounds.removeFromTop(appInfoHeight + appInfoLinkHeight);
    auto appInfoLeftPadding = (bounds.getWidth() - appInfoWidth) / 2;
    appInfoBounds.removeFromLeft(appInfoLeftPadding > 0 ? appInfoLeftPadding : 0);
    auto appInfoLinkBounds = appInfoBounds;
	auto appDrawableBounds = appInfoBounds.removeFromLeft(appInfoHeight).removeFromTop(appInfoHeight);
	m_appIconDrawable->setTransformToFit(appDrawableBounds.toFloat(), RectanglePlacement(RectanglePlacement::stretchToFit));
	auto appInfoLabelBounds = appInfoBounds;
	m_appInfoLabel->setBounds(appInfoLabelBounds);
	// app link below app info
    appInfoLinkBounds.removeFromLeft(appInfoHeight + 3);
	m_appInfoLink->setBounds(appInfoLinkBounds.removeFromTop(appInfoHeight + 3).removeFromBottom(appInfoLinkHeight));

	bounds.removeFromTop(spacing);

	// juce copyright text right of the logo
	auto juceInfoBounds = bounds.removeFromTop(juceInfoHeight).removeFromLeft(370);
	auto juceDrawableBounds = juceInfoBounds.removeFromLeft(100);
	auto juceLinkBounds = juceDrawableBounds;
	juceDrawableBounds = juceDrawableBounds.removeFromTop(juceDrawableHeight);
	m_juceIconDrawable->setTransformToFit(juceDrawableBounds.toFloat(), RectanglePlacement(RectanglePlacement::stretchToFit));
	auto juceLabelBounds = juceInfoBounds.removeFromRight(270);
	m_juceLabel->setBounds(juceLabelBounds);
	// juce link below info
	m_juceLink->setBounds(juceLinkBounds.removeFromBottom(juceLinkBounds.getHeight() - juceDrawableHeight + 10));

	// material copyright text right of the logo
	auto materialInfoBounds = bounds.removeFromTop(materialInfoHeight).removeFromLeft(370);
	auto materialDrawableBounds = materialInfoBounds.removeFromLeft(100);
	auto materialLinkBounds = materialDrawableBounds;
	materialDrawableBounds = materialDrawableBounds.removeFromTop(materialDrawableHeight);
	m_materialIconDrawable->setTransformToFit(materialDrawableBounds.toFloat(), RectanglePlacement(RectanglePlacement::stretchToFit));
	auto materialLabelBounds = materialInfoBounds.removeFromRight(270);
	m_materialLabel->setBounds(materialLabelBounds);
	// material link below info
	m_materialLink->setBounds(materialLinkBounds.removeFromBottom(materialLinkBounds.getHeight() - materialDrawableHeight + 10));

	// servus copyright text right of the logo
	auto servusInfoBounds = bounds.removeFromTop(servusInfoHeight).removeFromLeft(370);
	auto servusDrawableBounds = servusInfoBounds.removeFromLeft(100);
	auto servusLinkBounds = servusDrawableBounds;
	servusDrawableBounds = servusDrawableBounds.removeFromTop(servusDrawableHeight);
	m_servusIconDrawable->setTransformToFit(servusDrawableBounds.removeFromLeft(servusDrawableHeight).toFloat(), RectanglePlacement(RectanglePlacement::stretchToFit));
	auto servusLabelBounds = servusInfoBounds.removeFromRight(270);
	m_servusLabel->setBounds(servusLabelBounds);
	// servus link below info
	m_servusLink->setBounds(servusLinkBounds.removeFromBottom(servusLinkBounds.getHeight() - servusDrawableHeight + 10));

	// GPLv3 Textfield
	auto GPLv3Bounds = bounds.removeFromTop(GPLv3LicenseHeight);
	m_licenseGPLv3Field->setBounds(GPLv3Bounds);

	bounds.removeFromTop(spacing);

	// Apachev2 Textfield
	auto APACHEv2Bounds = bounds.removeFromTop(APACHEv2LicenseHeight);
	m_licenseAPACHEv2Field->setBounds(APACHEv2Bounds);

	bounds.removeFromTop(spacing);

	// LGPLv3 Textfield
	auto LGPLv3Bounds = bounds.removeFromTop(LGPLv3LicenseHeight);
	m_licenseLGPLv3Field->setBounds(LGPLv3Bounds);
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
 * @param init	True to ignore any changed flags and update the processor parameters
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
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		JUCEAppBasics::Image_utils::getDrawableButtonImages(imageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

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
