#include <raylib.h>
#include <chipmunk.h>
#include <cpwrlap.h>

#include "levelgen.h"

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(640, 480, "rotworld");

	Camera2D cam;
	cam.offset = Vector2{ 320, 240 };
	cam.zoom = 2.0f;

	cp::Space* space = generateLevel(10, 10, 125);

	float angle = 0.0f;

	space->setGravity(cpvforangle(DEG2RAD * (-angle + 90.0f)) * 25);

	while (!WindowShouldClose())
	{
		const float ft = GetFrameTime();
		if (IsKeyDown(KEY_A)) { angle -= ft * 90.0f; }
		if (IsKeyDown(KEY_D)) { angle += ft * 90.0f; }

		space->setGravity(cpvforangle(DEG2RAD * (-angle + 90.0f)) * 25);
		space->step(1.0 / 60.0);

		cpVect pos = reinterpret_cast<cp::Body*>(space->getUserData())->getPosition();
		cam.target = Vector2{ pos.x, pos.y };
		cam.rotation = angle;

		BeginDrawing();

		ClearBackground(BLACK);

		BeginMode2D(cam);
		space->draw(GREEN, RED);
		EndMode2D();

		EndDrawing();
	}

	delete space;

	CloseWindow();

	return 0;
}
