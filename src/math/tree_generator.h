namespace octet
{

  class tree_generator
	{
	private:
		//Create a structure to hold the information for
		//the branches in our tree
		struct TreeBox
		{
			int branchIndex;
			int parentBranchIndex;
			float branchAngleX;
			float branchAngleZ;
			float parentHeightAtBase;
			float branchHeight;
			float radiusOffset;
			mat4t modelToWorld;
			vec4 branchStartPoint;
		};

		//Declare all variables here
		int iterations, maxIterations, ignoreSize, numberOfBranches, branchSubSections;
		float height, width, depth, branchRadius, angle, leafWidth;
		string originalAxiom, ignore;
		bool isKeyPressed, cameraReset;

		dynarray<std::string> rules;
		dynarray<TreeBox*> formedTree;
		dynarray<GLfloat> vertices;
		dynarray<GLushort> indices, texCoords;
		std::vector<char> formula, newFormula;

		GLuint vbo, lbo, ibo, cbo, tbo, program, *textures;
		GLint attribute_position, attribute_v_color, attribute_tex, uniform_matrix;

	public:

		//Enumeration to tell what kind of tree to make.
		enum
		{
			alive,
			dying,
			dead,
		};
		
		tree_generator() 
		{
			srand((unsigned) time(0));

			//Set the size of the branches for our tree.
			height = 1.0f;
			width = 0.10f;
			depth = 0.10f;
			leafWidth = 10 * width;
			branchRadius = 0.05f;
			branchSubSections = 8;

			//Load the necessary buffers to be used.
			//vbo shall be used for vertices.
			//ibo shall be used for indices.
			//cbo shall be used to store colors.
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &ibo);
			glGenBuffers(1, &cbo);
			glGenBuffers(1, &tbo);
			glGenBuffers(1, &lbo);

			textures = new GLuint[2];

			//Generate a buffer for the bark texture
			glGenTextures(2, textures);
			glBindTexture(GL_TEXTURE_2D, textures[0]);

			int width, height;
			unsigned char* image = SOIL_load_image("../../assets/thronecraft/treebark.jpg", &width, &height, 0, SOIL_LOAD_RGB);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

			SOIL_free_image_data(image);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, textures[1]);

			image = SOIL_load_image("../../assets/thronecraft/bigtreeleafd.png", &width, &height, 0, SOIL_LOAD_RGBA);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

			SOIL_free_image_data(image);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//glGenerateMipmap(GL_TEXTURE_2D);

			//Read the initial file.  This will trigger a domino
			//effect where the entire tree is made from this file.
			ReadFile("../../assets/thronecraft/Stochastic_Sample.txt");
		}

		~tree_generator()
		{
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ibo);
			glDeleteBuffers(1, &cbo);
			glDeleteBuffers(1, &tbo);
			glDeleteBuffers(1, &lbo);
		}

		void render(glm::mat4 projection, tree_shader &tree_shader_)
		{
			tree_shader_.render(projection);

			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_uv);
			glEnableVertexAttribArray(attribute_color);

			glBindTexture(GL_TEXTURE_2D, textures[0]);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glVertexAttribPointer(attribute_pos, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

			glBindBuffer(GL_ARRAY_BUFFER, cbo);
			glVertexAttribPointer(attribute_color, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

			glBindBuffer(GL_ARRAY_BUFFER, tbo);
			glVertexAttribPointer(attribute_uv, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(GLushort) * 2, 0);

			//Bind the ibo to the element array buffer.
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

			//Get the size of the ibo buffer.  This will be
			//how many bytes the ibo buffer currently is.
			int size;
			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

			//Draw the indices into the window.
			glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
		}

		//A command to generate another tree.
		void AddTree(int treeType, glm::mat4 origin)
		{
			FormFormula(origin);
		}

		//The ReadFile() function is used to read the outside
		//text file and translate the information into the
		//data variables used for building the L-System.
		void ReadFile(const char* fileName)
		{
			//Declare and initialize the variables needed to
			//read the file.
			originalAxiom = new char;
			char* buffer;
			long bufferSize;
			ignoreSize = 0;

			//Openthe file to be read.
			FILE* file = fopen(fileName, "r");

			//Check to see if the file opened successfully.
			//If not inform the user.
			if (file == NULL)
				printf("File does not exist!\n");

			else
			{
				//Find the end of the file, get the size of the
				//file, and reqind back to the beginning for
				//the reading process.
				fseek(file, 0, SEEK_END);
				bufferSize = ftell(file);
				rewind(file);

				//Set the buffer variable to the number of bufferSize
				//characters.  Read the entire file into the buffer.
				buffer = (char*)malloc(sizeof(char*) * bufferSize);
				fread(buffer, 1, bufferSize, file);

				//Find the end of the first line.
				strtok(buffer, "\n");

				do
				{
					//Check to see if the line refers to the Axiom.
					if (strcmp(buffer, "Axiom") == 0)
					{
						//Find the end of the next line.
						buffer = strtok(NULL, "\n");
						
						//Store the current line into the formula.  This
						//is the axiom of the formula.
						for (unsigned int i = 0; i < strlen(buffer); i++)
						{
							formula.push_back(buffer[i]);
						}

						//Store the axiom into the originalAxiom in case
						//we need to revert the formula back to the axiom.
						originalAxiom = buffer;

						//Read the next line.
						buffer = strtok(NULL, "\n");
					}

					//Check to see if the line refers to a rule.  If it
					//does push the line onto the rules stack.
					else if (strcmp(buffer, "Rule") == 0)
					{
						buffer = strtok(NULL, "\n");
						rules.push_back(buffer);
						buffer = strtok(NULL, "\n");
					}

					//Check to see if the line refers to the angle of
					//the L-System.  If so, convert the line to a float
					//and save it in the angle variable.
					else if (strcmp(buffer, "Angle") == 0)
					{
						buffer = strtok(NULL, "\n");
						angle = (float)atof(buffer);
						buffer = strtok(NULL, "\n");
					}

					//Check to see if the line refers to the iterations of
					//the L-System.  If so convert the line to an integer
					//and save it in the iterations variable.
					else if (strcmp(buffer, "Iterations") == 0)
					{
						buffer = strtok(NULL, "\n");
						iterations = atoi(buffer) + ((rand() % 3) - 1);
						buffer = strtok(NULL, "\n");
					}

					//Check to see if the line refers to the maximum iterations
					//we want this L-System to go to.  If so change the line
					//an integer and store it in the maxIterations variable.
					else if (strcmp(buffer, "Max Iterations") == 0)
					{

						buffer = strtok(NULL, "\n");
						maxIterations = atoi(buffer);
						buffer = strtok(NULL, "\n");
					}

					//Check to see if the line refers to the ignore items.  If
					//so store the line in the ignore variable.
					else if (strcmp(buffer, "Ignore") == 0)
					{
						buffer = strtok(NULL, "\n");
						ignore = buffer;
						buffer = strtok(NULL, "\n");
					}
				}
				//Continue reading lines until the buffer is empty.
				while (buffer != NULL);

				//Delete the buffer to prevent a memory leak and
				//close the file.
				delete buffer;
				fclose(file);
			}
		}

		//This function will form the forumla based on the rules
		//and number of iterations that the L-System is currently
		//set to.
		void FormFormula(glm::mat4 origin)
		{
			//Go through each iteration to form the formula.
			for (int i = 0; i < iterations; i++)
			{
				//Go through each character in the formula checking
				//each to all the rules.
				for (unsigned int j = 0; j < formula.size(); j++)
				{
					//Get a random number for stochastic L-Systems.
					//Keep track of the accumulated percentage in the
					//stochastic L-System.
					//Declare a ruleApplied booleon to see if a
					//rule has been applied to this character in the
					//formula.
					bool ruleApplied = false;
					int random = rand() % 100;
					float accumulatedPercentage = 0.0f;

					//Go through each rule.  Check the character in the formula
					//to the rule.  Apply the rule if the lhs of the rule matches.
					for (unsigned int k = 0; k < rules.size(); k++)
					{
						//Get the lhs of the rule.
						int equalIndex = rules[k].find("=");

						//Check to see if the lhs of the rule is greater than 1
						//and if a rule has already been applied.
						if (rules[k].substr(0, equalIndex).size() > 1 && !ruleApplied)
						{
							//Check to see if this rule is a context sensitive rule.
							if (rules[k][1] == '<' && rules[k][3] == '>' && (unsigned int)j < formula.size())
							{
								std::string previous, base, next;

								int firstInstance, secondInstance;
								int previousIndex = 0;
								int nextIndex = 0;

								bool previousFound = false;
								bool nextFound = false;

								//Find the index in the lhs of the rule of the < and >.
								firstInstance = rules[k].find("<");
								secondInstance = rules[k].find(">");

								//Divide the rule into the previous character, next character,
								//and base character of the lhs of the rule.
								previous = rules[k].substr(0, firstInstance);
								base = rules[k].substr(firstInstance + 1, secondInstance - (firstInstance + 1));
								next = rules[k].substr(secondInstance + 1, equalIndex - (secondInstance + 1));
								
								//Check to see if the character in the formula matches the base.
								//If so continue.
								if (formula[j] == base[0])
								{
									//Check to see if the previous character in the rule is a
									//wild character.  If so set the previousFound to true.
									//If the character being checked is at the beginning of
									//the formula, there is no previous to find so set the
									//previousFound to false.
									if (previous[0] == '*')
										previousFound = true;
									else if (j == 0)
										previousFound = false;
									else
									{
										//Cycle through all the characters before the base in
										//the formula.
										while (j - previousIndex > 0 && !previousFound)
										{
											previousIndex++;
											bool ignoreFound = false;

											//Cycle through the ignore characters.  We want to skip over
											//these characters.
											for (int l = 0; l < ignore.size() && !ignoreFound; l++)
											{
												//Check to see if this previous character is an ignored
												//character.  If so set ignoreFound to true.
												if (formula[j - previousIndex] == ignore[l])
												{
													ignoreFound = true;
												}
											}

											//If no ignore character was found then this is the first
											//character previous the base that we want to check against
											//the previous in the lhs of the rule.
											if (!ignoreFound)
												previousFound = true;
										}
									}

									//Check to see if the next character in the rule is a
									//wild character.  If so set the nextFound to true.
									//If the character being checked is at the beginning of
									//the formula, there is no next to find so set the
									//nextFound to false.
									if (next[0] == '*')
										nextFound = true;
									else if (j + 1 == formula.size())
										nextFound = false;
									else
									{
										//Cycle through all the characters after the base in
										//the formula.
										while ((unsigned int)(j + nextIndex) < formula.size() - 1 && !nextFound)
										{
											nextIndex++;
											bool ignoreFound = false;

											//Cycle through the ignore characters.  We want to skip over
											//these characters.
											for (int l = 0; l < ignore.size() && !ignoreFound; l++)
											{
												//Check to see if this previous character is an ignored
												//character.  If so set ignoreFound to true.
												if (formula[j + nextIndex] == ignore[l])
												{
													ignoreFound = true;
												}
											}

											//If no ignore character was found then this is the first
											//character next of the base that we want to check against
											//the next in the lhs of the rule.
											if (!ignoreFound)
												nextFound = true;
										}
									}
									
									//Check to see if the previous and next character was found to check.  If not,
									//skip to the next character in the formula.
									if (nextFound && previousFound)
									{
										//Check to see if the characters before and after the base match the rule or
										//a combination of wild characters and matches.  If so, apply the rule.
										if ((formula[j - previousIndex] == previous[0] && formula[j + nextIndex] == next[0]) ||
											(formula[j - previousIndex] == previous[0] && next[0] == '*') ||
											(previous[0] == '*' && formula[j + nextIndex] == next[0]) ||
											(previous[0] == '*' && next[0] == '*'))
										{
											//Apply the rule to this character.
											for (unsigned int l = 0; l < rules[k].substr(equalIndex + 1, rules[k].size()).size(); l++)
												newFormula.push_back(rules[k][equalIndex + 1 + l]);

											//Signal that a rule has been applied to this character.
											ruleApplied = true;
										}
									}
								}
							}

							//Check to see if the rule is a stochastic L-System.
							else if (rules[k].find("%") != std::string::npos)
							{
								//Get the percent index, space index after the percentage, and the base
								//of the rule.
								int percentIndex = rules[k].find("%");
								int spaceIndex = rules[k].find(" ");
								std::string base = rules[k].substr(spaceIndex + 1, equalIndex - (spaceIndex + 1));

								//Check to see if the base matches the character being checked in the
								//formula.
								if (base[0] == formula[j])
								{
									//Convert the percentage in the rule to a float.  Store it
									//in the percentChance variable.
									//Add the percentChange to the accumulated percentage.
									float percentChance = (float)atof(rules[k].substr(0, percentIndex).c_str());
									accumulatedPercentage += percentChance;

									//Check to see if the random percentage generated earlier is within the
									//accumulated percentage range.  If so, apply the stochastis rule.
									if ((float)random < accumulatedPercentage)
									{
										for (unsigned int l = 0; l < rules[k].substr(equalIndex + 1, rules[k].size() - (equalIndex + 1)).size(); l++)
											newFormula.push_back(rules[k][equalIndex + 1 + l]);

										//Signal that a rule has been applied to this character.
										ruleApplied = true;
									}
								}
							}
						}
					
						//The L-System is neither stochastic or context-sensitive.
						else
						{
							//Check that the base of the rule matches the character checked.
							//Also check to see if a rule has been applied.
							if (rules[k].substr(0, equalIndex)[0] == formula[j] && !ruleApplied)
							{
								//The base matches the character in the formula.  Apply the rule
								//to the formula.
								for (unsigned int l = 0; l < rules[k].substr(equalIndex + 1, rules[k].size()).size(); l++)
									newFormula.push_back(rules[k][equalIndex + 1 + l]);

								//Signal that a rule has been applied to this character.
								ruleApplied = true;
							}
						}

						//No rules were applied.  The character does not change.
						//Push the character onto the formula being built.
						if (!ruleApplied && k == rules.size() - 1)
						{
							newFormula.push_back(formula[j]);
						}
					}
				}

				//Place the new formula into formula to prepare for the next
				//iteration.  Clear the new formula to build the next iteration
				//formula.
				formula = newFormula;
				newFormula.clear();
			}

			//Execute MakeTree() to form the 3D objects for rendering.
			MakeTree(origin);
		}

		//This funnction will compute the vertices and indices of the tree for drawing.
		void MakeTree(glm::mat4 origin)
		{
			TreeBox* treeBox;
			std::vector<mat4t> positionStack;
			std::vector<int> indexStack;

			treeBox = new TreeBox();
			treeBox->branchHeight = 0;
			treeBox->branchIndex = 0;
			treeBox->parentBranchIndex = -1;
			treeBox->parentHeightAtBase = 0;
			treeBox->branchAngleX = 0.0f;
			treeBox->branchAngleZ = 0.0f;

			treeBox->modelToWorld[0][0] = origin[0].x;
			treeBox->modelToWorld[0][1] = origin[0].y;
			treeBox->modelToWorld[0][2] = origin[0].z;
			treeBox->modelToWorld[0][3] = origin[0].w;
			treeBox->modelToWorld[1][0] = origin[1].x;
			treeBox->modelToWorld[1][1] = origin[1].y;
			treeBox->modelToWorld[1][2] = origin[1].z;
			treeBox->modelToWorld[1][3] = origin[1].w;
			treeBox->modelToWorld[2][0] = origin[2].x;
			treeBox->modelToWorld[2][1] = origin[2].y;
			treeBox->modelToWorld[2][2] = origin[2].z;
			treeBox->modelToWorld[2][3] = origin[2].w;
			treeBox->modelToWorld[3][0] = origin[3].x;
			treeBox->modelToWorld[3][1] = origin[3].y;
			treeBox->modelToWorld[3][2] = origin[3].z;
			treeBox->modelToWorld[3][3] = origin[3].w;

			treeBox->branchStartPoint = treeBox->modelToWorld.row(3);

			mat4t modelToWorld = treeBox->modelToWorld;
			bool formCheck = false;
			int totalTreeHeight = 0;
			float baseAngle = max((float)(rand() % (int)(angle * 100)) / 100, angle / 2);

			//Cycle through the formula checking each character.
			for (unsigned int i = 0; i < formula.size(); i++)
			{
				//If the character is an 'F', we add 1 to the branch height
				//and move the modelToWorld up by one height.  This keeps
				//the modelToWorld halfway up the tree for rendering the
				//box.
				if (formula[i] == 'F')
				{
					vec4 branchVector = modelToWorld.row(3) - treeBox->branchStartPoint;
					float branchMagnitude = sqrt(pow(branchVector[0], 2) + pow(branchVector[1], 2) + pow(branchVector[3], 2));

					if (treeBox->branchHeight <= floor((branchMagnitude / 2.0) + 0.5))
					{
						treeBox->branchHeight++;
						treeBox->modelToWorld.translate(0, height, 0);
						totalTreeHeight++;
					}

					modelToWorld.translate(0, height * 2.0f, 0);
				}

				//If the character is a '[', move the modelToWorld to the end
				//of the branch, save the modeltoWorld on the position stack,
				//and reset the model to world back to its original place.
				else if (formula[i] == '[')
				{
					indexStack.push_back(treeBox->branchIndex);
					positionStack.push_back(modelToWorld);
				}

				//If the character is a ']', check to see if the current
				//branch has a height.  If it does, push the branch onto the
				//formedTree stack.  Create a new branch and set the modelToWorld
				//to the position on the back of the position stack.
				//Pop the modelToWorld on the back of the position stack.
				else if (formula[i] == ']')
				{
					if (treeBox->branchIndex >= (int)formedTree.size())
					{
						treeBox->branchIndex = formedTree.size();
						formedTree.push_back(treeBox);
					}

					else
						formedTree[treeBox->branchIndex] = treeBox;


					treeBox = formedTree[indexStack.back()];
					modelToWorld = positionStack.back();

					indexStack.pop_back();
					positionStack.pop_back();
				}

				//If the character is '-' or '+', check to see if the current branch
				//has a height greater than 0.  If so push the branch onto the back
				//of the formedTree stack.  Move the modelToWorld to the end of the
				//current branch as well.  Create a new branch and rotate the
				//modelToWorld on the Z axis by the angle.
 				else if ((formCheck = formula[i] == '-') || formula[i] == '+')
				{
					if (treeBox->branchIndex >= (int)formedTree.size())
						formedTree.push_back(treeBox);
					else
						formedTree[treeBox->branchIndex] = treeBox;
					
					int parentIndex = treeBox->branchIndex;
					float parentHeight = treeBox->branchHeight;
					float parentAngleX = treeBox->branchAngleX;
					float parentAngleZ = treeBox->branchAngleZ;

					treeBox = new TreeBox();
					treeBox->branchHeight = 0;
					treeBox->branchIndex = formedTree.size();
					treeBox->parentBranchIndex = parentIndex;
					treeBox->parentHeightAtBase = parentHeight;
					
					float derivedAngle = baseAngle + (float)(rand() % 500) / 100;

					if (derivedAngle > angle)
						derivedAngle -= angle;
					
					treeBox->branchAngleX = baseAngle + (float)(rand() % 500) / 100;
					treeBox->branchAngleZ = (float)(rand() % 36000) / 100;
					
					if (formCheck)
					{
						modelToWorld.rotateY(-(treeBox->branchAngleZ));
						modelToWorld.rotateZ(-(treeBox->branchAngleX));
					}
					else
					{
						modelToWorld.rotateY(treeBox->branchAngleZ);
						modelToWorld.rotateZ(treeBox->branchAngleX);
					}

					mat4t swap = modelToWorld;
					treeBox->modelToWorld = swap;
					treeBox->branchStartPoint = treeBox->modelToWorld.row(3);
				}

				//If the character is '\' or '/', check to see if the current branch
				//has a height greater than 0.  If so push the branch onto the back
				//of the formedTree stack.  Move the modelToWorld to the end of the
				//current branch as well.  Create a new branch and rotate the
				//modelToWorld on the X axis by the angle.
				else if ((formCheck = formula[i] == '\\') || formula[i] == '/')
				{	
					if (treeBox->branchIndex >= (int)formedTree.size())
						formedTree.push_back(treeBox);
					else
						formedTree[treeBox->branchIndex] = treeBox;

					int parentIndex = treeBox->branchIndex;
					float parentHeight = treeBox->branchHeight;
					float parentAngleX = treeBox->branchAngleX;
					float parentAngleZ = treeBox->branchAngleZ;
					
					treeBox = new TreeBox();
					treeBox->branchHeight = 0;
					treeBox->branchIndex = formedTree.size();
					treeBox->parentBranchIndex = parentIndex;
					treeBox->parentHeightAtBase = parentHeight;

					treeBox->branchAngleX = abs(parentAngleX) + (((float)(rand() % 1000) / 1000.0f) * 33);
					treeBox->branchAngleZ = abs(parentAngleZ) + (((float)(rand() % 1000) / 1000.0f) * 33);
					
					if (formCheck)
					{
						modelToWorld.rotateZ(-(treeBox->branchAngleZ));
						modelToWorld.rotateX(-(treeBox->branchAngleX));
					}
					else
					{
						modelToWorld.rotateZ(treeBox->branchAngleZ);
						modelToWorld.rotateX(treeBox->branchAngleX);
					}

					mat4t swap = modelToWorld;
					treeBox->modelToWorld = swap;
					treeBox->branchStartPoint = treeBox->modelToWorld.row(3);
				}

				//Check to see if this is the last character in the formula.
				//If so and the branch height is greater than 0, push the branch
				//onto the formedTree stack.
				if (treeBox->branchHeight > 0 && i + 1 == formula.size())
				{
					if (treeBox->branchIndex >= (int)formedTree.size())
					{
						treeBox->branchIndex = formedTree.size();
						formedTree.push_back(treeBox);
					}

					else
						formedTree[treeBox->branchIndex] = treeBox;
				}
			}

			//Set the modelToWorld back to the identity.
			modelToWorld[0][0] = origin[0].x;
			modelToWorld[0][1] = origin[0].y;
			modelToWorld[0][2] = origin[0].z;
			modelToWorld[0][3] = origin[0].w;
			modelToWorld[1][0] = origin[1].x;
			modelToWorld[1][1] = origin[1].y;
			modelToWorld[1][2] = origin[1].z;
			modelToWorld[1][3] = origin[1].w;
			modelToWorld[2][0] = origin[2].x;
			modelToWorld[2][1] = origin[2].y;
			modelToWorld[2][2] = origin[2].z;
			modelToWorld[2][3] = origin[2].w;
			modelToWorld[3][0] = origin[3].x;
			modelToWorld[3][1] = origin[3].y;
			modelToWorld[3][2] = origin[3].z;
			modelToWorld[3][3] = origin[3].w;

			//Keep track of the number of branches that we have calculated.
			numberOfBranches = 0;

			//Initialize arrays to hold vertices, indices, and colors.
			GLfloat *verts = new GLfloat[totalTreeHeight * branchSubSections * 4];
			//GLfloat *leafVertices = new GLfloat[formedTree.size() * 12 * 4];
			GLushort *inds = new GLushort[totalTreeHeight * branchSubSections * 6];
			GLfloat *colors = new GLfloat[totalTreeHeight * branchSubSections * 4];
			GLushort *texCs = new GLushort[totalTreeHeight * branchSubSections * 2];

			//Keep track of the number of elements in each array.
			int vertsSize = totalTreeHeight * branchSubSections * 4;
			//int leafSize = formedTree.size() * 48 * 4;
			int indsSize = totalTreeHeight * branchSubSections * 6;
			int colorsSize = totalTreeHeight * branchSubSections * 4;
			int texCsSize = totalTreeHeight * branchSubSections * 2;

			float angleOffset = 360.0f / branchSubSections;
			int currentHeight = 0;

			//Cycle through all the branches in the formedTree stack.
			for (dynarray<TreeBox*>::iterator iter = formedTree.begin(); iter != formedTree.end(); iter++)
			{
				float radiusOffset;
				float currentRadius;

				GLushort texCoordY = 1;

				currentRadius = branchRadius * (*iter)->branchHeight;
				
				if ((*iter)->parentBranchIndex > -1 && currentRadius > formedTree[(*iter)->parentBranchIndex]->radiusOffset * formedTree[(*iter)->parentBranchIndex]->branchHeight - (formedTree[(*iter)->parentBranchIndex]->radiusOffset * (*iter)->parentHeightAtBase))
					currentRadius = formedTree[(*iter)->parentBranchIndex]->radiusOffset * formedTree[(*iter)->parentBranchIndex]->branchHeight - (formedTree[(*iter)->parentBranchIndex]->radiusOffset * ((*iter)->parentHeightAtBase + 1));

				radiusOffset = currentRadius / (*iter)->branchHeight;
				(*iter)->radiusOffset = radiusOffset;

				for (int i = 0; i < (*iter)->branchHeight; i++)
				{
					float currentAngle = 0.0f;
					currentRadius -= radiusOffset;

					if ((*iter)->parentBranchIndex < 0 && i <= 0)
						currentRadius = currentRadius + (radiusOffset * 10);

					GLushort texCoordX = 1;

					if (texCoordY > 0)
						texCoordY = 0;
					else
						texCoordY = 1;

					for (int j = 0; j < branchSubSections; j++)
					{
						if (texCoordX > 0)
							texCoordX = 0;
						else
							texCoordX = 1;

						vec4 vert = vec4(currentRadius * cos(currentAngle * 0.0174532925f), ((float)i * 2.0f) - (*iter)->branchHeight, currentRadius * sin(currentAngle * 0.0174532925f), 1.0f);

						vert = vert * (*iter)->modelToWorld;

						verts[0 + (j * 4) + (i * branchSubSections * 4) + (currentHeight * branchSubSections * 4)] = vert.x();
						verts[1 + (j * 4) + (i * branchSubSections * 4) + (currentHeight * branchSubSections * 4)] = vert.y();
						verts[2 + (j * 4) + (i * branchSubSections * 4) + (currentHeight * branchSubSections * 4)] = vert.z();
						verts[3 + (j * 4) + (i * branchSubSections * 4) + (currentHeight * branchSubSections * 4)] = vert.w();

						colors[0 + (j * 4) + (i * branchSubSections * 4) + (currentHeight * branchSubSections * 4)] = 1.0f;
						colors[1 + (j * 4) + (i * branchSubSections * 4) + (currentHeight * branchSubSections * 4)] = 0.0f;
						colors[2 + (j * 4) + (i * branchSubSections * 4) + (currentHeight * branchSubSections * 4)] = 0.0f;
						colors[3 + (j * 4) + (i * branchSubSections * 4) + (currentHeight * branchSubSections * 4)] = 1.0f;

						texCs[0 + (j * 2) + (i * branchSubSections * 2) + (currentHeight * branchSubSections * 2)] = texCoordX;
						texCs[1 + (j * 2) + (i * branchSubSections * 2) + (currentHeight * branchSubSections * 2)] = texCoordY;

						currentAngle += angleOffset;
						
						if (i < (*iter)->branchHeight - 1)
						{
							if (j < branchSubSections - 1)
							{
								inds[0 + (j * 6) + (i * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(j + branchSubSections + (i * branchSubSections) + (currentHeight * branchSubSections));
								inds[1 + (j * 6) + (i  * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(j + (i * branchSubSections) + (currentHeight * branchSubSections));
								inds[2 + (j * 6) + (i  * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(1 + j + (i  * branchSubSections) + (currentHeight * branchSubSections));
								inds[3 + (j * 6) + (i * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(1 + j + (i * branchSubSections) + (currentHeight * branchSubSections));
								inds[4 + (j * 6) + (i * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(j + (branchSubSections + 1) + (i * branchSubSections) + (currentHeight * branchSubSections));
								inds[5 + (j * 6) + (i * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(j + branchSubSections + (i * branchSubSections) + (currentHeight * branchSubSections));
							}

							else if (j == branchSubSections - 1)
							{
								inds[0 + (j * 6) + (i * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(j + branchSubSections + (i * branchSubSections) + (currentHeight * branchSubSections));
								inds[1 + (j * 6) + (i  * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(j + (i * branchSubSections) + (currentHeight * branchSubSections));
								inds[2 + (j * 6) + (i  * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(1 + j - branchSubSections + (i  * branchSubSections) + (currentHeight * branchSubSections));
								inds[3 + (j * 6) + (i * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(1 + j - branchSubSections + (i  * branchSubSections) + (currentHeight * branchSubSections));
								inds[4 + (j * 6) + (i * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(j + 1 + (i * branchSubSections) + (currentHeight * branchSubSections));
								inds[5 + (j * 6) + (i * branchSubSections * 6) + (currentHeight * branchSubSections * 6)] = 
									(j + branchSubSections + (i * branchSubSections) + (currentHeight * branchSubSections));
							}
						}
					}

					if ((*iter)->parentBranchIndex < 0 && i <= 0)
						currentRadius = currentRadius - (radiusOffset * 10);
				}

				//Count the height that we have traversed thus far.
				currentHeight += (int)(*iter)->branchHeight;
			}

			//Bind an arraybuffer to the vbo for vertices.  Load the vertices array into 
			//the array buffer.
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, vertsSize * sizeof(GLfloat), verts, GL_STATIC_DRAW);

			//Bind an array buffer to the cbo for colors.  Load the colors array into
			//the array buffer.
			glBindBuffer(GL_ARRAY_BUFFER, cbo);
			glBufferData(GL_ARRAY_BUFFER, colorsSize * sizeof(GLfloat), colors, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, tbo);
			glBufferData(GL_ARRAY_BUFFER, texCsSize * sizeof(GLushort), texCs, GL_STATIC_DRAW);
			
			//Bind an element array buffer to the ibo for indices.  Load the indices
			//into the element array buffer.
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indsSize * sizeof(GLushort), inds, GL_STATIC_DRAW);

			//glBindBuffer(GL_ARRAY_BUFFER, lbo);
			//glBufferData(GL_ARRAY_BUFFER, leafSize, leafVertices, GL_STATIC_DRAW);

			//for (int i = 0; i < vertsSize; i++)
				//vertices.push_back(verts[i]);

			//for (int i = 0; i < indsSize; i++)
				//indices.push_back(inds[i]);

			//for (int i = 0; i < texCsSize; i++)
				//texCoords.push_back(texCs[i]);

			//Release the arrays created as they are loaded into the buffers.
			//This prevents memory leaks.
			delete[] verts;
			//delete[] leafVertices;
			delete[] colors;
			delete[] inds;
			delete[] texCs;
		}
	};
}