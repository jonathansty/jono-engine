#pragma once

#include "singleton.h"

class SoundManager : public TSingleton<SoundManager>
{
private:
    SoundManager();
    friend class TSingleton<SoundManager>;

public:
	
	virtual ~SoundManager( );

	// C++11 make the class non-copyable
	SoundManager( const SoundManager& ) = delete;
	SoundManager& operator=( const SoundManager& ) = delete;

    sound* LoadSound(const String& fileName);
    sound* LoadMusic(const String& fileName);
    void UnLoadMusic(sound* sndPtr);
    void UnLoadSound(sound* sndPtr);

    void SetSoundVolume(double volume);
    void SetMusicVolume(double volume);

    sound* GetCurrentSong();

    bool FadeIn(sound* tmpSoundPtr, double deltaTime);
    bool FadeOut(sound* tmpSoundPtr, double deltaTime);

    bool isSoundMuted();
    bool isMusicMuted();

    void MuteAll();
    void UnMuteAll();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    double m_FadeAccuTime = 0;
    int m_MaxSize = 20;
    bool m_IsMusicMuted = false;
    bool m_IsSoundMuted = false;
    std::vector<String>m_FileNamesArr;
    std::vector<sound*> m_SoundsPtrArr;
    std::vector<double>m_SoundLevelsPtrArr;
    int m_NumbersOfStoredSounds = 0;

    std::vector<String>m_MusicFileNamesArr;
    std::vector<sound*>m_MusicPtrArr;
    std::vector<double>m_MusicLevelsPtrArr;
    int m_NumberOfStoredMusicTracks = 0;

};

 
