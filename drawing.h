#include <chipmunk.h>

enum DrawType : unsigned long long
{
	SegmentShape = 1,
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
