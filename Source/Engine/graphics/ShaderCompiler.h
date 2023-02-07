#pragma once

#include "ShaderStage.h"

namespace ShaderCompiler
{

enum class CompilerFlags : u32
{
	CompileDebug = 1 << 0,
	SkipValidation = 1 << 2,
	SkipOptimization = 1 << 3,
	OptimizationLevel0 = 1 << 14,
	OptimizationLevel1 = 0,
	OptimizationLevel2 = (1 << 14) | (1 << 15),
	OptimizationLevel3 = 1 << 15,
	WarningsAreErrors = 1 << 18,
};
ENUM_BITFLAGS(CompilerFlags);

enum class CompilerEffectFlags : u32
{
	ChildEffect = 1 << 0,
	AllowSlowOps = 1 << 1
};
ENUM_BITFLAGS(CompilerEffectFlags);

struct CompileParameters
{
	struct Define
	{
		std::string name;
		std::optional<std::string> value;

	};

	ShaderStage stage;
	CompilerFlags flags;
	CompilerEffectFlags effect_flags;

	std::string entry_point;
	std::vector<Define> defines;
};

inline bool operator==(CompileParameters::Define const& lhs, CompileParameters::Define const& rhs)
{
	return lhs.name == rhs.name && lhs.value == rhs.value;
}

inline bool operator==(CompileParameters const& lhs, CompileParameters const& rhs)
{
	return lhs.entry_point == rhs.entry_point 
		&& lhs.defines == rhs.defines 
		&& lhs.flags == rhs.flags 
		&& lhs.stage == rhs.stage 
		&& lhs.effect_flags == rhs.effect_flags;
}



bool compile(const char* shader, CompileParameters const& parameters, std::vector<u8>& out_bytecode);

} // namespace ShaderCompiler

template <>
struct std::hash<ShaderCompiler::CompileParameters>
{
	std::size_t operator()(ShaderCompiler::CompileParameters const& params) const noexcept
	{
		size_t h = 0;
		Hash::combine(h, Hash::fnv1a(params.effect_flags));
		Hash::combine(h, Hash::fnv1a(params.flags));
		Hash::combine(h, Hash::fnv1a(params.stage));
		for (ShaderCompiler::CompileParameters::Define const& define : params.defines)
		{
			Hash::combine(h, Hash::fnv1a(define.name));

			if(define.value.has_value())
			{
				Hash::combine(h, Hash::fnv1a(define.value.value()));
			}
		}
		Hash::combine(h, Hash::fnv1a(params.entry_point));
		return h;
	}
};
