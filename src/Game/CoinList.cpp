#include "game.stdafx.h"		
	
#include "CoinList.h"
#include "Coin.h"

CoinList::CoinList()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

CoinList::~CoinList()
{
    RemoveAll();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void CoinList::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void CoinList::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void CoinList::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void CoinList::Tick(double deltatime)
{
    for (size_t i = 0; i < m_CoinsPtrArr.size(); i++)
    {
        if (m_CoinsPtrArr[i] != nullptr)
        {
            m_CoinsPtrArr[i]->Tick(deltatime);
        }
    }
}
void CoinList::Paint(graphics::D2DRenderContext& ctx)
{
    for (size_t i = 0; i < m_CoinsPtrArr.size(); i++)
    {
        if (m_CoinsPtrArr[i] != nullptr)
        {
            m_CoinsPtrArr[i]->Paint(ctx);
        }
    }
}
void CoinList::PaintDebug(graphics::D2DRenderContext& ctx)
{
    for (size_t i = 0, n = m_CoinsPtrArr.size(); i < n; i++)
    {
        if (m_CoinsPtrArr[i] != nullptr)
        {
            float2 position = m_CoinsPtrArr[i]->GetPosition() - float2(10,30);
            String name = m_CoinsPtrArr[i]->GetName();
            ctx.draw_string(name,position);
        }
    }
}
void CoinList::Add(Coin* coinPtr)
{
    coinPtr->SetIndex(m_AmountOfCoins);
    for (size_t i = 0; i < m_CoinsPtrArr.size(); i++)
    {
        if (m_CoinsPtrArr[i] == nullptr)
        {
            
            m_CoinsPtrArr[i] = coinPtr;
        }
    }
    m_AmountOfCoins++;
    m_CoinsPtrArr.push_back(coinPtr);
}
void CoinList::Remove(Coin* coinPtr)
{

    for (size_t i = 0; i < m_CoinsPtrArr.size(); i++)
    {
        if (m_CoinsPtrArr[i] == coinPtr)
        {
            m_CoinsPtrArr[i]->RemoveContactListener();
            delete m_CoinsPtrArr[i];
            m_CoinsPtrArr[i] = nullptr;
        }
    }
}
void CoinList::RemoveAll()
{
    
    for (size_t i = 0; i < m_CoinsPtrArr.size(); i++)
    {
        if (m_CoinsPtrArr[i] != nullptr)
        {
            m_CoinsPtrArr[i]->RemoveContactListener();
            Remove(m_CoinsPtrArr[i]);
        }
    }
    m_CoinsPtrArr.clear();
}
void CoinList::RemoveAllContactListeners()
{
    for (size_t i = 0; i < m_CoinsPtrArr.size(); i++)
    {
        if (m_CoinsPtrArr[i] != nullptr)
        {
            m_CoinsPtrArr[i]->RemoveContactListener();
        }
        
    }
}
int CoinList::GetValue()
{
    int sumValue = 0;
    for (size_t i = 0; i < m_CoinsPtrArr.size(); i++)
    {
        if (m_CoinsPtrArr[i] != nullptr && m_CoinsPtrArr[i]->IsHit())
        {
            sumValue += m_CoinsPtrArr[i]->GetCoinValue();
        }
    }
    return sumValue;
}
int CoinList::IsHit()
{
    int sumValue = 0;
    for (size_t i = 0; i < m_CoinsPtrArr.size(); i++)
    {
        if (m_CoinsPtrArr[i] != nullptr && m_CoinsPtrArr[i]->IsHit())
        {
            sumValue += m_CoinsPtrArr[i]->GetCoinValue();
            Remove(m_CoinsPtrArr[i]);
            return sumValue;
        }
    }
    return 0;
}