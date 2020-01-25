#pragma once

namespace rtti
{
	class Object;

	template<typename T>
	class TConstructor : public Constructor
	{
	public:
		rtti::Object invoke() override
		{
			return rtti::Object::create<T>();
		}
	};
}