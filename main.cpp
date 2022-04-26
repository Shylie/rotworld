#include <raylib.h>
#include <chipmunk.h>
#include <cpwrlap.h>

#include "levelgen.h"

constexpr int MAX_SAMPLES = 512;
constexpr int MAX_SAMPLES_PER_UPDATE = 4096;
constexpr int SAMPLE_RATE = 44100;

struct AudioData
{
	AudioStream stream;
	float timePlaying;
};

void BeepCallback(short* buffer, unsigned int frames);
int OnCollideBegin(cp::Arbiter arbiter, cp::Space& space, AudioData* data);

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(640, 480, "rotworld");
	InitAudioDevice();

	AudioData data;
	data.stream = LoadAudioStream(44100, 8 * sizeof(short), 1);
	data.timePlaying = 0;
	SetAudioStreamCallback(data.stream, (AudioCallback)&BeepCallback);

	PlayAudioStream(data.stream);
	PauseAudioStream(data.stream);

	Camera2D cam;
	cam.offset = Vector2{ 320, 240 };
	cam.zoom = 2.0f;

	cp::Space* space = generateLevel(10, 10, 125);

	space->setDefaultCollisionBeginFunc((cp::Space::CollisionBeginFunc)&OnCollideBegin);
	space->setDefaultCollisionUserData(&data);

	float angle = 0.0f;

	space->setGravity(cpvforangle(DEG2RAD * (-angle + 90.0f)) * 25);
	space->setDamping(0.8);

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

int OnCollideBegin(cp::Arbiter arbiter, cp::Space& space, AudioData* data)
{
	ResumeAudioStream(data->stream);
	data->timePlaying += 0.125f;
	return true;
}
