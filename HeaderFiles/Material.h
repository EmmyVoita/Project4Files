#pragma once

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"Shader2.h"

class Material
{
private:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	GLint diffuseTex;
	GLint shadowMap;

public:
	Material(
	glm::vec3 ambient,
	glm::vec3 diffuse,
	glm::vec3 specular,
	GLint diffuseTex,
	GLint shadowMap
	)
	{
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->diffuseTex = diffuseTex;
		this->shadowMap = shadowMap;
	}

	~Material(){}


	GLint getDiffuseID()
	{
		return diffuseTex;
	}

	GLint getShadowMapID()
	{
		return shadowMap;
	}
	//Function
	void sendToShader(Shader2 &ourShader)
	{
		ourShader.setVec3f(this->ambient, "material.ambient");
		ourShader.setVec3f(this->diffuse, "material.diffuse");
		ourShader.setVec3f(this->specular, "material.specular");
		ourShader.set1i(this->shadowMap, "material.shadowMap");
		ourShader.set1i(this->diffuseTex, "material.diffuseTex");
		
	}
};
