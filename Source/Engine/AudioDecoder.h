#pragma once

#if FEATURE_XAUDIO

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>
#include <Xaudio2.h>
#include <string>

class AudioDecoder final
{
public:
	AudioDecoder(const std::wstring& filenameRef, XAUDIO2_BUFFER& bufferRef, WAVEFORMATEXTENSIBLE* pWfx);
	virtual ~AudioDecoder();

	HRESULT Open(const std::wstring& wFilenameRef, XAUDIO2_BUFFER& bufferRef, WAVEFORMATEXTENSIBLE* pWfx);
	// Pointer to the source reader.
	// Receives the audio format

	HRESULT ConfigureAudioStream(IMFSourceReader *pReader, IMFMediaType **ppPCMAudio);

	HRESULT WriteWaveData(
		XAUDIO2_BUFFER& bufferRef,  // Output 
		IMFSourceReader *pReader,   // Source reader.
		DWORD cbMaxAudioData,       // Maximum amount of audio data (bytes).
		DWORD *pcbDataWritten       // Receives the amount of data written.
		);

	HRESULT GetDuration(IMFSourceReader *pReader, LONGLONG *phnsDuration) const;

private:
	AudioDecoder(const AudioDecoder& t);
	AudioDecoder& operator=(const AudioDecoder& t);
};

#endif