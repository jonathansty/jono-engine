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
// HUD Class									
//-----------------------------------------------------
class Avatar;
class Level;
class BitmapManager;
class Slider;
class Game;
class Lever;
class HUD //: public ContactListener
{
public:
	HUD(Game* gamePtr);
	virtual ~HUD( );

	// C++11 make the class non-copyable
	HUD( const HUD& ) = delete;
	HUD& operator=( const HUD& ) = delete;

    enum class gameState
    {
        RUNNING,
        PAUSE
    };
	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Paint();
    void Tick(double deltaTime);
    void ResetTimer();
    double GetTime();
    double GetTotalTime();
    void SetTime(double time);
    void CreateSoundMuteBtn();
    void RemoveSoundMuteBtn();

    void PaintGameOverWindow(DOUBLE2 position);
    void PaintMoneyWindow(DOUBLE2 position);
    void PaintDeathCounterWindow(DOUBLE2 position);
    void PaintMuteSound();
    void CreatePauseMenu();
    void RemovePauseMenu();



    bool IsGoToStartMenu();

    void LinkLevers(std::vector<Lever*>tmpArray);
    void PaintLeverInfo();

    void ResetIsInMenu();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    double m_AccuTime = 0;

    Sound* m_BgMusicPtr = nullptr;
    Avatar* m_AvatarPtr = nullptr;
    Level* m_LevelPtr = nullptr;
    Font* m_FntPtr = nullptr;
    Bitmap* m_BmpMoneyDisplayPtr = nullptr;
    Bitmap* m_BmpDeathIconPtr = nullptr;
    Bitmap* m_BmpGameOverPtr = nullptr;
    bool m_IsGameOverDrawn = false;
    int m_GameOverOpacity = 255;
    Button* m_BtnSndMutePtr = nullptr;


    bool m_Muted = false;

    Button *m_BtnQuitGamePtr = nullptr, *m_BtnQuitToMenuPtr = nullptr;

    gameState m_GameState = gameState::RUNNING;

    std::vector<Lever*>m_LeversPtrArr;
    bool m_AreThereLevers = false;
    bool m_IsInMenu = false;
    bool m_GoToStartMenu = false;
};

 
