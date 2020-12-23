#include "stdafx.h"
#include "Functions.h"
//bool Functions::IsOverlapping(const Rectf& a, const Rectf& b)
//{
//	// If one rectangle is on left side of the other
//	if (a.bottomLeft.x + a.width < b.bottomLeft.x || b.bottomLeft.x + b.width < a.bottomLeft.x)
//	{
//		return false;
//	}
//
//	// If one rectangle is under the other
//	if (a.bottomLeft.y > b.bottomLeft.y + b.height || b.bottomLeft.y > a.bottomLeft.y + a.height)
//	{
//		return false;
//	}
//
//	return true;
//}
bool Functions::IsOverlapping(const Rectf& a, const Rectf& b)
{
	//const Rectf adjustedA{ {a.bottomLeft.x + 150.f,a.bottomLeft.y + 150.f},a.width,a.height };
	//const Rectf adjustedB{ {b.bottomLeft.x + 150.f,b.bottomLeft.y + 150.f},b.width,b.height };

	//// If one rectangle is on left side of other 
	//if (adjustedA.bottomLeft.x > (adjustedB.bottomLeft.x + adjustedB.width) || adjustedB.bottomLeft.x > (adjustedA.bottomLeft.x + adjustedA.width))
	//	return false;

	//// If one rectangle is above other 
	//if ((adjustedA.bottomLeft.y + adjustedA.height) < adjustedB.bottomLeft.y || (adjustedB.bottomLeft.y + adjustedB.height) < adjustedA.bottomLeft.y)
	//	return false;

	//return true;
	// If one rectangle is on left side of other 
	if ((a.bottomLeft.x > (b.bottomLeft.x + b.width)) || (b.bottomLeft.x > (a.bottomLeft.x + a.width)))
		return false;

	// If one rectangle is above other 
	if (((a.bottomLeft.y + a.height) < b.bottomLeft.y) || ((b.bottomLeft.y + b.height) < a.bottomLeft.y))
		return false;

	return true;
}