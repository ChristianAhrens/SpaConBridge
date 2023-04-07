/* Copyright(c) 2023, Christian Ahrens
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

namespace SpaConBridge
{

namespace AssignmentEditOverlayBaseComponents
{

class AssignmentEditComponent : public Component
{
public:
    explicit AssignmentEditComponent();
    ~AssignmentEditComponent();
};

class AssignmentsListingComponent : public Component
{
public:
    explicit AssignmentsListingComponent();
    ~AssignmentsListingComponent();

    virtual void setWidth(int width) = 0;
    void setMinHeight(int height);

    //==============================================================================
    virtual bool AddAssignment() = 0;
    void ClearAssignments();

    //==============================================================================
    virtual const String DumpCurrentAssignmentsToCsvString() = 0;
    virtual bool ReadAssignmentsFromCsvString(const String& csvAssignmentsString) = 0;

    //==============================================================================
    void paint(Graphics&) override;
    virtual void resized() override = 0;

protected:
    //==============================================================================
    std::vector<std::unique_ptr<AssignmentEditComponent>>   m_editComponents;

    //==============================================================================
    bool IsAvailableUiAreaExceeded();
    
    //==============================================================================
    int m_editorWidth;
    int m_editorHeight;
    int m_editorMargin;
    int m_minHeight;

};

class AssignmentsViewingComponent : public Component
{
public:
    explicit AssignmentsViewingComponent();
    ~AssignmentsViewingComponent();

    //==============================================================================
    void onAddClicked();
    void onClearClicked();
    virtual void onExportClicked() = 0;
    virtual void onImportClicked() = 0;
    virtual void onCloseClicked() = 0;

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==========================================================================
    void lookAndFeelChanged() override;

    //==============================================================================
    void SetPreferredWidth(int width);

protected:
    //==============================================================================
    std::unique_ptr<AssignmentsListingComponent>    m_contentComponent;
    std::unique_ptr<Viewport>					    m_contentViewport;

private:
    //==============================================================================
    std::unique_ptr<TextButton>                     m_addButton;
    std::unique_ptr<TextButton>                     m_clearButton;
    std::unique_ptr<DrawableButton>                 m_exportButton;
    std::unique_ptr<DrawableButton>                 m_importButton;
    std::unique_ptr<TextButton>                     m_closeButton;

    int m_preferredWidth{ -1 };

};

};

};
