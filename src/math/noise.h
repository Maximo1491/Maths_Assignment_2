namespace octet {
  class noise{

    float seed;
    float smoothness;
  public:
    noise()
    {
      seed = 0;
      smoothness = 25.0f;
    }

    float findnoise(float x, float y)
    {
      int n = (int)(x + y * 57);
      n = (n << 13) ^ n;
      return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
    }

    float interpolate(float a, float b, float x)
    {
      float ft = x * 3.1415927f;
      float f = (1 - glm::cos(ft)) * 0.5f;
      return a * (1 - f) + (b*f);
    }

    float smoothNoise( float x, float y)
    {
      float corners = (findnoise(x-1, y-1) + findnoise(x+1, y-1) + findnoise(x-1, y+1) + findnoise(x+1, y+1))/16;
      float sides = (findnoise(x-1, y) + findnoise(x+1, y) + findnoise(x, y-1) + findnoise(x, y+1))/8;
      float center = findnoise(x, y)/4;
      return corners + sides + center;
    }

    float getNoise(float x, float y)
    {
      int intX = int(x);
      int intY = int(y);
      float fracX = x - intX;
      float fracY = y - intY;

      float v1 = smoothNoise(intX, intY);
      float v2 = smoothNoise(intX + 1, intY);
      float v3 = smoothNoise(intX, intY + 1);
      float v4 = smoothNoise(intX + 1, intY + 1);

      float i1 = interpolate(v1, v2, fracX);
      float i2 = interpolate(v3, v4, fracX);

      return interpolate(i1, i2, fracY);
    }

    float perlinNoise( float x, float y )
    {
      float total = 0;
      float persistance = 0.5f;
      float octaves = 2;

      for (float i = 0; i < octaves - 1; i++)
      {
        float frequency = glm::pow(2.0f, i);
        float amplitude = glm::pow(persistance, i);

        total += getNoise((x + seed) * frequency/smoothness, (y + seed) *  frequency /smoothness) * amplitude;
      }

      return total;
    }

    float getSeed() { return seed; }

    void setSeed(float s)
    {
      seed = s;
    }

    void setSmoothness( float s )
    {
      smoothness = s;
    }

    void setRandomSeed()
    {
      srand(time(NULL));

      seed = rand() % rand();
    }
  };
}