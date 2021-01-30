//-----------------------------------------------------
// Name, first name: Steyfkens, Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "game.stdafx.h"	
#include "BitmapManager.h"

BitmapManager::BitmapManager() : m_BitmapPtrArr{}
{
    for (int i = 0; i < SIZE; i++)
    {
        m_BitmapPtrArr[i] = nullptr;
    }
}

BitmapManager::~BitmapManager()
{
    for (int i = 0; i < m_NumberOfStoredBitmaps; i++)
    {
        delete m_BitmapPtrArr[i];
        m_BitmapPtrArr[i] = nullptr;
    }
}

Bitmap* BitmapManager::load_image(const String& fileName)
{
    // Check the whole array for the filename, if it already exists return the existing bitmap
    if (m_NumberOfStoredBitmaps+ 1 > SIZE)
    {
        GameEngine::instance()->message_box(String("Maximum amount of bitmaps reached. Please increase the size of the array."));
    }
    for (int i = 0; i < m_NumberOfStoredBitmaps; i++)
    {
        if (m_FileNameArr[i] == fileName)
        {
            return m_BitmapPtrArr[i];
        }
        if (m_FileNameArr[i] == String("NULL"))
        {
            Bitmap* tmpBitmap = new Bitmap(fileName);
            m_FileNameArr[i] = fileName;
            m_BitmapPtrArr[i] = tmpBitmap;
            return tmpBitmap;
        }
    }

    //We never loaded this bitmap before then execute this
    std::wcout << L"The bitmap with name " << fileName.C_str() << L" is loaded" << std::endl;
    Bitmap * bmpPtr = new Bitmap(fileName);
    m_BitmapPtrArr[m_NumberOfStoredBitmaps] = bmpPtr;
    m_FileNameArr[m_NumberOfStoredBitmaps] = fileName;

    m_NumberOfStoredBitmaps++;
    return bmpPtr;
}
void BitmapManager::RemoveBitmapFile(const String& fileName)
{
    for (int i = 0; i < m_NumberOfStoredBitmaps; i++)
    {
        if (m_FileNameArr[i] == fileName)
        {
            m_FileNameArr[i] = String("NULL");
            delete m_BitmapPtrArr[i];
            m_BitmapPtrArr[i] = nullptr;
            GameEngine::instance()->print_string(String("Succesfully deleted ") + fileName);
            return;
        }
    }
    GameEngine::instance()->print_string(fileName + String(" was not found in the BitmapManager"));
}



