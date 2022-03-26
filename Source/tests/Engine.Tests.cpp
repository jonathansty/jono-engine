#include "tests.pch.h"

#include "Core/Math.h"

using namespace math;

TEST_CLASS(MathGeometryTests) 
{
	TEST_METHOD(calculate_up) 
	{
		float3 normal = compute_normal({ 0, 0, 0 }, { 100, 0, 1 }, { 1, 0, 0 });

		bool is_equal = hlslpp::all(normal == hlslpp::float3{ 0, 1, 0 });
		Assert::IsTrue(is_equal);
	}

	TEST_METHOD(transform_normal)
	{
		float4 normal = normalize(float4(1, 0, 1, 0));

		float4x4 trans = float4x4::scale(float3{ 2.0f, 1.0f, 1.0f });

		float4 result = hlslpp::mul(trans, normal);
		Assert::IsTrue(false);

	
	}
};

