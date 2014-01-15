namespace octet {
  class bump_shader : public shader {

    GLuint modelToProjection_index; // index for model space to projection space matrix
    GLuint modelToCamera_index;     // second matrix used for lighting maps model to camera space
    GLuint cameraToProjection_index;// used in skinned shader
    GLuint light_uniforms_index;    // lighting parameters for fragment shader
    GLuint num_lights_index;        // how many lights?
    GLuint samplers_index;          // index for texture samplers

    void init_uniforms(const char *vertex_shader, const char *fragment_shader) {
      shader::init(vertex_shader, fragment_shader);

      modelToProjection_index = glGetUniformLocation(program(), "modelToProjection");
      cameraToProjection_index = glGetUniformLocation(program(), "cameraToProjection");
      modelToCamera_index = glGetUniformLocation(program(), "modelToCamera");
      light_uniforms_index = glGetUniformLocation(program(), "light_uniforms");
      num_lights_index = glGetUniformLocation(program(), "num_lights");
      samplers_index = glGetUniformLocation(program(), "samplers");
    }

  public:
    void init(bool is_skinned=false) {

      const char vertex_shader[] = SHADER_STR(
        varying vec2 uv_;
        varying vec3 normal_;
        varying vec3 tangent_;
        varying vec3 bitangent_;
        varying vec4 color_;
      
        attribute vec4 pos;
        attribute vec3 normal;
        attribute vec3 tangent;
        attribute vec3 bitangent;
        attribute vec2 uv;
        attribute vec4 color;
      
        uniform mat4 modelToProjection;
        uniform mat4 modelToCamera;
      
        void main() {
          uv_ = uv;
          color_ = color;
          normal_ = (modelToCamera * vec4(normal,0)).xyz;
          tangent_ = (modelToCamera * vec4(tangent,0)).xyz;
          bitangent_ = (modelToCamera * vec4(bitangent,0)).xyz;
          gl_Position = modelToProjection * pos;
        }
      );

      const char skinned_vertex_shader[] = SHADER_STR(
        varying vec2 uv_;
        varying vec3 normal_;
        varying vec3 tangent_;
        varying vec3 bitangent_;
      
        attribute vec4 pos;
        attribute vec3 normal;
        attribute vec3 tangent;
        attribute vec3 bitangent;
        attribute vec2 uv;
        attribute vec3 blendweight;
        attribute vec4 blendindices;
      
        uniform mat4 cameraToProjection;
        uniform mat4 modelToCamera[96];
      
        void main() {
          uv_ = uv;
          color_ = color;
          ivec4 index = ivec4(blendindices);
          mat4 m2c0 = modelToCamera[index.x];
          mat4 m2c1 = modelToCamera[index.y];
          mat4 m2c2 = modelToCamera[index.z];
          mat4 m2c3 = modelToCamera[index.w];
          float blend0 = 1.0 - blendweight.x - blendweight.y - blendweight.z;
          mat4 blendedModelToCamera = m2c0 * blend0 + m2c1 * blendweight.x + m2c2 * blendweight.y + m2c3 * blendweight.z;
          normal_ = normalize((blendedModelToCamera * vec4(normal,0)).xyz);
          tangent_ = normalize((blendedModelToCamera * vec4(tangent,0)).xyz);
          bitangent_ = normalize((blendedModelToCamera * vec4(bitangent,0)).xyz);
          gl_Position = cameraToProjection * (blendedModelToCamera * pos);
        }
      );

      const char fragment_shader[] = SHADER_STR(
        const int max_lights = 4;
        varying vec2 uv_;
        varying vec3 normal_;
        varying vec4 color_;
        varying vec3 tangent_;
        varying vec3 bitangent_;

        uniform vec4 light_uniforms[1+max_lights*4];
        uniform int num_lights;
        uniform sampler2D samplers[6];
      
        void main() {
          float shininess = color_.x * 255.0;
          vec3 bump = normalize(vec3(color_.xy-vec2(0.5, 0.5), 1));
          vec3 nnormal = normal_; //normalize(bump.x * tangent_ + bump.y * bitangent_ + bump.z * normal_);
          vec3 diffuse_light = vec3(0.3, 0.3, 0.3);
          vec3 specular_light = vec3(0, 0, 0);

          for (int i = 0; i != num_lights; ++i) {
            vec3 light_direction = light_uniforms[i * 4 + 2].xyz;
            vec3 light_color = light_uniforms[i * 4 + 3].xyz;
            vec3 half_direction = normalize(light_direction + vec3(0, 0, 1));

            float diffuse_factor = max(dot(light_direction, nnormal), 0.0);
            float specular_factor = pow(max(dot(half_direction, nnormal), 0.0), shininess) * diffuse_factor;

            diffuse_light += diffuse_factor * light_color;
            specular_light += specular_factor * light_color;
          }

          vec4 diffuse = color_;
          vec4 ambient = color_;
          vec4 emission = color_;
          vec4 specular = color_;

          vec3 ambient_light = light_uniforms[0].xyz;

          gl_FragColor = color_;

          gl_FragColor.xyz = 
            ambient_light * ambient.xyz +
            diffuse_light * diffuse.xyz +
            emission.xyz +
            specular_light * specular.xyz
          ;
          gl_FragColor.w = diffuse.w;

        }
      );

      init_uniforms(is_skinned ? skinned_vertex_shader : vertex_shader, fragment_shader);
    }

    void render(const mat4t &modelToProjection, const mat4t &modelToCamera, const vec4 *light_uniforms, int num_light_uniforms, int num_lights) {

      shader::render();

      glUniformMatrix4fv(modelToProjection_index, 1, GL_FALSE, modelToProjection.get());
      glUniformMatrix4fv(modelToCamera_index, 1, GL_FALSE, modelToCamera.get());

      glUniform4fv(light_uniforms_index, num_light_uniforms, (float*)light_uniforms);
      glUniform1i(num_lights_index, num_lights);

      static const GLint samplers[] = { 0, 1, 2, 3, 4, 5 };
      glUniform1iv(samplers_index, 6, samplers);
    }

    void render_skinned(const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_matrices, const vec4 *light_uniforms, int num_light_uniforms, int num_lights) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniformMatrix4fv(cameraToProjection_index, 1, GL_FALSE, cameraToProjection.get());
      glUniformMatrix4fv(modelToCamera_index, num_matrices, GL_FALSE, (float*)modelToCamera);

      glUniform4fv(light_uniforms_index, num_light_uniforms, (float*)light_uniforms);
      glUniform1i(num_lights_index, num_lights);

      // we use textures 0-3 for material properties.
      static const GLint samplers[] = { 0, 1, 2, 3, 4 };
      glUniform1iv(samplers_index, 5, samplers);
    }
  };
}
