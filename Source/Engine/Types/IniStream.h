#pragma once

struct IniSectionInfo
{
	std::string_view m_Name;
	std::string_view m_Type;
	std::string_view m_Data;

	std::unordered_map<std::string_view, std::string_view> m_Attributes;
};

class IniStream
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
