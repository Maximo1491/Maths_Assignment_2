namespace octet {
  class shadow_shader : public shader 
	{
    GLuint modelToProjectionIndex_;

  public:
    void init() 
		{
		const char vertex_shader[] = SHADER_STR(
			attribute vec4 pos;

			uniform mat4 lightProjection;

			void main()
			{
				gl_Position = lightProjection * vec4(pos.xyz, 1);
			}
		);

		const char fragment_shader[] = SHADER_STR(

			void main()
			{
				 
			}
		);

      shader::init(vertex_shader, fragment_shader);

      modelToProjectionIndex_ = glGetUniformLocation(program(), "lightProjection");
    }

    void render(mat4t &modelToProjection) 
	{
		shader::render();

		glUniformMatrix4fv( modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get() );
    }
  };
}