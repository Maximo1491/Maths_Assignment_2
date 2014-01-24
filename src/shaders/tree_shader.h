namespace octet {
  class tree_shader : public shader 
	{
    GLuint modelToProjectionIndex_, uniform_texture0, uniform_texture1, uniform_ambient, uniform_diffuse, uniform_light, uniform_numOfLights;

  public:
    void init() 
		{
		const char vertex_shader[] = SHADER_STR(
			const int maxLights = 8;
			
			attribute vec4 pos;
			attribute vec2 uv;
			attribute vec4 normal;
			
			uniform mat4 matrix;
			uniform vec4 ambient;
			uniform vec4 diffuse[maxLights];
			uniform vec4 light[maxLights];
			uniform int numOfLights;
			
			varying vec2 f_texCoord;
			varying float texType;
			varying vec4 f_color;
			
			void main()
			{
				gl_Position = matrix * vec4(pos.xyz, 1.0);
				f_texCoord = uv;
				texType = pos.w;

				vec4 ambientColor = ambient * vec4(1.0, 1.0, 1.0, 1.0);
				vec4 diffuseFactor = vec4(0.0, 0.0, 0.0, 0.0);

				vec3 v_normal = normalize(normal).xyz;
				
				for (int i = 0; i < numOfLights; i++)
				{
					vec3 L = normalize(light[i]).xyz;

					float diffusePercentage = max(dot(L, v_normal), 0.0);
					diffuseFactor += diffusePercentage * diffuse[i];
				}

				vec4 diffuseColor = diffuseFactor * vec4(1.0, 1.0, 1.0, 1.0);

				f_color = vec4((ambientColor + diffuseColor).xyz, 1.0);
			}
		);

		const char fragment_shader[] = SHADER_STR(
			varying vec2 f_texCoord;
			varying float texType;
			varying vec4 f_color;
			
			uniform sampler2D bark;
			uniform sampler2D leaf;
			
			void main()
			{
				if (texType < 1.5)
					gl_FragColor = texture2D(bark, f_texCoord) * f_color;
				else
				{
					if (texture2D(leaf, f_texCoord).a > 0.0)
						gl_FragColor = texture2D(leaf, f_texCoord) * f_color;
					else
						discard;
				}
			}
			);

      shader::init(vertex_shader, fragment_shader);

      modelToProjectionIndex_ = glGetUniformLocation(program(), "matrix");
	  uniform_texture0 = glGetUniformLocation(program(), "bark");
	  uniform_texture1 = glGetUniformLocation(program(), "leaf");
	  uniform_ambient = glGetUniformLocation(program(), "ambient");
	  uniform_diffuse = glGetUniformLocation(program(), "diffuse");
	  uniform_light = glGetUniformLocation(program(), "light");
	  uniform_numOfLights = glGetUniformLocation(program(), "numOfLights");
    }

    void render(glm::mat4 &modelToProjection, int numOfLights, const glm::vec4 *light_information, glm::vec4 ambient, glm::vec4 *diffuse) 
	{
		shader::render();

		mat4t modelToWorld;
		modelToWorld[0][0] = modelToProjection[0].x;
		modelToWorld[0][1] = modelToProjection[0].y;
		modelToWorld[0][2] = modelToProjection[0].z;
		modelToWorld[0][3] = modelToProjection[0].w;
		modelToWorld[1][0] = modelToProjection[1].x;
		modelToWorld[1][1] = modelToProjection[1].y;
		modelToWorld[1][2] = modelToProjection[1].z;
		modelToWorld[1][3] = modelToProjection[1].w;
		modelToWorld[2][0] = modelToProjection[2].x;
		modelToWorld[2][1] = modelToProjection[2].y;
		modelToWorld[2][2] = modelToProjection[2].z;
		modelToWorld[2][3] = modelToProjection[2].w;
		modelToWorld[3][0] = modelToProjection[3].x;
		modelToWorld[3][1] = modelToProjection[3].y;
		modelToWorld[3][2] = modelToProjection[3].z;
		modelToWorld[3][3] = modelToProjection[3].w;

		modelToWorld.transpose4x4();

		glUniformMatrix4fv( modelToProjectionIndex_, 1, GL_FALSE, modelToWorld.get() );
		glUniform1i( uniform_texture0, 0 );
		glUniform1i( uniform_texture1, 1 );
		glUniform4f( uniform_ambient, ambient.x, ambient.y, ambient.z, ambient.w );
		glUniform4fv( uniform_diffuse, numOfLights, (float*)diffuse );
		glUniform4fv( uniform_light, numOfLights, (float*)light_information );
		glUniform1i( uniform_numOfLights, numOfLights );
    }
  };
}