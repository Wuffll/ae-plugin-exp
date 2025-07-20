/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007-2023 Adobe Inc.                                  */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Inc. and its suppliers, if                    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Inc. and its                    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Inc.            */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*	Skeleton.cpp	

	This is a compiling husk of a project. Fill it in with interesting
	pixel processing code.
	
	Revision History

	Version		Change													Engineer	Date
	=======		======													========	======
	1.0			(seemed like a good idea at the time)					bbb			6/1/2002

	1.0			Okay, I'm leaving the version at 1.0,					bbb			2/15/2006
				for obvious reasons; you're going to 
				copy these files directly! This is the
				first XCode version, though.

	1.0			Let's simplify this barebones sample					zal			11/11/2010

	1.0			Added new entry point									zal			9/18/2017
	1.1			Added 'Support URL' to PiPL and entry point				cjr			3/31/2023

*/

#include "Skeleton.h"

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name), 
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

	out_data->out_flags =  PF_OutFlag_DEEP_COLOR_AWARE;	// just 16bpc, not 32bpc
	
	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def;	

	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Gain_Param_Name), 
							SKELETON_GAIN_MIN, 
							SKELETON_GAIN_MAX, 
							SKELETON_GAIN_MIN, 
							SKELETON_GAIN_MAX, 
							SKELETON_GAIN_DFLT,
							PF_Precision_HUNDREDTHS,
							0,
							0,
							GAIN_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_ADD_CHECKBOX(STR(StrID_New_Effect_Mode_Yes_Param_Name),
		STR(StrID_New_Effect_Mode_Checkbox_Label),
		0,
		0,
		NEW_EFFECT_MODE_DISK_ID);
	
	out_data->num_params = SKELETON_NUM_PARAMS;

	return err;
}

/* GAIN EFFECT FUNCTIONS - start */

static PF_Err
ApplyGainEffectFunction16 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel16	*inP, 
	PF_Pixel16	*outP)
{
	PF_Err		err = PF_Err_NONE;

	GainInfo	*giP	= reinterpret_cast<GainInfo*>(refcon);
	PF_FpLong	tempF	= 0;
					
	if (giP){
		tempF = (giP->gainF / 255.0) * PF_MAX_CHAN16 ;

		// Make sure gain value doesn't go over channel max value
		if (tempF > PF_MAX_CHAN16){
			tempF = PF_MAX_CHAN16;
		};

		outP->alpha		=	inP->alpha;
		outP->red		=	MIN((inP->red	+ (A_u_short) tempF), PF_MAX_CHAN16);
		outP->green		=	MIN((inP->green	+ (A_u_short) tempF), PF_MAX_CHAN16);
		outP->blue		=	MIN((inP->blue	+ (A_u_short) tempF), PF_MAX_CHAN16);
	}

	return err;
}

static PF_Err
ApplyGainEffectFunction8 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;

	GainInfo	*giP	= reinterpret_cast<GainInfo*>(refcon);
	PF_FpLong	tempF	= 0;
					
	if (giP){
		tempF = (giP->gainF / 255.0) * PF_MAX_CHAN8;

		// Make sure gain value doesn't go over channel max value
		if (tempF > PF_MAX_CHAN8){
			tempF = PF_MAX_CHAN8;
		};

		outP->alpha		=	inP->alpha;
		outP->red		=	MIN((inP->red	+ (A_u_char) tempF), PF_MAX_CHAN8);
		outP->green		=	MIN((inP->green	+ (A_u_char) tempF), PF_MAX_CHAN8);
		outP->blue		=	MIN((inP->blue	+ (A_u_char) tempF), PF_MAX_CHAN8);
	}

	return err;
}

/* GAIN EFFECT FUNCTIONS - end */

/* CONTRAST BOOST EFFECT FUNCTIONS - start */

/// <summary>
/// Calculates a normalized contrast boost value based on a color value and a threshold.
/// </summary>
/// <param name="InNormalizedColorValueF">The normalized color value (typically in the range [0.0, 1.0]).</param>
/// <param name="InNormalizedThresholdF">The normalized threshold value (typically in the range [0.0, 1.0]).</param>
/// <returns>A contrast boost value in the range [-1.0, 1.0], representing the relative difference between the color value and the threshold.</returns>
static PF_FpLong
CalculateContrastBoost(
	PF_FpLong InNormalizedColorValueF,
	PF_FpLong InNormalizedThresholdF)
{
	// Clamp threshold to avoid extreme slopes
	InNormalizedThresholdF = MIN(MAX(InNormalizedThresholdF, 0.05), 0.95);

	// Calculate linear boost increase based on distance from the threshold
	PF_FpLong boost = 0.0;

	if (InNormalizedColorValueF < InNormalizedThresholdF)
	{
		boost = (InNormalizedColorValueF - InNormalizedThresholdF) / InNormalizedThresholdF;
	}
	else
	{
		PF_FpLong inverseThreshold = 1.0 - InNormalizedThresholdF;
		if (inverseThreshold < 0.05) inverseThreshold = 0.05;

		boost = (InNormalizedColorValueF - InNormalizedThresholdF) / inverseThreshold;
	}

	// Clamp boost to [-1.0, 1.0]
	if (boost < -1.0) boost = -1.0;
	if (boost > 1.0) boost = 1.0;

	return boost;
}

static PF_Err
ContrastBoostEffectFunction16(
	void* refcon,
	A_long		xL,
	A_long		yL,
	PF_Pixel16* inP,
	PF_Pixel16* outP)
{
	PF_Err err = PF_Err_NONE;
	GainInfo* giP = reinterpret_cast<GainInfo*>(refcon);

	PF_FpLong threshold = giP->gainF / 255.0; // Normalize threshold to [0–1]

	outP->alpha = inP->alpha;

	auto processChannel16 = [&](A_u_short colorValue) -> A_u_short {
		
		PF_FpLong normalizedColorValue = static_cast<PF_FpLong>(colorValue) / PF_MAX_CHAN16; // [0.0, 255.0] -> [0.0, 1.0]
		PF_FpLong boost = CalculateContrastBoost(normalizedColorValue, threshold); // [-1.0, 1.0]

		// Clamp boost to not be extreme
		boost = MIN(MAX(boost, -0.5f), 0.5f); 

		// Apply boost and return color value with applied boost
		A_long adjusted = static_cast<A_long>(colorValue + (boost * PF_MAX_CHAN16));
		return static_cast<A_u_short>(MIN(MAX(adjusted, 0L), static_cast<A_long>(PF_MAX_CHAN16)));
		};

	outP->red = processChannel16(inP->red);
	outP->green = processChannel16(inP->green);
	outP->blue = processChannel16(inP->blue);

	return err;
}

static PF_Err
ContrastBoostEffectFunction8(
	void* refcon,
	A_long		xL,
	A_long		yL,
	PF_Pixel8* inP,
	PF_Pixel8* outP)
{
	PF_Err err = PF_Err_NONE;
	GainInfo* giP = reinterpret_cast<GainInfo*>(refcon);

	PF_FpLong threshold = giP->gainF / 255.0; // Normalized (value in interval [0.0, 1.0])

	outP->alpha = inP->alpha;

	auto processChannel8 = [&](A_u_char val) -> A_u_char {
		PF_FpLong norm = static_cast<PF_FpLong>(val) / PF_MAX_CHAN8;
		PF_FpLong boost = CalculateContrastBoost(norm, threshold);
		boost = MIN(MAX(boost, -0.5f), 0.5f);
		A_long adjusted = static_cast<A_long>(val + (boost * PF_MAX_CHAN8));
		return static_cast<A_u_char>(MIN(MAX(adjusted, 0L), PF_MAX_CHAN8));
		};

	outP->red = processChannel8(inP->red);
	outP->green = processChannel8(inP->green);
	outP->blue = processChannel8(inP->blue);

	return err;
}

/* CONTRAST BOOST EFFECT FUNCTIONS - start */

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	/*	Put interesting code here. */
	GainInfo			giP;
	AEFX_CLR_STRUCT(giP);
	A_long				linesL	= 0;

	linesL 		= output->extent_hint.bottom - output->extent_hint.top;
	giP.gainF 	= params[SKELETON_GAIN]->u.fs_d.value;

	PF_ParamValue checkbox = params[SKELETON_NEW_EFFECT_MODE]->u.bd.value;
	
	if (!checkbox)
	{
		if (PF_WORLD_IS_DEEP(output)) {
			ERR(suites.Iterate16Suite2()->iterate(in_data,
				0,												// progress base
				linesL,											// progress final
				&params[SKELETON_INPUT]->u.ld,					// src 
				NULL,											// area - null for all pixels
				(void*)&giP,									// refcon - your custom data pointer
				ApplyGainEffectFunction16,						// pixel function pointer
				output));
		}
		else {
			ERR(suites.Iterate8Suite2()->iterate(in_data,
					0,											// progress base
					linesL,										// progress final
					&params[SKELETON_INPUT]->u.ld,				// src 
					NULL,										// area - null for all pixels
					(void*)&giP,								// refcon - your custom data pointer
				ApplyGainEffectFunction8,						// pixel function pointer
				output));
		}
	}
	else
	{
		if (PF_WORLD_IS_DEEP(output)) {
			ERR(suites.Iterate16Suite2()->iterate(in_data,
				0,								// progress base
				linesL,							// progress final
				&params[SKELETON_INPUT]->u.ld,	// src 
				NULL,							// area - null for all pixels
				(void*)&giP,					// refcon - your custom data pointer
				ContrastBoostEffectFunction16,				// pixel function pointer
				output));
		}
		else {
			ERR(suites.Iterate8Suite2()->iterate(in_data,
				0,								// progress base
				linesL,							// progress final
				&params[SKELETON_INPUT]->u.ld,	// src 
				NULL,							// area - null for all pixels
				(void*)&giP,					// refcon - your custom data pointer
				ContrastBoostEffectFunction8,				// pixel function pointer
				output));
		}
	}

	return err;
}


extern "C" DllExport
PF_Err PluginDataEntryFunction2(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB2 inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT_EXT2(
		inPtr,
		inPluginDataCallBackPtr,
		"Skeleton", // Name
		"ADBE Skeleton", // Match Name
		"Sample Plug-ins", // Category
		AE_RESERVED_INFO, // Reserved Info
		"EffectMain",	// Entry point
		"https://www.adobe.com");	// support URL

	return result;
}


PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

