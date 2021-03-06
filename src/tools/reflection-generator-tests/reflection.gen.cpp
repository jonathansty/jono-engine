// This file is generated by reflection-generator. DO NOT CHANGE! 
#include "runtime/reflection.h"
#include "reflection.gen.h"
using namespace reflection;

void reflection::init_database() {
	// Class: "character_data"
	{
		type_info info{};
		info.name = "character_data";
		info.size = 64;
		{
			field_info field{};
			field.name = "name";
			field.offset = 0;
			field.type = &(get_database().types[Identifier64("std::string")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "max_health";
			field.offset = 32;
			field.type = &(get_database().types[Identifier64("uint32_t")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "intellect";
			field.offset = 36;
			field.type = &(get_database().types[Identifier64("uint32_t")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "strength";
			field.offset = 40;
			field.type = &(get_database().types[Identifier64("uint32_t")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "agility";
			field.offset = 44;
			field.type = &(get_database().types[Identifier64("uint32_t")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "inv_size";
			field.offset = 48;
			field.type = &(get_database().types[Identifier64("uint32_t")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "inv";
			field.offset = 56;
			field.type = &(get_database().types[Identifier64("inventory_item")]);
			info.fields.push_back(field);
		}
		get_database().types[Identifier64("character_data")] = info;
	}
	// Class: "data"
	{
		type_info info{};
		info.name = "data";
		info.size = 12;
		{
			field_info field{};
			field.name = "a";
			field.offset = 0;
			field.type = &(get_database().types[Identifier64("bool")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "c";
			field.offset = 4;
			field.type = &(get_database().types[Identifier64("int")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "d";
			field.offset = 8;
			field.type = &(get_database().types[Identifier64("float")]);
			info.fields.push_back(field);
		}
		get_database().types[Identifier64("data")] = info;
	}
	// Class: "float"
	{
		type_info info{};
		info.name = "float";
		info.size = 4;
		get_database().types[Identifier64("float")] = info;
	}
	// Class: "bool"
	{
		type_info info{};
		info.name = "bool";
		info.size = 1;
		get_database().types[Identifier64("bool")] = info;
	}
	// Class: "int"
	{
		type_info info{};
		info.name = "int";
		info.size = 4;
		get_database().types[Identifier64("int")] = info;
	}
	// Class: "std::string"
	{
		type_info info{};
		info.name = "std::string";
		info.size = 32;
		get_database().types[Identifier64("std::string")] = info;
	}
	// Class: "inventory_item"
	{
		type_info info{};
		info.name = "inventory_item";
		info.size = 8;
		{
			field_info field{};
			field.name = "has_item";
			field.offset = 0;
			field.type = &(get_database().types[Identifier64("bool")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "count";
			field.offset = 4;
			field.type = &(get_database().types[Identifier64("uint32_t")]);
			info.fields.push_back(field);
		}
		get_database().types[Identifier64("inventory_item")] = info;
	}
	// Class: "uint32_t"
	{
		type_info info{};
		info.name = "uint32_t";
		info.size = 4;
		get_database().types[Identifier64("uint32_t")] = info;
	}
	// Class: "enemy_data"
	{
		type_info info{};
		info.name = "enemy_data";
		info.size = 40;
		{
			field_info field{};
			field.name = "name";
			field.offset = 0;
			field.type = &(get_database().types[Identifier64("std::string")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "type";
			field.offset = 32;
			field.type = &(get_database().types[Identifier64("uint32_t")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "health";
			field.offset = 36;
			field.type = &(get_database().types[Identifier64("uint32_t")]);
			info.fields.push_back(field);
		}
		get_database().types[Identifier64("enemy_data")] = info;
	}
	// Class: "book_information"
	{
		type_info info{};
		info.name = "book_information";
		info.size = 40;
		{
			field_info field{};
			field.name = "name";
			field.offset = 0;
			field.type = &(get_database().types[Identifier64("std::string")]);
			info.fields.push_back(field);
		}
		{
			field_info field{};
			field.name = "number_of_pages";
			field.offset = 32;
			field.type = &(get_database().types[Identifier64("int")]);
			info.fields.push_back(field);
		}
		get_database().types[Identifier64("book_information")] = info;
	}
}
