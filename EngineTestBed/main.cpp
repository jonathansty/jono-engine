#include "stdafx.h"

#include "HelloWorld.h"

int main()
{
	return RunGame(NULL, 1, new HelloWorldGame());
}
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
//{
//	return RunGame(hInstance, iCmdShow, new HelloWorldGame());
//	//return RunGame(hInstance, iCmdShow, new TestGame());
//}
