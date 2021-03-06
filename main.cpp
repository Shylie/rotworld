#include <raylib.h>
#include <chipmunk.h>
#include <cpwrlap.h>

#include "levelgen.h"

constexpr int MAX_SAMPLES = 512;
constexpr int MAX_SAMPLES_PER_UPDATE = 4096;
constexpr int SAMPLE_RATE = 44100;

struct CallbackData12
{
	AudioStream stream;
	float timePlaying;
	float hp;
};

void BeepCallback(short* buffer, unsigned int frames);
int OnCollideBegin12(cp::Arbiter arbiter, cp::Space& space, CallbackData12* data);
int OnCollideBegin13(cp::Arbiter arbiter, cp::Space& space, bool* data);

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(640, 480, "rotworld");
	InitAudioDevice();

	CallbackData12 data;
	data.stream = LoadAudioStream(44100, 8 * sizeof(short), 1);
	data.timePlaying = 0;
	data.hp = 1000.0f;
	SetAudioStreamCallback(data.stream, (AudioCallback)&BeepCallback);

	PlayAudioStream(data.stream);
	PauseAudioStream(data.stream);

	bool won = false;

	Camera2D cam;
	cam.offset = Vector2{ 320, 240 };
	cam.zoom = 2.0f;

	cpFloat gridSize = 150;

	cp::Space* space = generateLevel(10, 10, gridSize);

	space->setCollisionBeginFunc(1, 2, (cp::Space::CollisionBeginFunc)&OnCollideBegin12);
	space->setCollisionUserData(1, 2, &data);

	space->setCollisionBeginFunc(1, 3, (cp::Space::CollisionBeginFunc)&OnCollideBegin13);
	space->setCollisionUserData(1, 3, &won);

	float angle = 0.0f;

	space->setGravity(cpvforangle(DEG2RAD * (-angle + 90.0f)) * 50);
	space->setDamping(0.5);

	while (!WindowShouldClose())
	{
		const float ft = GetFrameTime();
		if (IsKeyDown(KEY_A)) { angle -= ft * 90.0f; }
		if (IsKeyDown(KEY_D)) { angle += ft * 90.0f; }

		if (IsAudioStreamPlaying(data.stream))
		{
			if (data.timePlaying > 0) { data.timePlaying -= ft; }
			if (data.timePlaying <= 0)
			{
				data.timePlaying = 0.0f;
				PauseAudioStream(data.stream);
			}
		}

		space->setGravity(cpvforangle(DEG2RAD * (-angle + 90.0f)) * 50);
		if (data.hp > 0)
		{
			space->step(1.0 / 60.0);
		}

		if (won)
		{
			won = false;
			data.hp = 1000.0f;
			delete space;
			gridSize -= 10;
			space = generateLevel(10, 10, gridSize);
			
			space->setDamping(0.5);

			space->setCollisionBeginFunc(1, 2, (cp::Space::CollisionBeginFunc)&OnCollideBegin12);
			space->setCollisionUserData(1, 2, &data);

			space->setCollisionBeginFunc(1, 3, (cp::Space::CollisionBeginFunc)&OnCollideBegin13);
			space->setCollisionUserData(1, 3, &won);
		}

		cpVect pos = reinterpret_cast<cp::Body*>(space->getUserData())->getPosition();
		cam.target = Vector2{ pos.x, pos.y };
		cam.rotation = angle;

		BeginDrawing();

		ClearBackground(BLACK);

		BeginMode2D(cam);
		space->draw(GREEN, RED);
		EndMode2D();

		DrawRectangle(18, 18, 104, 24, Color{ 100, 15, 30, 255 });
		DrawRectangle(20, 20, static_cast<int>(data.hp / 10.0f), 20, RED);

		if (data.hp <= 0)
		{
			DrawText("You lost!", 200, 20, 20, RED);
		}

		EndDrawing();
	}

	delete space;

	CloseAudioDevice();
	CloseWindow();

	return 0;
}

void BeepCallback(short* buffer, unsigned int frames)
{
	constexpr float frequency = 440.0f;
	constexpr float increment = frequency / static_cast<float>(SAMPLE_RATE);

	static float sineIndex = 0.0f;
	for (int i = 0; i < frames; i++)
	{
		buffer[i] = (short)(32000.0f * sinf(2 * PI * sineIndex));
		sineIndex += increment;
		if (sineIndex > 1.0f) { sineIndex -= 1.0f; }
	}
}

int OnCollideBegin12(cp::Arbiter arbiter, cp::Space& space, CallbackData12* data)
{
	ResumeAudioStream(data->stream);
	data->timePlaying += 0.125f;

	cp::Body* body = static_cast<cp::Body*>(space.getUserData());
	float lost = cpvdot(body->getVelocity(), arbiter.getNormal());
	if (lost > 0)
	{
		data->hp -= lost;
		if (data->hp < 0) { data->hp = 0; }
	}

	return true;
}

int OnCollideBegin13(cp::Arbiter arbiter, cp::Space& space, bool* data)
{
	*data = true;

	return true;
}
