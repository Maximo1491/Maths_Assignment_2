
namespace octet {
  class my_shader : public shader {

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;

    // index for texture sampler
    GLuint samplerIndex_;

		// index for player color
		GLuint emissive_colorIndex_;
  public:
    void init() {
			
			//Outputs texture UV, and if it is on the player or not
      const char vertex_shader[] = SHADER_STR(
        varying vec2 uv_;
				varying float player_;

        attribute vec4 pos;
        attribute vec2 uv;
				attribute float player;

        uniform mat4 modelToProjection;


        void main() { gl_Position = modelToProjection * pos; uv_ = uv; player_ = player; }
      );

      //Draws the textures, if it is the player then draw a random colour
      const char fragment_shader[] = SHADER_STR(
        varying vec2 uv_;
				varying float player_;
        uniform sampler2D sampler;
				uniform vec4 emissive_color;

        void main() 
				{ 
					gl_FragColor = texture2D(sampler, uv_);
					//If it is the player then set a random colour on top of the texture
					if(player_ == 1)
					{
						gl_FragColor = gl_FragColor * emissive_color;
					}
				}
      );
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
      samplerIndex_ = glGetUniformLocation(program(), "sampler");
			emissive_colorIndex_ = glGetUniformLocation(program(), "emissive_color");
    }

    void render(const mat4t &modelToProjection, int sampler, const vec4 &emissive_color) {
      shader::render();

      // customize the program with uniforms
			glUniform4fv(emissive_colorIndex_, 1, emissive_color.get());
      glUniform1i(samplerIndex_, sampler);
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
    }
  };
}
