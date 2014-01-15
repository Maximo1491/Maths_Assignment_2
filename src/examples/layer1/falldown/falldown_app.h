namespace octet 
{
	class mySprite_2d {
    // Box2d body information for the sprite.
    b2Body *body;

    // The texture for the sprite
    int texture;

    // The texture uv are derived fromn positions with these scales.
    float uscale;
    float vscale;
    float uoffset;
    float voffset;

    // true if this mySprite_2d is enabled.
    bool enabled;

		// used to tell the shader if it is a player or not
		float player;
  public:
    mySprite_2d(
      b2World &world, b2BodyDef &body_def, b2FixtureDef &fixture_def,
      int _texture, float uscale_ = 1.0f, float vscale_ = 1.0f, float uoffset_ = 0.5f, float voffset_ = 0.5f, float player_ = 0.0f
    ) {
      texture = 0;
      enabled = true;
			player = player_;
      uscale = uscale_;
      vscale = vscale_;
      uoffset = uoffset_;
      voffset = voffset_;

      body = world.CreateBody(&body_def);
      body->CreateFixture(&fixture_def);

      texture = _texture;
      enabled = true;
    }

		// Returns the box2d body of the sprite, used for moving objects around
		b2Body* getBody()
		{
			return body;
		}

    void render(my_shader &shader, mat4t &cameraToWorld, vec4 c) {
      if (!texture) return;

      mat4t modelToWorld;
      modelToWorld.loadIdentity();
      modelToWorld.translate(body->GetPosition().x, body->GetPosition().y, 0);
      modelToWorld.rotateZ(body->GetAngle() * (180/3.14159265f));

      // builds a projection matrix
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      // set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      shader.render(modelToProjection, 0, c);

      b2Fixture *fixture_def = body->GetFixtureList();
      b2Shape *shape = fixture_def->GetShape();

      float vertices[b2_maxPolygonVertices*5];
      int count = 0;

			// If the shape is a polygon then set the vertices, the texture coords and if it is the player or not
      if (shape->GetType() == b2Shape::e_polygon) {
        b2PolygonShape *poly = (b2PolygonShape *)shape;
     		count = poly->GetVertexCount();

		    float *p = vertices;
		    for (int i = 0; i != count; ++i) {
		      b2Vec2 vtx = poly->GetVertex(i);
		      p[0] = vtx.x;
		      p[1] = vtx.y;
		      p[2] = vtx.x * uscale + uoffset;
		      p[3] = vtx.y * vscale + voffset;
					p[4] = player;
		      p += 5;
		    }
      }

      glVertexAttribPointer(attribute_pos, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)vertices );
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(vertices + 2) );
			glVertexAttribPointer(attribute_player, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(vertices + 4) );
      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_uv);
			glEnableVertexAttribArray(attribute_player);
   
      // finally, draw the sprite as a fan
      glDrawArrays(GL_TRIANGLE_FAN, 0, count);
    }

		//Sets if the sprite is enabled or not
		void setEnabled(bool b)
		{
			enabled = b;
		}

    bool &is_enabled() {
      return enabled;
    }
  };

  class falldown_app : public octet::app, public b2ContactListener 
	{
    // physics systems need a "world" to stitch together rigid bodies.
    b2World box2d_world;

    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // Defines the shader
    my_shader texture_shader_;

    // keeps the game state
    bool game_over;

    // big array of sprites
    dynarray<mySprite_2d*> sprites;

		//Player color
		vec4 color;

		// the game objects
		mySprite_2d* player;
		mySprite_2d* platforms[10];
		mySprite_2d* walls[3];
		mySprite_2d* backgrounds[2];
		mySprite_2d* loseImage;
		unsigned int numWalls;
		unsigned int numPlats;
		unsigned int numBackgrounds;
		unsigned int score;
		unsigned int timer;
		//The position of the gap in the platforms
		float platPos;

  public:
    // this is called when we construct the class
    falldown_app(int argc, char **argv) : app(argc, argv), box2d_world(b2Vec2(0, -9.8f)) 
		{
			
    }

    // this is called once OpenGL is initialized
    void app_init() 
		{

      // set up the shader
      texture_shader_.init();

      // set up the matrices with a camera 5 units from the origin
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 5);

			//Used to keep increase platform movement speed after a certain amount
			timer = 0;

      GLuint blockT = resources::get_texture_handle(GL_RGBA, "assets/falldown/brick.gif");
      GLuint groundT = resources::get_texture_handle(GL_RGBA, "assets/falldown/ground.gif");
			GLuint backgroundT = resources::get_texture_handle(GL_RGBA, "assets/falldown/background.gif");
			GLuint loseT = resources::get_texture_handle(GL_RGBA, "assets/falldown/lose.gif");

			numPlats = sizeof(platforms)/sizeof(mySprite_2d*);
			numWalls = sizeof(walls)/sizeof(mySprite_2d*);
			numBackgrounds = sizeof(backgrounds)/sizeof(mySprite_2d*);
			score = 0;

			srand(time(NULL));

      b2PolygonShape poly;

      b2BodyDef body_def;
      body_def.position.Set(0, 0);
      body_def.angle = 0;
  	  body_def.type = b2_dynamicBody;

      b2FixtureDef fixture_def;
      fixture_def.shape = &poly;
			fixture_def.density = 10;

			//Sets 2 backgrounds that move at the same speed as the platforms to give the illusion of a scrolling background
			poly.SetAsBox(5.0f, 5.0f);
			body_def.type = b2_kinematicBody;
			body_def.position.Set(0.0f, 0.0f);
			//Makes the backgrounds non-collidable
			fixture_def.isSensor = true;
			backgrounds[0] = new mySprite_2d(box2d_world, body_def, fixture_def, backgroundT, 0.2f, 0.2f);
			backgrounds[0]->getBody()->SetLinearVelocity(b2Vec2(0, 1));
			sprites.push_back(backgrounds[0]);
			body_def.position.Set(0.0f, -5.0f);
			backgrounds[1] = new mySprite_2d(box2d_world, body_def, fixture_def, backgroundT, 0.2f, 0.2f);
			backgrounds[1]->getBody()->SetLinearVelocity(b2Vec2(0, 1));
			sprites.push_back(backgrounds[1]);

			//Sets walls at the edges of the screen so that the player cannot fall off the edges or bottom of the screen
			poly.SetAsBox(10.0f, 0.2f);
			body_def.type = b2_staticBody;
			body_def.angle = 90*(3.14/180);
			body_def.position.Set(-5.2f, 0.0f);
			fixture_def.friction = 0.6f;
			fixture_def.isSensor = false;
			walls[0] = new mySprite_2d(box2d_world, body_def, fixture_def, blockT);
			sprites.push_back(walls[0]);
			body_def.position.Set(5.2f, 0.0f);
			walls[1] = new mySprite_2d(box2d_world, body_def, fixture_def, blockT);
			sprites.push_back(walls[1]);
			body_def.angle = 0;
			body_def.position.Set(0.0f, -5.2f);
			walls[2] = new mySprite_2d(box2d_world, body_def, fixture_def, blockT);
			sprites.push_back(walls[2]);

			//Sets the platforms as 2 rectangles per platform line with a gap of 2 set to a random point along the line
      poly.SetAsBox(5.0f, 0.2f);
  	  body_def.type = b2_kinematicBody;
      body_def.angle = 0;
			for(int i = 0; i < numPlats; i+=2)
			{
				platPos = ((rand() % 500)*0.01) - 2.5f;
				for(int j = 0; j < 2; j++)
				{
					body_def.position.Set((platPos - 6.0f) + (j*12.0f), (-2*i));
					platforms[i+j] = new mySprite_2d(box2d_world, body_def, fixture_def, groundT, 1, 1, 0.8, 0.8);
					platforms[i+j]->getBody()->SetLinearVelocity(b2Vec2(0, 1));
					sprites.push_back(platforms[i+j]);
				}
			}

			//Sets the player
			poly.SetAsBox(0.25f, 0.25f);
      body_def.position.Set(0.0f, 0.0f);
			fixture_def.friction = 0.1f;
			body_def.type = b2_dynamicBody;
      body_def.angle = 0;
			player = new mySprite_2d(box2d_world, body_def, fixture_def, blockT, 2, 2, 0.5, 0.5, 1.0f);
      sprites.push_back( player );

			//Sets the Game Over image for when the player goes off the top of the screen
			poly.SetAsBox(1, 1);
			fixture_def.isSensor = true;
			body_def.type = b2_staticBody;
			loseImage = new mySprite_2d(box2d_world, body_def, fixture_def, loseT, 0.5f, 0.5f);
			loseImage->setEnabled(false);
			sprites.push_back( loseImage );

      game_over = false;
    }

		//Runs this function when the game ends
		void gameOver()
		{
			game_over = true;
			loseImage->setEnabled(true);
		}

    // called every frame to move things
    void simulate() 
		{
			//If the player is above the top of the screen
			if(player->getBody()->GetPosition().y > 5.25)
			{
				gameOver();
			}
			//If the game is over return
      if (game_over) 
			{
        return;
      }

			//Sets a random colour for the player
			float x = (rand() % 100)*0.01;
			float y = (rand() % 100)*0.01;
			float z = (rand() % 100)*0.01;
			color = vec4(x,y,z,1);

			//Simulates the box2d world
      box2d_world.Step(1.0f/30, 6, 2);

			//Increases the timer
			timer++;

			//If the timer is over 500, reset the timer and increase the speed of the platforms by 0.5
			if(timer > 500)
			{
				for(int i = 0; i < numPlats; i++)
				{
					platforms[i]->getBody()->SetLinearVelocity(b2Vec2(0, platforms[i]->getBody()->GetLinearVelocity().y + 0.5f));
				}
				for(int i = 0; i < numBackgrounds; i++)
				{
					backgrounds[i]->getBody()->SetLinearVelocity(b2Vec2(0, backgrounds[i]->getBody()->GetLinearVelocity().y + 0.5f));
				}
				timer = 0;
			}

			//If the player is pressing left then move the player left
			if(is_key_down(key_left))
			{
				player->getBody()->ApplyLinearImpulse(b2Vec2(-0.5, 0), player->getBody()->GetWorldCenter());
			}

			//If the player is pressing right then move the player right
			if(is_key_down(key_right))
			{
				player->getBody()->ApplyLinearImpulse(b2Vec2(0.5, 0), player->getBody()->GetWorldCenter());
			}

			//If the platform is off the screen then move the platform below the current set of platforms and set a new random gap
			for(int i = 0; i < numPlats; i+=2)
			{
				platPos = ((rand() % 500)*0.01) - 2.5f;
				for(int j = 0; j < 2; j++)
				{
					if(platforms[i+j]->getBody()->GetPosition().y > 6)
					{
						platforms[i+j]->getBody()->SetTransform(b2Vec2((platPos - 6.0f) + (j*12.0f), -14), 0);
						//Increase the score that the player recieves
						score++;
					}
				}
			}

			//If the background is off the screen then move it back down the bottom
			for(int i = 0; i < numBackgrounds; i++)
			{
				if(backgrounds[i]->getBody()->GetPosition().y > 5)
				{
					backgrounds[i]->getBody()->SetTransform(b2Vec2(0, -5.0f), 0);
				}
			}
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) 
		{
      simulate();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glDisable(GL_DEPTH_TEST);

      // allow alpha blend (transparency when alpha channel is 0)
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // draw all the sprites
      for (int i = 0; i != sprites.size(); ++i) 
			{
				if(sprites[i]->is_enabled())
				{
					sprites[i]->render(texture_shader_, cameraToWorld, color);
				}
      }

      // move the listener with the camera
      vec4 &cpos = cameraToWorld.w();
      alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
    }
  };
}
