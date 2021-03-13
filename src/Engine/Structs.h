//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

#pragma once

// Store Box2D contactdata retrieved from userData pointers
struct ContactData
{
	void *contactListenerPtr = nullptr;
	void *actThisPtr = nullptr, *actOtherPtr = nullptr;
};

struct ImpulseData
{
	void *contactListenerAPtr = nullptr, *contactListenerBPtr = nullptr;
	double impulseA = 0, impulseB = 0;
	void *actAPtr = nullptr, *actBPtr = nullptr;
};

//-----------------------------------------------------------------
// COLOR Struct
//-----------------------------------------------------------------
//! @struct Color
struct COLOR
{
	// -------------------------
	// Constructors 
	// -------------------------

	//
	//! Default constructor fills the struct with color values 255, resulting in a white color 
	COLOR();

	//! Constructor: params are color values from 0 to 255
	//! Example: COLOR myColor(255,0,127);
	//! @param redVal red color value from 0 to 255
	//! @param greenVal red color value from 0 to 255
	//! @param blueVal red color value from 0 to 255
	//! @param alphaVal alpha color value from 0 to 255
	COLOR(unsigned char redVal, unsigned char greenVal, unsigned char blueVal, unsigned char alphaVal = 255);

	// -------------------------
	// Datamembers 
	// -------------------------	
	unsigned char red, green, blue, alpha;
};

//-----------------------------------------------------------------
// RECT2 Struct
//-----------------------------------------------------------------
struct RECT2
{
	// -------------------------
	// Constructors 
	// -------------------------	

	//! Constructor: creates a RECT2 struct with 4 double values, used to hold floating point coordinates.
	//! This default constructor sets these values to 0
	//! Example: RECT2 myRECT2();
	RECT2();

	//! Constructor: creates a RECT2 struct with 4 double values, used to hold floating point coordinates.
	//! Example: RECT2 myRECT2(1.5, 1.5, 254.2, 452.6);
	RECT2(double leftVal, double topVal, double rightVal, double bottomVal);

	// -------------------------
	// Datamembers 
	// -------------------------	
	double left, top, right, bottom;
};



namespace helpers {

// Wraps around hlslpp float4 to provide support for RTTR
struct WrapperFloat4 {
	hlslpp::float4 value;

	operator hlslpp::float4() {
		return value;
	}

private:
	RTTR_REGISTRATION_FRIEND;
	float get_x() { return value.x; }
	float get_y() { return value.y; }
	float get_z() { return value.z; }
	float get_w() { return value.w; }

	void set_x(float v) { value.x = v; }
	void set_y(float v) { value.y = v; }
	void set_z(float v) { value.z = v; }
	void set_w(float v) { value.w = v; }
};

// Wraps around hlslpp float3 to provide support for RTTR
struct WrapperFloat3 {
	hlslpp::float3 value;

	operator hlslpp::float3() {
		return value;
	}

	private:
	RTTR_REGISTRATION_FRIEND;

	public:
	float get_x() { return value.x; }
	float get_y() { return value.y; }
	float get_z() { return value.z; }

	void set_x(float v) { value.x = v; }
	void set_y(float v) { value.y = v; }
	void set_z(float v) { value.z = v; }


};

} // namespace helpers

using helpers::WrapperFloat3;
using helpers::WrapperFloat4;
