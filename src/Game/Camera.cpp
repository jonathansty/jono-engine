#include "stdafx.h"		
	
#include "Camera.h"
#include "Avatar.h"
#include "Level.h"
#include "CameraTriggerRotate.h"

const double Camera::EPICMODEDELAY = 1.2;

Camera::Camera(Level* levelPtr, Avatar* avatarPtr,double scale):
m_AvatarPtr(avatarPtr),
m_LevelPtr(levelPtr),
m_CameraPosition(GameEngine::instance()->get_width() / 2, GameEngine::instance()->get_height()/2),
m_CameraPositionWithoutShaking(GameEngine::instance()->get_width() / 2, GameEngine::instance()->get_height()/2),
m_OldCameraPosition(GameEngine::instance()->get_width()/2,GameEngine::instance()->get_height()/2),
m_CameraDimension(GameEngine::instance()->get_width(), GameEngine::instance()->get_height()),
m_StartPosition(GameEngine::instance()->get_width() / 2, GameEngine::instance()->get_height() / 2),
m_Direction(0,1)
{
    m_ActPtr = new PhysicsActor(m_CameraPosition, 0, BodyType::DYNAMIC);
    m_ActPtr->AddBoxShape(10, 10);
    m_ActPtr->SetGravityScale(0);
    m_ActPtr->SetTrigger(true);
    m_ActPtr->SetName(String("Camera"));

    m_Scale = scale;
    m_ActBoundsPtr = new PhysicsActor(m_CameraPosition, 0, BodyType::KINEMATIC);
    std::vector<DOUBLE2>pointsPtrArr;
    pointsPtrArr.push_back(DOUBLE2(-GameEngine::instance()->get_width()/2,-GameEngine::instance()->get_height()/2));
    pointsPtrArr.push_back(DOUBLE2(-GameEngine::instance()->get_width()/2, 20 + GameEngine::instance()->get_height()/2));
    pointsPtrArr.push_back(DOUBLE2( GameEngine::instance()->get_width()/2, 20 + GameEngine::instance()->get_height()/2));

    for (size_t i = 0; i < pointsPtrArr.size(); i++)
    {
        MATRIX3X2 matScale;
        matScale.SetAsScale(scale);
        pointsPtrArr[i] = matScale.TransformPoint(pointsPtrArr[i]);
    }
    m_ActBoundsPtr->AddChainShape(pointsPtrArr,false);
    m_ActBoundsPtr->SetName(String("CameraDeathBounds"));
    m_ActBoundsPtr->SetTrigger(true);

}

Camera::~Camera()
{
    delete m_ActPtr;
    m_ActPtr = nullptr;
    delete m_ActBoundsPtr;
    m_ActBoundsPtr = nullptr;
}

//! Returns the viewmatrix of the camera.
MATRIX3X2 Camera::GetViewMatrix(){
    DOUBLE2 position(0,0);
    if (m_AvatarPtr != nullptr)
    {
        position = m_AvatarPtr->GetPosition();
    }
    

    // These 2 if statements check for the boundaries
    MATRIX3X2 matTranslate, matCenter, matRotate,matScale;
    MATRIX3X2 matWorldTransform;
    matCenter.SetAsTranslate(-GameEngine::instance()->get_width() / 2, -GameEngine::instance()->get_height() / 2);
    GameEngine::instance()->set_bitmap_interpolation_mode(bitmap_interpolation_mode::linear);
    matTranslate.SetAsTranslate(m_CameraPosition);
    matRotate.SetAsRotate(m_Angle);
    matScale.SetAsScale(m_Scale);
    matWorldTransform = matCenter*matRotate*matScale * matTranslate;
    return matWorldTransform.Inverse();
}

void Camera::Tick(double dTime)
{
    m_AccuTime += dTime;
    HandleScreenShaking(dTime);
    //m_CameraPosition = m_ActPtr->GetPosition();
    m_ActBoundsPtr->SetPosition(m_CameraPosition);
    m_ActBoundsPtr->SetAngle(m_Angle);

    DOUBLE2 positionBeforeShaking = m_CameraPositionWithoutShaking;
    int offsetx = 0;
    int offsety = 0;
    if (m_ScreenShakeOffset != 0)
    {
        offsetx = (rand() % m_ScreenShakeOffset) - m_ScreenShakeOffset / 2;
        offsety = (rand() % m_ScreenShakeOffset) - m_ScreenShakeOffset / 2;
    }
    if (m_CameraPosition.x + (m_CameraDimension.x*m_Direction.x)/2 < m_LevelPtr->GetBgBmpPtr()->GetWidth() + m_CameraDimension.x/2)
    {
        if (m_CameraControlState == controlState::FOLLOWAVATAR)
        {
            FollowAvatar(dTime, positionBeforeShaking);
        }
        if (m_CameraControlState == controlState::AUTOMATIC)
        {
            AutomaticMode(dTime, positionBeforeShaking);
        }
        if (m_CameraControlState == controlState::MANUAL)
        {
            ManualMode(dTime, positionBeforeShaking);
        }
    }
    //Updating the camera position
    m_ActPtr->SetPosition(positionBeforeShaking + DOUBLE2(offsetx, offsety));
    m_CameraPosition = m_ActPtr->GetPosition();
    m_CameraPositionWithoutShaking = positionBeforeShaking;
    if (GameEngine::instance()->is_key_pressed(VK_F7))
    {
        switch (m_CameraControlState)
        {
        case Camera::controlState::MANUAL:
            Reset(m_OldCameraPosition);
            m_CameraControlState = controlState::FOLLOWAVATAR;
            std::cout << "Camera is now following the avatar!" << std::endl;
            break;
        case Camera::controlState::FOLLOWAVATAR:
            m_OldCameraPosition = m_CameraPosition;
            m_Speed = 0;
            Reset(m_StartPosition);
            m_CameraControlState = controlState::AUTOMATIC;
            std::cout << "Camera is now automatic" << std::endl;
            break;
        case Camera::controlState::AUTOMATIC:
            m_OldCameraPosition = m_CameraPosition;
            m_Speed = 300;
            m_CameraControlState = controlState::MANUAL;
            std::cout << "Camera is now manual controlled!" << std::endl;
        default:
            break;
        }
    }
}
void Camera::FollowAvatar(double deltaTime, DOUBLE2 &oldPosition)
{
    
    DOUBLE2 avatarPosition = m_AvatarPtr->GetPosition();
    double distance = abs(avatarPosition.x - m_CameraPosition.x);
    double yDistance = abs(avatarPosition.y - m_CameraPosition.y);
    double fraction = distance / (MAX_RIGHT);
    fraction = fraction*fraction;

    //Hold the position before we apply the shake to it.

    if (avatarPosition.x > (m_CameraPosition.x))
    {
        oldPosition.x += fraction * distance;
    }
    if (avatarPosition.x < (m_CameraPosition.x))
    {
        fraction = distance / (MAX_LEFT);
        fraction = fraction*fraction;
        oldPosition.x -= fraction * distance;
    }
    if (avatarPosition.x > m_CameraPosition.x + m_CameraDimension.x)
    {
        m_CameraPosition = avatarPosition;
    }
    if (avatarPosition.y > m_CameraPosition.y)
    {
        fraction = yDistance / MAX_BOTTOM;
        fraction = fraction*fraction;
        oldPosition.y += fraction *yDistance;
    }
    if (avatarPosition.y < m_CameraPosition.y)
    {
        fraction = yDistance / MAX_TOP;
        fraction = fraction*fraction;
        oldPosition.y -= fraction *yDistance;
    }
    if (avatarPosition.y < m_CameraPosition.y - m_CameraDimension.y / 2)
    {
        oldPosition.y = avatarPosition.y;
    }
}
void Camera::AutomaticMode(double deltaTime, DOUBLE2 &oldPosition)
{
    // Interpolation algorithme for starting
    if (m_AccuTime>0.5)
    {
        if (m_Speed < m_MaxSpeed)
        {
            m_Speed += 50 * deltaTime;
        }
        double angle = m_ActPtr->GetAngle();
        m_Direction.x = cos(angle);
        m_Direction.y = sin(angle);
        int offsetx = 0;
        int offsety = 0;
        if (m_ScreenShakeOffset != 0)
        {
            offsetx = rand() % m_ScreenShakeOffset - m_ScreenShakeOffset / 2;
            offsety = rand() % m_ScreenShakeOffset - m_ScreenShakeOffset / 2;
        }
        oldPosition = m_CameraPositionWithoutShaking + m_Direction.Normalized()*m_Speed*deltaTime;

        DOUBLE2 vectorToAvatar = m_AvatarPtr->GetPosition() - oldPosition;
        DOUBLE2 normalizedVector = vectorToAvatar.Normalized();
        double length = vectorToAvatar.Length();
        DOUBLE2 offsetPosition2 = DOUBLE2(GameEngine::instance()->get_width() - 200, 0);
        MATRIX3X2 matTranslate, matRotate, matScale, matTransform, matPivot;
        matPivot.SetAsTranslate(DOUBLE2(-m_CameraDimension.x / 2, -m_CameraDimension.y / 2));
        matTranslate.SetAsTranslate(oldPosition);
        matRotate.SetAsRotate(m_Angle);
        matScale.SetAsScale(m_Scale);
        matTransform = matPivot * matRotate * matScale * matTranslate;
        offsetPosition2 = matTransform.TransformPoint(offsetPosition2);
        if (m_AvatarPtr->GetPosition().x > offsetPosition2.x)
        {
            m_Speed += (m_AvatarPtr->GetPosition().x - offsetPosition2.x) * deltaTime;
        }
        if (m_AvatarPtr->GetPosition().x < offsetPosition2.x - 10 && m_Speed > m_MaxSpeed)
        {
            m_Speed -= abs(m_AvatarPtr->GetPosition().x - offsetPosition2.x) * deltaTime;
        }


    }
}
void Camera::ManualMode(double deltaTime, DOUBLE2 &oldPosition)
{
    DOUBLE2 actorPosition = m_ActPtr->GetPosition();
    DOUBLE2 newPosition = DOUBLE2();
    if (GameEngine::instance()->is_key_down('I'))newPosition += DOUBLE2(0, -m_Speed * deltaTime);
    if (GameEngine::instance()->is_key_down('J'))newPosition += DOUBLE2(-m_Speed * deltaTime, 0);
    if (GameEngine::instance()->is_key_down('K'))newPosition += DOUBLE2(0, m_Speed * deltaTime);
    if (GameEngine::instance()->is_key_down('L'))newPosition += DOUBLE2(m_Speed * deltaTime, 0);
    oldPosition = actorPosition + newPosition;

    if (GameEngine::instance()->is_key_down('2'))
    {
        m_Angle += 0.01;
        m_ActPtr->SetAngle(m_Angle);

    }
    if (GameEngine::instance()->is_key_down('1'))
    {
        m_Angle -= 0.01;
        m_ActPtr->SetAngle(m_Angle);
    }
}

void Camera::Paint(graphics::D2DRenderContext& ctx)
{
    auto engine = GameEngine::instance();
    if (m_ShakeMode == Shakemode::EPICEFFECT)
    {
        
        if (m_EpicEffectBarsSize <EPICMODEBARSIZE && m_AccuTime > EPICMODEDELAY)
        {
            m_EpicEffectBarsSize += EPICMODEBARSIZE/10;
        }
        
    }
    else
    {
        if (m_EpicEffectBarsSize > 1)
        {
            m_EpicEffectBarsSize -= 10;
        }
    }
    if (m_EpicEffectBarsSize > 2)
    {
        int width = engine->get_width();
        int height = engine->get_height();
        engine->set_color(COLOR(0, 0, 0));
        engine->fill_rect(0, 0, width, static_cast<int>(m_EpicEffectBarsSize));
        engine->fill_rect(0, static_cast<int>(height - m_EpicEffectBarsSize), width, height);
    }
    
}
//! Gets the camera Dimensions
DOUBLE2 Camera::GetCameraDimension() const
{
    return m_CameraDimension;
}

//! Sets the current cameraPosition
void Camera::SetCameraPosition(DOUBLE2 position)
{
    m_ActPtr->SetPosition(position);
    m_ActBoundsPtr->SetPosition(position);
    m_CameraPosition = position;
    m_CameraPositionWithoutShaking = position;
}
//! Sets the cameraStartPosition
void Camera::SetCameraStartPosition(DOUBLE2 position)
{
    m_StartPosition = position;
}
//! Sets the camera MaxSpeed
void Camera::SetCameraSpeed(double speed)
{
    m_MaxSpeed = speed;
}
//! Returns the cameraPosition
DOUBLE2 Camera::GetCameraPosition() const
{
    return m_ActPtr->GetPosition();
}
//! Returns the current speed of the camera.
double Camera::GetCameraSpeed() const
{
    return m_Speed;
}
//! Gets the startPosition of the camera in the level.
DOUBLE2 Camera::GetCameraStartPosition() const
{
    return m_StartPosition;
}
//!Gets the current Camere Direction
DOUBLE2 Camera::GetCameraDirection() const
{
    return m_Direction;
}

//! Resets the camera
//! Resets Speed
//! Resets angle
//! Set CameraPosition
//! Reset CameraControlState to default state
void Camera::Reset(DOUBLE2 position)
{
    m_Speed = 0;
    m_AccuTime = 0;
    m_Angle = 0;
    SetCameraPosition(position);
    m_CameraControlState = m_DefaultCameraControlState;
}
//! Gets the current camera Angle
double Camera::GetAngle() const
{
    return m_Angle;
}
//! Sets the angle of the camera
void Camera::SetAngle(double angle)
{
    m_Angle = angle;
    if (m_ActPtr != nullptr)
    {
        m_ActPtr->SetAngle(angle);
    }
    
    
}
//! Sets the zooming factor of the camera. Bigger than 0.1
void Camera::SetScale(double scale)
{
    if (scale < 0.1)
    {
        scale = 0.1;
    }
    m_Scale = scale;
}

/*
* Methods involving Camera Shake
*/
//!Set current camera Mode (NOSHAKE , ATTACKSHAKE, EPICEFFECT)
void Camera::SetCameraMode(controlState testPtr)
{
    m_CameraControlState = testPtr;
}
//!Shakes the screen. Amount determines how strong it is and length determines the time period
void Camera::ShakeScreen(int amount, double length, double deltaTime)
{
    m_ScreenShakeAccuTime += deltaTime;
    if (m_ScreenShakeAccuTime < length)
    {
        if (amount > 0)
        {
            m_ScreenShakeOffset = amount;
        }
        
    }
    else
    {
        if (amount > 0)
        {
            m_ScreenShakeOffset = 0;
            m_ScreenShakeAccuTime = 0;
            switch (m_ShakeMode)
            {
            case Camera::Shakemode::NOSHAKE:
                break;
            case Camera::Shakemode::ATTACKSHAKE:
                m_ShakeMode = Shakemode::NOSHAKE;
                break;
            case Camera::Shakemode::EPICEFFECT:
                break;
            default:
                break;
            }
            
        }
    }
    
    
}
//! Returns the camera ShakeMode ( NOSHAKE, ATTACKSHAKE or EPICEFFECT)
Camera::Shakemode Camera::GetCameraShakeMode() const
{
    return m_ShakeMode;
}
//! Sets the current shake mode ( NOSHAKE, ATTACKSHAKE or EPICEFFECT)
void Camera::SetCameraShakeMode(Camera::Shakemode tmpShakeMode)
{
    m_AccuTime = 0;
    m_ShakeMode = tmpShakeMode;
}
//! Sets the default Camera controlState.
void Camera::SetDefaultCameraMode(Camera::controlState controlState)
{
    m_DefaultCameraControlState = controlState;
}
//! Updates the screen shaking modes for use with buttons certain actions etc
void Camera::HandleScreenShaking(double deltaTime)
{
    if (GameEngine::instance()->is_key_down('X') && (m_AvatarPtr->GetMoveState() == Avatar::moveState::ATTACK) && m_ShakeMode != Shakemode::EPICEFFECT)
    {
        m_ShakeMode = Shakemode::ATTACKSHAKE;
    }

    switch (m_ShakeMode)
    {
    case Camera::Shakemode::NOSHAKE:
        m_ScreenShakeOffset = 0;
        break;
    case Camera::Shakemode::ATTACKSHAKE:
        ShakeScreen(25, 0.1, deltaTime);
        break;
    case Camera::Shakemode::EPICEFFECT:
        if (m_AccuTime > EPICMODEDELAY)
        {
            ShakeScreen(10, 1, deltaTime);
        }

        break;
    default:
        break;
    }
}
Camera::controlState Camera::GetCameraMode() const
{

    return m_CameraControlState;
}
