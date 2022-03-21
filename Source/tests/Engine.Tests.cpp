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
};

