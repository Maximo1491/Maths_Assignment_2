namespace octet
{
	class shadow
	{
	private:

		GLuint fbo, shadowMap;

	public:
		
		shadow(int width, int height) 
		{
			glGenFramebuffers(1, &fbo);
			glGenTextures(1, &shadowMap);
			
			glBindTexture(GL_TEXTURE_2D, shadowMap);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if (status != GL_FRAMEBUFFER_COMPLETE)
				printf("Problem initializing the shadow frame buffer.\n");
		}

		~shadow()
		{
			glDeleteFramebuffers(1, &fbo);
			glDeleteTextures(1, &shadowMap);
		}

		void BindShadowTexture(GLenum textureUnit)
		{
			glActiveTexture(textureUnit);
			glBindTexture(GL_TEXTURE_2D, shadowMap);
		}
	};
}