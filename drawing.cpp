#include "userdata.h"
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
	DrawRing(Vector2{ position.x, position.y }, radius - 1, radius, 0, 360, 36, color);
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
		DrawLineEx(Vector2{ current.x, current.y }, Vector2{ next.x, next.y }, 1, color);
	}
}

static void DrawShape(cpShape* shape, Color* data)
{
	Color color = data ? *data : WHITE;
	switch (reinterpret_cast<UserData*>(cpShapeGetUserData(shape))->drawType)
	{
	case DrawType::SegmentShape:
		DrawSegmentShape(shape, color);
		break;

	case DrawType::CircleShape:
		DrawCircleShape(shape, color);
		break;

	case DrawType::PolyShape:
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

static void DrawPivotJoint(cpConstraint* constraint, Color color)
{
	// get bodies the contraint is attached to
	cpBody* a = cpConstraintGetBodyA(constraint);
	cpBody* b = cpConstraintGetBodyB(constraint);

	// get anchor coordinates
	cpVect anchorA = cpPivotJointGetAnchorA(constraint);
	cpVect anchorB = cpPivotJointGetAnchorB(constraint);

	// transform anchor coordinates from body-local space to world space
	anchorA = cpBodyLocalToWorld(a, anchorA);
	anchorB = cpBodyLocalToWorld(b, anchorB);

	// draw circles at anchors
	DrawCircleLines(anchorA.x, anchorA.y, 5, color);
	DrawCircleLines(anchorB.x, anchorB.y, 5, color);
}

static void DrawGrooveJoint(cpConstraint* constraint, Color color)
{
	// get bodies the contraint is attached to
	cpBody* a = cpConstraintGetBodyA(constraint);
	cpBody* b = cpConstraintGetBodyB(constraint);

	// get groove coordinates and anchor coordinates
	cpVect grooveA1 = cpGrooveJointGetGrooveA(constraint);
	cpVect grooveA2 = cpGrooveJointGetGrooveB(constraint);
	cpVect anchorB = cpGrooveJointGetAnchorB(constraint);

	// transform coordinates to world space
	grooveA1 = cpBodyLocalToWorld(a, grooveA1);
	grooveA2 = cpBodyLocalToWorld(a, grooveA2);
	anchorB = cpBodyLocalToWorld(b, anchorB);

	// draw line for groove
	DrawLineEx(Vector2{ grooveA1.x, grooveA1.y }, Vector2{ grooveA2.x, grooveA2.y }, 1, color);

	// draw circle for anchor
	DrawCircleLines(anchorB.x, anchorB.y, 5, color);
}

// https://github.com/slembcke/Chipmunk2D/blob/master/src/cpSpaceDebug.c#L124-L151
static void DrawDampedSprint(cpConstraint* constraint, Color color)
{
	// get bodies the contraint is attached to
	cpBody* a = cpConstraintGetBodyA(constraint);
	cpBody* b = cpConstraintGetBodyB(constraint);

	// get anchor points
	cpVect anchorA = cpDampedSpringGetAnchorA(constraint);
	cpVect anchorB = cpDampedSpringGetAnchorB(constraint);

	// transform anchor points to world space
	anchorA = cpBodyLocalToWorld(a, anchorA);
	anchorB = cpBodyLocalToWorld(b, anchorB);

	// get vector between anchorA and anchorB
	cpVect diff = anchorB - anchorA;

	// separate x and y component to form a rotation matrix out of them
	cpFloat cos = diff.x;
	cpFloat sin = diff.y;

	// get reciprocal of length of diff
	cpFloat scaling = 1.0 / cpvlength(diff);

	// construct rotation matrix
	cpVect r1 = cpv(cos, -sin * scaling);
	cpVect r2 = cpv(sin, cos * scaling);

	// declare untransformed vertices
	cpVect verts[] =
	{
		cpv(0, 0),
		cpv(0.2, 0),
		cpv(0.25, 3),
		cpv(0.3, -6),
		cpv(0.35, 6),
		cpv(0.4, -6),
		cpv(0.45, 6),
		cpv(0.475, -6),
		cpv(0.525, 6),
		cpv(0.55, -6),
		cpv(0.6, 6),
		cpv(0.65, -6),
		cpv(0.7, 6),
		cpv(0.75, -3),
		cpv(0.8, 0),
		cpv(1, 0)
	};

	constexpr int numVerts = sizeof(verts) / sizeof(*verts);

	// transform vertices
	for (int i = 0; i < numVerts; i++)
	{
		verts[i] = cpv(cpvdot(verts[i], r1) + anchorA.x, cpvdot(verts[i], r2) + anchorA.y);
	}

	// draw vertices
	for (int i = 0; i < numVerts - 1; i++)
	{
		DrawLineEx(Vector2{ verts[i].x, verts[i].y }, Vector2{ verts[i + 1].x, verts[i + 1].y }, 1, color);
	}
}

static void DrawConstraint(cpConstraint* constraint, Color* data)
{
	Color color = data ? *data : WHITE;
	switch (reinterpret_cast<UserData*>(cpConstraintGetUserData(constraint))->drawType)
	{
	case DrawType::PinJoint:
		DrawPinJoint(constraint, color);
		break;

	case DrawType::SlideJoint:
		DrawSlideJoint(constraint, color);
		break;

	case DrawType::PivotJoint:
		DrawPivotJoint(constraint, color);
		break;

	case DrawType::GrooveJoint:
		DrawGrooveJoint(constraint, color);
		break;

	case DrawType::DampedSpring:
		DrawDampedSprint(constraint, color);
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
