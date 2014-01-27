namespace octet {
	class noise{

		float seed;
	public:
		noise()
		{
			seed = 0;
		}

		float findnoise(float x, float y)
		{
			int n = (int)(x + y * 57);
			n = (n << 13) ^ n;
			int nn = (n*(n*n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
			return 1.0f - (nn / 1073741824.0f);
		}

		float interpolate(float a, float b, float x)
		{
			float ft = x * 3.1415927f;
			float f = (1 - glm::cos(ft)) * 0.5f;
			return a * (1 - f) + (b*f);
		}

		float getNoise(float x, float y)
		{
			float floorx = glm::floor(x);
			float floory = glm::floor(y);

			float s, t, u, v;

			s = findnoise(floorx, floory);
			t = findnoise(floorx + 1, floory);
			u = findnoise(floorx, floory + 1);
			v = findnoise(floorx + 1, floory + 1);

			float int1 = interpolate(s, t, x - floorx);
			float int2 = interpolate(u, v, x - floorx);

			return interpolate(int1, int2, y - floory);
		}

		float getSeed() { return seed; }

		void setSeed(float s)
		{
			seed = s;
		}

		void setRandomSeed()
		{
			srand(time(NULL));

			seed = rand() % rand();
		}
	};
}