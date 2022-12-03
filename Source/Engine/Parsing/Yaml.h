#pragma once

#include "mini-yaml/MiniYaml.hpp"

namespace yaml
{

class Document final
{
	public:
		Document(const char* path);

		~Document();

		Yaml::Node& GetRoot()  { return m_Root; }

		bool IsValid() const { return m_IsValid; }
	private:
		char* m_Data;
		bool m_IsValid;
		Yaml::Node m_Root;
};

} // namespace yaml
