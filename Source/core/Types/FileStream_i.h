#pragma once

#include "Types/TypeManager.h"

#include "Parsing/mini-yaml/MiniYaml.hpp"

enum class SerializationMode
{
	Read,
	Write
};


template<typename Container> 
struct is_container : std::false_type { };
template <typename...Ts> struct is_container<std::vector<Ts...>> : std::true_type{};
template <typename T, std::size_t N> struct is_container<std::array<T, N>> : std::true_type{};

class IFileStream
{
public:

	IFileStream(){}; 
	~IFileStream(){};


	template<typename T, std::enable_if_t<std::is_class<T>::value && !is_container<T>::value, bool> = true>
	bool ReadProperty(const char* propertyName, T& propertyValue)
	{
		TypeMetaData const* meta = T::GetStaticType();
		return ReadObject(propertyName, meta, reinterpret_cast<void*>(&propertyValue));
	}

	template<typename T, std::size_t N, std::enable_if_t<is_container<T>::value, bool> = true>
	bool ReadProperty(const char* propertyName, std::array<T, N>& propertyValue)
	{
		return false;
	}

	template <typename T, std::enable_if_t<is_container<T>::value, bool> = true>
	bool ReadProperty(const char* propertyName, std::vector<T>& propertyValue)
	{
		return false;
	}

	// Specialization to handle enum classes, they are serialized as u32
	template <typename T, std::enable_if_t<std::is_enum<T>::value, bool> = true>
	bool ReadProperty(const char* propertyName, T& propertyValue)
	{
		u32 value = (u32)propertyValue;
		bool result = ReadProperty(propertyName, value);
		if(result)
		{
			propertyValue = (T)value;
		}
		return result;
	}

	template<typename T, std::enable_if_t<std::is_fundamental<T>::value, bool> = true>
	bool ReadProperty(const char* propertyName, T& propertyValue);


	template <> bool ReadProperty(const char* propertyName, std::string& propertyValue) { return this->ReadStringProperty(propertyName, propertyValue); }
	template <> bool ReadProperty(const char* propertyName, int& propertyValue) { return this->ReadIntProperty(propertyName, propertyValue); }
	template <> bool ReadProperty(const char* propertyName, float& propertyValue) { return this->ReadFloatProperty(propertyName, propertyValue); }
	template <> bool ReadProperty(const char* propertyName, double& propertyValue) { return this->ReadDoubleProperty(propertyName, propertyValue); }
	template <> bool ReadProperty(const char* propertyName, bool& propertyValue) { return this->ReadBoolProperty(propertyName, propertyValue); }
	template <> bool ReadProperty(const char* propertyName, unsigned int& propertyValue) { return this->ReadUIntProperty(propertyName, propertyValue); }

	virtual bool ReadObject(const char* propertyName, TypeMetaData const* meta, void* obj) { return false; }
	virtual bool ReadStringProperty(const char* propertyName, std::string& propertyValue) { return false; }
	virtual bool ReadFloatProperty(const char* propertyName, float& propertyValue) { return false; }
	virtual bool ReadDoubleProperty(const char* propertyName, double& propertyValue) { return false; }
	virtual bool ReadIntProperty(const char* propertyName, int& propertyValue) { return false; }
	virtual bool ReadBoolProperty(const char* propertyName, bool& propertyValue) { return false; }
	virtual bool ReadUIntProperty(const char* propertyName, unsigned int& propertyValue) { return false; }

	SerializationMode GetMode() const { return m_Mode; }

protected: 
	SerializationMode m_Mode;

};
