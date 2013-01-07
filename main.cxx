// main.cxx
// 
// Copyright 2013 Mael N. <contact@maeln.com>
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.


#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <GL/glew.h>
#include <GL/glfw.h>

typedef struct // Le triangle est composé de 3 point à 3 composante (x,y,z) car dans un plan 3D.
{
	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
} triangle;

typedef struct 
{
	GLuint vao_addr;
	std::vector<GLuint> vbo_addr;
} vao;

glm::vec3 barycentreTriangle(triangle i_triangle); 	// Calcule les coordonées du barycentre du triangle.
int generateTriangle(triangle i_triangle, std::vector<triangle>& o_triangle); 	// Calcule les triangle généré à partir d'un triangle et de son barycentre ( 1 -> 3 triangle ) 
																				// et le rajoute dans la liste o_triangle.
void GLFWCALL Wresize(int width, int height);	// Redimensionne le contexte OpenGL quand la fenêtre est redimensionner.
int triangleVAO(std::vector<triangle>& i_triangle, vao *o_vao); // Creer un VAO pour les triangle.
int cleanVAO(vao i_vao); // Supprime le VAO de la mémoire vidéo.
int subdivide(triangle i_triangle, uint n, std::vector<triangle>& o_triangle); // Subdivise un triangle n fois et renvoi le resultat dans le tableau o_triangle.

glm::mat4 projection; // Matrice de projection.

int main(int argc, char **argv)
{
	std::cerr << "Init:" << std::endl;
	if(glfwInit())
	{
		std::cerr << "Glfw: Done." << std::endl;
	}
	else
	{
		std::cerr << "Glfw: Fail." << std::endl;
	}
	
	if(glfwOpenWindow(1024, 640, 8,8,8,8,16,16, GLFW_WINDOW))
	{
		std::cerr << "Window: Done." << std::endl;
	}
	else
	{
		std::cerr << "Window: Fail." << std::endl;
		glfwTerminate();
	}
	
	GLenum err = glewInit();
	if (GLEW_OK != err) // Vérifie que Glew est bien initialisé, sinon, renvoi l'erreur.
	{
		std::cerr << "Erreur: " << glewGetErrorString(err) << std::endl;
	}

	std::cerr << "Info: Glew " << glewGetString(GLEW_VERSION) << " en cours d'utilisation." << std::endl;
	
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwSetWindowTitle("Triangulate");
	glfwSetWindowSizeCallback(Wresize);
	
	std::cerr << std::endl << "Context:" << std::endl 
		<< "OpenGL Major: " << glfwGetWindowParam(GLFW_OPENGL_VERSION_MAJOR) << std::endl
		<< "OpenGL Minor: " << glfwGetWindowParam(GLFW_OPENGL_VERSION_MINOR) << std::endl
		<< "Refresh Rate: " << glfwGetWindowParam(GLFW_REFRESH_RATE) << std::endl;
		
	glClearColor(0.f, 0.f, 0.f, 0.f);
	
	bool running(true);
	
	triangle initt;
	initt.a = glm::vec3(0,0,0);
	initt.b = glm::vec3(0,1.f/2.f,0);
	initt.c = glm::vec3(1.f/2.f,1.f/2.f,0);
	
	
	std::vector<triangle> output;
	subdivide(initt, 8, output);
	
			
	vao datvao;
	triangleVAO(output, &datvao);
	
	// glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	
	while(running)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glBindVertexArray(datvao.vao_addr);
			glDrawArrays(GL_POINTS, 0, output.size()*3*3);
		glBindVertexArray(0);
		
		glfwSwapBuffers();
		
		running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
	}
	
	cleanVAO(datvao);
	glfwCloseWindow();
	glfwTerminate();
	
	return 0;
}

int subdivide(triangle i_triangle, uint iteration, std::vector<triangle>& o_triangle)
{
	std::vector<triangle> res;
	generateTriangle(i_triangle, res);
	if(iteration==1)
	{
		o_triangle.push_back(res[0]);
	}
	else
	{
		std::vector<triangle> increment(res);
		for(uint i=0; i<iteration-1; i++)
		{
			std::vector<triangle> tmp;
			for(uint n=0; n < increment.size(); n++)
			{
				generateTriangle(increment[n], tmp);
			}
			
			increment = tmp;
		}
		
		for(uint n=0; n < increment.size(); n++)
		{
			o_triangle.push_back(increment[n]);
		}
	}
	
	return 0;
}

int cleanVAO(vao i_vao)
{
	glDeleteBuffers(i_vao.vbo_addr.size(), i_vao.vbo_addr.data());
	glDeleteVertexArrays(1, &i_vao.vao_addr);
	
	return 0;
}

int triangleVAO(std::vector<triangle>& i_triangle, vao *o_vao)
{
	float buffer[i_triangle.size()*3*3]; // 3*glm::vec3; glm::vec3 -> 3*float; 3*3*size.
	for(uint n=0; n < i_triangle.size(); n++)
	{
		buffer[n*9+0] = i_triangle[n].a.x;
		buffer[n*9+1] = i_triangle[n].a.y;
		buffer[n*9+2] = i_triangle[n].a.z;
		
		buffer[n*9+3] = i_triangle[n].b.x;
		buffer[n*9+4] = i_triangle[n].b.y;
		buffer[n*9+5] = i_triangle[n].b.z;
		
		buffer[n*9+6] = i_triangle[n].c.x;
		buffer[n*9+7] = i_triangle[n].c.y;
		buffer[n*9+8] = i_triangle[n].c.z;
	}
	
	GLuint VAOid;
	glGenVertexArrays(1, &VAOid);
    glBindVertexArray(VAOid);
    
	GLuint VBOid;
	glGenBuffers(1, &VBOid);
	glBindBuffer(GL_ARRAY_BUFFER, VBOid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);
	
	o_vao->vao_addr = VAOid;
	o_vao->vbo_addr.push_back(VBOid);
	
	return(0);
}

void GLFWCALL Wresize(int width, int height)
{
	glViewport(0, 0, width, height);
	projection = glm::perspective(90.f, (float)width/(float)height, 1.f, 10.f);
}

glm::vec3 barycentreTriangle(triangle i_triangle)
{
	glm::vec3 barycentre;
	barycentre.x = (i_triangle.a.x + i_triangle.b.x + i_triangle.c.x) / 3.f;
	barycentre.y = (i_triangle.a.y + i_triangle.b.y + i_triangle.c.y) / 3.f;
	barycentre.z = (i_triangle.a.z + i_triangle.b.z + i_triangle.c.z) / 3.f;
	
	return barycentre;
}

int generateTriangle(triangle i_triangle, std::vector<triangle>& o_triangle)
{
	triangle ftriangle, striangle, ttriangle;
	glm::vec3 barycentre = barycentreTriangle(i_triangle);
	
	// First triangle :
	ftriangle.a = i_triangle.a;
	ftriangle.b = i_triangle.b;
	ftriangle.c = barycentre;
	
	// Second triangle :
	striangle.a = i_triangle.b;
	striangle.b = i_triangle.c;
	striangle.c = barycentre;
	
	// Third triangle :
	ttriangle.a = i_triangle.c;
	ttriangle.b = i_triangle.a;
	ttriangle.c = barycentre;
	
	o_triangle.push_back(ftriangle);
	o_triangle.push_back(striangle);
	o_triangle.push_back(ttriangle);
	
	return 0;
}
