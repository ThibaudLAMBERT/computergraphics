
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#include <../finalProject/render/shader.h>

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb/stb_image.h>

#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb/stb_image_write.h>


#include <iomanip>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

static bool playAnimation = true;
static float playbackSpeed = 0.5f;



// OpenGL camera view parameters
//static glm::vec3 eye_center;

static glm::vec3 eye_center;
static glm::vec3 lookat(0, 0, -1);
static glm::vec3 up(0, 1, 0);
float cameraSpeed = 100.0f;
float yaw = -90.0f;
float pitch = 0.0f;

const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);

static glm::vec3 lightIntensity = (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
static glm::vec3 light_lookat(0.0f, -1.0f, 0.0f);
static glm::vec3 lightDirection (-1.0f, -1.0f, -1.0f);
static glm::vec3 lightPosition(9900,9900,9900);

// Lighting
static glm::vec3 lightIntensity2(5e6f, 5e6f, 5e6f);
static glm::vec3 lightPosition2(-275.0f, 500.0f, 800.0f);

static glm::vec3 lightIntensity3(3e5f, 3e5f, 3e5f);
static glm::vec3 lightPosition3(-275.0f, 500.0f, 800.0f);

static glm::vec3 Position(0,0,0);
static glm::vec3 Target = Position + lightDirection;



float orthoLeft = -17000.0f;
float orthoRight = 17000.0f;
float orthoBottom = -17000.0f;
float orthoTop = 17000.0f;
float orthoNear = 10.0f;
float orthoFar = 50000.0f;


static float depthFoV = 80.f;
static float depthNear = 100.f;
static float depthFar = 10000.f;


static glm::vec3 lightUp(0, 1, 0);
static int shadowMapWidth = 1024;
static int shadowMapHeight = 768;


GLuint fbo;
GLuint depthTexture;

// View control
static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = -15.0f;

static int windowWidth = 1024;
static int windowHeight = 768;

static bool saveDepth = true;

static void saveDepthTexture(GLuint fbo, std::string filename) {
	int width = shadowMapWidth;
	int height = shadowMapHeight;
	if (shadowMapWidth == 0 || shadowMapHeight == 0) {
		width = windowWidth;
		height = windowHeight;
	}
	int channels = 3;

	std::vector<float> depth(width * height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadBuffer(GL_DEPTH_COMPONENT);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::vector<unsigned char> img(width * height * 3);
	for (int i = 0; i < width * height; ++i) img[3*i] = img[3*i+1] = img[3*i+2] = depth[i] * 255;

	stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);

}

static GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // To tile textures on a box, we set wrapping to repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}



struct Building {
	glm::vec3 position;		// Position of the box
	glm::vec3 scale;		// Size of the box in each axis

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box

		// Front face
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Back face
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		// Left face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// Right face
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// Bottom face
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};



	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23,
	};

    // TODO: Define UV buffer data
    // ---------------------------
    // ---------------------------
	GLfloat uv_buffer_data[48] = {
		// Front
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
				// Back
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
				 // Left
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
				 // Right
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// Top - we do not want texture the top
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		 // Bottom - we do not want texture the bottom
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
	};

	GLfloat normal_buffer_data[72] = {
		// Front
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Back
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		// Left
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		// Right
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Top
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Bottom
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
	};



	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint BuildingtextureID;
	GLuint normalBufferID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureBuildingSamplerID;
	GLuint BuildingprogramID;
	GLuint depthmvpMatrixID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint depthProgramID;
	GLuint lightmvpMatrixID;
	GLuint shadowMapID;
	GLuint lightDirectionID;
	GLuint modelID;

	void initialize(glm::vec3 position, glm::vec3 scale) {
		// Define scale of the building geometry
		this->position = position;
		this->scale = scale;

		// Create a vertex array object
		for (int i = 0; i < 72; ++i) color_buffer_data[i] = 0.0f;
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
        // TODO:
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		// TODO: Create a vertex buffer object to store the UV data



		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);
		// --------------------------------------------------------
		// --------------------------------------------------------
		glGenBuffers(1, &normalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		BuildingprogramID = LoadShadersFromFile("../finalProject/shader/building/building.vert", "../finalProject/shader/building/building.frag");
		if (BuildingprogramID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}
		glm::mat4 modelMatrix = glm::mat4();
		// Scale the box along each axis to make it look like a building

		modelMatrix = glm::translate(modelMatrix, position);

		modelMatrix = glm::scale(modelMatrix, scale);

		std::cout << "Model matrix" << std::endl;
		//afficherMatrice(modelMatrix);


		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(BuildingprogramID, "MVP");
		lightPositionID = glGetUniformLocation(BuildingprogramID, "lightPosition");
		lightIntensityID = glGetUniformLocation(BuildingprogramID, "lightIntensity");
		lightDirectionID = glGetUniformLocation(BuildingprogramID, "lightDirection");



		lightmvpMatrixID = glGetUniformLocation(BuildingprogramID, "lightSpaceMatrix");
		shadowMapID = glGetUniformLocation(BuildingprogramID, "shadowMap");

		BuildingtextureID = LoadTextureTileBox("../finalProject/textures/facade0.jpg");
		textureBuildingSamplerID = glGetUniformLocation(BuildingprogramID,"buildingSampler");

		depthProgramID = LoadShadersFromFile("../finalProject/shader/depth/depth.vert", "../finalProject/shader/depth/depth.frag");
		if (depthProgramID == 0) {
			std::cerr << "Failed to load depth shaders." << std::endl;
		}

		depthmvpMatrixID = glGetUniformLocation(depthProgramID, "lightSpaceMatrix");
		modelID = glGetUniformLocation(depthProgramID, "model");
	}

	void render(glm::mat4 cameraMatrix,glm::mat4 lightSpaceMatrix) {
		glUseProgram(BuildingprogramID);


		glBindVertexArray(vertexArrayID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// TODO: Model transform
		// -----------------------
        glm::mat4 modelMatrix = glm::mat4();
        // Scale the box along each axis to make it look like a building

		modelMatrix = glm::translate(modelMatrix, position);

        modelMatrix = glm::scale(modelMatrix, scale);


        // -----------------------

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		glm::mat4 lightmvp = lightSpaceMatrix * modelMatrix;

		glUniformMatrix4fv(lightmvpMatrixID, 1, GL_FALSE, &lightmvp[0][0]);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(shadowMapID, 5);


		// TODO: Enable UV buffer and texture sampler
		// ------------------------------------------
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		// Set textureSampler to use texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, BuildingtextureID);
		glUniform1i(textureBuildingSamplerID, 0);


		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
		glUniform3fv(lightDirectionID, 1, &lightDirection[0]);
		// ------------------------------------------

		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			36,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}
	void render_depth(glm::mat4 lightSpaceMatrix) {

		glUseProgram(depthProgramID);

		glBindVertexArray(vertexArrayID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// Set light-space matrix

		glm::mat4 modelMatrix = glm::mat4();
		// Scale the box along each axis to make it look like a building

		modelMatrix = glm::translate(modelMatrix, position);

		modelMatrix = glm::scale(modelMatrix, scale);

		glm::mat4 mvpLight = lightSpaceMatrix * modelMatrix;

		glUniformMatrix4fv(depthmvpMatrixID, 1, GL_FALSE, &mvpLight[0][0]);

		// Draw the box
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);

		glDisableVertexAttribArray(0);
	}
	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteBuffers(1, &normalBufferID);
		glDeleteBuffers(1, &uvBufferID);
		glDeleteTextures(1, &textureBuildingSamplerID);
		glDeleteTextures(1, &BuildingtextureID);
		glDeleteProgram(BuildingprogramID);
		glDeleteProgram(depthProgramID);
	}
};


struct SkyBox {
	glm::vec3 position;		// Position of the box
	glm::vec3 scale;		// Size of the box in each axis

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Back face
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		// Left face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// Right face
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// Bottom face
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};



	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
		// Front face
		0, 2, 1,  // triangle 1
		0, 3, 2,  // triangle 2

		// Back face
		4, 6, 5,  // triangle 1
		4, 7, 6,  // triangle 2

		// Left face
		8, 10, 9, // triangle 1
		8, 11, 10,// triangle 2

		// Right face
		12, 14, 13, // triangle 1
		12, 15, 14, // triangle 2

		// Top face
		16, 18, 17, // triangle 1
		16, 19, 18, // triangle 2

		// Bottom face
		20, 22, 21, // triangle 1
		20, 23, 22, // triangle 2
	};

	GLfloat uv_buffer_data[72] = {
		// pos Z
		0.25f, 0.666666666f,
		0.50f, 0.666666666f,
		0.5f, 0.333333333f,
		0.25f, 0.3333333333f,

		// neg Z
		0.75f, 0.667f,
		1.0f, 0.667f,
		1.0f, 0.333333333f,
		0.75f, 0.333333333f,

		// pos X
		0.0f, 0.667f,
		0.25f, 0.667f,
		0.25f, 0.333333333f,
		0.0f, 0.333333333f,

		// neg X
		0.50f, 0.667f,
		0.75f, 0.667f,
		0.75f, 0.333333333f,
		0.50f, 0.333333333f,

		// neg Y
		0.25f, 0.333333333f,
		0.49f, 0.333333333f,
		0.49f, 0.0f,
		0.25f, 0.0f,

		// pos Y
		0.25f, 1.0f,
		0.49f, 1.0f,
		0.49f, 0.666666666f,
		0.25f, 0.666666666f,
	};




	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint SkyboxtextureID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSkyboxSamplerID;
	GLuint SkyboxprogramID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint lightmvpMatrixID;
	GLuint shadowMapID;
	GLuint depthProgramID;
	GLuint depthmvpMatrixID;

	void initialize(glm::vec3 position, glm::vec3 scale) {
		// Define scale of the building geometry
		this->position = position;
		this->scale = scale;

		// Create a vertex array object
		for (int i = 0; i < 72; ++i) color_buffer_data[i] = 1.0f;
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
        // TODO:
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		// TODO: Create a vertex buffer object to store the UV data



		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);
		// --------------------------------------------------------
		// --------------------------------------------------------


		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		SkyboxprogramID = LoadShadersFromFile("../finalProject/shader/skybox/skybox.vert", "../finalProject/shader/skybox/skybox.frag");
		if (SkyboxprogramID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}



		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(SkyboxprogramID, "MVP");
		lightPositionID = glGetUniformLocation(SkyboxprogramID, "lightPosition");
		lightIntensityID = glGetUniformLocation(SkyboxprogramID, "lightIntensity");
		lightmvpMatrixID = glGetUniformLocation(SkyboxprogramID, "lightSpaceMatrix");
		shadowMapID = glGetUniformLocation(SkyboxprogramID, "shadowMap");

        // TODO: Load a texture
        // --------------------



		SkyboxtextureID = LoadTextureTileBox("../finalProject/textures/skybox.png");

        // TODO: Get a handle to texture sampler
        // -------------------------------------
		textureSkyboxSamplerID = glGetUniformLocation(SkyboxprogramID,"SkyboxSampler");

		depthProgramID = LoadShadersFromFile("../finalProject/shader/depth/depth.vert", "../finalProject/shader/depth/depth.frag");
		if (depthProgramID == 0) {
			std::cerr << "Failed to load depth shaders." << std::endl;
		}

		depthmvpMatrixID = glGetUniformLocation(depthProgramID, "lightSpaceMatrix");
        // -------------------------------------
	}

	void render(glm::mat4 cameraMatrix,glm::mat4 lightSpaceMatrix) {
		glUseProgram(SkyboxprogramID);


		glBindVertexArray(vertexArrayID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// TODO: Model transform
		// -----------------------
        glm::mat4 modelMatrix = glm::mat4();
        // Scale the box along each axis to make it look like a building

		modelMatrix = glm::translate(modelMatrix, position);

        modelMatrix = glm::scale(modelMatrix, scale);


        // -----------------------

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


		glm::mat4 lightmvp = lightSpaceMatrix * modelMatrix;


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(shadowMapID, 0);

		glUniformMatrix4fv(lightmvpMatrixID, 1, GL_FALSE, &lightmvp[0][0]);


		// Set light data
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);

		// TODO: Enable UV buffer and texture sampler
		// ------------------------------------------
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		// Set textureSampler to use texture unit 0
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SkyboxtextureID);
		glUniform1i(textureSkyboxSamplerID, 1);
        // ------------------------------------------

		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			36,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
        //glDisableVertexAttribArray(2);
	}
	void render_depth(glm::mat4 lightSpaceMatrix) {

		glUseProgram(depthProgramID);

		glBindVertexArray(vertexArrayID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		glm::mat4 modelMatrix = glm::mat4();
		// Scale the box along each axis to make it look like a building

		modelMatrix = glm::translate(modelMatrix, position);

		modelMatrix = glm::scale(modelMatrix, scale);

		glm::mat4 lightmvp = lightSpaceMatrix * modelMatrix;

		// Set light-space matrix

		glUniformMatrix4fv(depthmvpMatrixID, 1, GL_FALSE, &lightmvp[0][0]);

		// Draw the box
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);

		glDisableVertexAttribArray(0);
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteBuffers(1, &uvBufferID);
		glDeleteTextures(1, &textureSkyboxSamplerID);
		glDeleteProgram(SkyboxprogramID);
		glDeleteProgram(depthProgramID);
	}
};




struct Instance_tree {
	glm::vec3 position;
	glm::vec3 scale;
};

std::vector<Instance_tree> Transform_tree;








struct Bird {

	glm::vec3 scale;
	glm::vec3 position;
	glm::vec3 axis;
	float angle;
	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint jointMatricesID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint programID;
	GLuint textureID;
	GLuint textureSamplerID;

	tinygltf::Model model;

	// Each VAO corresponds to each mesh primitive in the GLTF model
	struct PrimitiveObject {
		GLuint vao;
		std::map<int, GLuint> vbos;
	};
	std::vector<PrimitiveObject> primitiveObjects;

	// Skinning
	struct SkinObject {
		// Transforms the geometry into the space of the respective joint
		std::vector<glm::mat4> inverseBindMatrices;

		// Transforms the geometry following the movement of the joints
		std::vector<glm::mat4> globalJointTransforms;

		// Combined transforms
		std::vector<glm::mat4> jointMatrices;
	};
	std::vector<SkinObject> skinObjects;

	// Animation
	struct SamplerObject {
		std::vector<float> input;
		std::vector<glm::vec4> output;
		int interpolation;
	};
	struct ChannelObject {
		int sampler;
		std::string targetPath;
		int targetNode;
	};
	struct AnimationObject {
		std::vector<SamplerObject> samplers;	// Animation data
	};
	std::vector<AnimationObject> animationObjects;

	glm::mat4 getNodeTransform(const tinygltf::Node& node) {
		glm::mat4 transform(1.0f);

		if (node.matrix.size() == 16) {
			transform = glm::make_mat4(node.matrix.data());
		} else {
			if (node.translation.size() == 3) {
				transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			}
			if (node.rotation.size() == 4) {
				glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
				transform *= glm::mat4_cast(q);
			}
			if (node.scale.size() == 3) {
				transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
			}
		}
		return transform;
	}

	void computeLocalNodeTransform(const tinygltf::Model& model,
		int nodeIndex,
		std::vector<glm::mat4> &localTransforms)
	{
		// ----------------------------------------
		// TODO: your code here

		const tinygltf::Node& node = model.nodes[nodeIndex];
		localTransforms[nodeIndex] = getNodeTransform(node);

		for (int childIndex : node.children) {
			computeLocalNodeTransform(model, childIndex, localTransforms);
		}
		// ----------------------------------------
	}

	void computeGlobalNodeTransform(const tinygltf::Model& model,
		const std::vector<glm::mat4> &localTransforms,
		int nodeIndex, const glm::mat4& parentTransform,
		std::vector<glm::mat4> &globalTransforms)
	{
		// ----------------------------------------
		// TODO: your code here

		globalTransforms[nodeIndex] = parentTransform * localTransforms[nodeIndex];
		for (int childIndex : model.nodes[nodeIndex].children) {
			computeGlobalNodeTransform(model, localTransforms, childIndex, globalTransforms[nodeIndex], globalTransforms);
		}
		// ----------------------------------------
	}

	std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model) {
		std::vector<SkinObject> skinObjects;

		// In our Blender exporter, the default number of joints that may influence a vertex is set to 4, just for convenient implementation in shaders.

		for (size_t i = 0; i < model.skins.size(); i++) {
			SkinObject skinObject;

			const tinygltf::Skin &skin = model.skins[i];
			std::cout << "Skin " << skin.joints.size() << std::endl;

			// Read inverseBindMatrices
			const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
			assert(accessor.type == TINYGLTF_TYPE_MAT4);
			const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
			const float *ptr = reinterpret_cast<const float *>(
            	buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);

			skinObject.inverseBindMatrices.resize(accessor.count);
			for (size_t j = 0; j < accessor.count; j++) {
				float m[16];
				memcpy(m, ptr + j * 16, 16 * sizeof(float));
				skinObject.inverseBindMatrices[j] = glm::make_mat4(m);
			}

			assert(skin.joints.size() == accessor.count);

			skinObject.globalJointTransforms.resize(skin.joints.size());
			skinObject.jointMatrices.resize(skin.joints.size());

			// ----------------------------------------------
			// TODO: your code here to compute joint matrices


			// Compute local transforms at each node
			int rootNodeIndex = skin.joints[0];
			std::vector<glm::mat4> localNodeTransforms(skin.joints.size());
			computeLocalNodeTransform(model, rootNodeIndex, localNodeTransforms);

			// Compute global transforms at each node
			glm::mat4 parentTransform(1.0f);
			computeGlobalNodeTransform(model, localNodeTransforms, rootNodeIndex, parentTransform, skinObject.globalJointTransforms);

			// Compute the inverseBindMatrix
			for(int j = 0; j < skinObject.jointMatrices.size(); j++) {
				int nodeIndex = skin.joints[j];
				skinObject.jointMatrices[j] = skinObject.globalJointTransforms[nodeIndex] * skinObject.inverseBindMatrices[j];
			}
			// ----------------------------------------------

			skinObjects.push_back(skinObject);
		}
		return skinObjects;
	}

	int findKeyframeIndex(const std::vector<float>& times, float animationTime)
	{
		int left = 0;
		int right = times.size() - 1;

		while (left <= right) {
			int mid = (left + right) / 2;

			if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
				return mid;
			}
			else if (times[mid] > animationTime) {
				right = mid - 1;
			}
			else { // animationTime >= times[mid + 1]
				left = mid + 1;
			}
		}

		// Target not found
		return times.size() - 2;
	}

	std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model)
	{
		std::vector<AnimationObject> animationObjects;
		for (const auto &anim : model.animations) {
			AnimationObject animationObject;

			for (const auto &sampler : anim.samplers) {
				SamplerObject samplerObject;

				const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
				const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
				const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

				assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

				// Input (time) values
				samplerObject.input.resize(inputAccessor.count);

				const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
				const float *inputBuf = reinterpret_cast<const float*>(inputPtr);

				// Read input (time) values
				int stride = inputAccessor.ByteStride(inputBufferView);
				for (size_t i = 0; i < inputAccessor.count; ++i) {
					samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
				}

				const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
				const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
				const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

				assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
				const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

				int outputStride = outputAccessor.ByteStride(outputBufferView);

				// Output values
				samplerObject.output.resize(outputAccessor.count);

				for (size_t i = 0; i < outputAccessor.count; ++i) {

					if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
						memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
					} else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
						memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
					} else {
						std::cout << "Unsupport accessor type ..." << std::endl;
					}

				}

				animationObject.samplers.push_back(samplerObject);
			}

			animationObjects.push_back(animationObject);
		}
		return animationObjects;
	}

	void updateAnimation(
		const tinygltf::Model &model,
		const tinygltf::Animation &anim,
		const AnimationObject &animationObject,
		float time,
		std::vector<glm::mat4> &nodeTransforms)
	{
		// There are many channels so we have to accumulate the transforms
		for (const auto &channel : anim.channels) {

			int targetNodeIndex = channel.target_node;
			const auto &sampler = anim.samplers[channel.sampler];

			// Access output (value) data for the channel
			const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
			const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
			const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

			// Calculate current animation time (wrap if necessary)
			const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
			float animationTime = fmod(time, times.back());

			// ----------------------------------------------------------
			// TODO: Find a keyframe for getting animation data

			int keyframeIndex = 0;

			keyframeIndex = findKeyframeIndex(times, animationTime);
			int nextKeyframeIndex = keyframeIndex + 1;

			float t0 = times[keyframeIndex];
			float t1 = times[nextKeyframeIndex];
			// ----------------------------------------------------------




			const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
			const float *outputBuf = reinterpret_cast<const float*>(outputPtr);



			// -----------------------------------------------------------
			// TODO: Add interpolation for smooth animation
			float interpolationFactor = (animationTime - t0) / (t1 - t0);

			if (channel.target_path == "translation") {
	            glm::vec3 translation0, translation1;

	            // Lire les données de translation des keyframes voisins
	            memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
	            memcpy(&translation1, outputPtr + nextKeyframeIndex * 3 * sizeof(float), 3 * sizeof(float));

	            // Interpolation linéaire
	            glm::vec3 interpolatedTranslation = glm::mix(translation0, translation1, interpolationFactor);

	            // Mettre à jour la transformation locale
	            nodeTransforms[targetNodeIndex] = glm::translate(nodeTransforms[targetNodeIndex], interpolatedTranslation);

	        } else if (channel.target_path == "rotation") {
	            glm::quat rotation0, rotation1;

	            // Lire les données de rotation des keyframes voisins
	            memcpy(&rotation0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));
	            memcpy(&rotation1, outputPtr + nextKeyframeIndex * 4 * sizeof(float), 4 * sizeof(float));

	            // Interpolation sphérique (SLERP) pour les quaternions
	            glm::quat interpolatedRotation = glm::slerp(rotation0, rotation1, interpolationFactor);

	            // Mettre à jour la transformation locale
	            nodeTransforms[targetNodeIndex] *= glm::mat4_cast(interpolatedRotation);

	        } else if (channel.target_path == "scale") {
	            glm::vec3 scale0, scale1;

	            // Lire les données d'échelle des keyframes voisins
	            memcpy(&scale0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
	            memcpy(&scale1, outputPtr + nextKeyframeIndex * 3 * sizeof(float), 3 * sizeof(float));

	            // Interpolation linéaire
	            glm::vec3 interpolatedScale = glm::mix(scale0, scale1, interpolationFactor);

	            // Mettre à jour la transformation locale
	            nodeTransforms[targetNodeIndex] = glm::scale(nodeTransforms[targetNodeIndex], interpolatedScale);
	        }
		}
	}

	void updateSkinning(const std::vector<glm::mat4> &nodeTransforms) {

		// -------------------------------------------------
		// TODO: Recompute joint matrices
		for(int j = 0; j < skinObjects[0].jointMatrices.size(); j++) {
			int nodeIndex = model.skins[0].joints[j];
			skinObjects[0].jointMatrices[j] = skinObjects[0].globalJointTransforms[nodeIndex] * skinObjects[0].inverseBindMatrices[j];
		}


		// -------------------------------------------------
	}

	void update(float time) {

		// -------------------------------------------------
		// TODO: your code here
		if (model.animations.size() > 0) {
			const tinygltf::Animation &animation = model.animations[0];
			const AnimationObject &animationObject = animationObjects[0];

			const tinygltf::Skin &skin = model.skins[0];
			std::vector<glm::mat4> nodeTransforms(skin.joints.size());
			for (size_t i = 0; i < nodeTransforms.size(); ++i) {
				nodeTransforms[i] = glm::mat4(1.0);
			}

			updateAnimation(model, animation, animationObject, time, nodeTransforms);

			// ----------------------------------------------

			// ----------------------------------------------
			// TODO: Recompute global transforms at each node
			std::vector<glm::mat4> globalNodeTransforms(skin.joints.size(), glm::mat4(1.0f));


			int rootNodeIndex = skin.joints[0]; // Premier joint (racine)
			computeGlobalNodeTransform(model, nodeTransforms, rootNodeIndex, glm::mat4(1.0f), globalNodeTransforms);


			skinObjects[0].globalJointTransforms = globalNodeTransforms;


			updateSkinning(nodeTransforms);
			// ----------------------------------------------
		}
		// -------------------------------------------------

	}

	bool loadModel(tinygltf::Model &model, const char *filename) {
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		if (!warn.empty()) {
			std::cout << "WARN: " << warn << std::endl;
		}

		if (!err.empty()) {
			std::cout << "ERR: " << err << std::endl;
		}

		if (!res)
			std::cout << "Failed to load glTF: " << filename << std::endl;
		else
			std::cout << "Loaded glTF: " << filename << std::endl;

		return res;
	}

	void initialize( glm::vec3 position,glm::vec3 scale, glm::vec3 axis, float angle) {
		this->position = position;
		this->scale = scale;
		this->axis = axis;
		this->angle = angle;

		// Modify your path if needed
		if (!loadModel(model, "../finalProject/bot/bird.gltf")) {
			return;
		}

		// Prepare buffers for rendering
		primitiveObjects = bindModel(model);

		// Prepare joint matrices
		skinObjects = prepareSkinning(model);

		// Prepare animation data
		animationObjects = prepareAnimation(model);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../finalProject/shader/bird/bird.vert", "../finalProject/shader/bird/bird.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for GLSL variables
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		lightPositionID = glGetUniformLocation(programID, "lightPosition");
		lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
		jointMatricesID = glGetUniformLocation(programID, "jointMatrices");

		glActiveTexture(GL_TEXTURE4);

		// Load a texture
		textureID = LoadTextureTileBox("../finalProject/textures/bird.png");

		// Get a handle for our "textureSampler" uniform
		textureSamplerID = glGetUniformLocation(programID,"textureSampler");

		// Ensure no VAO or shader program is left active
		glBindVertexArray(0);  // Unbind any VAO
		glUseProgram(0);       // Unbind any shader program
		glBindBuffer(GL_ARRAY_BUFFER, 0);


	}

	void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {

		std::map<int, GLuint> vbos;
		for (size_t i = 0; i < model.bufferViews.size(); ++i) {
			const tinygltf::BufferView &bufferView = model.bufferViews[i];

			int target = bufferView.target;

			if (bufferView.target == 0) {
				// The bufferView with target == 0 in our model refers to
				// the skinning weights, for 25 joints, each 4x4 matrix (16 floats), totaling to 400 floats or 1600 bytes.
				// So it is considered safe to skip the warning.
				//std::cout << "WARN: bufferView.target is zero" << std::endl;
				continue;
			}

			const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(target, vbo);
			glBufferData(target, bufferView.byteLength,
						&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

			vbos[i] = vbo;
		}

		// Each mesh can contain several primitives (or parts), each we need to
		// bind to an OpenGL vertex array object
		for (size_t i = 0; i < mesh.primitives.size(); ++i) {

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			GLuint vao;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			for (auto &attrib : primitive.attributes) {
				tinygltf::Accessor accessor = model.accessors[attrib.second];
				int byteStride =
					accessor.ByteStride(model.bufferViews[accessor.bufferView]);
				glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

				int size = 1;
				if (accessor.type != TINYGLTF_TYPE_SCALAR) {
					size = accessor.type;
				}

				int vaa = -1;
				if (attrib.first.compare("POSITION") == 0) vaa = 0;
				if (attrib.first.compare("NORMAL") == 0) vaa = 1;
				if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
				if (attrib.first.compare("JOINTS_0") == 0) vaa = 3;
				if (attrib.first.compare("WEIGHTS_0") == 0) vaa = 4;
				if (vaa > -1) {
					glEnableVertexAttribArray(vaa);
					glVertexAttribPointer(vaa, size, accessor.componentType,
										accessor.normalized ? GL_TRUE : GL_FALSE,
										byteStride, BUFFER_OFFSET(accessor.byteOffset));
				} else {
					std::cout << "vaa missing: " << attrib.first << std::endl;
				}
			}

			// Record VAO for later use
			PrimitiveObject primitiveObject;
			primitiveObject.vao = vao;
			primitiveObject.vbos = vbos;
			primitiveObjects.push_back(primitiveObject);

			glBindVertexArray(0);
		}
	}

	void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
						tinygltf::Model &model,
						tinygltf::Node &node) {
		// Bind buffers for the current mesh at the node
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			bindMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}

		// Recursive into children nodes
		for (size_t i = 0; i < node.children.size(); i++) {
			assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
		}
	}

	std::vector<PrimitiveObject> bindModel(tinygltf::Model &model) {
		std::vector<PrimitiveObject> primitiveObjects;

		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
		}

		return primitiveObjects;
	}

	void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {

		for (size_t i = 0; i < mesh.primitives.size(); ++i)
		{
			GLuint vao = primitiveObjects[i].vao;
			std::map<int, GLuint> vbos = primitiveObjects[i].vbos;

			glBindVertexArray(vao);

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

			glDrawElements(primitive.mode, indexAccessor.count,
						indexAccessor.componentType,
						BUFFER_OFFSET(indexAccessor.byteOffset));

			glBindVertexArray(0);
		}
	}

	void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
						tinygltf::Model &model, tinygltf::Node &node) {
		// Draw the mesh at the node, and recursively do so for children nodes
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}
		for (size_t i = 0; i < node.children.size(); i++) {
			drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
		}
	}
	void drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
				tinygltf::Model &model) {
		// Draw all nodes
		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
		}
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);


		glm::mat4 modelMatrix = glm::mat4();
		modelMatrix = glm::scale(modelMatrix, scale);
		modelMatrix = glm::rotate(modelMatrix, angle, axis);
		modelMatrix = glm::translate(modelMatrix, position);
		// Set camera
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


		// -----------------------------------------------------------------
		// TODO: Set animation data for linear blend skinning in shader

		glUniformMatrix4fv(jointMatricesID, skinObjects[0].jointMatrices.size(), GL_FALSE, &skinObjects[0].jointMatrices[0][0][0]);

		// -----------------------------------------------------------------

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 4);

		// Set light data
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);

		// Draw the GLTF model
		drawModel(primitiveObjects, model);
	}

	void cleanup() {
		glDeleteProgram(programID);
	}
};





struct Bamboo {

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint jointMatricesID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint programID;
	GLuint textureID;
	GLuint textureSamplerID;
	GLuint instanceBufferID;

	tinygltf::Model model;

	// Each VAO corresponds to each mesh primitive in the GLTF model
	struct PrimitiveObject {
		GLuint vao;
		std::map<int, GLuint> vbos;
	};
	std::vector<PrimitiveObject> primitiveObjects;


	glm::mat4 getNodeTransform(const tinygltf::Node& node) {
		glm::mat4 transform(1.0f);

		if (node.matrix.size() == 16) {
			transform = glm::make_mat4(node.matrix.data());
		} else {
			if (node.translation.size() == 3) {
				transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			}
			if (node.rotation.size() == 4) {
				glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
				transform *= glm::mat4_cast(q);
			}
			if (node.scale.size() == 3) {
				transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
			}
		}
		return transform;
	}

	bool loadModel(tinygltf::Model &model, const char *filename) {
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		if (!warn.empty()) {
			std::cout << "WARN: " << warn << std::endl;
		}

		if (!err.empty()) {
			std::cout << "ERR: " << err << std::endl;
		}

		if (!res)
			std::cout << "Failed to load glTF: " << filename << std::endl;
		else
			std::cout << "Loaded glTF: " << filename << std::endl;

		return res;
	}

	void initialize() {

		// Modify your path if needed
		if (!loadModel(model, "../finalProject/bot/bamboo.gltf")) {
			return;
		}

		// Prepare buffers for rendering
		primitiveObjects = bindModel(model);


		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../finalProject/shader/bamboo/bamboo.vert", "../finalProject/shader/bamboo/bamboo.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for GLSL variables
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		lightPositionID = glGetUniformLocation(programID, "lightPosition");
		lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
		jointMatricesID = glGetUniformLocation(programID, "jointMatrices");

		glActiveTexture(GL_TEXTURE5);


		glGenBuffers(1, &instanceBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);
		glBufferData(GL_ARRAY_BUFFER, Transform_tree.size() * sizeof(Instance_tree), Transform_tree.data(), GL_STATIC_DRAW);




		// Load a texture
		textureID = LoadTextureTileBox("../finalProject/textures/bamboo.png");

		// Get a handle for our "textureSampler" uniform
		textureSamplerID = glGetUniformLocation(programID,"textureSampler");

		// Ensure no VAO or shader program is left active
		glBindVertexArray(0);  // Unbind any VAO
		glUseProgram(0);       // Unbind any shader program
		glBindBuffer(GL_ARRAY_BUFFER, 0);


	}

	void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {

		std::map<int, GLuint> vbos;
		for (size_t i = 0; i < model.bufferViews.size(); ++i) {
			const tinygltf::BufferView &bufferView = model.bufferViews[i];

			int target = bufferView.target;

			if (bufferView.target == 0) {
				// The bufferView with target == 0 in our model refers to
				// the skinning weights, for 25 joints, each 4x4 matrix (16 floats), totaling to 400 floats or 1600 bytes.
				// So it is considered safe to skip the warning.
				//std::cout << "WARN: bufferView.target is zero" << std::endl;
				continue;
			}

			const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(target, vbo);
			glBufferData(target, bufferView.byteLength,
						&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

			vbos[i] = vbo;
		}

		// Each mesh can contain several primitives (or parts), each we need to
		// bind to an OpenGL vertex array object
		for (size_t i = 0; i < mesh.primitives.size(); ++i) {

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			GLuint vao;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			for (auto &attrib : primitive.attributes) {
				tinygltf::Accessor accessor = model.accessors[attrib.second];
				int byteStride =
					accessor.ByteStride(model.bufferViews[accessor.bufferView]);
				glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

				int size = 1;
				if (accessor.type != TINYGLTF_TYPE_SCALAR) {
					size = accessor.type;
				}

				int vaa = -1;
				if (attrib.first.compare("POSITION") == 0) vaa = 0;
				if (attrib.first.compare("NORMAL") == 0) vaa = 1;
				if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
				if (attrib.first.compare("JOINTS_0") == 0) vaa = 3;
				if (attrib.first.compare("WEIGHTS_0") == 0) vaa = 4;
				if (vaa > -1) {
					glEnableVertexAttribArray(vaa);
					glVertexAttribPointer(vaa, size, accessor.componentType,
										accessor.normalized ? GL_TRUE : GL_FALSE,
										byteStride, BUFFER_OFFSET(accessor.byteOffset));
				} else {
					std::cout << "vaa missing: " << attrib.first << std::endl;
				}
			}

			// Record VAO for later use
			PrimitiveObject primitiveObject;
			primitiveObject.vao = vao;
			primitiveObject.vbos = vbos;
			primitiveObjects.push_back(primitiveObject);

			glBindVertexArray(0);
		}
	}

	void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
						tinygltf::Model &model,
						tinygltf::Node &node) {
		// Bind buffers for the current mesh at the node
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			bindMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}

		// Recursive into children nodes
		for (size_t i = 0; i < node.children.size(); i++) {
			assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
		}
	}

	std::vector<PrimitiveObject> bindModel(tinygltf::Model &model) {
		std::vector<PrimitiveObject> primitiveObjects;

		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
		}

		return primitiveObjects;
	}

	void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {

		for (size_t i = 0; i < mesh.primitives.size(); ++i)
		{
			GLuint vao = primitiveObjects[i].vao;
			std::map<int, GLuint> vbos = primitiveObjects[i].vbos;

			glBindVertexArray(vao);

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

			glEnableVertexAttribArray(6);
			glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);
			glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Instance_tree), (void*)0);
			glVertexAttribDivisor(6, 1);

			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(Instance_tree), (void*)offsetof(Instance_tree, scale));
			glVertexAttribDivisor(7, 1);

			glDrawElementsInstanced(primitive.mode, indexAccessor.count,
						indexAccessor.componentType,
						BUFFER_OFFSET(indexAccessor.byteOffset),
						Transform_tree.size()
						);

			glBindVertexArray(0);
		}
	}

	void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
						tinygltf::Model &model, tinygltf::Node &node) {
		// Draw the mesh at the node, and recursively do so for children nodes
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}
		for (size_t i = 0; i < node.children.size(); i++) {
			drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
		}
	}
	void drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
				tinygltf::Model &model) {
		// Draw all nodes
		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
		}
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);


		glm::mat4 modelMatrix = glm::mat4();
		modelMatrix = glm::rotate(modelMatrix,glm::radians(90.0f),glm::vec3(1,0,0));
		// Set camera
		glm::mat4 mvp = cameraMatrix * modelMatrix;


		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


		// -----------------------------------------------------------------
		// TODO: Set animation data for linear blend skinning in shader

		// -----------------------------------------------------------------

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 5);

		// Set light data
		glUniform3fv(lightPositionID, 1, &lightPosition3[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity3[0]);

		// Draw the GLTF model
		drawModel(primitiveObjects, model);
	}

	void cleanup() {
		glDeleteProgram(programID);
	}
};





int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, "Final Project", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	glfwGetFramebufferSize(window, &shadowMapWidth, &shadowMapHeight);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);



	glGenTextures(1, &depthTexture);


	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,shadowMapWidth,shadowMapHeight,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Erreur : FBO incomplet" << std::endl;
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	// Background
	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);


	for (int i = 0; i < 20; ++i) {
		glm::vec3 position(-3500 - (i * 100), -1600 + (i*100), 200 );
		glm::vec3 scale(1, 1, 1);
		Transform_tree.push_back({position, scale});
	}
	for (int i = 0; i < 20; ++i) {
		glm::vec3 position(-4000 - (i * 100), -1600 + (i*100), 100 );
		glm::vec3 scale(1, 1, 1);
		Transform_tree.push_back({position, scale});
	}

	Bird my_bird;
	my_bird.initialize(glm::vec3(0,3000,1000),
		glm::vec3(1,1,1),
		glm::vec3(1,0,0),
		glm::radians(45.0f)
		);


	Bamboo bamboo;
	bamboo.initialize();


	Building my_building;
	my_building.initialize(glm::vec3(9000,-10000,9000),glm::vec3(1000,10000,1000));

	Building my_building2;
	my_building2.initialize(glm::vec3(0,-10000,9000),glm::vec3(1000,8000,1000));

	Building my_building3;
	my_building3.initialize(glm::vec3(-9000,-10000,-9000),glm::vec3(1000,10000,1000));

	Building my_building4;
	my_building4.initialize(glm::vec3(300,150,9000),glm::vec3(200,400,200));

	SkyBox my_sky_box;
	my_sky_box.initialize(glm::vec3 (0, 0, 0),
					 glm::vec3(10000, 10000, 10000)

	);



	// Camera setup


    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);

	/*
	eye_center.x = -1500.0f;
	eye_center.y = 0.0f;
	eye_center.z = 0.0f;
	*/

	glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 45;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 20000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);





	glm::mat4 lightProjectionMatrix = glm::ortho(
	orthoLeft, orthoRight,
	orthoBottom, orthoTop,
	orthoNear, orthoFar
);


	static double lastTime = glfwGetTime();
	float time = 0.0f;			// Animation time
	float fTime = 0.0f;			// Time for measuring fps
	unsigned long frames = 0;


	do
	{

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, glm::vec3(-9900.0f,-9900.0f,-9900.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		//glm::mat4 lightViewMatrix = glm::lookAt(lightPosition,lightPosition + glm::normalize(lightDirection),glm::vec3(0,1,0));
		glm::mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;



		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		my_sky_box.render_depth(lightSpaceMatrix);

		my_building.render_depth(lightSpaceMatrix);
		my_building2.render_depth(lightSpaceMatrix);





		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		viewMatrix = glm::lookAt(eye_center, lookat+eye_center, up);


	//Camera Lab4 //viewMatrix = glm::lookAt(eye_center, glm::vec3(0,0,0), glm::vec3(0,1,0));
		glm::mat4 vp = projectionMatrix * viewMatrix;



		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		if (playAnimation) {
			time += deltaTime * playbackSpeed;
			my_bird.update(time);
		}

		//glDepthMask(GL_FALSE);
		my_sky_box.render(vp,lightSpaceMatrix);
		//glDepthMask(GL_TRUE);
		bamboo.render(vp);
		my_bird.render(vp);
		my_building.render(vp,lightSpaceMatrix);
		my_building2.render(vp,lightSpaceMatrix);
		my_building3.render(vp,lightSpaceMatrix);
		my_building4.render(vp,lightSpaceMatrix);




		frames++;
		fTime += deltaTime;
		if (fTime > 2.0f) {
			float fps = frames / fTime;
			frames = 0;
			fTime = 0;

			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << "Final Project | Frames per second (FPS): " << fps;
			glfwSetWindowTitle(window, stream.str().c_str());
		}




		if (saveDepth) {
			std::string filename = "../finalProject/depth_camera.png";
			saveDepthTexture(fbo, filename);
			std::cout << "Depth texture saved to " << filename << std::endl;
			saveDepth = false;
		}
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	my_sky_box.cleanup();
	my_building.cleanup();
	my_bird.cleanup();
	bamboo.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	static float deltaTime = 0.016f;
	static float pitch = 0.0f;
	static float yaw = 0.0f;
	const float maxPitch = 89.0f;

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_W) {
			eye_center += lookat * cameraSpeed * deltaTime;
		}
		if (key == GLFW_KEY_S) {
			eye_center -= lookat * cameraSpeed * deltaTime;
		}
		if (key == GLFW_KEY_LEFT) {
			yaw -= 5.0f;
		}
		if (key == GLFW_KEY_RIGHT) {
			yaw += 5.0f;
		}
		if (key == GLFW_KEY_UP) {
			pitch += 5.0f;
			if (pitch > maxPitch) pitch = maxPitch;
		}
		if (key == GLFW_KEY_DOWN) {
			pitch -= 5.0f;
			if (pitch < -maxPitch) pitch = -maxPitch;
		}
	}


	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	lookat = glm::normalize(direction);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}







void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	static bool firstMouse = true;
	static float lastX = 400, lastY = 300;
	static float sensitivity = 0.05f;

	static float yaw = -90.0f;
	static float pitch = 0.0f;

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;


	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;


	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookat = glm::normalize(front);
}
