namespace octet {
  class color_shader : public shader 
	{
    GLuint modelToProjectionIndex_, uniform_type, uniform_ambient, uniform_diffuse, uniform_light, uniform_numOfLights;

  public:
    void init() 
		{
			const char vertex_shader[] = SHADER_STR(
			const int maxLights = 8;
			attribute vec4 pos;
			attribute vec3 normal;

			uniform mat4 modelToProjection;
			uniform vec4 ambient;
			uniform vec4 diffuse[maxLights];
			uniform vec4 light[maxLights];
			uniform int numOfLights;
		
			varying vec4 f_color;

			void main()
			{
				vec4 color;
				vec3 v_normal;

				gl_Position = modelToProjection * vec4(pos.xyz, 1);

				if (pos.w == 0.0)
				{
					color = vec4(1, 0, 0, 1);
				}

				else if (pos.w == 1.0) // grass
				{
					color = vec4(0.160, 0.570, 0.168, 1);
				}

				else if (pos.w == 2.0) // dirt
				{
					color = vec4(0.6, 0.3, 0, 1);
				}

				else if (pos.w == 3.0) // stone
				{
					color = vec4(0.5, 0.5, 0.5, 1);
				}
				
				else if (pos.w == 4.0) // wood
				{
					color = vec4(0.4, 0.2, 0, 1);
				}

				else if (pos.w == 5.0) // bush/leaves
				{
					color = vec4(0.06, 0.25, 0.06, 1);
				}

				else if (pos.w == 6.0) // snow
				{
					color = vec4(0.75, 0.75, 0.75, 1);
				}

        else if (pos.w == 7.0) // sand
				{
					color = vec4(0.841, 0.865, 0.353, 1);
				}

        else if (pos.w == 8.0) // brick
				{
					color = vec4(0.948, 0.254, 0.254, 1);
				}

				else
					color = vec4(1, 1, 1, 1);

				vec4 ambientColor = ambient * color;
				vec4 diffuseFactor = vec4(0.0, 0.0, 0.0, 0.0);

				v_normal = normalize(normal);
				
				for (int i = 0; i < numOfLights; i++)
				{
					vec3 L = normalize(light[i]).xyz;

					float diffusePercentage = max(dot(L, v_normal), 0.0);
					diffuseFactor += diffusePercentage * diffuse[i];
				}

				vec4 diffuseColor = diffuseFactor * color;

				f_color = vec4((ambientColor + diffuseColor).xyz, 1.0);
			}
      );

      const char fragment_shader[] = SHADER_STR(
        varying vec4 f_color;

        void main() 
		{
			gl_FragColor = f_color;
		}
      );

      shader::init(vertex_shader, fragment_shader);

      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
	  uniform_ambient = glGetUniformLocation(program(), "ambient");
	  uniform_diffuse = glGetUniformLocation(program(), "diffuse");
	  uniform_light = glGetUniformLocation(program(), "light");
	  uniform_numOfLights = glGetUniformLocation(program(), "numOfLights");
    }

    void render(glm::mat4 &modelToProjection, int numOfLights, const glm::vec4 *light_information, glm::vec4 ambient, glm::vec4 *diffuse) 
	{
      shader::render();

      glUniformMatrix4fv( modelToProjectionIndex_, 1, GL_FALSE, &modelToProjection[0][0] );
	  glUniform4f( uniform_ambient, ambient.x, ambient.y, ambient.z, ambient.w );
	  glUniform4fv( uniform_diffuse, numOfLights, (float*)diffuse );
	  glUniform4fv( uniform_light, numOfLights, (float*)light_information );
	  glUniform1i( uniform_numOfLights, numOfLights );
    }
  };
}
