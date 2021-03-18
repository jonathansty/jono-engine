#include "engine.pch.h"
#include "SVGParser.h"

#include "EString.h"
#include "GameEngine.h"

// This ifdef block is here because in the 1st year students still need to learn what it means/put it in themselves. In the 2nd year engine it is defined in GameDefines.h
#ifdef _UNICODE
#define tifstream wifstream
#else
#define tifstream ifstream
#endif


using std::search;
using std::tifstream;

SVGParser::SVGParser(const String& svgFilePathRef, std::vector<std::vector<float2>> &verticesArrRef)
{
	LoadGeometryFromSvgFile(svgFilePathRef, verticesArrRef);
}

SVGParser::~SVGParser()
{}

bool SVGParser::LoadGeometryFromSvgFile(const String& svgFilePathRef, std::vector<std::vector<float2>> &verticesArrRef)
{
	// open the file, close if not found
	tifstream svgFile(svgFilePathRef.C_str());

	if (!svgFile)
	{
		GameEngine::instance()->message_box(String("Failed to load geometry from file ") + svgFilePathRef);
		return false;
	}
	else
	{
		// read the file
		tstring sLine;
		tstring svgText;
		while (!svgFile.eof())
		{
			getline(svgFile, sLine);
			svgText += sLine;
		}

		// close the file
		svgFile.close();

		// extract the transform information, if present
		tstring sTransformInfo;
		bool result = ExtractTransformInformation(svgText, sTransformInfo);

		// extract the geometry information, exit if not found
		tstring sGeometryInfo;
		result = ExtractPathInformation(svgText, sGeometryInfo);
		if (result == false)
		{
			GameEngine::instance()->message_box(String("Malformed geometry information in file ") + svgFilePathRef);
			return false;
		}

		// process the geometry information
		return ReadSvgFromData(sTransformInfo, sGeometryInfo, verticesArrRef);
	}
}

bool SVGParser::ExtractTransformInformation(tstring& svgTextRef, tstring& sTransformInfoRef)
{
	tstring pattern = _T("transform=");
	tstring::iterator posTransform = search(svgTextRef.begin(), svgTextRef.end(), pattern.begin(), pattern.end());
	if (posTransform == svgTextRef.end()) // d attribute not found, maybe with a space?
	{
		pattern = _T("transform =");
		posTransform = search(svgTextRef.begin(), svgTextRef.end(), pattern.begin(), pattern.end());
	}
	if (posTransform == svgTextRef.end()) // try in caps?
	{
		pattern = _T("TRANSFORM=");
		posTransform = search(svgTextRef.begin(), svgTextRef.end(), pattern.begin(), pattern.end());
	}
	if (posTransform == svgTextRef.end()) // last try
	{
		pattern = _T("TRANSFORM =");
		posTransform = search(svgTextRef.begin(), svgTextRef.end(), pattern.begin(), pattern.end());
	}
	if (posTransform == svgTextRef.end()) // give up
	{
		return false;
	}

	tstring::iterator posOpeningDoubleQuote = find(posTransform, svgTextRef.end(), _T('\"'));
	tstring::iterator posClosingDoubleQuote = find(posOpeningDoubleQuote + 1, svgTextRef.end(), _T('\"'));

	sTransformInfoRef = tstring(posOpeningDoubleQuote + 1, posClosingDoubleQuote);

	return true;
}

bool SVGParser::ExtractPathInformation(tstring& svgTextRef, tstring& sGeometryInfoRef)
{
	// extract the path information part
	tstring pattern(_T("<path"));
	tstring::iterator posPath = search(svgTextRef.begin(), svgTextRef.end(), pattern.begin(), pattern.end());
	pattern = _T(" d=");
	tstring::iterator posD = search(posPath, svgTextRef.end(), pattern.begin(), pattern.end());
	if (posD == svgTextRef.end()) // d attribute not found, maybe with a space?
	{
		pattern = _T(" d =");
		posD = search(posPath, svgTextRef.end(), pattern.begin(), pattern.end());
	}
	if (posD == svgTextRef.end()) // try in caps?
	{
		pattern = _T(" D=");
		posD = search(posPath, svgTextRef.end(), pattern.begin(), pattern.end());
	}
	if (posD == svgTextRef.end()) // last try
	{
		pattern = _T(" D =");
		posD = search(posPath, svgTextRef.end(), pattern.begin(), pattern.end());
	}
	if (posD == svgTextRef.end()) // give up
	{
		return false;
	}

	tstring::iterator posOpeningDoubleQuote = find(posD, svgTextRef.end(), _T('\"'));
	tstring::iterator posClosingDoubleQuote = find(posOpeningDoubleQuote + 1, svgTextRef.end(), _T('\"'));

	sGeometryInfoRef = tstring(posOpeningDoubleQuote + 1, posClosingDoubleQuote);

	return true;
}

// Points are transformed by groups into the same space.
bool SVGParser::ReadSvgFromData(tstring& sTransformInfoRef, tstring& sPathInfoRef, std::vector<std::vector<float2>> &verticesArrRef)
{
	std::vector<ID2D1Geometry*> outGeometryArr;

	// parse the geometry information
	//ID2D1PathGeometry* pathGeometryPtr;
	bool result = ReadSvgPath(sPathInfoRef, verticesArrRef);
	if (!result)
	{
		GameEngine::instance()->message_box(String("SVG path read failed"));
		return false;
	}
	return true;
}

bool SVGParser::GetGeometryOrGroup(ID2D1Factory* factoryPtr, std::vector<ID2D1Geometry*>& childrenRef, ID2D1Geometry*& geometryPtrRef)
{
	if (!childrenRef.empty())
	{
		if (childrenRef.size() == 1)
		{
			geometryPtrRef = childrenRef[0];
		}
		else
		{
			ID2D1GeometryGroup* geometryGroupPtr;
			HRESULT hr = factoryPtr->CreateGeometryGroup(D2D1_FILL_MODE_WINDING, &childrenRef[0], UINT32(childrenRef.size()), &geometryGroupPtr);
			if (FAILED(hr))
			{
				GameEngine::instance()->message_box(String("CreateTransformedGeometry failed"));
				return false;
			}

			geometryPtrRef = geometryGroupPtr;
		}
	}

	return true;
}

float3x3 SVGParser::ReadTransform(tstring& sTransformInfoRef)
{
	int argCount;
	float3x3 matrix = float3x3::identity();

	float2 dirX = {};
	float2 dirY = {};
	float2 orig = {};

	if (sTransformInfoRef != _T(""))
	{
		switch (sTransformInfoRef.c_str()[0])
		{
		case _T('m'):
			_stscanf_s(sTransformInfoRef.c_str(), _T("matrix(%f,%f,%f,%f,%f,%f)"), (float*)&dirX.x, (float*)&dirX.y, (float*)&dirY.x, (float*)&dirY.y, (float*)&orig.x, (float*)&orig.y);
			matrix = float3x3(float3(dirX, 0.0), float3(dirY, 0.0), float3(orig, 1.0));
			break;

		case _T('t'):
			argCount = _stscanf_s(sTransformInfoRef.c_str(), _T("translate(%f,%f)"), (float*)&orig.x, (float*)&orig.y);
			matrix = float3x3(float3(1.0f,0.0f, 0.0), float3(0.0, 1.0f, 0.0), float3(orig, 1.0));
			break;

		case _T('s'):
			argCount = _stscanf_s(sTransformInfoRef.c_str(), _T("scale(%f,%f)"), (float*)&dirX.x, (float*)&dirY.y);
			if (argCount == 1)
			{
				dirY.y = dirX.x;
			}
			matrix = float3x3(float3(dirX, 0.0f), float3(dirY, 0.0), float3(orig, 1.0));
			break;

		case _T('r'):
		{
			float angle;
			float2 center;
			argCount = _stscanf_s(sTransformInfoRef.c_str(), _T("rotate(%f,%f,%f)"), (float*)&angle, (float*)&center.x, (float*)&center.y);

			switch (argCount)
			{
			case 1:
				matrix = float3x3::rotation_z(angle * float(M_PI) / 180.0f);
				break;
			case 3:
				matrix = float3x3::translation(-center) * float3x3::rotation_z(angle * float(M_PI) / 180.0f) * float3x3::translation(center);
				break;
			}
		}
			break;

		default:
			GameEngine::instance()->message_box(String("unsupported transform attribute"));
			break;
		}
	}

	return matrix;
}

bool SVGParser::ReadSvgPath(tstring& sPathInfoRef, std::vector<std::vector<float2>> &verticesArrRef)
{
	// Use streamstream for parsing
	tstringstream ss(sPathInfoRef);

	TCHAR cmd = 0;
	tstring svgCmds(_T("mMZzLlHhVvCcSsQqTtAa"));
	float2 cursor;
	float2 abscur;
	float2 startPoint;//At the end of the z command, the new current point is set to the initial point of the current subpath.

	bool isOpen = true;

	// http://www.w3.org/TR/SVG/paths.html#Introduction

	std::vector<float2> currentChain;
	float2 vertex;
	while (true)
	{
		TCHAR c;
		ss >> c;

		if (ss.eof())
			break;

		if (_tcschr(svgCmds.c_str(), c) != 0)
		{
			cmd = c;
		}
		else
		{
			ss.putback(c);
		}

		switch (cmd)
		{
		case _T('Z'):
		case _T('z'):
			// Close the current subpath by drawing a straight line from the current point to current subpath's initial point. Since the Z and z commands take no parameters, they have an identical effect.
			verticesArrRef.push_back(currentChain);
			currentChain.clear();
			cursor = startPoint;//relative path: new point is set to the initial statpoint of the current path
			isOpen = true;
			break;

		case _T('M'):
		case _T('m'):
			if (isOpen)
			{
				cursor = FirstSvgPoint(ss, cursor, cmd, isOpen, true);
				startPoint = cursor;//record startpoint, cursor is set back to startPoint when path closes
				//cbs.point2 = cursor.ToPoint2F();
				//geometrySinkPtr->BeginFigure(cbs.point2, D2D1_FIGURE_BEGIN_FILLED);
				currentChain.push_back(cursor);
				isOpen = false;
				break;
			}
			// Fallthrough when isOpen
		case _T('L'):
		case _T('l'):
			vertex = NextSvgPoint(ss, cursor, cmd, isOpen, true);
			currentChain.push_back(vertex);
			//geometrySinkPtr->AddLine(cbs.point2);
			break;

		case _T('h'):
		case _T('H'):
			vertex = NextSvgCoordX(ss, cursor, cmd, isOpen);
			currentChain.push_back(vertex);
			//geometrySinkPtr->AddLine(cbs.point2);
			break;

		case _T('v'):
		case _T('V'):
			vertex = NextSvgCoordY(ss, cursor, cmd, isOpen);
			currentChain.push_back(vertex);
			//geometrySinkPtr->AddLine(cbs.point2);
			break;

		case _T('C'):
		case _T('c'):
			GameEngine::instance()->message_box(String(cmd) + String(" Beziers are not supported. Have another look at the guide, or select all nodes in inkscape and press shift + L"));
			break;

			//case _T('S'):
			//case _T('s'):
			//	abscur = cursor;
			//	cbs.point1.x = (FLOAT)(abscur.x + (abscur.x - cbs.point2.x));
			//	cbs.point1.y = (FLOAT)(abscur.y + (abscur.y - cbs.point2.y));
			//	cbs.point2 = NextSvgPoint(ss, cursor, cmd, isOpen, false).ToPoint2F();
			//	cbs.point3 = NextSvgPoint(ss, cursor, cmd, isOpen, true).ToPoint2F();
			//	geometrySinkPtr->AddBezier(cbs);
			//	break;

			//case _T('Q'):
			//case _T('q'):
			//	qbs.point1 = NextSvgPoint(ss, cursor, cmd, isOpen, false).ToPoint2F();
			//	qbs.point2 = NextSvgPoint(ss, cursor, cmd, isOpen, true).ToPoint2F();
			//	geometrySinkPtr->AddQuadraticBezier(qbs);
			//	break;

			//case _T('T'):
			//case _T('t'):
			//	abscur = cursor;
			//	qbs.point1.x = (FLOAT)((abscur.x + (abscur.x - qbs.point2.x)));
			//	qbs.point1.y = (FLOAT)((abscur.y + (abscur.y - qbs.point2.y)));
			//	qbs.point2 = NextSvgPoint(ss, cursor, cmd, isOpen, true).ToPoint2F();
			//	geometrySinkPtr->AddQuadraticBezier(qbs);
			//	break;

			//case _T('A'):
			//case _T('a'):
			//	// (rx ry x-axis-rotation large-arc-flag sweep-flag x y)+	
			//	// Draws an elliptical arc from the current point to (x, y). 
			//	// The size and orientation of the ellipse are defined by two radii (rx, ry) and an x-axis-rotation, 
			//	// which indicates how the ellipse as a whole is rotated relative to the current coordinate system. 
			//	// The center (cx, cy) of the ellipse is calculated automatically to satisfy the constraints imposed by the other parameters. 
			//	// large-arc-flag and sweep-flag contribute to the automatic calculations and help determine how the arc is drawn.
			//{
			//	OutputDebugString(String("WARNING: SVG Arcs not supported yet\n"));
			//	D2D1_ARC_SEGMENT arc;
			//	arc.size.width = (float)ReadSvgValue(ss, true);
			//	arc.size.height = (float)ReadSvgValue(ss, true);
			//	arc.rotationAngle = (float)ReadSvgValue(ss, true);
			//	arc.arcSize = (D2D1_ARC_SIZE)(int)ReadSvgValue(ss, true);
			//	arc.sweepDirection = (D2D1_SWEEP_DIRECTION)(int)ReadSvgValue(ss, true);
			//	cbs.point2 = arc.point = NextSvgPoint(ss, cursor, cmd, isOpen, true).ToPoint2F();
			//	geometrySinkPtr->AddArc(arc);
			//}
			//	break;
		default:
			GameEngine::instance()->message_box(String(cmd) + String(" is not a supported SVG command"));
			//return false;
		}
	}

	if (!isOpen)
	{
		verticesArrRef.push_back(currentChain);
	}

	return true;
}

// Replace all whitespace by space.
TCHAR SVGParser::WhiteSpaceMapper(TCHAR c)
{
	switch (c)
	{
	case _T('\n'):
	case _T('\r'):
	case _T('\t'):
		return _T(' ');

	default:
		return c;
	}
}

// Skips any optional commas in the stream
// SVG has a really funky format,
// not sure this code works for all cases.
// TODO: Test cases!
void SVGParser::SkipSvgComma(tstringstream& ssRef, bool isRequired)
{
	while (true)
	{
		TCHAR c = ssRef.get();

		if (ssRef.eof())
		{
			if (isRequired) GameEngine::instance()->message_box(String("expected comma or whitespace"));
			break;
		}

		if (c == _T(','))
			return;

		if (!isspace(c))
		{
			ssRef.unget();
			return;
		}
	}
}

double SVGParser::ReadSvgValue(tstringstream& ssRef, double defaultValue)
{
	double s;
	ssRef >> s;

	if (ssRef.eof())
	{
		s = defaultValue;
	}
	else
	{
		SkipSvgComma(ssRef, false);
	}

	return s;
}

double SVGParser::ReadSvgValue(tstringstream& ssRef, bool separatorRequired)
{
	double s;
	ssRef >> s;
	SkipSvgComma(ssRef, separatorRequired);
	return s;
}

// Reads a single point
float2 SVGParser::ReadSvgPoint(tstringstream& ssRef)
{
	float2 p;
	p.x = (float)ReadSvgValue(ssRef, true);
	p.y = (float)ReadSvgValue(ssRef, false);
	return p;
}

float2 SVGParser::FirstSvgPoint(tstringstream& ssRef, float2& cursor, TCHAR cmd, bool isOpen, bool advance)
{
	if (!isOpen) GameEngine::instance()->message_box(String("expected 'Z' or 'z' command"));

	float2 p = ReadSvgPoint(ssRef);

	if (islower(cmd))
	{
		// Relative point
		p = cursor + p;
	}

	if (advance)
	{
		cursor = p;
	}

	return p;
}
// Read the next point, 
// taking into account relative and absolute positioning.
// Advances the cursor if requested.
// Throws an exception if the figure is not open
float2 SVGParser::NextSvgPoint(tstringstream& ssRef, float2& cursor, TCHAR cmd, bool isOpen, bool advance)
{
	if (isOpen) GameEngine::instance()->message_box(String("expected 'M' or 'm' command"));

	float2 p = ReadSvgPoint(ssRef);

	if (islower(cmd))
	{
		// Relative point
		p = cursor + (p - float2());
	}

	if (advance)
	{
		cursor = p;
	}

	return p;
}

// Reads next point, given only the new x coordinate 
float2 SVGParser::NextSvgCoordX(tstringstream& ssRef, float2& cursor, TCHAR cmd, bool isOpen)
{
	if (isOpen)
		GameEngine::instance()->message_box(String("expected 'M' or 'm' command"));

	double c;
	ssRef >> c;

	if (islower(cmd))
	{
		// Relative point
		cursor += float2(c, 0);
	}
	else
	{
		cursor.x = (float)c;
	}

	return cursor;
}

// Reads next point, given only the new y coordinate 
float2 SVGParser::NextSvgCoordY(tstringstream& ssRef, float2& cursor, TCHAR cmd, bool isOpen)
{
	if (isOpen)
		GameEngine::instance()->message_box(String("expected 'M' or 'm' command"));

	double c;
	ssRef >> c;

	if (islower(cmd))
	{
		// Relative point
		cursor += float2(0, c);
	}
	else
	{
		cursor.y = (float)c;
	}

	return cursor;
}
