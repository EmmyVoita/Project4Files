


#include"Texture.h"
#include"OBJloader.h"
#include"MyMeshClass.h"
#include"Shader2.h"
#include"Material.h"

using namespace std;

class MyModelClass
{
private:
	Material* material;
	Texture overrideTextureDiffuse;
	//Texture* overrideTextureDiffusePointer;
	vector<MyMeshClass*> meshes;
	glm::mat4 ViewMatrix;
	unsigned int *depthMap;


public:

	//OBJ file loaded model
	MyModelClass(Material* material, const char* difFileName,  const char* fileName, glm::mat4 &ViewMatrix)
	: material(material), overrideTextureDiffuse(Texture(difFileName,  GL_TEXTURE_2D)),  ViewMatrix(ViewMatrix)
	{
		
		cout <<  "Diffuse Texture [height, width, id]: " << overrideTextureDiffuse.getHeight() << ":" << overrideTextureDiffuse.getWidth() << ":" <<  overrideTextureDiffuse.getID() << endl;

	    //Load the model from the file
	    vector<Vertex> mesh = loadOBJ(fileName);
	    Vertex* vertexArray = mesh.data();
	    unsigned nrOfVertices =  mesh.size();
	    GLuint* indexArray = NULL;
	    unsigned nrOfIndices = 0;
    
	    this->meshes.push_back(new MyMeshClass(vertexArray, nrOfVertices, indexArray, nrOfIndices, glm::vec3(1.f, 0.f, 0.0f),
			glm::vec3(0.f),
			glm::vec3(0.f),
			glm::vec3(1.f)));
	}

	
	void setViewMatrix(glm::mat4 &ViewMatrix)
	{
		this->ViewMatrix = ViewMatrix;
	}

	~MyModelClass()
	{
		for (auto*& i : this->meshes)
			delete i;
	}
	
	//Functions
	
	void setOrigin(const glm::vec3 origin)
	{
		for (auto& i : this->meshes)
			i->setOrigin(origin);
	}
	
	void setRotation(const glm::vec3 rotation)
	{
		for (auto& i : this->meshes)
			i->setRotation(rotation);
	}
	
	void setScale(const glm::vec3 scaled)
	{
		for (auto& i : this->meshes)
			i->setScale(scaled);
	}

	void setPosition(const glm::vec3 position)
	{
		for (auto& i : this->meshes)
			i->setPosition(position);
	}
	
	void move(const glm::vec3 position)
	{
		for (auto& i : this->meshes)
			i->move(position);
	}

	void setShadowTex(unsigned int *depthMap)
	{
		this->depthMap = depthMap;
	}


	void renderForDepth(Shader2 &ourShader)
	{
		ourShader.Use();
		
		for(unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->renderForDepth(ourShader);
		}
	}

	void render(Shader2 &ourShader)
	{
	
		ourShader.Use();
	
		this->material->sendToShader(ourShader);
		
		for(unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->render(ourShader, ViewMatrix);
		}
		
		ourShader.Use();
		
	

		//Draw Textures
		for (auto& i : this->meshes)
		{   
			this->overrideTextureDiffuse.bind(material->getDiffuseID());
			//this->overrideTextureDiffuse.bind(material->getDiffuseID());

			ourShader.set1i( material->getShadowMapID(), "material.shadowMap");
			glActiveTexture(GL_TEXTURE0 + material->getShadowMapID()); // activate texture unit 1
			//glActiveTexture(GL_TEXTURE0 + 1); 
			glBindTexture(GL_TEXTURE_2D, *depthMap); // bind the depth map texture to unit 1

		}

		

		//glBindTexture(GL_TEXTURE_2D, 0);

	}
};
