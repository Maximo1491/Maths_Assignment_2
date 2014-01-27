namespace octet
{
  class sky
	{
	private:
		GLuint vbo;
		int vertexSize;

	public:

		sky()
		{
			glGenBuffers(1, &vbo);

			float radius = 250.0f;
			int subdivisions = 5;

			GLfloat* vert_swap = new GLfloat[24 * 3 * (int)pow(4.0f, subdivisions)];
			GLfloat* vertices = new GLfloat[24 * 3 * (int)pow(4.0f, subdivisions)];

			vertexSize = 24 * 3 * (int)pow(4.0f, subdivisions);

			//Form an octahedron
			vertices[0] = 0.0f;
			vertices[1] = 0.0f;
			vertices[2] = 1.0f;

			vertices[3] = 0.0f;
			vertices[4] = 1.0f;
			vertices[5] = 0.0f;

			vertices[6] = 1.0f;
			vertices[7] = 0.0f;
			vertices[8] = 0.0f;

			vertices[9] = 0.0f;
			vertices[10] = 1.0f;
			vertices[11] = 0.0f;

			vertices[12] = 0.0f;
			vertices[13] = 0.0f;
			vertices[14] = 1.0f;

			vertices[15] = -1.0f;
			vertices[16] = 0.0f;
			vertices[17] = 0.0f;

			vertices[18] = 0.0f;
			vertices[19] = 1.0f;
			vertices[20] = 0.0f;

			vertices[21] = -1.0f;
			vertices[22] = 0.0f;
			vertices[23] = 0.0f;

			vertices[24] = 0.0f;
			vertices[25] = 0.0f;
			vertices[26] = -1.0f;

			vertices[27] = 0.0f;
			vertices[28] = 1.0f;
			vertices[29] = 0.0f;
			
			vertices[30] = 0.0f;
			vertices[31] = 0.0f;
			vertices[32] = -1.0f;
			
			vertices[33] = 1.0f;
			vertices[34] = 0.0f;
			vertices[35] = 0.0f;

			vertices[36] = 1.0f;
			vertices[37] = 0.0f;
			vertices[38] = 0.0f;
			
			vertices[39] = 0.0f;
			vertices[40] = 0.0f;
			vertices[41] = -1.0f;
			
			vertices[42] = 0.0f;
			vertices[43] = -1.0f;
			vertices[44] = 0.0f;

			vertices[45] = 0.0f;
			vertices[46] = 0.0f;
			vertices[47] = 1.0f;

			vertices[48] = 1.0f;
			vertices[49] = 0.0f;
			vertices[50] = 0.0f;
			
			vertices[52] = -1.0f;			
			vertices[51] = 0.0f;
			vertices[53] = 0.0f;

			vertices[54] = 0.0f;
			vertices[55] = 0.0f;
			vertices[56] = -1.0f;
			
			vertices[57] = -1.0f;
			vertices[58] = 0.0f;
			vertices[59] = 0.0f;
			
			vertices[60] = 0.0f;
			vertices[61] = -1.0f;
			vertices[62] = 0.0f;

			vertices[63] = -1.0f;
			vertices[64] = 0.0f;
			vertices[65] = 0.0f;
			
			vertices[66] = 0.0f;
			vertices[67] = 0.0f;
			vertices[68] = 1.0f;
			
			vertices[69] = 0.0f;
			vertices[70] = -1.0f;
			vertices[71] = 0.0f;

			//Subdivide the octrahedron as many times as specified by subdivisions.
			for (int i = 0; i < subdivisions; i++)
			{
				int currentNumOfTriangles = 8 * (int)pow(4.0f, i);

				for (int j = 0; j < currentNumOfTriangles; j++)
				{
					vec3 newVert1;
					vec3 newVert2;
					vec3 newVert3;

					vec3 vert1 = vec3((vertices[0 + (j * 9)] + vertices[3 + (j * 9)]) / 2, (vertices[1 + (j * 9)] + vertices[4 + (j * 9)]) / 2, (vertices[2 + (j * 9)] + vertices[5 + (j * 9)]) / 2);
					vec3 vert2 = vec3((vertices[0 + (j * 9)] + vertices[6 + (j * 9)]) / 2, (vertices[1 + (j * 9)] + vertices[7 + (j * 9)]) / 2, (vertices[2 + (j * 9)] + vertices[8 + (j * 9)]) / 2);

					newVert1 = vert1;

					vert_swap[0 + (j * 36)] = vertices[0 + (j * 9)];
					vert_swap[1 + (j * 36)] = vertices[1 + (j * 9)];
					vert_swap[2 + (j * 36)] = vertices[2 + (j * 9)];

					vert_swap[3 + (j * 36)] = vert1.x();
					vert_swap[4 + (j * 36)] = vert1.y();
					vert_swap[5 + (j * 36)] = vert1.z();

					vert_swap[6 + (j * 36)] = vert2.x();
					vert_swap[7 + (j * 36)] = vert2.y();
					vert_swap[8 + (j * 36)] = vert2.z();

					vert1 = vec3((vertices[3 + (j * 9)] + vertices[6 + (j * 9)]) / 2, (vertices[4 + (j * 9)] + vertices[7 + (j * 9)]) / 2, (vertices[5 + (j * 9)] + vertices[8 + (j * 9)]) / 2);
					vert2 = vec3((vertices[3 + (j * 9)] + vertices[0 + (j * 9)]) / 2, (vertices[4 + (j * 9)] + vertices[1 + (j * 9)]) / 2, (vertices[5 + (j * 9)] + vertices[2 + (j * 9)]) / 2);

					newVert2 = vert1;

					vert_swap[9 + (j * 36)] = vertices[3 + (j * 9)];
					vert_swap[10 + (j * 36)] = vertices[4 + (j * 9)];
					vert_swap[11 + (j * 36)] = vertices[5 + (j * 9)];

					vert_swap[12 + (j * 36)] = vert1.x();
					vert_swap[13 + (j * 36)] = vert1.y();
					vert_swap[14 + (j * 36)] = vert1.z();

					vert_swap[15 + (j * 36)] = vert2.x();
					vert_swap[16 + (j * 36)] = vert2.y();
					vert_swap[17 + (j * 36)] = vert2.z();

					vert1 = vec3((vertices[6 + (j * 9)] + vertices[0 + (j * 9)]) / 2, (vertices[7 + (j * 9)] + vertices[1 + (j * 9)]) / 2, (vertices[8 + (j * 9)] + vertices[2 + (j * 9)]) / 2);
					vert2 = vec3((vertices[6 + (j * 9)] + vertices[3 + (j * 9)]) / 2, (vertices[7 + (j * 9)] + vertices[4 + (j * 9)]) / 2, (vertices[8 + (j * 9)] + vertices[5 + (j * 9)]) / 2);

					newVert3 = vert1;

					vert_swap[18 + (j * 36)] = vertices[6 + (j * 9)];
					vert_swap[19 + (j * 36)] = vertices[7 + (j * 9)];
					vert_swap[20 + (j * 36)] = vertices[8 + (j * 9)];

					vert_swap[21 + (j * 36)] = vert1.x();
					vert_swap[22 + (j * 36)] = vert1.y();
					vert_swap[23 + (j * 36)] = vert1.z();

					vert_swap[24 + (j * 36)] = vert2.x();
					vert_swap[25 + (j * 36)] = vert2.y();
					vert_swap[26 + (j * 36)] = vert2.z();

					vert_swap[27 + (j * 36)] = newVert1.x();
					vert_swap[28 + (j * 36)] = newVert1.y();
					vert_swap[29 + (j * 36)] = newVert1.z();

					vert_swap[30 + (j * 36)] = newVert2.x();
					vert_swap[31 + (j * 36)] = newVert2.y();
					vert_swap[32 + (j * 36)] = newVert2.z();

					vert_swap[33 + (j * 36)] = newVert3.x();
					vert_swap[34 + (j * 36)] = newVert3.y();
					vert_swap[35 + (j * 36)] = newVert3.z();
				}

				//Normalize the octrahedron to form a unit length sphere.
				//Multiply the vertices by the radius to expand the sphere to the radius size.
				for (int j = 0; j < currentNumOfTriangles * 12; j++)
				{
					float magnitude = sqrt(pow(vert_swap[0 + (j * 3)], 2) + pow(vert_swap[1 + (j * 3)], 2) + pow(vert_swap[2 + (j * 3)], 2));

					vert_swap[0 + (j * 3)] = radius * (vert_swap[0 + (j * 3)] / magnitude);
					vert_swap[1 + (j * 3)] = radius * (vert_swap[1 + (j * 3)] / magnitude);
					vert_swap[2 + (j * 3)] = radius * (vert_swap[2 + (j * 3)] / magnitude);

					vertices[0 + (j * 3)] = vert_swap[0 + (j * 3)];
					vertices[1 + (j * 3)] = vert_swap[1 + (j * 3)];
					vertices[2 + (j * 3)] = vert_swap[2 + (j * 3)];
				}
			}

			//Bind the sphere vertices into the buffer.
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, vertexSize * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

			//Delete the arrays used to form the sphere.
			delete[] vert_swap;
			delete[] vertices;
		}

		~sky()
		{
			glDeleteBuffers(1, &vbo);
		}

		void render(glm::mat4 projection, glm::vec3 playerPos, glm::vec3 sunDirection, sky_shader &sky_shader_)
		{
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			sky_shader_.render(projection, playerPos, sunDirection);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glEnableVertexAttribArray(attribute_pos);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glDrawArrays(GL_TRIANGLES, 0, vertexSize);

			glDisableVertexAttribArray(attribute_pos);
		}
	};
}