namespace octet
{
  class superChunk
	{
	public:
		//Creates a 3D array of chunks
		chunk *c[SCX][SCY][SCZ];

		superChunk()
		{
      //Sets enough memory for the chunk
			memset(c, 0, sizeof c);
		}

		~superChunk()
		{
      //When the superChunk gets deleted, delete all the objects in the superChunk
			for(int x = 0; x < SCX; x++)
				for(int y = 0; y < SCY; y++)
					for(int z = 0; z < SCZ; z++)
						delete c[x][y][z];
		}

		uint8_t get(int x, int y, int z)
		{
      /*This function allows us to get a voxel within a superChunk so for example
        if we were to say get(128, 10, 17) it would find what chunk that voxel is
        in and then return that voxel from that chunk*/

			int cx = x / CX;
			int cy = y / CY;
			int cz = z / CZ;

			x %= CX;
			y %= CY;
			z %= CZ;

			if(!c[cx][cy][cz])
				return 0;
			else
				return c[cx][cy][cz]->get(x, y, z);
		}

		void set(int x, int y, int z, uint8_t type)
		{
      /*Like the get function above, this function allows us to set an individual
        voxel in our superChunk so ie we set(108,10,7) to type 1 we find where
        that voxel is supposed to be in our array of chunks and set that voxel to
        the type and checks to make sure there is already a chunk there otherwise
        it makes a new one*/

			int cx = x / CX;
			int cy = y / CY;
			int cz = z / CZ;

			x %= CX;
			y %= CY;
			z %= CZ;

			if(!c[cx][cy][cz])
				c[cx][cy][cz] = new chunk();

			c[cx][cy][cz]->set(x, y, z, type);

			if (x == 0 && cx > 0 && c[cx - 1][cy][cz] != NULL)
				c[cx - 1][cy][cz]->setChanged(true);

			if (x == CX - 1 && cx < SCX - 1 && c[cx + 1][cy][cz] != NULL)
				c[cx + 1][cy][cz]->setChanged(true);

			if (y == 0 && cy > 0 && c[cx][cy - 1][cz] != NULL)
				c[cx][cy - 1][cz]->setChanged(true);

			if (y == CY - 1 && cy < SCY - 1 && c[cx][cy + 1][cz] != NULL)
				c[cx][cy + 1][cz]->setChanged(true);

			if (z == 0 && cz > 0 && c[cx][cy][cz - 1] != NULL)
				c[cx][cy][cz - 1]->setChanged(true);

			if (z == CZ - 1 && cz < SCZ - 1 && c[cx][cy][cz + 1] != NULL)
				c[cx][cy][cz + 1]->setChanged(true);
		}

		void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection, int numOfLights, glm::vec4 *light_information, glm::vec4 light_ambient, glm::vec4 light_diffuse, color_shader color_shader_)
		{
      //Create a temporary modelToWorld to store the keep track of the start point
      glm::mat4 temp = model;
			for(int x = 0; x < SCX; x++){
				for(int y = 0; y < SCY; y++){
					for(int z = 0; z < SCZ; z++){
            //If the chunk exists
						if(c[x][y][z])
						{
							//Reset the modelToWorld back to its start point
							model = temp;

							//Translate the modelToWorld to the new chunk position
							model = glm::translate(glm::mat4(1.0f), glm::vec3((float)(x * CX), (float)(y * CY), (float)(z * CZ)));

							//Build a new projection onto the screen
							glm::mat4 mvp = projection * view * model;

							//Send the colour shader our new projection
							color_shader_.render(mvp, numOfLights, light_information, light_ambient, light_diffuse);

							//Draw the chunk at the current position

							chunk* chunkArray[6];

							if (x > 0)
								chunkArray[0] = c[x - 1][y][z];
							else
								chunkArray[0] = 0;
							
							if (x < SCX - 1)
								chunkArray[1] = c[x + 1][y][z];
							else
								chunkArray[1] = 0;

							if (y > 0)
								chunkArray[2] = c[x][y - 1][z];
							else
								chunkArray[2] = 0;
							
							if (y < SCY - 1)
								chunkArray[3] = c[x][y + 1][z];
							else
								chunkArray[3] = 0;

							if (z > 0)
								chunkArray[4] = c[x][y][z - 1];
							else
								chunkArray[4] = 0;
							
							if (z < SCZ - 1)
								chunkArray[5] = c[x][y][z + 1];
							else
								chunkArray[5] = 0;


							c[x][y][z]->render(chunkArray);
						}
					}
				}
			}
		}
	};
}