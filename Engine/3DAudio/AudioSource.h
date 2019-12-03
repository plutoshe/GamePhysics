#pragma once

#include <Engine/Results/Results.h>
#include <xaudio2.h>
#include <tchar.h>
#include <Engine/Math/sVector.h>

// Interface
//==========

namespace eae6320
{
	namespace Audio3D
	{
		struct AudioInitParams
		{
			char* filename;
			eae6320::Math::sVector position;
		};

		class AudioSource
		{
		private:
			// data we'll need goes here
			WAVEFORMATEXTENSIBLE wfx = { 0 };
			XAUDIO2_BUFFER buffer = { 0 };
			IXAudio2SourceVoice* pSourceVoice;
			bool isPlaying;
			

		public:
			static cResult Load(const char* i_path, AudioSource*& o_source);

			cResult Initialize(const char* strFilename);
			cResult CleanUp();

			cResult PlayOnce();
			cResult PlayLooped();
			cResult Pause();
			cResult Reset();
		};
	}
}
