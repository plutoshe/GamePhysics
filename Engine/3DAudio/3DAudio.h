#pragma once

#include <Engine/Results/Results.h>
#include <Engine/Math/sVector.h>

#include "AudioSource.h"
#include "AudioListener.h"

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

// Interface
//==========

namespace eae6320
{
	namespace Audio3D
	{
		
		struct sInitializationParameters
		{
			// don't know if we need any of these yet, but I'll keep it here.
		};
		
		cResult Initialize(const sInitializationParameters& i_initializationParameters);
		cResult CleanUp();

		cResult SubmitListener(AudioListener& listener);
		cResult SubmitAudioSourceForUpdate(AudioSource& source);
	}

}
