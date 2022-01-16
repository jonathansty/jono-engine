//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

#pragma once

interface IAudioSystem
{
	HRESULT intialize();

	// Placeholder API
	HRESULT create_source_voice();
};

std::shared_ptr<IAudioSystem> create_audio_system();

#include <Xaudio2.h>

#if FEATURE_XAUDIO
class XAudioSystem final : public IAudioSystem {
public:
	XAudioSystem() {}
	virtual ~XAudioSystem(void) {}

	XAudioSystem(const XAudioSystem& t) = delete;
	XAudioSystem& operator=(const XAudioSystem& t) = delete;

	HRESULT init();

	void cleanup();

	IXAudio2* get_xaudio() const { return m_pXAudio2; }

private:
	IXAudio2* m_pXAudio2;

	IXAudio2MasteringVoice* m_pMasterVoice;
};
#endif
