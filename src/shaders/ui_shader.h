namespace octet {
	class ui_shader : public shader
	{
		GLuint matrixIndex_, colorIndex_;

	public:
		void init()
		{
			const char vertex_shader[] = SHADER_STR(

				attribute vec2 position;
			attribute vec2 texcoord;
			uniform mat4 matrix;
			uniform vec4 uicolor;

			varying vec4 f_color;

			void main() {
				gl_Position = matrix * vec4(position, 0.0, 1.0);

				f_color = vec4(uicolor);
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
			matrixIndex_ = glGetUniformLocation(program(), "matrix");
			colorIndex_ = glGetUniformLocation(program(), "uicolor");
		}

		void render(glm::mat4 &view, glm::vec4 &color = glm::vec4(1.0, 0.5, 0, 1.0))
		{
			shader::render();
			glUniformMatrix4fv(matrixIndex_, 1, GL_FALSE, glm::value_ptr(view));
			glUniform4f(colorIndex_, color.x, color.y, color.z, color.w);
		}
	};
}
