namespace octet {
  class tree_shader : public shader 
	{
    GLuint modelToProjectionIndex_;

  public:
    void init() 
		{
		const char vertex_shader[] = SHADER_STR(
			attribute vec3 pos;\n
			attribute vec2 uv;\n
			attribute vec4 color;\n
			\n
			uniform mat4 matrix;\n
			\n
			varying vec2 f_texCoord;\n
			varying vec4 f_color;\n
			\n
			void main()\n
			{\n
				gl_Position = matrix * vec4(pos, 1.0);\n
				f_texCoord = uv;\n
				f_color = color;\n
			}\n
		);

		const char fragment_shader[] = SHADER_STR(
			varying vec2 f_texCoord;\n
			varying vec4 f_color;\n
			\n
			uniform sampler2D tex;\n
			\n
			void main()\n
			{\n
				//gl_FragColor = vec4(f_color.xyz, 1.0);\n
				gl_FragColor = texture(tex, f_texCoord);\n
			}\n
		);

      shader::init(vertex_shader, fragment_shader);

      modelToProjectionIndex_ = glGetUniformLocation(program(), "matrix");
    }

    void render(glm::mat4 &modelToProjection) 
	{
      shader::render();

      glUniformMatrix4fv( modelToProjectionIndex_, 1, GL_FALSE, &modelToProjection[0][0] );
    }
  };
}