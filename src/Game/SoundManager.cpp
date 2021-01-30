#include "game.stdafx.h"		
	
#include "SoundManager.h"

SoundManager::SoundManager()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

SoundManager::~SoundManager()
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

sound* SoundManager::LoadSound(const String& filename)
{
    for (int i = 0; i < m_NumbersOfStoredSounds; i++)
    {
        if (m_FileNamesArr[i] == filename)
        {
            return m_SoundsPtrArr[i];
        }
        if (m_FileNamesArr[i] == String("NULL"))
        {
            sound* soundPtr = new sound(filename);
            m_FileNamesArr[i] = filename;
            m_SoundsPtrArr[i] = soundPtr;
            std::wcout << L"The Sound with name " << filename.C_str() << L" has been loaded into an empty slot." << std::endl;
            return soundPtr;
        }
    }
    std::wcout << L"The Sound with name " << filename.C_str() << L" is loaded" << std::endl;
    sound* soundPtr = new sound(filename);
    m_SoundsPtrArr.push_back(soundPtr);
    m_FileNamesArr.push_back(filename);
    m_NumbersOfStoredSounds++;
    return soundPtr;
}
sound* SoundManager::LoadMusic(const String& fileName)
{
    for (int i = 0; i < m_NumberOfStoredMusicTracks; i++)
    {
        if (m_MusicFileNamesArr[i] == fileName)
        {
            return m_MusicPtrArr[i];
        }
        if (m_MusicFileNamesArr[i] == String("NULL"))
        {
            sound* soundPtr = new sound(fileName);
            m_MusicFileNamesArr[i] = fileName;
            m_MusicPtrArr[i] = soundPtr;
            std::wcout << L"The music with name " << fileName.C_str() << L" has been loaded into an empty slot" << std::endl;
            return soundPtr;
        }
    }
    std::wcout << L"The music with name " << fileName.C_str() << L" is loaded" << std::endl;
    sound* soundPtr = new sound(fileName);
    m_MusicFileNamesArr.push_back(fileName);
    m_MusicPtrArr.push_back(soundPtr);
    m_NumberOfStoredMusicTracks++;
    return soundPtr;
}

void SoundManager::MuteAll()
{
    for (size_t i = 0, n = m_SoundsPtrArr.size(); i < n; i++)
    {
        if (m_SoundsPtrArr[i] != nullptr)
        {
            m_SoundLevelsPtrArr.push_back(m_SoundsPtrArr[i]->get_volume());
            m_SoundsPtrArr[i]->set_volume(0);
            m_IsSoundMuted = true;
        }
    }
    for (size_t i = 0; i < m_MusicPtrArr.size(); i++)
    {
        if (m_MusicPtrArr[i] != nullptr)
        {
            m_MusicLevelsPtrArr.push_back(m_MusicPtrArr[i]->get_volume());
            m_MusicPtrArr[i]->set_volume(0);
            m_IsMusicMuted = true;
        }
    }
}
void SoundManager::UnMuteAll()
{
    for (size_t i = 0, n = m_SoundsPtrArr.size(); i < n; i++)
    {
        if (m_SoundsPtrArr[i] != nullptr)
        {
            m_SoundsPtrArr[i]->set_volume(m_SoundLevelsPtrArr[i]);
        }
    }
    m_SoundLevelsPtrArr.clear();

    for (size_t i = 0; i < m_MusicPtrArr.size(); i++)
    {
        if (m_MusicPtrArr[i] != nullptr)
        {
            m_MusicPtrArr[i]->set_volume(m_MusicLevelsPtrArr[i]);
        }
    }
}
void SoundManager::SetMusicVolume(double volume)
{
    for (size_t i = 0; i < m_MusicPtrArr.size(); i++)
    {
        if (m_MusicPtrArr[i] != nullptr && volume != m_MusicPtrArr[i]->get_volume())
        {
            m_MusicPtrArr[i]->set_volume(volume);
        }
    }


}
void SoundManager::SetSoundVolume(double volume)
{
    for (size_t i = 0; i < m_SoundsPtrArr.size(); i++)
    {
        double OldVolume = m_SoundsPtrArr[i]->get_volume();
        if (m_SoundsPtrArr[i] != nullptr && (int)(OldVolume*100) == (int)(volume*100))
        {
            m_SoundsPtrArr[i]->set_volume(volume);
            
        }
    }

}
void SoundManager::UnLoadMusic(sound* sndPtr)
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
void SoundManager::UnLoadSound(sound* sndPtr)
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
bool SoundManager::FadeIn(sound* tmpSoundPtr, double deltaTime)
{
    
    if (tmpSoundPtr->get_volume() > 1 - 0.01)
    {
        return true;
    }
    m_FadeAccuTime += deltaTime;
    double fadeSpeed = 0.05;
    if (m_FadeAccuTime >  fadeSpeed)
    {
        m_FadeAccuTime -= fadeSpeed;
        if (tmpSoundPtr->get_volume() + 0.01 < 1)
        {
            tmpSoundPtr->set_volume(tmpSoundPtr->get_volume() + 0.01);
        }
    }
    return false;
    
}
bool SoundManager::FadeOut(sound* tmpSoundPtr, double deltaTime)
{
    m_FadeAccuTime += deltaTime;
    double fadeSpeed = 0.05;
    if (tmpSoundPtr->get_volume() < 0 + 0.01)
    {
        return true;
    }
    if (m_FadeAccuTime >  0)
    {
        m_FadeAccuTime -= fadeSpeed;
        if (tmpSoundPtr->get_volume() - 0.01 > 0)
        {
            tmpSoundPtr->set_volume(tmpSoundPtr->get_volume() - 0.01);
        }
    }
    return false;
}
bool SoundManager::isMusicMuted()
{
    return m_IsMusicMuted;
}
bool SoundManager::isSoundMuted()
{
    return m_IsSoundMuted;
}