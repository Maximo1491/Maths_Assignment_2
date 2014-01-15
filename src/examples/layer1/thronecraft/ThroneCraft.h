namespace octet
{
	class ThroneCraft_app : public app
	{
	public:

		color_shader color_shader_;
		ui_shader ui_shader_;

    int windowWidth, windowHeight;
		superChunk* c;

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;

		glm::vec3 position;
		glm::vec2 angles;

		int numOfLights;
		glm::vec4 light_information[8];
		glm::vec4 light_ambient;
		glm::vec4 light_diffuse;
    float light_angle;

		uint8_t selectedBlock;
    float current_block_position;
		bool gravity;

		int chunksX, chunksY, chunksZ;

    enum{
      empty,
      grass,
      dirt,
      stone,
      wood,
      leaves,
      snow,
      sand,
      brick,

      numTypes,
    };

    glm::vec4 blockColors[numTypes];

		enum{
      ui_empty,
			ui_grass_block,
      ui_dirt_block,
      ui_stone_block,
      ui_wood_block,
      ui_leaves_block,
      ui_snow_block,
      ui_sand_block,
      ui_brick_block,
      ui_hotbar_background,
			current_block_background,
      crosshair_left,
      crosshair_right,
      crosshair_top,
      crosshair_bottom,

			num_elements,
		};

		ui_element ui[num_elements];

		ThroneCraft_app(int argc, char **argv) : app(argc, argv) { }

		void app_init()
		{
			//Initialise our colour shader
			color_shader_.init();
			ui_shader_.init();

			//player position & gravity
			position = glm::vec3(176, 60, 176);
			gravity = false;

			//Get window width and height for UI elements
			get_viewport_size(windowWidth, windowHeight);

      blockColors[grass] = glm::vec4(0.160, 0.570, 0.168, 1);
      blockColors[dirt] = glm::vec4(0.6, 0.3, 0, 1);
      blockColors[stone] = glm::vec4(0.5, 0.5, 0.5, 1);
      blockColors[wood] = glm::vec4(0.4, 0.2, 0, 1);
      blockColors[leaves] = glm::vec4(0.06, 0.25, 0.06, 1);
      blockColors[snow] = glm::vec4(0.75, 0.75, 0.75, 1);
      blockColors[sand] = glm::vec4(0.841, 0.865, 0.353, 1);
      blockColors[brick] = glm::vec4(0.948, 0.254, 0.254, 1);

      float hotbar_width = 60 * 8 + 10;
      current_block_position = (windowWidth*0.5f) - (hotbar_width*0.5f) + 5;
      ui[ui_hotbar_background].init((windowWidth*0.5f) - (hotbar_width*0.5f), windowHeight - 80, hotbar_width, 70, windowWidth, windowHeight);
      ui[current_block_background].init(current_block_position, windowHeight - 75, 60, 60, windowWidth, windowHeight);

			//Create the UI elements
      float inventoryX = ((windowWidth*0.5f) - (hotbar_width*0.5f)) + 10;
      for(int i = ui_grass_block; i <= ui_brick_block; i++)
      {
			  ui[i].init(inventoryX, windowHeight - 70, 50, 50, windowWidth, windowHeight);
        inventoryX += 60;
      }

      ui[crosshair_left].init((windowWidth*0.5f) - 6, (windowHeight*0.5f) - 2, 5, 4, windowWidth, windowHeight);
      ui[crosshair_right].init((windowWidth*0.5f) + 1, (windowHeight*0.5f) - 2, 5, 4, windowWidth, windowHeight);
      ui[crosshair_top].init((windowWidth*0.5f) - 2, (windowHeight*0.5f) - 6, 4, 5, windowWidth, windowHeight);
      ui[crosshair_bottom].init((windowWidth*0.5f) - 2, (windowHeight*0.5f) + 1, 4, 5, windowWidth, windowHeight);

			//Number of Lights
			numOfLights = 1;
			
			//Ambient Light
			light_ambient = glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f );
			
			//Diffuse Light
			light_diffuse = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );

			//Light 1
			light_information[0] = glm::vec4( 1.0f, 2.0f, 3.0f, 0.0f );

      light_angle = 0.0f;

			//Light 2
			//light_information[1] = glm::vec4( -2.0f, -1.0f, -4.0f, 0.0f );

			projection = glm::perspective(45.0f, 1.0f*windowWidth/windowHeight, 0.1f, 256.0f);
			
			c = new superChunk();

			int i = 1;
			bool downhill = false;

			//Start by selecting a grass block
			selectedBlock = grass;

			FILE* file = fopen("../../assets/thronecraft/saveFile.txt", "r");

			//Tell the progam how many chunks we want.
			if (file == NULL)
			{
				chunksX = SCX;
				chunksY = SCY;
				chunksZ = SCZ;

				//Loops through 1 chunk and creates either a type 1 or type 2,
				//uncomment the SCX stuff to fill the whole superChunk
				for(int x = 0; x < CX * chunksX/* *SCX */; x++)
				{
					for(int y = 0; y < CY * chunksY/2/* *SCY */; y++)
					{
						 //if(y < i)
						 {
							 for(int z = 0; z < CZ * chunksZ/* *SCZ */; z++)
							 {
								 if(y == 31)
									 c->set(x,y,z, 1);
								 if(y <= 30 && y > 27)
									 c->set(x,y,z, 2);
								 if(y <= 27)
									 c->set(x,y,z, 3);
							 }
						}
					}
				}
			}

			else
			{
				fscanf(file, "%i", &chunksX);
				fscanf(file, "%i", &chunksY);
				fscanf(file, "%i", &chunksZ);

				int intScan;

				for (int x = 0; x < CX * chunksX; x++)
				{
					for (int y = 0; y < CY * chunksY; y++)
					{
						for (int z = 0; z < CZ * chunksZ; z++)
						{
							fscanf(file, "%i", &intScan);
							c->set(x, y, z, intScan);
						}
					}
				}
			}
		}

		void SaveChunks()
		{
			FILE* file = fopen("../../assets/thronecraft/saveFile.txt", "w");

			fprintf(file, "%i %i %i\n", chunksX, chunksY, chunksZ);

			for (int x = 0; x < CX * chunksX; x++)
			{
				for (int y = 0; y < CY * chunksY; y++)
				{
					for (int z = 0; z < CZ * chunksZ; z++)
					{
						fprintf(file, "%i ", c->get(x, y, z));
					}

					fprintf(file, "\n");
				}
			}

			fclose(file);
		}

		//mouse movement
		void motion(int x, int y, HWND* w)
		{
			static bool wrap = false;

			if(!wrap)
			{
				int ww, wh;
				get_viewport_size(ww, wh);

				int dx = x - ww/2;
				int dy = y - wh/2;

				const float mouseSpeed = -0.001f;

				angles.x += dx * mouseSpeed;
				angles.y += dy * mouseSpeed;

				if(angles.x < -3.14)
					angles.x += 3.14*2;
				else if (angles.x > 3.14)
					angles.x -= 3.14*2;

				if(angles.y < -3.14 / 2)
					angles.y = -3.14 / 2;
				else if(angles.y > 3.14 / 2)
					angles.y = 3.14 / 2;

				wrap = true;

				tagPOINT p;
				p.x = ww/2;
				p.y = wh/2;
				ClientToScreen(*w, &p);
				SetCursorPos(p.x, p.y);
				} else {
					wrap = false;
				}
		}

		float dti(float val) {
			return fabsf(val - glm::round(val));
		}

		//mouse controls
		void mouse_controls(int width, int height)
		{
			if(is_key_down(key_rmb))
			{
				set_key(key_rmb, false);

				GLfloat depth;

				glReadPixels(width * 0.5, height * 0.5, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
				glm::vec4 viewport = glm::vec4(0, 0, width, height);
				glm::vec3 wincoord = glm::vec3(width * 0.5, height * 0.5, depth);
				glm::vec3 objcoord = glm::unProject(wincoord, view, projection, viewport);
				if (objcoord.x < 0)
					objcoord.x = 0;
				if (objcoord.y < 0)
					objcoord.y = 0;
				if (objcoord.z < 0)
					objcoord.z = 0;

				if (dti(objcoord.x) < 0.01f && dti(objcoord.x) > -0.01f)
					objcoord.x = glm::round(objcoord.x);
				if (dti(objcoord.y) < 0.01f && dti(objcoord.y) > -0.01f)
					objcoord.y = glm::round(objcoord.y);
				if (dti(objcoord.z) < 0.01f && dti(objcoord.z) > -0.01f)
					objcoord.z = glm::round(objcoord.z);

				float nx = objcoord.x;
				float ny = objcoord.y;
				float nz = objcoord.z;

				if (dti(objcoord.x) < dti(objcoord.y))
				{
					if (dti(objcoord.x) < dti(objcoord.z))
					{
						if (position.x > objcoord.x)
							nx -= 0.1;
						else
							nx += 0.1;
					}
					else
					{
						if (position.z > objcoord.z)
							nz -= 0.1;
						else
							nz += 0.1;
					}
				}
				else
				{
					if (dti(objcoord.y) < dti(objcoord.z))
					{
						if (position.y > objcoord.y)
							ny -= 0.1;
						else
							ny += 0.1;
					}
					else
					{
						if (position.z > objcoord.z)
							nz -= 0.1;
						else
							nz += 0.1;
					}
				}

				int x = glm::floor(nx);
				int y = glm::floor(ny);
				int z = glm::floor(nz);

				glm::vec3 end = glm::vec3(x, y, z);

				if(c->get(end.x, end.y, end.z) != 0)
				{
					c->set(end.x, end.y, end.z, 0);
				}
			}

			if(is_key_down(key_lmb))
			{
				set_key(key_lmb, false);

				GLfloat depth;

				glReadPixels(width * 0.5, height * 0.5, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
				glm::vec4 viewport = glm::vec4(0, 0, width, height);
				glm::vec3 wincoord = glm::vec3(width * 0.5, height * 0.5, depth);
				glm::vec3 objcoord = glm::unProject(wincoord, view, projection, viewport);
				if (objcoord.x < 0)
					objcoord.x = 0;
				if (objcoord.y < 0)
					objcoord.y = 0;
				if (objcoord.z < 0)
					objcoord.z = 0;

				if (dti(objcoord.x) < 0.01f && dti(objcoord.x) > -0.01f)
					objcoord.x = glm::round(objcoord.x);
				if (dti(objcoord.y) < 0.01f && dti(objcoord.y) > -0.01f)
					objcoord.y = glm::round(objcoord.y);
				if (dti(objcoord.z) < 0.01f && dti(objcoord.z) > -0.01f)
					objcoord.z = glm::round(objcoord.z);

				float nx = objcoord.x;
				float ny = objcoord.y;
				float nz = objcoord.z;

				if (dti(objcoord.x) < dti(objcoord.y))
				{
					if (dti(objcoord.x) < dti(objcoord.z))
					{
						if (position.x > objcoord.x)
							nx += 0.1;
						else
							nx -= 0.1;
					}
					else
					{
						if (position.z > objcoord.z)
							nz += 0.1;
						else
							nz -= 0.1;
					}
				}
				else
				{
					if (dti(objcoord.y) < dti(objcoord.z))
					{
						if (position.y > objcoord.y)
							ny += 0.1;
						else
							ny -= 0.1;
					}
					else
					{
						if (position.z > objcoord.z)
							nz += 0.1;
						else
							nz -= 0.1;
					}
				}

				int x = glm::floor(nx);
				int y = glm::floor(ny);
				int z = glm::floor(nz);

				glm::vec3 end = glm::vec3(x, y, z);
			  
				if (x > -1 && x < CX * SCX && y > -1 && y < CY * SCY && z > -1 && z < CZ * SCZ && c->get(end.x, end.y, end.z) == 0)
				{
					c->set(end.x, end.y, end.z, selectedBlock);
				}
			}
		}

		void keyboard_controls()
		{
			const float movespeed = 0.5f;

			glm::vec3 forward_dir = glm::vec3(sinf(angles.x), 0, cosf(angles.x));
			glm::vec3 right_dir = glm::vec3(-forward_dir.z, 0, forward_dir.x);
			glm::vec3 temp_w;
			glm::vec3 temp_a;
			glm::vec3 temp_s;
			glm::vec3 temp_d;

			if(is_key_down('A'))
				temp_a = position - (right_dir * (movespeed * 3));
				if(c->get(glm::floor(temp_a.x), glm::floor(temp_a.y), glm::floor(temp_a.z)) == 0){
					position -= right_dir * movespeed;
				}
			if(is_key_down('D'))
				temp_d = position + (right_dir * (movespeed * 3));
				if(c->get(glm::floor(temp_d.x), glm::floor(temp_d.y), glm::floor(temp_d.z)) == 0){
					position += right_dir * movespeed;
				}
			if(is_key_down('W'))
				temp_w = position + (forward_dir * (movespeed * 3));
				if(c->get(glm::floor(temp_w.x), glm::floor(temp_w.y), glm::floor(temp_w.z)) == 0){
					position += forward_dir * movespeed;
				}
			if(is_key_down('S'))
				temp_s = position - (forward_dir * (movespeed * 3));
				if(c->get(glm::floor(temp_s.x), glm::floor(temp_s.y), glm::floor(temp_s.z)) == 0){
					position -= forward_dir * movespeed;
				}
			if(is_key_down('Q'))
				position.y += movespeed;
			if(is_key_down('E'))
				position.y -= movespeed;
			if (is_key_down('P'))
			{
				set_key('P', false);
				SaveChunks();
			}
			if(is_key_down('O'))
			{
				set_key('O', false);
				if (c->get(0, 0, 0) == 3)
				{
					for (int i = 0; i < CZ; i++)
						c->set(0, 0, i, 0);
				}

				else
				{
					for (int i = 0; i < CZ; i++)
						c->set(0, 0, i, 3);
				}
			}

			if (is_key_down('1')) // grass
			{
				set_key('1', false);
        ui[current_block_background].setPos(current_block_position + ((grass-1) * 60), windowHeight - 75);
				selectedBlock = grass;
			}
			if (is_key_down('2')) // dirt
			{
				set_key('2', false);
        ui[current_block_background].setPos(current_block_position + ((dirt-1) * 60), windowHeight - 75);
				selectedBlock = dirt;
			}
			if (is_key_down('3')) // stone
			{
				set_key('3', false);
        ui[current_block_background].setPos(current_block_position + ((stone-1) * 60), windowHeight - 75);
				selectedBlock = stone;
			}
			if (is_key_down('4')) // wood
			{
				set_key('4', false);
        ui[current_block_background].setPos(current_block_position + ((wood-1) * 60), windowHeight - 75);
				selectedBlock = wood;
			}
			if (is_key_down('5')) // leaves
			{
				set_key('5', false);
        ui[current_block_background].setPos(current_block_position + ((leaves-1) * 60), windowHeight - 75);
				selectedBlock = leaves;
			}
			if (is_key_down('6')) // snow
			{
				set_key('6', false);
        ui[current_block_background].setPos(current_block_position + ((snow-1) * 60), windowHeight - 75);
				selectedBlock = snow;
			}
      if (is_key_down('7')) // sand
			{
				set_key('7', false);
        ui[current_block_background].setPos(current_block_position + ((sand-1) * 60), windowHeight - 75);
				selectedBlock = sand;
			}
      if (is_key_down('8')) // brick
			{
				set_key('8', false);
        ui[current_block_background].setPos(current_block_position + ((brick-1) * 60), windowHeight - 75);
				selectedBlock = brick;
			}
			if(is_key_down(key_backspace)){
				gravity = !gravity;
				set_key(key_backspace, false);
			}

			glm::vec3 lookat;
			lookat.x = sinf(angles.x) * cosf(angles.y);
			lookat.y = sinf(angles.y);
			lookat.z = cosf(angles.x) * cosf(angles.y);

			view = glm::lookAt(position, position + lookat, glm::vec3(0, 1, 0));
		}

		void update_player()
		{
			if (c->get(glm::floor(position.x), glm::floor(position.y - 1), glm::floor(position.z)) == 0 && gravity == true){
				position.y -= 0.6f;
			}
		}

		void draw_world(int x, int y, int width, int height, int windowX, int windowY)
		{
			//Sets our viewpoint
			glViewport(x, y, width, height);
			glClearColor(0.25f, 0.95f, 1.0f, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (is_key_down(key_space))
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			else
				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

			if (is_key_down(27))
			{
				SaveChunks();
				exit(1);
			}

      light_information[0] = glm::vec4(sin(light_angle * 0.0174532925f), cos(light_angle * 0.0174532925f), 0.0f, 0.0f);
      light_angle += 1.0f;

			//Draws our super chunk
			c->render(model, view, projection, numOfLights, light_information, light_diffuse, light_ambient, color_shader_);
      
      for(int i = ui_grass_block; i <= ui_brick_block; i++)
        ui[i].render(ui_shader_, blockColors[i]);
      ui[current_block_background].render(ui_shader_, glm::vec4(1, 0, 0.6, 1));
			ui[ui_hotbar_background].render(ui_shader_, glm::vec4(0, 0, 0, 1));

      for(int i = crosshair_left; i <= crosshair_bottom; i++)
        ui[i].render(ui_shader_, glm::vec4(0,0,0,1));

			keyboard_controls();
			mouse_controls(width, height);
			update_player();
		}
	};
}
