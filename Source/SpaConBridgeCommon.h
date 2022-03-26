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

#include <RemoteProtocolBridgeCommon.h>


namespace SpaConBridge
{


/**
 * Type definitions.
 */
typedef juce::int32 SoundobjectId;
typedef juce::int8 MappingId;
typedef juce::int32 MatrixInputId;
typedef juce::int32 MatrixOutputId;
typedef juce::int32 SoundobjectProcessorId;
typedef juce::int32 MatrixInputProcessorId;
typedef juce::int32 MatrixOutputProcessorId;
typedef juce::uint64 DataChangeType;
typedef juce::uint32 ProtocolBridgingType;
typedef juce::uint8 ComsMode;

/**
 * UI Page Indices
 */
enum UIPageId
{
	UPI_InvalidMin = 0,
	UPI_SoundObjects,
	UPI_MultiSlider,
	UPI_MatrixIOs,
	UPI_Scenes,
	UPI_EnSpace,
	UPI_Statistics,
	UPI_Settings,
	UPI_InvalidMax
};
String GetPageNameFromId(UIPageId pageId);
UIPageId GetPageIdFromName(String pageName);

/**
 * Data Change Participant (Source or Sink)
 * Enum used to define where a parameter or property change has originated or is processed in.
 */
enum DataChangeParticipant
{
	DCP_SoundobjectProcessor = 0,	//< Change was caused or is queried by the SoundobjectProcessor UI, i.e. the user turning a knob to change a value.
	DCP_SoundobjectTable,			//< Change was caused or is queried by the soundsource overview table
	DCP_MultiSlider,				//< Change was caused or is queried by the multislider
	DCP_Settings,					//< Change was caused or is queried by the SettingsPage UI.	
	DCP_Host,						//< Change was caused or is queried by the VST/AU/AAX host, i.e. a project was loaded or a DAW preset was recalled.
	DCP_Protocol,					//< Change was caused or is queried by an incoming protocol message, or caused by internal operations by the Controller.
	DCP_Init,						//< Change was caused or is queried by Application initialization process (defaults)
	DCP_MatrixInputProcessor,		//< Change was caused or is queried by the MatrixInputProcessor UI, i.e. the user turning a knob to change a value.
	DCP_MatrixInputTable,			//< Change was caused or is queried by the matrix inputs table
	DCP_MatrixOutputProcessor,		//< Change was caused or is queried by the MatrixOutputProcessor UI, i.e. the user turning a knob to change a value.
	DCP_MatrixOutputTable,			//< Change was caused or is queried by the matrix outputs table
	DCP_MatrixIO,					//< Change was caused or is queried by the matrix io channel page
	DCP_PageContainer,				//< Change was caused or is queried by the page container base component
	DCP_Max							//< Number of change sources.
};

/**
 * Soundobject parameter indices
 */
enum SoundobjectParameterIndex
{
	SPI_ParamIdx_X = 0,
	SPI_ParamIdx_Y,
	SPI_ParamIdx_ReverbSendGain,
	SPI_ParamIdx_ObjectSpread,
	SPI_ParamIdx_DelayMode,
	SPI_ParamIdx_MaxIndex
};

/**
 * MatrixInput parameter indices
 */
enum MatrixInputParameterIndex
{
	MII_ParamIdx_LevelMeterPreMute = 0,
	MII_ParamIdx_Gain,
	MII_ParamIdx_Mute,
	MII_ParamIdx_MaxIndex
};

/**
 * MatrixOutput parameter indices
 */
enum MatrixOutputParameterIndex
{
	MOI_ParamIdx_LevelMeterPostMute = 0,
	MOI_ParamIdx_Gain,
	MOI_ParamIdx_Mute,
	MOI_ParamIdx_MaxIndex
};

/**
 * Enum to identify differen TableModelComponent derivates
 */
enum TableType
{
	TT_Invalid,
	TT_Soundobjects,
	TT_MatrixInputs,
	TT_MatrixOutputs
};

/**
 * Data Change Type
 * Bitfields used to flag parameter changes.
 */
static constexpr DataChangeType DCT_None						= 0x0000000000; //< Nothing has changed. */
static constexpr DataChangeType DCT_NumProcessors				= 0x0000000001; //< The number of SoundobjectProcessor instances in the project has changed. */
static constexpr DataChangeType DCT_IPAddress					= 0x0000000002; //< The user has entered a new IP address for the DS100. */
static constexpr DataChangeType DCT_RefreshInterval				= 0x0000000004; //< The user has entered a new interval for controller refreshing. */
static constexpr DataChangeType DCT_Connected					= 0x0000000008; //< The bridging module communication connection status has changed, based on the time since last response. */
static constexpr DataChangeType DCT_CommunicationConfig			= (DCT_IPAddress | DCT_RefreshInterval | DCT_Connected); //< IP address, rate, and Online status. */
static constexpr DataChangeType DCT_SoundobjectID				= 0x0000000010; //< The SoundobjectID of this processor instance has been changed. */
static constexpr DataChangeType DCT_MappingID					= 0x0000000020; //< The user has selected a different coordinate mapping for this Plug-in. */
static constexpr DataChangeType DCT_ComsMode					= 0x0000000040; //< The Rx / Tx mode of a soundobject channel has been changed. */
static constexpr DataChangeType DCT_SoundobjectColourAndSize	= 0x0000000080; //< The colour or size of a soundobject has been changed. */
static constexpr DataChangeType DCT_MatrixInputID				= 0x0000100000; //< The MatrixInputId of this processor instance has been changed. */
static constexpr DataChangeType DCT_MatrixOutputID				= 0x0001000000; //< The MatrixOutputId of this processor instance has been changed. */
static constexpr DataChangeType DCT_SoundobjectProcessorConfig	= (DCT_SoundobjectID | DCT_MappingID | DCT_ComsMode | DCT_SoundobjectColourAndSize); //< SoundobjectID, MappingID, Rx/Tx. */
static constexpr DataChangeType DCT_MatrixInputProcessorConfig	= (DCT_MatrixInputID | DCT_MappingID | DCT_ComsMode); //< MatrixInputID, MappingID, Rx/Tx .v
static constexpr DataChangeType DCT_MatrixOutputProcessorConfig = (DCT_MatrixOutputID | DCT_MappingID | DCT_ComsMode); //< MatrixOutputID, MappingID, Rx/Tx. */
static constexpr DataChangeType DCT_SoundobjectPosition			= 0x0000000100; //< The X/Y coordinates of this Soundobject have changed. */
static constexpr DataChangeType DCT_ReverbSendGain				= 0x0000000200; //< The En-Space Gain for this Soundobject has changed. */
static constexpr DataChangeType DCT_SoundobjectSpread			= 0x0000000400; //< The En-Scene Spread factor for this Soundobject has changed. */
static constexpr DataChangeType DCT_DelayMode					= 0x0000000800; //< The En-Scene Delay mode (Off/Tight/Full) of this Soundobject has changed. */
static constexpr DataChangeType DCT_SoundobjectParameters		= (DCT_SoundobjectPosition | DCT_ReverbSendGain | DCT_SoundobjectSpread | DCT_DelayMode); //< All parameters used by soundobject processor. */
static constexpr DataChangeType DCT_MatrixInputLevelMeter		= 0x0000200000; //< The level meter value for this matrix input has changed. */
static constexpr DataChangeType DCT_MatrixInputGain				= 0x0000400000; //< The gain for this matrix input has changed. */
static constexpr DataChangeType DCT_MatrixInputMute				= 0x0000800000; //< The mute state for this matrix input has changed. */
static constexpr DataChangeType DCT_MatrixInputParameters		= (DCT_MatrixInputLevelMeter | DCT_MatrixInputGain | DCT_MatrixInputMute); //< All parameters used by matrix input processor. */
static constexpr DataChangeType DCT_MatrixOutputLevelMeter		= 0x0002000000; //< The level meter value for this matrix output has changed. */
static constexpr DataChangeType DCT_MatrixOutputGain			= 0x0004000000; //< The gain for this matrix output has changed. */
static constexpr DataChangeType DCT_MatrixOutputMute			= 0x0008000000; //< The mute state for this matrix output has changed. */
static constexpr DataChangeType DCT_MatrixOutputParameters		= (DCT_MatrixOutputLevelMeter | DCT_MatrixOutputGain | DCT_MatrixOutputMute); //< All parameters used by matrix output processor. */
static constexpr DataChangeType DCT_ExtensionMode				= 0x0000001000; //< The extensionmode of bridging module has changed. */
static constexpr DataChangeType DCT_MuteState					= 0x0000002000; //< The mute state for a channel of a bridging protocol has changed. */
static constexpr DataChangeType DCT_NumBridgingModules			= 0x0000004000; //< The count of active bridging protocols has changed. */
static constexpr DataChangeType DCT_BridgingConfig				= (DCT_ExtensionMode | DCT_MuteState | DCT_NumBridgingModules); // < All bridging related parameters. */
static constexpr DataChangeType DCT_DebugMessage				= 0x0000010000; //< There is a new debug message to be displayed on the GUI. */
static constexpr DataChangeType DCT_ProcessorSelection			= 0x0000020000; //< The currently selected SourceID has changed. */
static constexpr DataChangeType DCT_TabPageSelection			= 0x0000040000; //< The currently selected Tab Index has changed. */
static constexpr DataChangeType DCT_AllConfigParameters			= (DCT_IPAddress | DCT_RefreshInterval | DCT_SoundobjectID | DCT_MatrixInputID | DCT_MatrixOutputID | DCT_MappingID | DCT_ComsMode | DCT_ExtensionMode | DCT_MuteState | DCT_NumBridgingModules); // < All app configuration related parameters.
static constexpr DataChangeType DCT_MatrixInputName				= 0x0100000000; //< The name of the MatrixInput has changed. */
static constexpr DataChangeType DCT_MatrixOutputName			= 0x0200000000; //< The name of the MatrixOutput has changed. */

/**
 * Protocol Bridging Type
 * Bitfields used to define different bridging types.
 */
static constexpr ProtocolBridgingType PBT_None				= 0x00000000;
static constexpr ProtocolBridgingType PBT_DiGiCo			= 0x00000001;
static constexpr ProtocolBridgingType PBT_BlacktraxRTTrPM	= 0x00000002;
static constexpr ProtocolBridgingType PBT_GenericOSC		= 0x00000004;
static constexpr ProtocolBridgingType PBT_GenericMIDI		= 0x00000008;
static constexpr ProtocolBridgingType PBT_YamahaSQ			= 0x00000010;
static constexpr ProtocolBridgingType PBT_HUI				= 0x00000020;
static constexpr ProtocolBridgingType PBT_YamahaOSC			= 0x00000040;
static constexpr ProtocolBridgingType PBT_ADMOSC			= 0x00000080;
static constexpr ProtocolBridgingType PBT_DAWPlugin			= 0x00000100;
static constexpr ProtocolBridgingType PBT_DS100				= 0x10000000;

static const std::vector<ProtocolBridgingType> ProtocolBridgingTypes{ PBT_DiGiCo, PBT_DAWPlugin, PBT_BlacktraxRTTrPM, PBT_GenericOSC, PBT_GenericMIDI, PBT_YamahaSQ, PBT_HUI, PBT_YamahaOSC, PBT_ADMOSC, PBT_DS100 };

String GetProtocolBridgingShortName(ProtocolBridgingType type);
String GetProtocolBridgingNiceName(ProtocolBridgingType type);
String GetProtocolBridgingSystemName(ProtocolBridgingType type);
const Colour GetProtocolBridgingColour(ProtocolBridgingType type);

/**
 * OSC Communication mode
 */
static constexpr ComsMode CM_Off =			0x0000; //< OSC communication is inactive.
static constexpr ComsMode CM_Rx =			0x0001; //< The Plug-in sends only requests, and accepts all responses, but sends no SET commands.
static constexpr ComsMode CM_Tx =			0x0002; //< The Plug-in sends SET commands when necessary. It sends no requests, and ignores all responses.

/**
 * Invalid ProcessorId used to signal when selection in CMultiSoundobjectSlider is empty etc..
 */
static constexpr SoundobjectProcessorId INVALID_PROCESSOR_ID = 0xFFFFFFFF;

/**
 * Static string definition for value units that are used in TextEditors
 */
static const std::string UNIT_MILLISECOND = " ms";
static const std::string UNIT_SECOND = " s";
static const std::string UNIT_MINUTE = " min";
static const std::string UNIT_HOUR = " h";
static const std::string UNIT_DECIBEL = " dB";

/**
 * Channelcount of a DS100 device
 */
static constexpr int DS100_CHANNELCOUNT = 64;
static constexpr int DS100_EXTMODE_CHANNELCOUNT = 2 * DS100_CHANNELCOUNT;

/**
 * Bridging ObjectHandlingMode parameters
 */
static constexpr float DS100_VALUCHANGE_SENSITIVITY = 0.001f;

/**
 * DS100 extension modes
 */
enum ExtensionMode
{
	EM_Off = 0,
	EM_Extend,
	EM_Mirror,
	EM_Parallel,
};

/**
 * Possibilities of active DS100 devices in parallel bridging mode
 */
enum ActiveParallelModeDS100
{
	APM_None = 0,
	APM_1st,
	APM_2nd
};

/**
 * Helper method to query web repository and documentation base urls (on gitub).
 */
String GetRepositoryBaseWebUrl();
String GetDocumentationBaseWebUrl();
String GetDocumentationSectionIdentification(UIPageId pageId);


/**
 * Rate at which the GUI will refresh, after parameter changes have been detected.
 * 33 ms translates to about 30 frames per second.
 */
static constexpr int GUI_UPDATE_RATE_FAST = 60;

/**
 * Rate at which the GUI will refresh, when no parameter changes have taken place for a while.
 */
static constexpr int GUI_UPDATE_RATE_SLOW = 120;

/**
 * Rate at which the Scenes/EnSpace pages will refresh.
 */
static constexpr int GUI_UPDATE_RATE_SUPERSLOW = 1500;

/**
 * After this number of timer callbacks without parameter changes, the timer will switch to GUI_UPDATE_RATE_SLOW.
 */
static constexpr int GUI_UPDATE_DELAY_TICKS = 15;

/**
 * Possible errors in SpaConBridge application that result in a user notification.
 */
enum SpaConBridgeErrorCode
{
	SEC_None = 0,
	SEC_LoadConfig_CannotAccess,
	SEC_LoadConfig_InternalError,
	SEC_LoadConfig_InvalidFile,
	SEC_LoadConfig_InvalidConfig,
	SEC_LoadConfig_ConfigInit,
	SEC_SaveConfig_CannotAccess,
	SEC_SaveConfig_InternalError,
	SEC_SaveConfig_InvalidInternalConfig,
	SEC_SaveConfig_CannotWrite,
	SEC_LoadImage_CannotAccess,
	SEC_LoadImage_CannotRead,
	SEC_LoadImage_InvalidImage
};

/**
 * Helper methods to handle user notification popups
 */
const String GetErrorTitle(const SpaConBridgeErrorCode errorCode);
const String GetErrorInfo(const SpaConBridgeErrorCode errorCode);
void ShowUserErrorNotification(const SpaConBridgeErrorCode errorCode);

/**
 * Helper method to update DrawableButton images with a given binary image resource according to current custom lookAndFeel settings.
 */
bool UpdateDrawableButtonImages(const std::unique_ptr<DrawableButton>& drawableButton, const String& binarySVGStringData, LookAndFeel* lookAndFeel);
bool UpdateDrawableButtonImages(const std::unique_ptr<JUCEAppBasics::TextWithImageButton>& drawableButton, const String& binarySVGStringData, LookAndFeel* lookAndFeel);
bool UpdateDrawableButtonImages(DrawableButton* drawableButton, const String& binarySVGStringData, LookAndFeel* lookAndFeel);



} // namespace SpaConBridge
