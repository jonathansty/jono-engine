#include "engine.pch.h"
#include "EngineCfg.h"

#include "CommandLine.h"
#include "Types/IniStream.h"


REGISTER_TYPE("/Types/Core/EngineCfg", EngineCfg);
REGISTER_TYPE("/Types/Core/GraphicsSettings", GraphicsSettings);

SERIALIZE_FN(EngineCfg)
{
	SERIALIZE_PROPERTY(UseD2D);
	SERIALIZE_PROPERTY(UseD2DAA);
	SERIALIZE_PROPERTY(UseD3D);
	SERIALIZE_PROPERTY(D3DMSAA);
	SERIALIZE_PROPERTY(MaxFrametime);
	SERIALIZE_PROPERTY(GraphicsSettings);
}

SERIALIZE_FN(GraphicsSettings)
{
	SERIALIZE_PROPERTY(Foo);
}
