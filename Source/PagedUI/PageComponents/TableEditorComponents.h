/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in and now in a derived version is part of SpaConBridge.

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

#include "../../SpaConBridgeCommon.h"
#include "../../AppConfiguration.h"
#include "../../../submodules/JUCE-AppBasics/Source/ColourAndSizePickerComponent.h"


namespace SpaConBridge
{


/**
 * Forward declarations
 */
class TableModelComponent;

/**
 * Class TableEditorComponent is an abstract base class for editor containers
 * used in tablemodelcomponent for soundobject/matrixinput/matrixoutput tables.
 * 
 */
class TableEditorComponent : public Component
{
public:
	explicit TableEditorComponent(TableModelComponent& td);
	~TableEditorComponent() override;

	TableModelComponent& GetParentTable();

	int GetRow();
	virtual void SetRow(int newRow);

private:
	TableModelComponent&	m_owner;		/**> Table where this component is contained. */
	int						m_row;			/**> Row number where this component is located inside the table. */
};

/**
 * Class ComboBoxContainer is a container for the MappingId Combo box component used in the Overview table.
 */
class ComboBoxContainer : public TableEditorComponent,
	public ComboBox::Listener
{
public:
	explicit ComboBoxContainer(TableModelComponent& td);
	~ComboBoxContainer() override;

	void SetRow(int newRow) override;

	void comboBoxChanged(ComboBox *comboBox) override;

	void resized() override;

private:
	ComboBox				m_comboBox;	/**> Actual combo box component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboBoxContainer)
};

/**
 * Class LabelContainer is a container for a label component used in the Overview table.
 */
class LabelContainer : public TableEditorComponent
{
public:
	explicit LabelContainer(TableModelComponent& td);
	~LabelContainer() override;

	void SetRow(int newRow) override;

	void resized() override;

private:
	Label	m_label;	/**> Actual label component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabelContainer)
};

/**
 * Class TextEditorContainer is a container for the SourceID TextEditor component used in the Overview table.
 */
class TextEditorContainer : public TableEditorComponent,
	public TextEditor::Listener
{
public:
	explicit TextEditorContainer(TableModelComponent& td);
	virtual ~TextEditorContainer() override;

	void SetRow(int newRow) override;

	void textEditorReturnKeyPressed(TextEditor &) override;

	void resized() override;

	void textEditorFocusLost(TextEditor&) override;

protected:
	TextEditor				m_editor;	/**> Actual text editor. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextEditorContainer)
};

/**
 * Class RadioButtonContainer is a container for the Tx/Rx buttons used in the Overview table.
 */
class RadioButtonContainer : public TableEditorComponent,
	public Button::Listener
{
public:
	explicit RadioButtonContainer(TableModelComponent& td);
	~RadioButtonContainer() override;

	void SetRow(int newRow) override;

	void lookAndFeelChanged() override;

	void buttonClicked(Button*) override;

	void resized() override;

private:
	DrawableButton			m_txButton;	/**> Actual Tx button. */
	DrawableButton			m_rxButton;	/**> Actual Rx button. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RadioButtonContainer)
};

/**
 * Class MuteButtonContainer is a container for the Bridging Mute buttons used in the Overview table.
 */
class MuteButtonContainer : public TableEditorComponent,
	public Button::Listener
{
public:
	explicit MuteButtonContainer(TableModelComponent& td);
	~MuteButtonContainer() override;

	void updateBridgingMuteButtons();

	void SetRow(int newRow) override;

	void lookAndFeelChanged() override;

	void buttonClicked(Button*) override;

	void resized() override;

private:
	std::map<ProtocolBridgingType, std::unique_ptr<DrawableButton>>	m_bridgingMutes;	/**< The mute buttons currently in use. */

	std::unique_ptr<juce::Drawable> m_NormalImage;
	std::unique_ptr<juce::Drawable> m_OverImage;
	std::unique_ptr<juce::Drawable> m_DownImage;
	std::unique_ptr<juce::Drawable> m_DisabledImage;
	std::unique_ptr<juce::Drawable> m_NormalOnImage;
	std::unique_ptr<juce::Drawable> m_OverOnImage;
	std::unique_ptr<juce::Drawable> m_DownOnImage;
	std::unique_ptr<juce::Drawable> m_DisabledOnImage;
	juce::Colour m_redColour;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MuteButtonContainer)
};

/**
 * Class ColourAndSizePickerContainer is a container for the colour and size picking popup buttons used in the Soundobjects table.
 */
class ColourAndSizePickerContainer : public TableEditorComponent
{
public:
	explicit ColourAndSizePickerContainer(TableModelComponent& td);
	~ColourAndSizePickerContainer() override;

	void SetRow(int newRow) override;

	void resized() override;

private:
	void SetSoundObjectColourAndSize(const juce::Colour& colour, double size);

	JUCEAppBasics::ColourAndSizePickerComponent		m_colourAndSizePicker;	/**< The actual colour and size picker component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColourAndSizePickerContainer)
};


} // namespace SpaConBridge
