#pragma once
#include "singleton.h"

// Bitmap manager singleton that manages caching bitmap files and cleaning up after use.
// This is a very primitive class and should be reworked using the resource loading instead
class bitmap_manager : public TSingleton<bitmap_manager>
{
private:
    bitmap_manager();				// Constructor
    template<typename T>
    friend class TSingleton;
public:
	
	virtual ~bitmap_manager();		// Destructor

	bitmap_manager(const bitmap_manager&) = delete;
	bitmap_manager& operator=(const bitmap_manager&) = delete;

    Bitmap* LoadBitmapFile(const String& fileName);

    void RemoveBitmapFile(const String& fileName);

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    
    static const int SIZE = 30;
    Bitmap* m_BitmapPtrArr[SIZE];
    String m_FileNameArr[SIZE];
    int m_NumberOfStoredBitmaps = 0;
    
};

 
