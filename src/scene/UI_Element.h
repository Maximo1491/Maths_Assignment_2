namespace octet {
	class ui_element {
		float x;
		float y;
		float width;
		float height;
		float windowH;
		float windowW;

		unsigned int vp_vbo, vt_vbo;

		int texture;
		bool enabled;

		glm::mat4 matrix, scale, pos;

	public:
		ui_element()
		{
			texture = 0;
			enabled = true;
		}

		void init(float x_, float y_, float w, float h, float ww, float wh)
		{
			x = x_;
			y = y_;
			width = w;
			height = h;
			windowW = ww;
			windowH = wh;

			scale = glm::scale(glm::mat4(1.0), glm::vec3(width / windowW, height / windowH, 0));
			float x_pos = -1.0f + ((x / (windowW*0.5f)) + (width / windowW));
			float y_pos = 1.0f - ((y / (windowH*0.5f)) + (height / windowH));
			pos = glm::translate(glm::mat4(1.0), glm::vec3(x_pos, y_pos, 0));

			matrix = pos * scale;

			float points[] = {
				-1, 1,
				-1, -1,
				1, -1,
				1, -1,
				1, 1,
				-1, 1
			};

			float tex_coords[] = {
				0.0, 1.0,
				0.0, 0.0,
				1.0, 0.0,
				1.0, 0.0,
				1.0, 1.0,
				0.0, 1.0
			};

			glGenBuffers(1, &vp_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
			glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(float), &points, GL_STATIC_DRAW);

			glGenBuffers(1, &vt_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
			glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(float), &tex_coords, GL_STATIC_DRAW);
		}

    void setPos(float x_, float y_)
    {
      x = x_;
      y = y_;

      scale = glm::scale(glm::mat4(1.0), glm::vec3(width / windowW, height / windowH, 0));
			float x_pos = -1.0f + ((x / (windowW*0.5f)) + (width / windowW));
			float y_pos = 1.0f - ((y / (windowH*0.5f)) + (height / windowH));
			pos = glm::translate(glm::mat4(1.0), glm::vec3(x_pos, y_pos, 0));

			matrix = pos * scale;
    }

		void render(ui_shader &shader, glm::vec4 color = glm::vec4(0, 0, 0, 0))
		{

			if (color.w != 0)
				shader.render(matrix, color);
			else
				shader.render(matrix);

			glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
			glEnableVertexAttribArray(attribute_position);
			glVertexAttribPointer(attribute_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
			glEnableVertexAttribArray(attribute_texcoord);
			glVertexAttribPointer(attribute_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	};
}