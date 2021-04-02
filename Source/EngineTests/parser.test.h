#pragma once

// #TODO: Add proper system includes to parser
//#include "GameEngine.h"
#include "parser_include.h"


struct rg_reflect() Type123 {

	int a;

	int b;

	float c;
};

class rg_reflect() IncludedType {
public:
	IncludedType()
			: _property(100.0f)
	{}

	~IncludedType() {}

	float _property;
};
