#include "levelgen.h"

#include <cstdlib>

class Maze
{
private:
	int width;
	int height;
	int* sets;
	int nextSet;
	bool* connections;

	static bool randomChance(float chance)
	{
		float roll = (rand() / static_cast<float>(RAND_MAX));
		TraceLog(LOG_INFO, "c: %f | rv: %s", roll, (roll < chance) ? "true" : "false");
		return roll < chance;
	}

	static int absolute(int x) { return x < 0 ? -x : x; }

	int numHorizontalConnections() const { return width * (height - 1); }
	int numVerticalConnections() const { return height * (width - 1); }
	int numConnections() const { return numHorizontalConnections() + numVerticalConnections(); }

	int connectionIndex(int a, int b) const
	{
		const int ax = a % width;
		const int ay = a / width;
		const int bx = b % width;
		const int by = b / width;

		const int distanceX = absolute(bx - ax);
		const int distanceY = absolute(by - ay);

		if (distanceX == 0 && distanceY == 1)
		{
			// horizontal connection
			if (by - ay > 0)
			{
				// a is above b
				return ax + ay * width;
			}
			else
			{
				// a is below b
				return bx + by * width;
			}
		}
		else if (distanceX == 1 && distanceY == 0)
		{
			// vertical connection
			if (bx - ax > 0)
			{
				// a is left of b
				return numHorizontalConnections() + ay + ax * height;
			}
			else
			{
				// a is right of b
				return numHorizontalConnections() + by + bx * height;
			}
		}
		else
		{
			// invalid connection
			return -1;
		}
	}

	void removeConnection(int a, int b)
	{
		const int ci = connectionIndex(a, b);
		if (ci >= 0) { connections[ci] = false; }
	}

	void assignRow(int row, bool last)
	{
		for (int i = row * width; i < (row + 1) * width; i++)
		{
			if (sets[i] == 0) { sets[i] = nextSet++; }
		}

		for (int i = row * width + 1; i < (row + 1) * width; i++)
		{
			// doesn't properly check if sets[i]
			// is connected to sets[i - 1] before
			// connecting them. but, good enough for now
			if (sets[i] != sets[i - 1] && randomChance(last ? 1.0f : 0.5f))
			{
				sets[i] = sets[i - 1];
				removeConnection(i, i - 1);
			}
		}
	}

	void mergeRows(int row)
	{
		int currentSet = sets[row * width];
		bool placed = false;
		for (int i = row * width; i < (row + 1) * width; i++)
		{
			// when the set changes
			if (sets[i] != currentSet)
			{
				// if a connection has not been placed
				// for the previous set yet, guarantee it
				if (!placed)
				{
					sets[i - 1 + width] = currentSet;
					removeConnection(i - 1, i - 1 + width);
				}

				placed = false;
				currentSet = sets[i];
			}
			else if (randomChance(0.5f))
			{
				// randomly place connections
				sets[i + width] = currentSet;
				removeConnection(i, i + width);
				placed = true;
			}
		}
		if (!placed)
		{
			// guarantee a connection for the last set
			// as the loop doesn't cover it
			sets[(row + 2) * width - 1] = currentSet;
			removeConnection((row + 1) * width - 1, (row + 2) * width - 1);
		}
	}

public:
	Maze(int width, int height) : width(width), height(height), sets(new int[width * height]), nextSet(1), connections(new bool[numConnections()])
	{
		for (int i = 0; i < width * height; i++) { sets[i] = 0; }
		for (int i = 0; i < numConnections(); i++) { connections[i] = true; }
		for (int i = 0; i < height - 1; i++)
		{
			assignRow(i, false);
			mergeRows(i);
		}
		assignRow(height - 1, true);
	}
	~Maze()
	{
		delete[] sets;
		delete[] connections;
	}

	bool* operator()(int a, int b)
	{
		const int ci = connectionIndex(a, b);
		if (ci >= 0)
		{
			return &connections[ci];
		}
		else
		{
			return nullptr;
		}
	}
};

cp::Space* generateLevel(int gridWidth, int gridHeight, cpFloat gridSize)
{
	Maze m(gridWidth, gridHeight);

	cp::Space* space = new cp::Space();

	cp::Body* staticBody = space->getStaticBody();

	// top border segment
	space->add(new cp::SegmentShape(staticBody, cpv(-gridSize / 2.0, -gridSize / 2.0), cpv(gridWidth * gridSize - gridSize / 2.0, -gridSize / 2.0), 0.0));
	// bottom border segment
	space->add(new cp::SegmentShape(staticBody, cpv(-gridSize / 2.0, gridHeight * gridSize - gridSize / 2.0), cpv(gridWidth * gridSize - gridSize / 2.0, gridHeight * gridSize - gridSize / 2.0), 0.0));
	// left border segment
	space->add(new cp::SegmentShape(staticBody, cpv(-gridSize / 2.0, -gridSize / 2.0), cpv(-gridSize / 2.0, gridHeight * gridSize - gridSize / 2.0), 0.0));
	// right border segment
	space->add(new cp::SegmentShape(staticBody, cpv(gridWidth * gridSize - gridSize / 2.0, -gridSize / 2.0), cpv(gridWidth * gridSize - gridSize / 2.0, gridHeight * gridSize - gridSize / 2.0), 0.0));

	// horizontal inner segments
	for (int j = 1; j < gridHeight; j++)
	{
		for (int i = 0; i < gridWidth; i++)
		{
			bool* conn = m(i + j * gridWidth, i + (j - 1) * gridWidth);
			if (conn && *conn)
			{
				space->add(new cp::SegmentShape(staticBody, cpv(i * gridSize - gridSize / 2.0, j * gridSize - gridSize / 2.0), cpv((i + 1) * gridSize - gridSize / 2.0, j * gridSize - gridSize / 2.0), 0.0));
			}
		}
	}

	// vertical inner segments
	for (int i = 1; i < gridWidth; i++)
	{
		for (int j = 0; j < gridHeight; j++)
		{
			bool* conn = m(i + j * gridWidth, i - 1 + j * gridWidth);
			if (conn && *conn)
			{
				space->add(new cp::SegmentShape(staticBody, cpv(i * gridSize - gridSize / 2.0, j * gridSize - gridSize / 2.0), cpv(i * gridSize - gridSize / 2.0, (j + 1) * gridSize - gridSize / 2.0), 0.0));
			}
		}
	}

	const float mass = 5.0;
	const float radius = 5.0;

	cp::Body* playerBody = space->add(new cp::Body(mass, cpMomentForCircle(mass, radius, 0, cpvzero)));
	space->add(new cp::CircleShape(playerBody, radius, cpvzero));
	space->setUserData(playerBody);

	return space;
}
