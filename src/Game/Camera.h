#pragma once
//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
#include "CameraTrigger.h"
//-----------------------------------------------------
// Camera Class									
//-----------------------------------------------------
class Avatar;
class Level;
class Camera //: public ContactListener
{
public:
	Camera(Level* levelPtr, Avatar* avatarPtr, double scale);
	virtual ~Camera( );

	// C++11 make the class non-copyable
	Camera( const Camera& ) = delete;
	Camera& operator=( const Camera& ) = delete;

    enum class controlState
    {
        MANUAL,
        FOLLOWAVATAR,
        AUTOMATIC
    };
    enum class Shakemode
    {
        NOSHAKE,
        ATTACKSHAKE,
        EPICEFFECT
    };

    void Tick(double dTime);
    void Paint();

    //! Returns the viewmatrix of the camera.
    MATRIX3X2 GetViewMatrix();

    //! Gets the camera Dimensions
    DOUBLE2 GetCameraDimension() const;

    //! Sets the current cameraPosition
    void SetCameraPosition(DOUBLE2 position);

    //! Sets the cameraStartPosition
    void SetCameraStartPosition(DOUBLE2 position);

    //! Returns the current speed of the camera.
    double GetCameraSpeed() const;

    //! Sets the camera MaxSpeed
    void SetCameraSpeed(double speed);

    //! Returns the cameraPosition
    DOUBLE2 GetCameraPosition() const;

    //! Gets the startPosition of the camera in the level.
    DOUBLE2 GetCameraStartPosition() const;

    //! Gets the current Camere Direction
    DOUBLE2 GetCameraDirection() const;

    //! Resets the camera
    //! Resets Speed
    //! Resets angle
    //! Set CameraPosition
    //! Reset CameraControlState to default state
    void Reset(DOUBLE2 position);

    //! Gets the current camera Angle
    double GetAngle() const;
    
    //! Sets the angle of the camera
    void SetAngle(double angle);

    //! Sets the zooming factor of the camera. Bigger than 0.1
    void SetScale(double scale);
    
    /*
    * Methods involving Camera Shake
    */
    //! Set current camera Mode (NOSHAKE , ATTACKSHAKE, EPICEFFECT)
    void SetCameraMode(controlState controlstate);

    //!Gets the current camera Mode.
    controlState GetCameraMode() const;

    //! Shakes the screen. Amount determines how strong it is and length determines the time period
    void ShakeScreen(int amount, double length, double deltaTime);

    //! Returns the camera ShakeMode ( NOSHAKE, ATTACKSHAKE or EPICEFFECT)
    Shakemode GetCameraShakeMode() const;

    //! Sets the current shake mode ( NOSHAKE, ATTACKSHAKE or EPICEFFECT)
    void SetCameraShakeMode(Shakemode tmpShakeMode);

    //! Sets the default Camera controlState.
    void SetDefaultCameraMode(controlState controlstate);

private: 
    // ------------------------------------------------
    // Private methods
    // ------------------------------------------------
    //! Updates the screen shaking modes for use with buttons certain actions etc
    void HandleScreenShaking(double deltaTime);
    void FollowAvatar(double deltaTime, DOUBLE2 &oldPosition);
    void AutomaticMode(double deltaTime, DOUBLE2 &oldPosition);
    void ManualMode(double deltaTime, DOUBLE2 &oldPosition);
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

    // Static const ints used for determining avatar movement square
    static const int MAX_LEFT = 600;
    static const int MAX_RIGHT = 600;
    static const int MAX_TOP = 350;
    static const int MAX_BOTTOM = 350;
    // static double to determine the delay of epicmode when turned on
    static double const EPICMODEDELAY;
    static const int EPICMODEBARSIZE = 100;

    Avatar* m_AvatarPtr = nullptr;
    Level* m_LevelPtr = nullptr;
    PhysicsActor* m_ActPtr = nullptr;
    PhysicsActor* m_ActBoundsPtr = nullptr;

    DOUBLE2 m_CameraPosition;
    DOUBLE2 m_CameraPositionWithoutShaking;
    DOUBLE2 m_OldCameraPosition;
    DOUBLE2 m_StartPosition;
    DOUBLE2 m_CameraDimension;
    DOUBLE2 m_Direction;

    double m_MaxSpeed = 250;
    double m_AccuTime = 0;
    double m_ScreenShakeAccuTime = 0;
    double m_EpicEffectBarsSize = 1;
    Shakemode m_ShakeMode = Shakemode::NOSHAKE;
    double m_Scale = 1;
    double m_Speed = 0;
    double m_Angle = 0;
    double m_yPosition;
    int m_ScreenShakeOffset = 0;

    std::vector<CameraTrigger*>m_CameraTriggersPtrArr;
    controlState m_DefaultCameraControlState = controlState::FOLLOWAVATAR;
    controlState m_CameraControlState = controlState::AUTOMATIC;
};

 
