#pragma once
namespace rtti
{
	template<typename T>
	class TDestructor : public Destructor
	{
	public:
		void destruct(rtti::Object& obj)
		{
			if (std::is_array<T>::value == 1)
			{
				delete[] (T*)obj._data;
			}
			else
			{
				delete (T*)obj._data;
			}
		}
	};
}