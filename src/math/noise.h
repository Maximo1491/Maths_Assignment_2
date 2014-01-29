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

    //A simple random number generator
    float findnoise(float x, float y)
    {
      int n = (int)(x + y * 57);
      n = (n << 13) ^ n;
      return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
    }

    //Interpolates between the two points
    float interpolate(float a, float b, float x)
    {
      float ft = x * 3.1415927f;
      float f = (1 - glm::cos(ft)) * 0.5f;
      return a * (1 - f) + (b*f);
    }

    //Smooths the noise by getting the neighbouring values and averag5ing between them
    float smoothNoise( float x, float y)
    {
      float corners = (findnoise(x-1, y-1) + findnoise(x+1, y-1) + findnoise(x-1, y+1) + findnoise(x+1, y+1))/16;
      float sides = (findnoise(x-1, y) + findnoise(x+1, y) + findnoise(x, y-1) + findnoise(x, y+1))/8;
      float center = findnoise(x, y)/4;
      return corners + sides + center;
    }

    float getNoise(float x, float y)
    {
      //Stores the floor value of the current noise
      int intX = glm::floor(x);
      int intY = glm::floor(y);

      //Stores the gap between integers in order to interpolate
      float fracX = x - intX;
      float fracY = y - intY;

      //Gets the neighbours of the current noise
      float v1 = smoothNoise(intX, intY);
      float v2 = smoothNoise(intX + 1, intY);
      float v3 = smoothNoise(intX, intY + 1);
      float v4 = smoothNoise(intX + 1, intY + 1);

      //Interpolate the X axis
      float i1 = interpolate(v1, v2, fracX);
      float i2 = interpolate(v3, v4, fracX);

      //Interpolate the Y axis
      return interpolate(i1, i2, fracY);
    }

    //The function which is used to return a height to thronecraft.h
    float perlinNoise( float x, float y )
    {
      //The size of the noise to send back
      float total = 0;
      //Used to specify the amplitude of each frequency
      float persistance = 0.5f;
      //The amount of loops to go through to get the resulting noise
      float octaves = 2;

      for (float i = 0; i < octaves - 1; i++)
      {
        //Doubles the frequency every loop
        float frequency = glm::pow(2.0f, i);
        //Used to define the maximums of the heights
        float amplitude = glm::pow(persistance, i);

        total += getNoise((x + seed) * frequency/smoothness, (y + seed) *  frequency /smoothness) * amplitude;
      }

      return total;
    }

    //Returns the seed
    float getSeed() { return seed; }

    //Allows you to set the seed to use
    void setSeed(float s)
    {
      seed = s;
    }

    //Allows you to smooth our the terrain
    void setSmoothness( float s )
    {
      smoothness = s;
    }
    
    //Sets the seed to a random value rather than specifying
    void setRandomSeed()
    {
      srand(time(NULL));

      seed = rand() % rand();
    }
  };
}