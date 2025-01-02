
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vector>
#include <iostream>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>




static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);



// OpenGL camera view parameters
static glm::vec3 eye_center;
static glm::vec3 lookat(0, 0, -1);
static glm::vec3 up(0, 1, 0);
float cameraSpeed = 100.0f;
float yaw = -90.0f;
float pitch = 0.0f;

const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);

static glm::vec3 lightIntensity = 100.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
static glm::vec3 light_lookat(0.0f, -1.0f, 0.0f);
static glm::vec3 lightDirection (-1.0f, -1.0f, -1.0f);
static glm::vec3 lightPosition(0,10000,0);

static glm::vec3 Position(0,0,0);
static glm::vec3 Target = Position + lightDirection;



float orthoLeft = -11000.0f;
float orthoRight = 11000.0f;
float orthoBottom = -11000.0f;
float orthoTop = 11000.0f;
float orthoNear = 10.0f;
float orthoFar = 30000.0f;


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

void afficherMatrice(const glm::mat4& mat) {
	for (int i = 0; i < 4; ++i) { // Parcours des lignes
		for (int j = 0; j < 4; ++j) { // Parcours des colonnes
			std::cout << mat[i][j] << " "; // Accès à l'élément [i][j]
		}
		std::cout << std::endl; // Nouvelle ligne après chaque ligne
	}
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
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		 // Bottom - we do not want texture the bottom
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
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
		glGenBuffers(1, &normalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		BuildingprogramID = LoadShadersFromFile("../finalProject3/box.vert", "../finalProject3/box.frag");
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

		BuildingtextureID = LoadTextureTileBox("../finalProject3/test.jpg");
		textureBuildingSamplerID = glGetUniformLocation(BuildingprogramID,"buildingSampler");

		depthProgramID = LoadShadersFromFile("../finalProject3/depth.vert", "../finalProject3/depth.frag");
		if (depthProgramID == 0) {
			std::cerr << "Failed to load depth shaders." << std::endl;
		}

		depthmvpMatrixID = glGetUniformLocation(depthProgramID, "lightSpaceMatrix");
		modelID = glGetUniformLocation(depthProgramID, "model");
	}

	void render(glm::mat4 cameraMatrix,glm::mat4 lightSpaceMatrix) {
		glUseProgram(BuildingprogramID);

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

		glUniform1i(shadowMapID,0);
		glUniformMatrix4fv(lightmvpMatrixID, 1, GL_FALSE, &lightmvp[0][0]);

		//glUniformMatrix4fv(modelID,1, GL_FALSE, &modelMatrix[0][0]);

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
		SkyboxprogramID = LoadShadersFromFile("../finalProject3/skybox.vert", "../finalProject3/skybox.frag");
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



		SkyboxtextureID = LoadTextureTileBox("../finalProject3/StandardCubeMap.png");

        // TODO: Get a handle to texture sampler
        // -------------------------------------
		textureSkyboxSamplerID = glGetUniformLocation(SkyboxprogramID,"SkyboxSampler");

		depthProgramID = LoadShadersFromFile("../finalProject3/depth.vert", "../finalProject3/depth.frag");
		if (depthProgramID == 0) {
			std::cerr << "Failed to load depth shaders." << std::endl;
		}

		depthmvpMatrixID = glGetUniformLocation(depthProgramID, "lightSpaceMatrix");
        // -------------------------------------
	}

	void render(glm::mat4 cameraMatrix,glm::mat4 lightSpaceMatrix) {
		glUseProgram(SkyboxprogramID);

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

		glUniform1i(shadowMapID, 0);

		glUniformMatrix4fv(lightmvpMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);


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

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// Set light-space matrix

		glUniformMatrix4fv(depthmvpMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		// Draw the box
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, (void*)0);

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
	glfwSetCursorPosCallback(window, mouse_callback);

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
	glEnable(GL_CULL_FACE);

	Building my_building;
	my_building.initialize(glm::vec3(-6000.0f,  -10000.0f, -6000.0f),
							glm::vec3(1000.0f, 3000.0f, 1000.0f)
							);

	SkyBox my_sky_box;
	my_sky_box.initialize(glm::vec3 (0, 0, 0),
					 glm::vec3(10000, 10000, 10000)

	);


	// Camera setup
    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);


	glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 45;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 20000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);


	//glm::float32 depthFov = 80.0f;
	//glm::float32 depthNear = 10.0f;
	//glm::float32 depthFar = 10000.0f;




	glm::mat4 lightProjectionMatrix = glm::ortho(
	orthoLeft, orthoRight,
	orthoBottom, orthoTop,
	orthoNear, orthoFar
);




/*

	glm::mat4 lightProjectionMatrix = glm::perspective(glm::radians(depthFov), (float)shadowMapWidth / shadowMapHeight, depthNear, depthFar);
*/
	do
	{

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		//glm::mat4 lightViewMatrix = glm::lookAt(lightPosition,lightPosition + glm::normalize(lightDirection),glm::vec3(0,1,0));
		glm::mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;



		//glViewport(0,0,shadowMapWidth,shadowMapHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		my_sky_box.render_depth(lightSpaceMatrix);
		my_building.render_depth(lightSpaceMatrix);



		//glViewport(0,0,windowWidth,windowHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		viewMatrix = glm::lookAt(eye_center, lookat+eye_center, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		//glBindTexture(GL_TEXTURE_2D, depthTexture);

		glDepthMask(GL_FALSE);
		my_sky_box.render(vp,lightSpaceMatrix);
		glDepthMask(GL_TRUE);

		my_building.render(vp,lightSpaceMatrix);

		if (saveDepth) {
			std::string filename = "../finalProject3/depth_camera3.png";
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
