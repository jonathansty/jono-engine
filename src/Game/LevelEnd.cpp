#include "stdafx.h"		
	
#include "BitmapManager.h"
#include "LevelEnd.h"
#include "Avatar.h"
#include "Lever.h"


LevelEnd::LevelEnd(DOUBLE2 position, String nextLevel) :
Entity(position),
m_NextLevelPath(nextLevel)
{
    m_BmpPtr = BitmapManager::instance()->load_image(String("Resources/Entity/LevelEnd.png"));
    m_ActPtr = new PhysicsActor(position, 0, BodyType::STATIC);
    m_ActPtr->SetTrigger(true);
    m_ActPtr->AddCircleShape(-20 + m_BmpPtr->GetWidth()/2);
    m_ActPtr->AddContactListener(this);
    m_ActPtr->SetName(String("LevelEnd"));

   
}

LevelEnd::~LevelEnd()
{
    if (m_ActPtr != nullptr)
    {
        m_ActPtr->RemoveContactListener(this);
        delete m_ActPtr;
        m_ActPtr = nullptr;
    }
    for (size_t i = 0; i < m_LeversPtrArr.size(); i++)
    {
        delete m_LeversPtrArr[i];
    }
    m_LeversPtrArr.clear();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void LevelEnd::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (m_AreAllLeversHit && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_IsHit = true;
    }
}

void LevelEnd::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (m_AreAllLeversHit && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_IsHit = false;
    }
}

void LevelEnd::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void LevelEnd::Tick(double deltaTime)
{
    m_Position = m_ActPtr->GetPosition();
    m_Angle += deltaTime*m_RotationSpeed;

    if (AreLeversHit() && m_AreAllLeversHit == false)
    {
        m_AreAllLeversHit = true;
    }
}
void LevelEnd::Paint(graphics::D2DRenderContext& ctx)
{
    if (m_AreAllLeversHit == true)
    {
        MATRIX3X2 matTranslate, matPivot, matRotate;
        matTranslate.SetAsTranslate(m_Position);
        matRotate.SetAsRotate(m_Angle);
        matPivot.SetAsTranslate(DOUBLE2(-m_BmpPtr->GetWidth() / 2, -m_BmpPtr->GetHeight() / 2));
        GameEngine::instance()->set_world_matrix(matPivot*matRotate* matTranslate);
        GameEngine::instance()->draw_bitmap(m_BmpPtr);
        GameEngine::instance()->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
    }
    
    for (size_t i = 0; i < m_LeversPtrArr.size(); i++)
    {
        m_LeversPtrArr[i]->Paint();
    }
}

void LevelEnd::Reset()
{

}
bool LevelEnd::isHit()
{
    return m_IsHit;
}
bool LevelEnd::AreLeversHit()
{
    for (size_t i = 0; i < m_LeversPtrArr.size(); i++)
    {
        if (!(m_LeversPtrArr[i]->isHit()))
        {
            return false;
        }
    }
    return true;
}
String LevelEnd::GetNextLevel()
{
    return m_NextLevelPath;
}
void LevelEnd::SetAvatar(Avatar* avatarPtr)
{
    Entity::SetAvatar(avatarPtr);
    for (size_t i = 0; i < m_LeversPtrArr.size(); i++)
    {
        m_LeversPtrArr[i]->SetAvatar(m_AvatarPtr);
    }
}
void LevelEnd::Add(Lever* tmpLeverPtr)
{
    m_LeversPtrArr.push_back(tmpLeverPtr);
}
std::vector<Lever*> LevelEnd::GetLeversArray()
{
    return m_LeversPtrArr;
}