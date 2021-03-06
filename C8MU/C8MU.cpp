// C8MU.cpp : Defines the entry point for the console application.
//

#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "mem.h"
#include "display.h"
#include "c8.h"

using namespace std;

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static vector<char> ReadAllBytes(char const* filename)
{
	ifstream ifs(filename, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();

	vector<char>  result(pos);

	ifs.seekg(0, ios::beg);
	ifs.read(&result[0], pos);

	return result;
}

void loop() {
	if (emulationAlive) {
		execute();
	}
}

int main(int argc, char* argv[])
{
	const char* romFile = "c8games/PONG";
	if (argc == 2) {
		romFile = argv[1];
	}
	vector<char> rom = ReadAllBytes(romFile);

	for (int i = 0; i < rom.size(); i++) {
		ram[0x200 + i] = rom[i];
	}

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		printf("Graphics initialisation failed!");
		exit(-1);
	}

	GLFWwindow* window = glfwCreateWindow(640, 320, "C8MU", NULL, NULL);
	if (!window){
		printf("Window or GL context initialisation failed!");
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glEnable(GL_TEXTURE_2D);
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	char* data = new char[64 * 32];
	for (int i = 0; i < 64 * 32; i++) {
		data[i] = 100;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, 64, 32, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, disp_data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		fprintf(stderr, "AAGLERROR %04X", err);
	}

	reset();

	while (!glfwWindowShouldClose(window)) {
		loop();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureId);
		if (drawFlag) {
			drawFlag = false;
			//char* data = new char[64 * 32];
			//for (int i = 0; i < 64 * 32; i++) {
			//	data[i] = 0xAA;
			//}
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_LUMINANCE, GL_UNSIGNED_BYTE, disp_data());
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(-1.0f, -1.0f);
		glTexCoord2f(0, 1);
		glVertex2f(-1.0f, 1.0f);
		glTexCoord2f(1, 1);
		glVertex2f(1.0f, 1.0f);
		glTexCoord2f(1, 0);
		glVertex2f(1.0f, -1.0f);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		bool hasFailed = false;
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			hasFailed = true;
			fprintf(stderr, "GLERROR %04X", err);
		}
		if (hasFailed)
			break;

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

    return 0;
}
