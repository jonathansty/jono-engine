#include "game.stdafx.h"		
#include "Animation.h"

Animation::Animation(float2 position)
		: m_Position(position) {
}

Animation::~Animation()
{
}

bool Animation::IsEnded()
{
    return m_IsEnded;
}


