#include "engine.pch.h"
#include "IniStream.h"
#include "CommandLine.h"

IniStream::IniStream(const char* data, u32 size)
		: m_Data(data)
		, m_Size(size)
{
	u32 marker = 0;

	std::vector<IniSectionInfo>& sections = m_Sections;
	u32 sectionStart = 0;
	u32 sectionEnd = 0;
	bool sectionRead = false;
	while (marker < size)
	{
		// Read the next line
		u32 lineMarker = marker;

		u32 lineStart = lineMarker;
		// Walk until next new line
		while (lineMarker < size)
		{
			if (data[lineMarker] == '\n')
			{
				break;
			}
			++lineMarker;
		}
		std::string_view l = std::string_view((const char*)(data + lineStart), lineMarker - lineStart);

		// Process the line

		if (l.empty())
		{
		}
		else if (*l.begin() == '[' && l.back() == ']')
		{
			sectionEnd = marker;
			if (sectionRead)
			{
				IniSectionInfo& lastSection = sections.back();
				lastSection.m_Data = std::string_view(data + sectionStart, sectionEnd - sectionStart);
			}

			std::string_view lhs, rhs;
			std::string_view v = std::string_view(l).substr(1, l.size() - 2);
			Helpers::split_string(v, ",", lhs, rhs);
			sections.emplace_back(
					IniSectionInfo{
							.m_Name = lhs,
							.m_Type = rhs });

			sectionStart = lineMarker + 1;
			sectionRead = true;
		}

		// Update marker, +1 for including new line character
		marker = lineMarker + 1;
	}

	if (sectionRead)
	{
		IniSectionInfo& lastSection = sections.back();
		lastSection.m_Data = std::string_view(data + sectionStart, size - sectionStart);
	}

	for (IniSectionInfo& section : m_Sections)
	{
		std::vector<std::string_view> lines;
		Helpers::split_lines(section.m_Data, lines);

		for (std::string_view const& l : lines)
		{
			std::string_view attribute;
			std::string_view value;
			Helpers::split_string(l, "=", attribute, value);

			if (!value.empty() && value[0] == '\"')
			{
				value = std::string_view(value.data() + 1, value.size() - 1);
			}

			if (!value.empty() && value.back() == '\"')
			{
				value = std::string_view(value.data(), value.size() - 1);
			}

			section.m_Attributes[attribute] = value;
		}
	}
}