#include "engine.pch.h"    // for compiler


#include "GameEngine.h"

#include "sound.h"
#include "AudioDecoder.h"
#include "AudioSystem.h"

// not for Win7
#if FEATURE_XAUDIO


//-----------------------------------------------------------------
// Sound methods
//-----------------------------------------------------------------
sound::sound(const std::string &filenameRef)
{
	Create(filenameRef);
    m_FilePath = filenameRef;
}

sound::sound(int resourceID)
{
	std::string sType("MP3");
	std::string fileName = std::to_string(resourceID) + "." + sType;
	std::string resultingFilename;
	Extract(resourceID, sType, resultingFilename);
	Create(resultingFilename);
}

sound::~sound()
{
	m_pSourceVoice->Stop(0);
	m_pSourceVoice->DestroyVoice();
	delete m_Buffer.pAudioData;
	
	for (auto it = m_OverlappingVoices.begin(); it != m_OverlappingVoices.end(); ++it)
	{
		(*it)->Stop(0);
		(*it)->DestroyVoice();
	}
}

void sound::Extract(int resourceID, const std::string& typeRef, std::string &resultingFilenameRef)
{
	HRSRC hrsrc = FindResourceA(NULL, MAKEINTRESOURCEA(resourceID), typeRef.c_str());
	HGLOBAL hLoaded = LoadResource(NULL, hrsrc);
	LPVOID lpLock = LockResource(hLoaded);
	DWORD dwSize = SizeofResource(NULL, hrsrc);

	std::string path("temp/");
	CreateDirectoryA(path.c_str(), NULL);

	resultingFilenameRef = path + std::to_string(resourceID) + "." + typeRef;
	HANDLE hFile = CreateFileA(resultingFilenameRef.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwByteWritten = 0;
	WriteFile(hFile, lpLock, dwSize, &dwByteWritten, NULL);
	CloseHandle(hFile);
	FreeResource(hLoaded);
}

void sound::Create(const std::string& filenameRef)
{
	// catch endplay event
	m_VoiceCallback.SetVoice(this);

	HRESULT hr = S_FALSE;
	//load sound, stores data in m_Buffer and m_Wfx
	//XSoundLoader(filenameRef, m_Buffer, m_Wfx);

	//from String to tstring
	std::string tFilename(filenameRef.c_str());

	// load mp3 using media foundation architecture: sourcereader
	std::wstring filename = std::wstring(tFilename.begin(), tFilename.end());
	AudioDecoder decoder = AudioDecoder(filename, m_Buffer, &m_Wfx);

	// 3. Create a source voice by calling the IXAudio2::CreateSourceVoice method on an instance of the XAudio2 engine.
	// Bart: We are friends with AudioSystem class, could use game engine singleton instead
	auto ge = GameEngine::instance();
	auto xaudio = ge->GetAudioSystem()->get_xaudio(); 
	if (FAILED(hr = GameEngine::instance()->GetAudioSystem()->get_xaudio()->CreateSourceVoice(&m_pSourceVoice, (WAVEFORMATEX*)&m_Wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &m_VoiceCallback, NULL, NULL)))
		//if (FAILED(hr = AudioSystem::m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, (WAVEFORMATEX*)&m_Wfx)))
	{
		MessageBoxA(NULL, "Error Creating the Sound", "GameEngine says NO", MB_OK);
		exit(-1);
	}
}

bool sound::play(play_mode mode)
{
	HRESULT hr = S_FALSE;
	if (mode == play_mode::Queued)
	{
		if (m_PlayState != PlayState::Playing)
		{
			// if stopped, queue a buffer
			if (m_PlayState == PlayState::Stopped)
			{
				// 4. Submit an XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
				if (FAILED(hr = m_pSourceVoice->SubmitSourceBuffer(&m_Buffer)))
				{
					MessageBoxA(NULL, "Error SubmitSourceBuffer", "GameEngine says NO", MB_OK);
					exit(-1);
				}
			}
			// if paused, then just continue playing the buffer
			hr = m_pSourceVoice->Start(0);
			if (SUCCEEDED(hr))
			{
				m_PlayState = PlayState::Playing;
				return true;
			}
		}
	}
	else
	{
		hr = S_FALSE;
		IXAudio2SourceVoice* pSourceVoice = nullptr;

		for (auto it = m_OverlappingVoices.begin(); it != m_OverlappingVoices.end(); ++it)
		{
			IXAudio2SourceVoice* pExistingVoice = *it;
			XAUDIO2_VOICE_STATE state;
			pExistingVoice->GetState(&state);

			if (state.BuffersQueued == 0) {
				pSourceVoice = pExistingVoice;
			}

		}
		if (pSourceVoice == nullptr)
		{

			if (FAILED(hr = GameEngine::instance()->GetAudioSystem()->get_xaudio()->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&m_Wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &m_VoiceCallback, NULL, NULL)))
				//if (FAILED(hr = AudioSystem::m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, (WAVEFORMATEX*)&m_Wfx)))
			{
				MessageBoxA(NULL, "Error Creating the Sound", "GameEngine says NO", MB_OK);
				exit(-1);
			}
			else
			{
				m_OverlappingVoices.push_back(pSourceVoice);
			}
		}

		// 4. Submit an XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
		//m_pSourceVoice->
		if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&m_Buffer)))
		{
			MessageBoxA(NULL, "Error SubmitSourceBuffer", "GameEngine says NO", MB_OK);
			exit(-1);
		}


		hr = pSourceVoice->Start(0);

		return hr != S_FALSE;
	}
	return false;
}

bool sound::stop()
{
	m_PlayState = PlayState::Stopped;
	HRESULT hr = m_pSourceVoice->Stop(0);
	if (SUCCEEDED(hr))
	{
		hr = m_pSourceVoice->FlushSourceBuffers();
		if (SUCCEEDED(hr)) return true;
	}
	return false;
}

bool sound::pause()
{
	m_PlayState = PlayState::Paused;
	HRESULT hr = m_pSourceVoice->Stop(0);
	if (SUCCEEDED(hr))return true;
	return false;
}

bool sound::set_volume(double volume)
{
	HRESULT hr = m_pSourceVoice->SetVolume((float)volume);
	if (SUCCEEDED(hr)) return true;
	return false;
}

double sound::get_volume() const
{
	float volume = 0;
	m_pSourceVoice->GetVolume(&volume);
	return volume;
}

void sound::SetPlayState(PlayState playState)
{
	m_PlayState = playState;
}

sound::PlayState sound::GetPlayState() const
{
	return m_PlayState;
}

void sound::set_repeat(bool repeat)
{
	if (repeat)m_Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	else m_Buffer.LoopCount = 0;
}

bool sound::get_repeat() const
{
	if (m_Buffer.LoopCount != 0) return true;
	return false;
}

void sound::set_pitch(double ratio)
{
	m_pSourceVoice->SetFrequencyRatio((float)ratio);
}

double sound::get_pitch() const
{
	float ratio = 0;
	m_pSourceVoice->GetFrequencyRatio(&ratio);
	return ratio;
}

double sound::get_duration() const
{
	double duration = (double)m_Buffer.AudioBytes / (m_Wfx.Format.nSamplesPerSec * m_Wfx.Format.wBitsPerSample / 8 * m_Wfx.Format.nChannels);
	return duration;
}
std::string const& sound::GetPath() const
{
    return m_FilePath;
}

//-----------------------------------------------------------------
// VoiceCallback methods
//-----------------------------------------------------------------
void __stdcall VoiceCallback::OnStreamEnd()
{
	m_pXSound->SetPlayState(sound::PlayState::Stopped);
}

void VoiceCallback::SetVoice(sound *pXSound)
{
	m_pXSound = pXSound;
}

#endif