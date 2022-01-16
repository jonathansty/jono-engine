#include "engine.pch.h"

#include "AudioSystem.h"

#if FEATURE_XAUDIO

void XAudioSystem::cleanup()
{
	if(m_pMasterVoice) m_pMasterVoice->DestroyVoice();
	if (m_pXAudio2)
	{
		m_pXAudio2->Release();
		m_pXAudio2 = nullptr;
	}
}
HRESULT XAudioSystem::init()
{
	HRESULT hr;
	 //Use the XAudio2Create function to create an instance of the XAudio2 engine.
	if (FAILED(hr = XAudio2Create(&m_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		return hr;

	// The mastering voices encapsulates an audio device. It is the ultimate destination for all audio that passes through an audio graph.
	if (FAILED(hr = m_pXAudio2->CreateMasteringVoice(&m_pMasterVoice)))
		return hr;

	return S_OK;
}

#endif

std::shared_ptr<IAudioSystem> create_audio_system() {
	return std::make_shared<XAudioSystem>();
}
