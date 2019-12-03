// 3DAudio.cpp : Defines the functions for the static library.
//

#include "3DAudio.h"
#include <xaudio2.h>
#include <tchar.h>

#include<Engine/Logging/Logging.h>

namespace eae6320
{
	namespace Audio3D
	{
		/*Static data we'll need for this system*/
		IXAudio2* pXAudio2 = NULL;
		IXAudio2MasteringVoice* masterVoice = NULL;

		cResult eae6320::Audio3D::Initialize(const sInitializationParameters& i_initializationParameters)
		{
			HRESULT hr;

			// Create XAudio2 device
			if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
				return Results::Failure;
			
			// Create Mastering Voice
			hr = pXAudio2->CreateMasteringVoice(&masterVoice);
			if (FAILED(hr))
			{
				eae6320::Logging::OutputMessage("Failed to create Master Voice: %d", hr);
				return Results::Failure;
			}

			//AudioSource newSource;// = new AudioSource();
			//newSource.Initialize(_T("data/MuckingAround.wav"));
			////newSource.PlayOnce();
			//newSource.PlayLooped();

			eae6320::Logging::OutputMessage("Successfully initialized audio.");

			return Results::Success;
		}

		cResult eae6320::Audio3D::CleanUp()
		{
			return cResult();
		}

		cResult SubmitListener(AudioListener& listener)
		{
			return cResult();
		}

		cResult eae6320::Audio3D::SubmitAudioSourceForUpdate(AudioSource& source)
		{
			return cResult();
		}
	}
}