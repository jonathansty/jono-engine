#include "CppUnitTest.h"

#include <hlsl++.h>
#include <DirectXMath.h>
#include "Engine/core/Math.h"
using namespace DirectX;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Helpers to convert hlslpp types to string
namespace Microsoft {
namespace VisualStudio {
namespace CppUnitTestFramework {

template <>
std::wstring ToString(const hlslpp::float4x4 &q) {
	wchar_t buffer[256];
	swprintf_s(buffer, L"[ [%f,%f,%f,%f], [%f,%f,%f,%f], [%f,%f,%f,%f], [%f,%f,%f,%f] ]",
			float(q._m00), float(q._m01), float(q._m02), float(q._m03),
			float(q._m10), float(q._m11), float(q._m12), float(q._m13),
			float(q._m20), float(q._m21), float(q._m22), float(q._m23),
			float(q._m30), float(q._m31), float(q._m32), float(q._m33));
	return buffer;
}

}
}
}

namespace EngineTests {
	using hlslpp::float4x4;
	using hlslpp::float3x3;
	TEST_CLASS(hlslpp_helpers_tests) {


	public:
		TEST_METHOD(store_translation) {
			XMMATRIX translation = XMMatrixTranslation(0.0, 100.0, 0.0);

			hlslpp::float4x4 result;
			hlslpp_helpers::XMStoreFloat4x4(translation, result);
			float t = result._m31;
			Assert::AreEqual<float>(t, 100.0f);
		};

		TEST_METHOD(load_translation) {

			hlslpp::float4x4 tmp = hlslpp::float4x4::translation(hlslpp::float3{ 1.0f, 100.0f, 69.0f });
			XMMATRIX result = hlslpp_helpers::XMLoadFloat4x4(tmp);

			XMFLOAT4X4 mat;
			XMStoreFloat4x4(&mat, result);
			float x = mat._41;
			float y = mat._42;
			float z = mat._43;
			Assert::AreEqual<float>(x, 1.0f);
			Assert::AreEqual<float>(y, 100.0f);
			Assert::AreEqual<float>(z, 69.0f);

		}



		TEST_METHOD(store_identity) {
			XMMATRIX translation = XMMatrixIdentity();

			hlslpp::float4x4 result;
			hlslpp_helpers::XMStoreFloat4x4(translation, result);

			hlslpp::float4x4 id = hlslpp::float4x4::identity(); 
			hlslpp::float4 row = hlslpp::float4(id._11_12_13_14);

			bool t = all(id == result);
			Assert::IsTrue(t);
		};

		TEST_METHOD(store_increasing) {
			XMMATRIX ref = XMMATRIX(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f);

			hlslpp::float4x4 result;
			hlslpp_helpers::XMStoreFloat4x4(ref, result);

			bool is_equal = all(result == hlslpp::float4x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f));
			Assert::IsTrue(is_equal);
		};

		TEST_METHOD(load_increasing) {
			float4x4 ref = float4x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f);
			XMMATRIX xmmat = hlslpp_helpers::XMLoadFloat4x4(ref);

			XMFLOAT4X4 res;
			XMStoreFloat4x4(&res, xmmat);

			XMFLOAT4X4 dataref = XMFLOAT4X4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f);
			Assert::AreEqual<float>(dataref._11, ref._11);
			Assert::AreEqual<float>(dataref._33, ref._33);
		};

		TEST_METHOD(verify_hlslpp_multiplication) {
			// 
			float4x4 result4x4;
			{
				auto t = float4x4::translation({ 100.0f, 0.0f, 0.0f});
				auto mirror = float4x4::scale(-1.0f, 1.0f, 1.0f);

				result4x4 = hlslpp::mul(mirror, t);
			}

			float3x3 result3x3;
			{
				auto t = float3x3::translation({ 100.0f, 0.0f});
				auto mirror = float3x3::scale(-1.0f, 1.0f);

				result3x3 = hlslpp::mul(mirror, t);
			}

			Assert::AreEqual<float>(result3x3._31, result4x4._41);
		};




	};

}