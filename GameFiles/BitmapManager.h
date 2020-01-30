#pragma once
#include "singleton.h"

//-----------------------------------------------------
// BitmapManager Class									
//-----------------------------------------------------
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

    //Static methods
    static BitmapManager* GetSingleton();
	//-------------------------------------------------
	// Methods - Member functions							
	//-------------------------------------------------
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

 
