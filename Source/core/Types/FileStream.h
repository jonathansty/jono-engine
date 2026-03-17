#pragma once

#include "Parsing/mini-yaml/MiniYaml.hpp"
#include "FileStream_i.h"

struct IniSectionInfo
{
	std::string_view m_Name;
	std::string_view m_Type;
	std::string_view m_Data;

	std::unordered_map<std::string_view, std::string_view> m_Attributes;
};



class YamlStream : public IFileStream
{
	public: 
		YamlStream(const char* data, u32 size);

		virtual ~YamlStream();

		bool ReadObject(const char* propertyName, TypeMetaData const* meta, void* obj);

		bool ReadStringProperty(const char* propertyName, std::string& outValue) { return ReadInternal<std::string>(propertyName, outValue); }
		bool ReadIntProperty(const char* propertyName, int& outValue) { return ReadInternal<int>(propertyName, outValue); }
		bool ReadUIntProperty(const char* propertyName, unsigned int& outValue) { return ReadInternal<unsigned int>(propertyName, outValue); }
		bool ReadFloatProperty(const char* propertyName, float& outValue) { return ReadInternal<float>(propertyName, outValue); }
		bool ReadDoubleProperty(const char* propertyName, double& outValue) { return ReadInternal<double>(propertyName, outValue); }
		bool ReadBoolProperty(const char* propertyName, bool& outValue) { return ReadInternal<bool>(propertyName, outValue); }

		template <typename T>
		bool ReadContainer(const char* propertyName, std::vector<T>& outContainer)
		{
			Yaml::Node n = m_Current[propertyName];
			if(!n.IsSequence())
			{
				return false;
			}

			for(auto it = n.Begin(); it != n.End(); it++)
			{
				Yaml::Node node = (*it).second;
				T value = node.As<T>();
				outContainer.push_back(std::move(value));
			}
			return true;
		}


	private:
		struct ScopedRead 
		{
			ScopedRead(Yaml::Node& n, const char* propertyName)
					: m_Parent(n)
					, m_TargetNode(&n)
			{
				*m_TargetNode = m_Parent[propertyName];
			}

			~ScopedRead() 
			{
				*m_TargetNode = m_Parent;
			}

			Yaml::Node* m_TargetNode;
			Yaml::Node m_Parent;
		
		};

		template<typename T>
		bool ReadInternal(const char* propertyName, T& outValue)
		{
			Yaml::Node n = m_Current[propertyName];
			if(!n.IsNone())
			{
				outValue = n.As<T>();
				return true;
			}
			return false;
		}

		bool m_IsValid;
		Yaml::Node m_Document;

		Yaml::Node m_Current;

};

#if 0
class IniStream : public IFileStream
{
public:
	IniStream(const char* data, u32 size);

	~IniStream() {}

	void SetCurrentInfo(u32 index)
	{
		m_CurrentInfo = &m_Sections[index];
	}

	void SetCurrentInfo(IniSectionInfo* info)
	{
		m_CurrentInfo = info;
	}

	using IniSections = std::vector<IniSectionInfo>;
	IniSections const& GetIniSections() const { return m_Sections; }

	template <typename T>
	T GetPropertyValue(std::string_view const& propertyName, T const& defaultValue = T());

	template <>
	std::string_view GetPropertyValue(std::string_view const& propertyName, std::string_view const& defaultValue)
	{
		ASSERT(HasProperty(propertyName));
		return m_CurrentInfo->m_Attributes[propertyName];
	}

	template <>
	bool GetPropertyValue(std::string_view const& propertyName, bool const& defaultValue)
	{
		ASSERT(HasProperty(propertyName));

		std::string_view prop = GetPropertyValue<std::string_view>(propertyName);
		if (prop[0] == '0')
		{
			return false;
		}
		return true;
	}

	template <>
	int GetPropertyValue(std::string_view const& propertyName, int const& defaultValue)
	{
		ASSERT(HasProperty(propertyName));

		std::string_view prop = GetPropertyValue<std::string_view>(propertyName);
		std::string v = std::string(prop);

		return std::stoi(v.data());
	}

	template <>
	float GetPropertyValue(std::string_view const& propertyName, float const& defaultValue)
	{
		ASSERT(HasProperty(propertyName));

		std::string_view prop = GetPropertyValue<std::string_view>(propertyName);
		std::string v = std::string(prop);

		return std::stof(v.data());
	}

	bool HasProperty(std::string_view const& propertyName)
	{
		return m_CurrentInfo->m_Attributes.find(propertyName) != m_CurrentInfo->m_Attributes.end();
	}

	IniSectionInfo* FindSectionInfo(std::string_view const& sectionName)
	{
		IniSections::iterator it = std::find_if(m_Sections.begin(), m_Sections.end(), [&](IniSectionInfo const& val)
				{ return val.m_Name == sectionName; });
		if (it != m_Sections.end())
		{
			return &*it;
		}
		return nullptr;
	}

private:
	IniSectionInfo* m_CurrentInfo;

	std::vector<IniSectionInfo> m_Sections;

	const char* m_Data;
	u32 m_Size;
};
#endif
