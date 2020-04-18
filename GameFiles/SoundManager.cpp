#include "stdafx.h"		
	
#include "SoundManager.h"

sound_manager::sound_manager()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);


}

sound_manager::~sound_manager()
{
    for (int i = 0; i < m_NumbersOfStoredSounds; i++)
    {
        delete m_SoundsPtrArr[i];
    }
    m_SoundsPtrArr.clear();

    for (int i = 0; i < m_NumberOfStoredMusicTracks; i++)
    {
        delete m_MusicPtrArr[i];
    }
    m_MusicPtrArr.clear();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void SoundManager::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void SoundManager::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void SoundManager::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
Sound* sound_manager::LoadSound(const String& filename)
{
    for (int i = 0; i < m_NumbersOfStoredSounds; i++)
    {
        if (m_FileNamesArr[i] == filename)
        {
            return m_SoundsPtrArr[i];
        }
        if (m_FileNamesArr[i] == String("NULL"))
        {
            Sound* soundPtr = new Sound(filename);
            m_FileNamesArr[i] = filename;
            m_SoundsPtrArr[i] = soundPtr;
            std::wcout << L"The Sound with name " << filename.C_str() << L" has been loaded into an empty slot." << std::endl;
            return soundPtr;
        }
    }
    std::wcout << L"The Sound with name " << filename.C_str() << L" is loaded" << std::endl;
    Sound* soundPtr = new Sound(filename);
    m_SoundsPtrArr.push_back(soundPtr);
    m_FileNamesArr.push_back(filename);
    m_NumbersOfStoredSounds++;
    return soundPtr;
}
Sound* sound_manager::LoadMusic(const String& fileName)
{
    for (int i = 0; i < m_NumberOfStoredMusicTracks; i++)
    {
        if (m_MusicFileNamesArr[i] == fileName)
        {
            return m_MusicPtrArr[i];
        }
        if (m_MusicFileNamesArr[i] == String("NULL"))
        {
            Sound* soundPtr = new Sound(fileName);
            m_MusicFileNamesArr[i] = fileName;
            m_MusicPtrArr[i] = soundPtr;
            std::wcout << L"The music with name " << fileName.C_str() << L" has been loaded into an empty slot" << std::endl;
            return soundPtr;
        }
    }
    std::wcout << L"The music with name " << fileName.C_str() << L" is loaded" << std::endl;
    Sound* soundPtr = new Sound(fileName);
    m_MusicFileNamesArr.push_back(fileName);
    m_MusicPtrArr.push_back(soundPtr);
    m_NumberOfStoredMusicTracks++;
    return soundPtr;
}

void sound_manager::MuteAll()
{
    for (size_t i = 0, n = m_SoundsPtrArr.size(); i < n; i++)
    {
        if (m_SoundsPtrArr[i] != nullptr)
        {
            m_SoundLevelsPtrArr.push_back(m_SoundsPtrArr[i]->GetVolume());
            m_SoundsPtrArr[i]->SetVolume(0);
            m_IsSoundMuted = true;
        }
    }
    for (size_t i = 0; i < m_MusicPtrArr.size(); i++)
    {
        if (m_MusicPtrArr[i] != nullptr)
        {
            m_MusicLevelsPtrArr.push_back(m_MusicPtrArr[i]->GetVolume());
            m_MusicPtrArr[i]->SetVolume(0);
            m_IsMusicMuted = true;
        }
    }
}
void sound_manager::UnMuteAll()
{
    for (size_t i = 0, n = m_SoundsPtrArr.size(); i < n; i++)
    {
        if (m_SoundsPtrArr[i] != nullptr)
        {
            m_SoundsPtrArr[i]->SetVolume(m_SoundLevelsPtrArr[i]);
        }
    }
    m_SoundLevelsPtrArr.clear();

    for (size_t i = 0; i < m_MusicPtrArr.size(); i++)
    {
        if (m_MusicPtrArr[i] != nullptr)
        {
            m_MusicPtrArr[i]->SetVolume(m_MusicLevelsPtrArr[i]);
        }
    }
}
void sound_manager::SetMusicVolume(double volume)
{
    for (size_t i = 0; i < m_MusicPtrArr.size(); i++)
    {
        if (m_MusicPtrArr[i] != nullptr && volume != m_MusicPtrArr[i]->GetVolume())
        {
            m_MusicPtrArr[i]->SetVolume(volume);
        }
    }


}
void sound_manager::SetSoundVolume(double volume)
{
    for (size_t i = 0; i < m_SoundsPtrArr.size(); i++)
    {
        double OldVolume = m_SoundsPtrArr[i]->GetVolume();
        if (m_SoundsPtrArr[i] != nullptr && (int)(OldVolume*100) == (int)(volume*100))
        {
            m_SoundsPtrArr[i]->SetVolume(volume);
            
        }
    }

}
void sound_manager::UnLoadMusic(Sound* sndPtr)
{
    for (int i = 0; i < m_NumberOfStoredMusicTracks; i++)
    {
        if (m_MusicPtrArr[i] == sndPtr)
        {
            delete m_MusicPtrArr[i];
            m_MusicPtrArr[i] = nullptr;
            sndPtr = nullptr;
            m_MusicFileNamesArr[i] = String("NULL");
        }
    }
}
void sound_manager::UnLoadSound(Sound* sndPtr)
{
    for (int i = 0; i <m_NumbersOfStoredSounds; i++)
    {
        if (m_SoundsPtrArr[i] == sndPtr)
        {
            delete m_SoundsPtrArr[i];
            m_SoundsPtrArr[i] = nullptr;
        }
    }
}
bool sound_manager::FadeIn(Sound* tmpSoundPtr, double deltaTime)
{
    
    if (tmpSoundPtr->GetVolume() > 1 - 0.01)
    {
        return true;
    }
    m_FadeAccuTime += deltaTime;
    double fadeSpeed = 0.05;
    if (m_FadeAccuTime >  fadeSpeed)
    {
        m_FadeAccuTime -= fadeSpeed;
        if (tmpSoundPtr->GetVolume() + 0.01 < 1)
        {
            tmpSoundPtr->SetVolume(tmpSoundPtr->GetVolume() + 0.01);
        }
    }
    return false;
    
}
bool sound_manager::FadeOut(Sound* tmpSoundPtr, double deltaTime)
{
    m_FadeAccuTime += deltaTime;
    double fadeSpeed = 0.05;
    if (tmpSoundPtr->GetVolume() < 0 + 0.01)
    {
        return true;
    }
    if (m_FadeAccuTime >  0)
    {
        m_FadeAccuTime -= fadeSpeed;
        if (tmpSoundPtr->GetVolume() - 0.01 > 0)
        {
            tmpSoundPtr->SetVolume(tmpSoundPtr->GetVolume() - 0.01);
        }
    }
    return false;
}
bool sound_manager::isMusicMuted()
{
    return m_IsMusicMuted;
}
bool sound_manager::isSoundMuted()
{
    return m_IsSoundMuted;
}