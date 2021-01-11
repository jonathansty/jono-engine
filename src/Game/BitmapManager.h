#pragma once
#include "singleton.h"

// Bitmap manager singleton that manages caching bitmap files and cleaning up after use.
// This is a very primitive class and should be reworked using the resource loading instead
class BitmapManager : public TSingleton<BitmapManager>
{
private:
    BitmapManager();				// Constructor
    template<typename T>
    friend class TSingleton;
public:
	
	virtual ~BitmapManager();		// Destructor

	BitmapManager(const BitmapManager&) = delete;
	BitmapManager& operator=(const BitmapManager&) = delete;

    Bitmap* load_image(const String& fileName);

    void RemoveBitmapFile(const String& fileName);

private: 
    static const int SIZE = 30;
    Bitmap* m_BitmapPtrArr[SIZE];
    String m_FileNameArr[SIZE];
    int m_NumberOfStoredBitmaps = 0;
    
};

 
