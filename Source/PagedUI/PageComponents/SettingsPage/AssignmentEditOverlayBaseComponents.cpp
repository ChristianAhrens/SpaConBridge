/* Copyright(c) 2022, Christian Ahrens
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

#include "AssignmentEditOverlayBaseComponents.h"

#include "Image_utils.h"
#include "../../../SpaConBridgeCommon.h"
//#include "../../PageContainerComponent.h"
//#include "../../PageComponentManager.h"

namespace SpaConBridge
{

namespace AssignmentEditOverlayBaseComponents
{

AssignmentEditComponent::AssignmentEditComponent()
    : Component("AssignmentEditComponent")
{
}

AssignmentEditComponent::~AssignmentEditComponent()
{
}

AssignmentsListingComponent::AssignmentsListingComponent()
{
}

AssignmentsListingComponent::~AssignmentsListingComponent() 
{
}

void AssignmentsListingComponent::setWidth(int width)
{
    auto editsCount = m_editComponents.size();
    auto fittingColumnCount = static_cast<int>(width / (m_editorWidth + 2.0f * m_editorMargin));
    auto totalEditsHeight = (editsCount + 1) * (m_editorHeight + 2.0f * m_editorMargin);
    auto minRequiredHeight = static_cast<int>(totalEditsHeight / fittingColumnCount);

    if (minRequiredHeight < m_minHeight)
        setSize(width, m_minHeight);
    else
        setSize(width, minRequiredHeight);
}

void AssignmentsListingComponent::setMinHeight(int height)
{
    m_minHeight = height;
}

void AssignmentsListingComponent::ClearAssignments()
{
    m_editComponents.clear();
    resized();
}

void AssignmentsListingComponent::paint(Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.setColour(getLookAndFeel().findColour(AlertWindow::backgroundColourId).darker());
    g.fillRect(bounds.toFloat());
}

void AssignmentsListingComponent::resized() 
{
    auto bounds = getLocalBounds();

    juce::FlexBox editsBox;
    editsBox.flexWrap = juce::FlexBox::Wrap::wrap;
    editsBox.flexDirection = juce::FlexBox::Direction::column;
    editsBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    for (auto const& editComponent : m_editComponents)
        editsBox.items.add(juce::FlexItem(*editComponent).withHeight(m_editorHeight).withWidth(m_editorWidth).withMargin(m_editorMargin));
    editsBox.performLayout(bounds.reduced(static_cast<int>(2.0f * m_editorMargin)));
}

bool AssignmentsListingComponent::IsAvailableUiAreaExceeded()
{
    auto bounds = getLocalBounds().reduced(55, 25).toFloat();

    // don't mess up when ui simply is not yet initialized
    if (bounds.getWidth() == 0 && bounds.getHeight() == 0)
        return false;

    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    // substract controls height
    h -= 33.0f;
    if (h > 0)
    {
        auto totalElmsHeight = static_cast<float>(33 * (m_editComponents.size() + 1)); // one additional edit, to achieve the 'forecast' behaviour of the method
        auto colCount = static_cast<int>((totalElmsHeight / h) + 0.5f);

        auto requiredWidth = colCount * 210;

        return requiredWidth >= w;
    }

    return true;
}

AssignmentsViewingComponent::AssignmentsViewingComponent()
{
    m_contentViewport = std::make_unique<Viewport>();
    addAndMakeVisible(m_contentViewport.get());

    m_addButton = std::make_unique<TextButton>("Add");
    m_addButton->onClick = [this] { onAddClicked(); };
    addAndMakeVisible(m_addButton.get());

    m_clearButton = std::make_unique<TextButton>("Clear");
    m_clearButton->onClick = [this] { onClearClicked(); };
    addAndMakeVisible(m_clearButton.get());

    m_exportButton = std::make_unique<DrawableButton>("Export", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    m_exportButton->setTooltip("Export assignments");
    m_exportButton->onClick = [this] { onExportClicked(); };
    addAndMakeVisible(m_exportButton.get());

    m_importButton = std::make_unique<DrawableButton>("Import", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    m_importButton->setTooltip("Import assignments");
    m_importButton->onClick = [this] { onImportClicked(); };
    addAndMakeVisible(m_importButton.get());

    m_closeButton = std::make_unique<TextButton>("Close");
    m_closeButton->onClick = [this] { onCloseClicked(); };
    addAndMakeVisible(m_closeButton.get());

    lookAndFeelChanged();
}

AssignmentsViewingComponent::~AssignmentsViewingComponent()
{
}

void AssignmentsViewingComponent::paint(Graphics& g)
{
    // Transparent background overlay
    g.setColour(Colours::black);
    g.setOpacity(0.5f);
    g.fillRect(getLocalBounds());
    g.setOpacity(1.0f);

    auto bounds = getLocalBounds().reduced(45, 25);

    g.setColour(getLookAndFeel().findColour(AlertWindow::outlineColourId));
    g.drawRect(bounds.toFloat(), 1.0f);

    bounds.reduce(1, 1);
    g.reduceClipRegion(bounds);

    // Background
    g.setColour(getLookAndFeel().findColour(AlertWindow::backgroundColourId));
    g.fillRect(bounds.toFloat());
}

void AssignmentsViewingComponent::resized()
{
    auto bounds = getLocalBounds().reduced(45, 25);

    auto controlsBounds = bounds.removeFromBottom(35);
    m_addButton->setBounds(controlsBounds.removeFromLeft(45).reduced(6));
    m_clearButton->setBounds(controlsBounds.removeFromLeft(50).reduced(0, 6));

    if (controlsBounds.getWidth() > 122)
    {
        m_exportButton->setVisible(true);
        m_importButton->setVisible(true);

        m_exportButton->setBounds(controlsBounds.removeFromLeft(37).reduced(6));
        m_importButton->setBounds(controlsBounds.removeFromLeft(25).reduced(0, 6));
    }
    else
    {
        m_exportButton->setVisible(false);
        m_importButton->setVisible(false);
    }

    m_closeButton->setBounds(controlsBounds.removeFromRight(60).reduced(6));

    bounds.removeFromTop(6);
    bounds.reduce(6, 0);
    m_contentViewport->setBounds(bounds);

    m_contentComponent->setMinHeight(bounds.getHeight() - 2);
    m_contentComponent->setWidth(bounds.getWidth() - 2);
}

void AssignmentsViewingComponent::lookAndFeelChanged()
{
    Component::lookAndFeelChanged();

    // Update drawable button images with updated lookAndFeel colours
    UpdateDrawableButtonImages(m_importButton, BinaryData::folder_open24px_svg, &getLookAndFeel());
    UpdateDrawableButtonImages(m_exportButton, BinaryData::save24px_svg, &getLookAndFeel());
}

void AssignmentsViewingComponent::onAddClicked()
{
    m_contentComponent->AddAssignment();

    resized();
}

void AssignmentsViewingComponent::onClearClicked()
{
    m_contentComponent->ClearAssignments();

    resized();
}

}

}
