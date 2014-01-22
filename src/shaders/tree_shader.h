namespace octet {
  class tree_shader : public shader 
	{
    GLuint modelToProjectionIndex_, uniform_texture0, uniform_texture1;

  public:
    void init() 
		{
		const char vertex_shader[] = SHADER_STR(
			attribute vec4 pos;\n
			attribute vec2 uv;\n
			\n
			uniform mat4 matrix;\n
			\n
			varying vec2 f_texCoord;\n
			varying float texType;\n
			\n
			void main()\n
			{\n
				gl_Position = matrix * vec4(pos.xyz, 1.0);\n
				f_texCoord = uv;\n
				texType = pos.w;\n
			}\n
		);

		const char fragment_shader[] = SHADER_STR(
			varying vec2 f_texCoord;\n
			varying float texType;\n
			\n
			uniform sampler2D bark;\n
			uniform sampler2D leaf;\n
			\n
			void main()\n
			{\n
				if (texType < 1.5)\n
					gl_FragColor = texture2D(bark, f_texCoord);\n
				else\n
					gl_FragColor = texture2D(leaf, f_texCoord);\n
			}\n
		);

      shader::init(vertex_shader, fragment_shader);

      modelToProjectionIndex_ = glGetUniformLocation(program(), "matrix");
	  uniform_texture0 = glGetUniformLocation(program(), "bark");
	  uniform_texture1 = glGetUniformLocation(program(), "leaf");
    }

    void render(glm::mat4 &modelToProjection) 
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
    }
  };
}