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
		int iterations, masterIterations, maxIterations, ignoreSize, numberOfBranches, branchSubSections;
		float height, width, depth, branchRadius, angle, leafWidth, scale;
		string originalAxiom, ignore;
		bool done;

		dynarray<std::string> rules;
		dynarray<TreeBox*> formedTree;
		dynarray<GLfloat> vertices, normals;
		dynarray<GLfloat> texCoords;
		std::vector<char> formula, newFormula;
		GLfloat* sphere;
		GLushort* sphereTex;
		int sphereSize, sphereTexSize;

		GLuint vbo, tbo, nbo, program;
		GLint uniform_matrix;

	public:

		//Enumeration to tell what kind of tree to make.
		enum
		{
			dead,
			dying,
			alive,
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
			done = false;

			//Set the tree's scale percentage.
			scale = 0.25f;

			//Load the necessary buffers to be used.
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &tbo);
			glGenBuffers(1, &nbo);

			GLuint textures[2];

			//Generate a buffer for the bark texture
			glGenTextures(2, textures);
			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[0]);

			int width, height;
			unsigned char* image = SOIL_load_image("../../assets/thronecraft/treebark.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

			SOIL_free_image_data(image);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[1]);

			image = SOIL_load_image("../../assets/thronecraft/leafs.png", &width, &height, 0, SOIL_LOAD_RGBA);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

			SOIL_free_image_data(image);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//Read the initial file.  This will trigger a domino
			//effect where the entire tree is made from this file.
			ReadFile("../../assets/thronecraft/Stochastic_Sample.txt");
		}

		~tree_generator()
		{
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &tbo);
			glDeleteBuffers(1, &nbo);
		}

		void render(glm::mat4 projection, tree_shader &tree_shader_, int numOfLights, const glm::vec4 *light_information, glm::vec4 ambient, glm::vec4 *diffuse)
		{
			tree_shader_.render(projection, numOfLights, light_information, ambient, diffuse);

			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_uv);
			glEnableVertexAttribArray(attribute_normal);

			glBindBuffer(GL_ARRAY_BUFFER, tbo);
			glVertexAttribPointer(attribute_uv, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(GLushort) * 2, 0);

			glBindBuffer(GL_ARRAY_BUFFER, nbo);
			glVertexAttribPointer(attribute_normal, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glVertexAttribPointer(attribute_pos, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

			glDrawArrays(GL_TRIANGLES, 0, vertices.size());

			glDisableVertexAttribArray(attribute_pos);
			glDisableVertexAttribArray(attribute_uv);
			glDisableVertexAttribArray(attribute_normal);

			glEnable(GL_CULL_FACE);
		}

		//A command to generate another tree.
		void AddTree(int treeType, glm::vec3 origin)
		{
			glm::mat4 pos = glm::translate(glm::mat4(1.0), origin);
			FormFormula(treeType, pos);
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
						masterIterations = atoi(buffer);
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
		void FormFormula(int treeType, glm::mat4 origin)
		{
			iterations = masterIterations + ((rand() % 2) - 1);

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
			MakeTree(treeType, origin);
		}

		//This funnction will compute the vertices and indices of the tree for drawing.
		void MakeTree(int treeType, glm::mat4 origin)
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

			treeBox->modelToWorld.loadIdentity();
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

					treeBox->branchHeight++;
					treeBox->modelToWorld.translate(0, height, 0);
					totalTreeHeight++;

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

			//Set the modelToWorld back to the origin.
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

			modelToWorld.transpose4x4();

			//Keep track of the number of branches that we have calculated.
			numberOfBranches = 0;

			//Keep track of the number of elements in each array.
			//int vertsSize = totalTreeHeight * branchSubSections * 24;
			//int texCsSize = totalTreeHeight * branchSubSections * 12;

			float angleOffset = 360.0f / branchSubSections;

			std::vector<float> leafHeight;
			std::vector<mat4t> leafMatrices;
			
			mat4t scaleMat;
			scaleMat.loadIdentity();
			scaleMat[0][0] = scale;
			scaleMat[1][1] = scale;
			scaleMat[2][2] = scale;

			//Cycle through all the branches in the formedTree stack.
			for (dynarray<TreeBox*>::iterator iter = formedTree.begin(); iter != formedTree.end(); iter++)
			{
				float radiusOffset;
				float currentRadius;
				float nextRadius;

				currentRadius = branchRadius * (*iter)->branchHeight;
				
				if ((*iter)->parentBranchIndex > -1 && currentRadius > formedTree[(*iter)->parentBranchIndex]->radiusOffset * formedTree[(*iter)->parentBranchIndex]->branchHeight - (formedTree[(*iter)->parentBranchIndex]->radiusOffset * (*iter)->parentHeightAtBase))
					currentRadius = formedTree[(*iter)->parentBranchIndex]->radiusOffset * formedTree[(*iter)->parentBranchIndex]->branchHeight - (formedTree[(*iter)->parentBranchIndex]->radiusOffset * ((*iter)->parentHeightAtBase + 1));

				radiusOffset = currentRadius / (*iter)->branchHeight;
				(*iter)->radiusOffset = radiusOffset;

				currentRadius += radiusOffset;
				nextRadius = currentRadius;

				if (treeType > 0)
				{
					leafHeight.push_back((*iter)->branchHeight);
					leafMatrices.push_back((*iter)->modelToWorld);
				}

				for (int i = 0; i < (*iter)->branchHeight; i++)
				{
					float currentAngle = 0.0f;
					currentRadius -= radiusOffset;
					nextRadius = currentRadius - radiusOffset;

					if ((*iter)->parentBranchIndex < 0 && i <= 0)
						currentRadius = currentRadius + (radiusOffset * 10);

					for (int j = 0; j < branchSubSections; j++)
					{	
						vec4 vert1 = vec4(currentRadius * cos(currentAngle * 0.0174532925f), ((float)i * (height * 2.0f)) - (*iter)->branchHeight, currentRadius * sin(currentAngle * 0.0174532925f), 1.0f) * (*iter)->modelToWorld;
						vec4 vert2 = vec4(nextRadius * cos(currentAngle * 0.0174532925f), ((float)(i + 1) * (height * 2.0f)) - (*iter)->branchHeight, nextRadius * sin(currentAngle * 0.0174532925f), 1.0f) * (*iter)->modelToWorld;
						vec4 vert3 = vec4(currentRadius * cos((currentAngle + angleOffset) * 0.0174532925f), ((float)i * (height * 2.0f)) - (*iter)->branchHeight, currentRadius * sin((currentAngle + angleOffset) * 0.0174532925f), 1.0f) * (*iter)->modelToWorld;
						vec4 vert4 = vec4(currentRadius * cos((currentAngle + angleOffset) * 0.0174532925f), ((float)i * (height * 2.0f)) - (*iter)->branchHeight, currentRadius * sin((currentAngle + angleOffset) * 0.0174532925f), 1.0f) * (*iter)->modelToWorld;
						vec4 vert5 = vec4(nextRadius * cos(currentAngle * 0.0174532925f), ((float)(i + 1) * (height * 2.0f)) - (*iter)->branchHeight, nextRadius * sin(currentAngle * 0.0174532925f), 1.0f) * (*iter)->modelToWorld;
						vec4 vert6 = vec4(nextRadius * cos((currentAngle + angleOffset) * 0.0174532925f), ((float)(i + 1) * (height * 2.0f)) - (*iter)->branchHeight, nextRadius * sin((currentAngle + angleOffset) * 0.0174532925f), 1.0f) * (*iter)->modelToWorld;

						normals.push_back(vert1.x());
						normals.push_back(vert1.y());
						normals.push_back(vert1.z());
						normals.push_back(vert1.w());

						normals.push_back(vert2.x());
						normals.push_back(vert2.y());
						normals.push_back(vert2.z());
						normals.push_back(vert2.w());

						normals.push_back(vert3.x());
						normals.push_back(vert3.y());
						normals.push_back(vert3.z());
						normals.push_back(vert3.w());

						normals.push_back(vert4.x());
						normals.push_back(vert4.y());
						normals.push_back(vert4.z());
						normals.push_back(vert4.w());

						normals.push_back(vert5.x());
						normals.push_back(vert5.y());
						normals.push_back(vert5.z());
						normals.push_back(vert5.w());

						normals.push_back(vert6.x());
						normals.push_back(vert6.y());
						normals.push_back(vert6.z());
						normals.push_back(vert6.w());

						vert1 = vert1 * scaleMat * modelToWorld;
						vert2 = vert2 * scaleMat * modelToWorld;
						vert3 = vert3 * scaleMat * modelToWorld;
						vert4 = vert4 * scaleMat * modelToWorld;
						vert5 = vert5 * scaleMat * modelToWorld;
						vert6 = vert6 * scaleMat * modelToWorld;

						vertices.push_back(vert1.x());
						vertices.push_back(vert1.y());
						vertices.push_back(vert1.z());
						vertices.push_back(vert1.w());

						texCoords.push_back(0);
						texCoords.push_back(0);

						vertices.push_back(vert2.x());
						vertices.push_back(vert2.y());
						vertices.push_back(vert2.z());
						vertices.push_back(vert2.w());

						texCoords.push_back(0);
						texCoords.push_back(1);

						vertices.push_back(vert3.x());
						vertices.push_back(vert3.y());
						vertices.push_back(vert3.z());
						vertices.push_back(vert3.w());

						texCoords.push_back(1);
						texCoords.push_back(1);

						vertices.push_back(vert4.x());
						vertices.push_back(vert4.y());
						vertices.push_back(vert4.z());
						vertices.push_back(vert4.w());

						texCoords.push_back(0);
						texCoords.push_back(1);

						vertices.push_back(vert5.x());
						vertices.push_back(vert5.y());
						vertices.push_back(vert5.z());
						vertices.push_back(vert5.w());

						texCoords.push_back(1);
						texCoords.push_back(1);

						vertices.push_back(vert6.x());
						vertices.push_back(vert6.y());
						vertices.push_back(vert6.z());
						vertices.push_back(vert6.w());

						texCoords.push_back(1);
						texCoords.push_back(0);

						currentAngle += angleOffset;
					}

					if ((*iter)->parentBranchIndex < 0 && i <= 0)
						currentRadius = currentRadius - (radiusOffset * 10);
				}
			}
			
			MakeLeaves(leafHeight, leafMatrices, modelToWorld, scaleMat, 4);

			ClearTreeForm(formedTree);
			ClearFormula();
			LoadOriginalAxiom();
		}

		void MakeLeaves(std::vector<float> &leafHeight, std::vector<mat4t> leafMat, mat4t &origin, mat4t &scaleMat, int numberOfLeafQuads)
		{
			float radiusOffset = 360.0f / (numberOfLeafQuads * 2);
			float highestBranch = 0.0f;

			for (int i = 0; i < leafHeight.size(); i++)
			{
				if (leafHeight[i] > highestBranch) highestBranch = leafHeight[i];

				float leafRadius = leafHeight[i] * 0.85;

				if (leafRadius > 6.0f) leafRadius = 6.0f;

				if (leafRadius > 0.2f && leafHeight[i] > 1.0f)
				{
					leafMat[i].translate(0.0f, leafHeight[i], 0.0f);
					mat4t branchPos;
					branchPos = leafMat[i];
					branchPos + scaleMat;

					if (branchPos.row(3).y() > highestBranch * 0.5)

					{
						for (int j = 0; j < numberOfLeafQuads; j++)
							//for (int j = 0; j < numberOfLeafQuads * 2; j++)
						{
							//if (j < numberOfLeafQuads)
							leafMat[i].rotateY(radiusOffset);
							//else
							//leafMat[i].rotateX(radiusOffset);

							leafMat[i].translate(-leafRadius, leafRadius, 0.0f);

							vec4 leafVert = vec4(leafMat[i].row(3).x(), leafMat[i].row(3).y(), leafMat[i].row(3).z(), 1.0f) * scaleMat;

							normals.push_back(leafVert.x());
							normals.push_back(leafVert.y());
							normals.push_back(leafVert.z());
							normals.push_back(2.0f);

							leafVert = leafVert * origin;

							vertices.push_back(leafVert.x());
							vertices.push_back(leafVert.y());
							vertices.push_back(leafVert.z());
							vertices.push_back(2.0f);

							texCoords.push_back(0);
							texCoords.push_back(0);

							leafMat[i].translate(leafRadius * 2, 0.0f, 0.0f);

							leafVert = vec4(leafMat[i].row(3).x(), leafMat[i].row(3).y(), leafMat[i].row(3).z(), 1.0f) * scaleMat;

							normals.push_back(leafVert.x());
							normals.push_back(leafVert.y());
							normals.push_back(leafVert.z());
							normals.push_back(2.0f);

							leafVert = leafVert * origin;

							vertices.push_back(leafVert.x());
							vertices.push_back(leafVert.y());
							vertices.push_back(leafVert.z());
							vertices.push_back(2.0f);

							texCoords.push_back(0);
							texCoords.push_back(1);

							leafMat[i].translate(0.0f, -leafRadius * 2, 0.0f);

							leafVert = vec4(leafMat[i].row(3).x(), leafMat[i].row(3).y(), leafMat[i].row(3).z(), 1.0f) * scaleMat;

							normals.push_back(leafVert.x());
							normals.push_back(leafVert.y());
							normals.push_back(leafVert.z());
							normals.push_back(2.0f);

							leafVert = leafVert * origin;

							vertices.push_back(leafVert.x());
							vertices.push_back(leafVert.y());
							vertices.push_back(leafVert.z());
							vertices.push_back(2.0f);

							texCoords.push_back(1);
							texCoords.push_back(1);

							leafMat[i].translate(-leafRadius * 2, leafRadius * 2, 0.0f);

							leafVert = vec4(leafMat[i].row(3).x(), leafMat[i].row(3).y(), leafMat[i].row(3).z(), 1.0f) * scaleMat;

							normals.push_back(leafVert.x());
							normals.push_back(leafVert.y());
							normals.push_back(leafVert.z());
							normals.push_back(2.0f);

							leafVert = leafVert * origin;

							vertices.push_back(leafVert.x());
							vertices.push_back(leafVert.y());
							vertices.push_back(leafVert.z());
							vertices.push_back(2.0f);

							texCoords.push_back(0);
							texCoords.push_back(0);

							leafMat[i].translate(leafRadius * 2, -leafRadius * 2, 0.0f);

							leafVert = vec4(leafMat[i].row(3).x(), leafMat[i].row(3).y(), leafMat[i].row(3).z(), 1.0f) * scaleMat;

							normals.push_back(leafVert.x());
							normals.push_back(leafVert.y());
							normals.push_back(leafVert.z());
							normals.push_back(2.0f);

							leafVert = leafVert * origin;

							vertices.push_back(leafVert.x());
							vertices.push_back(leafVert.y());
							vertices.push_back(leafVert.z());
							vertices.push_back(2.0f);

							texCoords.push_back(1);
							texCoords.push_back(1);

							leafMat[i].translate(-leafRadius * 2, 0.0f, 0.0f);

							leafVert = vec4(leafMat[i].row(3).x(), leafMat[i].row(3).y(), leafMat[i].row(3).z(), 1.0f) * scaleMat;

							normals.push_back(leafVert.x());
							normals.push_back(leafVert.y());
							normals.push_back(leafVert.z());
							normals.push_back(2.0f);

							leafVert = leafVert * origin;

							vertices.push_back(leafVert.x());
							vertices.push_back(leafVert.y());
							vertices.push_back(leafVert.z());
							vertices.push_back(2.0f);

							texCoords.push_back(1);
							texCoords.push_back(0);

							leafMat[i].translate(leafRadius, leafRadius, 0.0f);
						}
					}
				}
			}
		}

		void PrepareTrees()
		{
			GLfloat* vs = new GLfloat[vertices.size()];
			GLfloat* ns = new GLfloat[normals.size()];
			GLushort* ts = new GLushort[texCoords.size()];

			for (int i = 0; i < vertices.size(); i++)
				vs[i] = vertices[i];

			for (int i = 0; i < texCoords.size(); i++)
				ts[i] = texCoords[i];

			for (int i = 0; i < normals.size(); i++)
				ns[i] = normals[i];

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vs, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, tbo);
			glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(GLushort), ts, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, nbo);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), ns, GL_STATIC_DRAW);

			delete[] vs;
			delete[] ts;
			delete[] ns;
		}

		//A function to clear all branched from the formedTree stack.
		void ClearTreeForm(dynarray<TreeBox*> &arr)
		{
			while (arr.size() > 0)
				arr.pop_back();
		}

		//A function to load the original axiom of the current L-System back into
		//the formula variable.
		void LoadOriginalAxiom()
		{
			for (unsigned int i = 0; i < originalAxiom.size(); i++)
				formula.push_back(originalAxiom[i]);
		}

		//A function to clear the newFormula and formula usually in preparation to
		//form a new L-System.
		void ClearFormula()
		{
			formula.clear();
			newFormula.clear();
		}
	};
}