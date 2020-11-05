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

#include <JuceHeader.h>


namespace SoundscapeBridgeApp
{


/**
 * Type definitions.
 */
typedef juce::int32 SourceId;
typedef juce::int8 MappingId;
typedef juce::int32 ProcessorId;
typedef juce::uint64 DataChangeType;
typedef juce::uint32 ProtocolBridgingType;
typedef juce::uint8 ComsMode;


/**
 * Data Change Source
 * Enum used to define where a parameter or property change has originated.
 */
enum DataChangeSource
{
	DCS_Gui = 0,		//< Change was caused by the GUI, i.e. the user turning a knob to change a value.
	DCS_Host,			//< Change was caused by the VST/AU/AAX host, i.e. a project was loaded or a DAW preset was recalled.
	DCS_Protocol,		//< Change was caused by an incoming protocol message, or caused by internal operations by the Controller.
	DCS_Overview,		//< Change was caused by the Overview. Similar to DCS_Gui, but specific to the Overview window's GUI.
	DCS_Init,			//< Change was caused by Application initialization process (defaults)
	DCS_Max				//< Number of change cources.
};


/**
 * Automation parameter indeces
 */
enum AutomationParameterIndex
{
	ParamIdx_X = 0,
	ParamIdx_Y,
	ParamIdx_ReverbSendGain,
	ParamIdx_SourceSpread,
	ParamIdx_DelayMode,
	ParamIdx_MaxIndex
};


/**
 * Data Change Type
 * Bitfields used to flag parameter changes.
 */
static constexpr DataChangeType DCT_None					= 0x00000000; //< Nothing has changed.
static constexpr DataChangeType DCT_NumProcessors			= 0x00000001; //< The number of SoundsourceProcessor instances in the project has changed.
static constexpr DataChangeType DCT_IPAddress				= 0x00000002; //< The user has entered a new IP address for the DS100.
static constexpr DataChangeType DCT_MessageRate				= 0x00000004; //< The user has entered a new interval for OSC messages.
static constexpr DataChangeType DCT_Online					= 0x00000008; //< The Plug-in's Online status has changed, based on the time since last response.
static constexpr DataChangeType DCT_OscConfig				= (DCT_IPAddress | DCT_MessageRate | DCT_Online); //< IP address, rate, and Online status.
static constexpr DataChangeType DCT_SourceID				= 0x00000010; //< The SourceID / Matrix input number of this Plug-in instance has been changed.
static constexpr DataChangeType DCT_MappingID				= 0x00000020; //< The user has selected a different coordinate mapping for this Plug-in.
static constexpr DataChangeType DCT_ComsMode				= 0x00000040; //< The Rx / Tx mode of this Plug-in has been changed.
static constexpr DataChangeType DCT_PluginInstanceConfig	= (DCT_SourceID | DCT_MappingID | DCT_ComsMode); //< SourceID, MappingID, and Rx/Tx.
static constexpr DataChangeType DCT_SourcePosition			= 0x00000080; //< The X/Y coordinates of this SourceID have changed.
static constexpr DataChangeType DCT_ReverbSendGain			= 0x00000100; //< The En-Space Gain for this SourceID has changed.
static constexpr DataChangeType DCT_SourceSpread			= 0x00000200; //< The En-Scene Spread factor for this SourceID has changed.
static constexpr DataChangeType DCT_DelayMode				= 0x00000400; //< The En-Scene Delay mode (Off/Tight/Full) of this SourceID has changed.
static constexpr DataChangeType DCT_AutomationParameters	= (DCT_SourcePosition | DCT_ReverbSendGain | DCT_SourceSpread | DCT_DelayMode); //< All automation parameters.
static constexpr DataChangeType DCT_DebugMessage			= 0x00001000; //< There is a new debug message to be displayed on the GUI.

/**
 * Protocol Bridging Type
 * Bitfields used to define different bridging types.
 */
static constexpr ProtocolBridgingType PBT_None				= 0x00000000;
static constexpr ProtocolBridgingType PBT_DiGiCo			= 0x00000001;
static constexpr ProtocolBridgingType PBT_GenericOSC		= 0x00000002;
static constexpr ProtocolBridgingType PBT_BlacktraxRTTrPM	= 0x00000004;
static constexpr ProtocolBridgingType PBT_GenericMIDI		= 0x00000008;
static constexpr ProtocolBridgingType PBT_YamahaSQ			= 0x00000010;
static constexpr ProtocolBridgingType PBT_HUI				= 0x00000020;


/**
 * OSC Communication mode
 */
static constexpr ComsMode CM_Off =			0x0000; //< OSC communication is inactive.
static constexpr ComsMode CM_Rx =			0x0001; //< The Plug-in sends only requests, and accepts all responses, but sends no SET commands.
static constexpr ComsMode CM_Tx =			0x0002; //< The Plug-in sends SET commands when necessary. It sends no requests, and ignores all responses.
static constexpr ComsMode CM_PollOnce =		0x0004; //< The X/Y coordinates have been requested once after a MappingID change. This flag is removed once the response is received.
static constexpr ComsMode CM_Sync =			(CM_Rx | CM_Tx); //< The Plug-in sends SET commands when necessary, else sends requests, and accepts all responses.


/**
 * Invalid ProcessorId used to signal when selection in CSurfaceMultiSlider is empty etc..
 */
static constexpr ProcessorId INVALID_PROCESSOR_ID = 0xFFFFFFFF;

} // namespace SoundscapeBridgeApp
