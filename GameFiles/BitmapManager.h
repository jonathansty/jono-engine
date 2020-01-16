#pragma once

//-----------------------------------------------------
// BitmapManager Class									
//-----------------------------------------------------
class BitmapManager
{
private:
    BitmapManager();				// Constructor
    static BitmapManager* m_BitmapManagerPtr;
public:
	
	virtual ~BitmapManager();		// Destructor

	// C++11 make the class non-copyable
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

 
