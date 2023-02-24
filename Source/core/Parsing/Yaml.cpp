#include "core.pch.h"
#include "Yaml.h"

#include "GlobalContext.h"	
#include "PlatformIO.h"
#include "Logging.h"

#include "mini-yaml/MiniYaml.hpp"

namespace yaml
{

struct TextReadStream
{
	const char* m_Data;
	u32 m_Size;
	u32 m_Idx;

	bool ReadLine(std::string_view& outView)
	{
		if(m_Idx >= m_Size)
		{
			return false;
		}

		if (m_Data[m_Idx] == '\n')
		{
			return false;
		}


		u32 start = m_Idx;
		u32 end = start;
		while (m_Data[end] != '\n')
		{
			++end;
		}

		// Update idx
		m_Idx = end + 1;

		outView = std::string_view(m_Data + start, m_Data + end);
		return true;
	}

};

Document::Document(const char* path)
: m_Data(nullptr)
{
	IO::IPlatformIO* io = GetGlobalContext()->m_PlatformIO;

	IO::IFileRef file = io->OpenFile(path, IO::Mode::Read, false);
	if(file)
	{
		u64 size = file->GetSize();

		m_Data = new char[size + 1];
		memset(m_Data, 0, size + 1);
		u32 readBytes = file->read(m_Data, (u32)size);

		try
		{
			Yaml::Parse(m_Root, m_Data, readBytes);
			m_IsValid = true;
		}
		catch(Yaml::ParsingException e)
		{
			LOG_ERROR(IO, "Failed to parse yaml: {}", e.what());
			m_IsValid = false;
		}

	}

}

Document::~Document()
{
	if (m_Data)
	{
		m_Data = nullptr;
	}
}

}
