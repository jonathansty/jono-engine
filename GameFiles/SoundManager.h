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

    //STATIC METHODS
    static SoundManager* GetSingleton();
	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    Sound* LoadSound(const String& fileName);
    Sound* LoadMusic(const String& fileName);
    void UnLoadMusic(Sound* sndPtr);
    void UnLoadSound(Sound* sndPtr);

    void SetSoundVolume(double volume);
    void SetMusicVolume(double volume);

    Sound* GetCurrentSong();

    bool FadeIn(Sound* tmpSoundPtr, double deltaTime);
    bool FadeOut(Sound* tmpSoundPtr, double deltaTime);

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
    std::vector<Sound*> m_SoundsPtrArr;
    std::vector<double>m_SoundLevelsPtrArr;
    int m_NumbersOfStoredSounds = 0;

    std::vector<String>m_MusicFileNamesArr;
    std::vector<Sound*>m_MusicPtrArr;
    std::vector<double>m_MusicLevelsPtrArr;
    int m_NumberOfStoredMusicTracks = 0;

};

 
