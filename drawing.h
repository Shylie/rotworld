#ifndef DRAWING_H
#define DRAWING_H

#include <chipmunk.h>

enum class DrawType
{
	None,
	SegmentShape,
	CircleShape,
	PolyShape,
	PinJoint,
	SlideJoint,
	PivotJoint,
	GrooveJoint,
	DampedSpring,
	DampedRotarySpring,
	RotaryLimitJoint,
	RatchetJoint,
	GearJoint,
	SimpleMotor
};

void DrawSpace(cpSpace* space);

#endif//DRAWING_H
