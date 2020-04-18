#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
//-----------------------------------------------------
// SoundManager Class									
//-----------------------------------------------------
#include "singleton.h"
class sound_manager : public TSingleton<sound_manager>
{
private:
    sound_manager();
    friend class TSingleton<sound_manager>;

public:
	
	virtual ~sound_manager( );

	// C++11 make the class non-copyable
	sound_manager( const sound_manager& ) = delete;
	sound_manager& operator=( const sound_manager& ) = delete;

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

 
