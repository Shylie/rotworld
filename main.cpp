#include <raylib.h>
#include <chipmunk.h>

#include <cmath>

#include "drawing.h"

cpVect* generatePolyVerts(int n, double r)
{
	constexpr double twopi = 2.0 * 3.1415926;

	cpVect* verts = new cpVect[n];
	double angle = 0.0;
	for (int i = 0; i < n; i++)
	{
		verts[i] = cpv(r * cos(angle), r * sin(angle));

		angle += twopi / static_cast<double>(n);
	}

	return verts;
}

int main()
{
	Camera2D cam;
	cam.offset = Vector2{ 320, 240 };
	cam.rotation = 0;
	cam.zoom = 1.8;

	cpSpace* space = cpSpaceNew();
	cpSpaceSetGravity(space, cpv(0, 250));

	cpShape* ground = cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(0, 0), cpv(320, 200), 0);
	cpShapeSetFriction(ground, 1);
	cpShapeSetUserData(ground, (void*)SegmentShape);
	cpSpaceAddShape(space, ground);

	cpShape* ground2 = cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(320, 240), cpv(640, 0), 0);
	cpShapeSetFriction(ground2, 1);
	cpShapeSetUserData(ground2, (void*)SegmentShape);
	cpSpaceAddShape(space, ground2);

	cpFloat mass = 1;
	cpFloat radius = 10;

	cpBody* ballBody = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0, radius, cpvzero)));
	cpBodySetPosition(ballBody, cpv(5, -5));

	cpShape* ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
	cpShapeSetFriction(ballShape, 1);
	cpShapeSetUserData(ballShape, (void*)CircleShape);

	cpFloat width = 15;
	cpFloat height = 15;

	cpBody* boxBody = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPosition(boxBody, cpv(5, -70));

	cpShape* boxShape = cpSpaceAddShape(space, cpBoxShapeNew(boxBody, width, height, 0));
	cpShapeSetFriction(boxShape, 1);
	cpShapeSetUserData(boxShape, (void*)PolyShape);

	cpConstraint* spr = cpSpaceAddConstraint(space, cpDampedSpringNew(ballBody, boxBody, cpvzero, cpvzero, 60, 20, 0));
	cpConstraintSetUserData(spr, (void*)DampedSpring);

	constexpr int numVerts = 7;
	cpVect* verts = generatePolyVerts(numVerts, 25);
	
	cpBody* polyBody = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, numVerts, verts, cpvzero, 0)));
	cpBodySetPosition(polyBody, cpv(50, -5));

	cpShape* polyShape = cpSpaceAddShape(space, cpPolyShapeNew(polyBody, numVerts, verts, cpTransformIdentity, 0));
	cpShapeSetFriction(polyShape, 1);
	cpShapeSetUserData(polyShape, (void*)PolyShape);

	delete[] verts;

	cpFloat timeStep = 1.0 / 60.0;

	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(640, 480, "rotworld");

	bool paused = true;

	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_SPACE)) { paused = false; }

		if (!paused) { cpSpaceStep(space, GetFrameTime()); }

		cpVect pos = cpBodyGetPosition(boxBody);
		cam.target = Vector2{ pos.x, pos.y };

		BeginDrawing();

		ClearBackground(BLACK);

		BeginMode2D(cam);
		DrawSpace(space);
		EndMode2D();

		EndDrawing();
	}

	cpShapeFree(polyShape);
	cpBodyFree(polyBody);
	cpConstraintFree(spr);
	cpShapeFree(boxShape);
	cpBodyFree(boxBody);
	cpShapeFree(ballShape);
	cpBodyFree(ballBody);
	cpShapeFree(ground2);
	cpShapeFree(ground);
	cpSpaceFree(space);

	CloseWindow();

	return 0;
}
