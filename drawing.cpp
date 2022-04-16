#include "drawing.h"
#include <raylib.h>

static void DrawSegmentShape(cpShape* shape, Color color)
{
	// get the body the shape is attached to
	cpBody* body = cpShapeGetBody(shape);
	// get the segment endpoints and translate to world coordinates
	cpVect a = cpBodyLocalToWorld(body, cpSegmentShapeGetA(shape));
	cpVect b = cpBodyLocalToWorld(body, cpSegmentShapeGetB(shape));
	// get the segment radius
	cpFloat thick = cpSegmentShapeGetRadius(shape);
	// draw the line
	DrawLineEx(Vector2{ a.x, a.y }, Vector2{ b.x, b.y }, thick + 1, color);
}

static void DrawCircleShape(cpShape* shape, Color color)
{
	// get the offset of the circle and translate it to world coordinates
	cpVect position = cpBodyLocalToWorld(cpShapeGetBody(shape), cpCircleShapeGetOffset(shape));
	cpFloat radius = cpCircleShapeGetRadius(shape);
	// draw the circle
	DrawCircleLines(position.x, position.y, radius, color);
}

static void DrawPolyShape(cpShape* shape, Color color)
{
	// get the body the shape is attached to
	cpBody* body = cpShapeGetBody(shape);

	for (int i = 0; i < cpPolyShapeGetCount(shape); i++)
	{
		// get the vertex coordinates in body-local coordinate space
		cpVect current = cpPolyShapeGetVert(shape, i);
		cpVect next = cpPolyShapeGetVert(shape, (i + 1) % cpPolyShapeGetCount(shape));

		// translate to world coordinates
		current = cpBodyLocalToWorld(body, current);
		next = cpBodyLocalToWorld(body, next);

		// draw a line between the two vertices
		DrawLine(current.x, current.y, next.x, next.y, color);
	}
}

static void DrawShape(cpShape* shape, Color* data)
{
	Color color = data ? *data : WHITE;
	switch (reinterpret_cast<unsigned long long>(cpShapeGetUserData(shape)))
	{
	case SegmentShape:
		DrawSegmentShape(shape, color);
		break;

	case CircleShape:
		DrawCircleShape(shape, color);
		break;

	case PolyShape:
		DrawPolyShape(shape, color);
		break;
	}
}

static void DrawPinJoint(cpConstraint* constraint, Color color)
{
	// get bodies the contraint is attached to
	cpBody* a = cpConstraintGetBodyA(constraint);
	cpBody* b = cpConstraintGetBodyB(constraint);

	// get anchor coordinates
	cpVect anchorA = cpPinJointGetAnchorA(constraint);
	cpVect anchorB = cpPinJointGetAnchorB(constraint);

	// translate to world coordinates
	anchorA = cpBodyLocalToWorld(a, anchorA);
	anchorB = cpBodyLocalToWorld(b, anchorB);

	// draw a line between the two anchor points
	DrawLineEx(Vector2{ anchorA.x, anchorA.y }, Vector2{ anchorB.x, anchorB.y }, 1, color);
}

static void DrawSlideJoint(cpConstraint* constraint, Color color)
{
	// get bodies the contraint is attached to
	cpBody* a = cpConstraintGetBodyA(constraint);
	cpBody* b = cpConstraintGetBodyB(constraint);

	// get anchor coordinates
	cpVect anchorA = cpSlideJointGetAnchorA(constraint);
	cpVect anchorB = cpSlideJointGetAnchorB(constraint);

	// translate to world coordinates
	anchorA = cpBodyLocalToWorld(a, anchorA);
	anchorB = cpBodyLocalToWorld(b, anchorB);

	cpVect diff = anchorB - anchorA;
	// current distance between the two anchor points
	cpFloat dist = cpvlength(diff);
	// min distance between the two anchor points
	cpFloat min = cpSlideJointGetMin(constraint);
	// distance at which to draw the larger box that
	// represents the sliding portion of the joint
	cpFloat largeBoxDist = (dist - min) / 2.0;
	// normalize diff for length calculations later on
	diff = cpvnormalize(diff);
	// get a vector perpendicular to diff 
	cpVect diffp = cpvperp(diff);

	// calculate the midpoints of the two sides of the box perpendicular to the diff line
	cpVect largeBoxAMid = anchorA + diff * largeBoxDist;
	cpVect largeBoxBMid = anchorA + diff * (largeBoxDist + min);

	// calculate the corners of the box
	cpVect largeBoxA1 = largeBoxAMid + diffp * 2.5;
	cpVect largeBoxA2 = largeBoxAMid - diffp * 2.5;
	cpVect largeBoxB1 = largeBoxBMid + diffp * 2.5;
	cpVect largeBoxB2 = largeBoxBMid - diffp * 2.5;

	// draw the lines on either side of the box
	DrawLineEx(Vector2{ anchorA.x, anchorA.y }, Vector2{ largeBoxAMid.x, largeBoxAMid.y }, 1, color);
	DrawLineEx(Vector2{ anchorB.x, anchorB.y }, Vector2{ largeBoxBMid.x, largeBoxBMid.y }, 1, color);

	// draw the box
	DrawLineEx(Vector2{ largeBoxA1.x, largeBoxA1.y }, Vector2{ largeBoxA2.x, largeBoxA2.y }, 1, color);
	DrawLineEx(Vector2{ largeBoxA2.x, largeBoxA2.y }, Vector2{ largeBoxB2.x, largeBoxB2.y }, 1, color);
	DrawLineEx(Vector2{ largeBoxB1.x, largeBoxB1.y }, Vector2{ largeBoxB2.x, largeBoxB2.y }, 1, color);
	DrawLineEx(Vector2{ largeBoxB1.x, largeBoxB1.y }, Vector2{ largeBoxA1.x, largeBoxA1.y }, 1, color);
}

static void DrawConstraint(cpConstraint* constraint, Color* data)
{
	Color color = data ? *data : WHITE;
	switch (reinterpret_cast<unsigned long long>(cpConstraintGetUserData(constraint)))
	{
	case PinJoint:
		DrawPinJoint(constraint, color);
		break;

	case SlideJoint:
		DrawSlideJoint(constraint, color);
		break;
	}
}

void DrawSpace(cpSpace* space)
{
	Color c = RED;
	cpSpaceEachConstraint(space, (cpSpaceConstraintIteratorFunc)&DrawConstraint, &c);
	c = GREEN;
	cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)&DrawShape, &c);
}
