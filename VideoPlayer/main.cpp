#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>

#include "LoadFrame.hpp"

int width = 600;
int height = 600;

int main(){

	if (!glfwInit())
	{
		std::cerr << "Error with the glfw initialization" << std::endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, "Video Player", nullptr, nullptr);

	if (!window)
	{
		std::cerr << "Error with the window initialization" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Error with the glew initialization" << std::endl;
		return -1;
	}

	glClearColor(0.55f, 0.55f, 0.55f,1.0f);

	int frame_width, frame_height;
	unsigned char* frameData;
	if (!LoadFrame("frog.mp4",&frame_width,&frame_height,&frameData))
	{
		std::cerr << "Couldn't load video frame" <<std::endl;
		return -1;
	}	

	GLuint tex_handle;
	glGenTextures(1, &tex_handle);
	glBindTexture(GL_TEXTURE_2D, tex_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,  frameData);

	int xOffset = 0;
	int yOffset = 0;


	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int wWidth, wHeight;
		glfwGetFramebufferSize(window, &wWidth, &wHeight);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, wWidth, wHeight, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);


		//render the texture
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex2i(xOffset, yOffset);
		glTexCoord2d(1, 0); glVertex2i(xOffset + frame_width, yOffset);
		glTexCoord2d(1, 1); glVertex2i(xOffset + frame_width, yOffset + frame_height);
		glTexCoord2d(0, 1); glVertex2i(xOffset, yOffset + frame_height);
		glEnd();
		glDisable(GL_TEXTURE_2D);


		glfwSwapBuffers(window);
	}


	glfwDestroyWindow(window);
	glfwTerminate();

	glm::vec3 xablauv{ 0,1,1 };
	std::cout << xablauv.z << std::endl;

	return 0;
}