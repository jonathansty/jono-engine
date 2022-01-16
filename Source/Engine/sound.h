#pragma once

#if FEATURE_XAUDIO

#include <Xaudio2.h>

//-----------------------------------------------------------------
// VoiceCallback Class: used by XSound to receive notifications from playing sounds
//-----------------------------------------------------------------
class sound;
class VoiceCallback final : public IXAudio2VoiceCallback
{
public:
	HANDLE hBufferEndEvent;
	VoiceCallback() : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)){}
	~VoiceCallback(){ CloseHandle(hBufferEndEvent); }

	//Called when the voice has just finished playing a contiguous audio stream.
	void __stdcall OnStreamEnd();

	//Unused methods are stubs
	void __stdcall OnVoiceProcessingPassEnd() { }
	void __stdcall OnVoiceProcessingPassStart(UINT32 SamplesRequired) { }
	void __stdcall OnBufferEnd(void * pBufferContext)    { }
	void __stdcall OnBufferStart(void * pBufferContext) { }
	void __stdcall OnLoopEnd(void * pBufferContext) { }
	void __stdcall OnVoiceError(void * pBufferContext, HRESULT Error) { }

	void SetVoice(sound *pXSound);

private:
	sound* m_pXSound = nullptr;
};

//-----------------------------------------------------------------
// XSound Class: represents a playable sound (including mp3, no mid)
//-----------------------------------------------------------------
class sound final
{
public:
	//! creates a playable sound.
	//! @param filenameRef is the file to be loaded
	sound(const std::string &filenameRef);
	//! creates a playable sound.
	//! @param resourceID is the resource to be loaded
	sound(int resourceID);

	virtual ~sound();

	// C++11 make the class non-copyable
	sound(const sound&) = delete;
	sound& operator=(const sound&) = delete;

	//! Playstate enumeration
	enum class PlayState
	{
		Playing,
		Paused,
		Stopped
	};

	enum class play_mode
	{
		Immediate,
		Queued,
	};

	//! Playback control. Returns false if the Sound could not be played
	bool play(play_mode mode = play_mode::Queued);

	//! Playback control. Returns false if the Sound could not be Stopped
	bool stop();
	//! Playback control. Returns false if the Sound could not be Paused
	bool pause();

	// Sound Settings
	//! A volume level of 1.0 means there is no attenuation or gain and 0 means silence. 
	bool set_volume(double volume);

	//! Returns the volume, value is between 0 and 1 or more if gain is enabled
	double get_volume() const;

	//! Set the repeat flag
	void set_repeat(bool repeat);

	//! Is the repeat flag set?
	bool get_repeat() const;

	//! Pitches are expressed as input rate / output rate ratios between 1 / 1, 024 and 1, 024 / 1, 
	//! inclusive.A ratio of 1 / 1, 024 lowers pitch by 10 octaves, while a ratio of 1, 024 / 1 raises it by 10 octaves.
	//! You can only use the IXAudio2SourceVoice::SetFrequencyRatio method to apply pitch adjustments to source voices, 
	//! and only if they were not created with the XAUDIO2_VOICE_NOPITCH flag.
	//! The default frequency ratio is 1 / 1: that is, no pitch change.
	void set_pitch(double ratio);
	//! returns the pitch
	double get_pitch() const;

	//! returns the total duration of the sound in seconds
	double get_duration() const;

    //! returns the filePath
    std::string const& GetPath() const;

	// Internal use only
	friend void __stdcall VoiceCallback::OnStreamEnd();

	PlayState GetPlayState() const;

private:
    std::string m_FilePath;
	void SetPlayState(PlayState playState);
	void Create(const std::string& sFilename);
	void Extract(int resourceID, const std::string& typeRef, std::string& resultingFilenameRef);
	VoiceCallback m_VoiceCallback;
	XAUDIO2_BUFFER m_Buffer;
	WAVEFORMATEXTENSIBLE m_Wfx;
	IXAudio2SourceVoice* m_pSourceVoice;
	PlayState m_PlayState = PlayState::Stopped;

	std::vector<IXAudio2SourceVoice*> m_OverlappingVoices;
};

#else
class sound final
{
public:
	sound(const string& ) {};
	sound(int ) {};

	virtual ~sound() {};

	// C++11 make the class non-copyable
	sound(const sound&) = delete;
	sound& operator=(const sound&) = delete;

	//! Playstate enumeration
	enum class PlayState
	{
		Playing,
		Paused,
		Stopped
	};

	enum class play_mode
	{
		Immediate,
		Queued,
	};

	bool play(play_mode mode = play_mode::Queued) {
		UNREFERENCED_PARAMETER(mode);
		return false;
	};
	bool stop() {
		return false;
	};
	bool pause() {
		return false;
	};
	bool set_volume(double) {
		return false;
	};
	double get_volume() const { return 0.0; };
	void set_repeat(bool) {};
	bool get_repeat() const {
		return false;
	};
	void set_pitch(double) {};
	double get_pitch() const { return 0.0; };
	double get_duration() const { return 0.0; };

	//! returns the filePath
	string GetPath() { return string(""); };
	PlayState GetPlayState() { return PlayState::Stopped; };
};
#endif
