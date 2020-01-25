#pragma once

namespace rtti
{
	class Destructor
	{
	public:
		virtual void destruct(rtti::Object& obj) = 0;
	};
}