#pragma once

#include <Engine/Results/Results.h>
#include <xaudio2.h>
#include "3DAudio.h"
#include "AudioSource.h"
#include "XAudio2/XAudio2Helpers.h"
#include "atlstr.h"

// Interface
//==========

namespace eae6320
{
	namespace Audio3D
	{
		extern IXAudio2* pXAudio2;

		cResult AudioSource::Initialize(const char* strFilename)
		{
			HRESULT hr;
			USES_CONVERSION;
			TCHAR strFileName[256];// = _T(strFilename);
			_tcscpy(strFileName, A2T(strFilename));

			eae6320::Logging::OutputMessage("Trying to open file %s", strFileName);
			// Open the file
			HANDLE hFile = CreateFile(
				strFileName,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			if (INVALID_HANDLE_VALUE == hFile)
			{
				eae6320::Logging::OutputMessage("invalid handle");
				return Results::Failure;
				//return HRESULT_FROM_WIN32(GetLastError());
			}

			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			{
				eae6320::Logging::OutputMessage("invalid set file pointer");
				return Results::Failure;
				//return HRESULT_FROM_WIN32(GetLastError());
			}

			DWORD dwChunkSize;
			DWORD dwChunkPosition;
			//check the file type, should be fourccWAVE or 'XWMA'
			FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
			DWORD filetype;
			ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
			if (filetype != fourccWAVE)
			{
				eae6320::Logging::OutputMessage("Wrong file format");
				return Results::Failure;
				//return S_FALSE;
			}

			FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
			ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

			//fill out the audio data buffer with the contents of the fourccDATA chunk
			FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
			BYTE* pDataBuffer = new BYTE[dwChunkSize];
			ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

			buffer.AudioBytes = dwChunkSize;  //buffer containing audio data
			buffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
			buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

			// create source voice
			if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& wfx)))
			{
				eae6320::Logging::OutputMessage("Failed to create Source Voice: %d", hr);
				return Results::Failure;
				//return hr;
			}

			return Results::Success;
		}
		cResult AudioSource::CleanUp()
		{
			pSourceVoice->DestroyVoice();
			return Results::Success;
		}
		cResult AudioSource::PlayOnce()
		{
			if (isPlaying)
				return Results::Failure;
			HRESULT hr;

			buffer.LoopBegin = 0;
			buffer.LoopCount = 0;

			// submit source voice
			if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
			{
				eae6320::Logging::OutputMessage("Failed to submit Source Buffer");
				return Results::Failure;
				//return hr;
			}

			// play
			if (FAILED(hr = pSourceVoice->Start(0)))
			{
				eae6320::Logging::OutputMessage("Failed to play");
				return Results::Failure;
				//return hr;
			}
			isPlaying = true;
			return Results::Success;
		}
		cResult AudioSource::PlayLooped()
		{
			if (isPlaying)
				return Results::Failure;
			HRESULT hr;

			buffer.LoopBegin = 0;
			buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

			// submit source voice
			if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
			{
				eae6320::Logging::OutputMessage("Failed to submit Source Buffer");
				return Results::Failure;
				//return hr;
			}

			// play
			if (FAILED(hr = pSourceVoice->Start(0)))
			{
				eae6320::Logging::OutputMessage("Failed to play");
				return Results::Failure;
				//return hr;
			}
			isPlaying = true;
			return Results::Success;
		}
		cResult AudioSource::Pause()
		{
			if (!isPlaying)
				return Results::Failure;
			auto hr = pSourceVoice->Stop();
			if (FAILED(hr))
				return Results::Failure;
			isPlaying = false;
			return Results::Success;
		}
		cResult AudioSource::Reset()
		{
			if (isPlaying)
				pSourceVoice->Stop();
			pSourceVoice->FlushSourceBuffers();

			return Results::Success;
		}
	}
}
