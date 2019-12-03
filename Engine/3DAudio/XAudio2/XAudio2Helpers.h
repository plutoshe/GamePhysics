// 3DAudio.cpp : Defines the functions for the static library.
//

#include "../3DAudio.h"
#include <xaudio2.h>
#include <tchar.h>

#include<Engine/Logging/Logging.h>

namespace eae6320
{
	namespace Audio3D
	{
		HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);

		HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
	}
}