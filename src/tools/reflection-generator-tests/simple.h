#pragma once

#include "runtime/reflection.h"

#include <string>
#include <stdint.h>

struct rg_reflect() inventory_item {
	bool has_item;
	uint32_t count;
};

struct rg_reflect() character_data {
	std::string name;

	uint32_t max_health;

	uint32_t intellect;
	uint32_t strength;
	uint32_t agility;

	uint32_t inv_size;
	inventory_item* inv;
};

struct rg_reflect() enemy_data {
	std::string name;

	uint32_t type;

	uint32_t health;
};

struct rg_reflect() book_information{
	std::string name;
	int number_of_pages;

	// dasdjalsjkd
};