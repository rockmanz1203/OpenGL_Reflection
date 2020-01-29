// Include standard headers

#include <stdio.h>
#include <stdlib.h>
#include <vector>
// Include GLEW
#include "include\GL\glew.h"

// Include GLFW
#include "include\GLFW\glfw3.h"
GLFWwindow* window; //建立一個windows

// Include GLM
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
//#include "glad/glad.h"
using namespace glm;


#include "Loadshader.h"
#include "shader.h"
#include "model.h"
#include "filesystem.h"
#include "camera.h"
#include<assimp/Importer.hpp>//測試assimp匯入 2018.7.15


//include library for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>



bool firstMouse = true;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void renderScene(const Shader &shader);
void renderShadowScene(const Shader &shader);
void renderCube();
void renderQuad();//render 手動設定的plane並計算tangent bitanget TBN矩陣

//setting
const unsigned int SCR_width = 1080;
const unsigned int SCR_height = 720;
float heightScale = 0.1;

//nothing
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 lightcube_Pos(0.2f, 0.8f, 2.0f);

//camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));//實際用到的camera 與位置

float lastX = SCR_width / 2.0f;
float lastY = SCR_height / 2.0f;

//timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int planeVAO;

int main(void)
{
	// Initialise GLFW
	{
	if (!glfwInit())//如果初始化不成功
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//初始化設定

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCR_width, SCR_height, "Tutorial 01", NULL, NULL);//設定視窗名與大小

	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);///讓特定window的context流行於thread中
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//glfwSetScrollCallback(window,scorll)

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {//check glew是否正常運作中
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
}
	
	//int shaderProgram = LoadShaders("vertex.glsl", "fragment.glsl");//透過loadshader.h的function來載入兩個shader程式
	//int lamp_shaderProgram = LoadShaders("lamp_vertex.glsl", "lamp_fragment.glsl");
	glEnable(GL_DEPTH_TEST);
	
	
	//要來shadow mapping的主shader 
	Shader ourshader("vertex.glsl","fragment.glsl");
	//生成shadow map的framebuffer shader
	Shader simpleDepthShader("shadow_map_depth_vs.glsl","shadow_map_depth_fs.glsl");
	//把shadow map呈現在畫面上debug用的shader
	Shader debugDepthQuad("print_shadow_map_vs.glsl", "print_shadow_map_fs.glsl");
	
	Shader skybox("skybox_vertex.glsl","skybox_fragment.glsl");

	Shader planeRender("plane_vertex.glsl", "plane_fragment.glsl");

	Model ourModel("resource/bunny.obj");

	// set up vertex data (and buffer(s)) and configure vertex attributes 建立plane的三角片座標與設定VAO VBO
	// ------------------------------------------------------------------
	float planeVertices[] = {
		// positions            // normals         // texcoords
		5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,

		5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,
		5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f
	};

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};


	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	
	glBindVertexArray(0);
	
	
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	

	glBindVertexArray(0);

	//load texture
	unsigned int woodTexture = loadTexture("resource/textures/grass.jpg");

	vector<string> faces{
		"resource/textures/skybox/new/right.jpg",
		"resource/textures/skybox/new/left.jpg",
		"resource/textures/skybox/new/top.jpg",
		"resource/textures/skybox/new/bottom.jpg",
		"resource/textures/skybox/new/front.jpg",
		"resource/textures/skybox/new/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	skybox.use();
	skybox.setInt("skybox", 0);

	//設定Framebuffer object FBO以產生shadow map(光源出發的depth map)
	const unsigned int SHADOW_WIDTH = 1024;
	const unsigned int SHADOW_HEIGHT = 1024;
	unsigned int shadowMapFBO;
	glGenFramebuffers(1, &shadowMapFBO);

	//產生光源出發的depth map (shadow map)
	unsigned int shadowMap;
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
		//建立2D texture給 shadow map FBO使用(輸出結果)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Bind 2D texture到 FBO的depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
			//告訴renderer 我們frame buffer只需要depth buffer ，沒有要畫任何顏色
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		//啟動前進入render loop前，一定要先宣告告訴給個shader他們的貼圖編號，不然有時候會mapping錯發生不明所以的問題
		ourshader.use();
		ourshader.setInt("diffuseTexture", 0);
		ourshader.setInt("shadowMap", 1);
		ourshader.setInt("skybox", 2);//在ourshader中mapping了環境貼圖

		planeRender.use();
		planeRender.setInt("diffuseTexture", 0);
		planeRender.setInt("shadowMap", 1);
		planeRender.setInt("skybox", 2);//在ourshader中mapping了環境貼圖

		debugDepthQuad.use();
		debugDepthQuad.setInt("depthMap", 0);

		glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

		
	

	// triangle

	
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	

	glfwSwapInterval(0);


	//render loop
	float previousTime = (float) glfwGetTime();
	int frameCount = 0;

	do {
		// -----
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;



		processInput(window);
		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!


		// 1.繪製光源深度圖到texture
		glm::mat4 lightProjection;
		glm::mat4 lightView;

		glm::mat4 lightSpaceMatrix;

		float near_plane = 1.0f, far_plane = 14.5f;

		//lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightProjection = glm::perspective(glm::radians(90.0f), (float)SCR_width / (float)SCR_height, near_plane, 100.0f);
		//glm::perspective(glm::radians(camera.Zoom), (float)SCR_width / (float)SCR_height, 0.1f, 100.0f);

		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		//從光源出發Render出深度圖
		simpleDepthShader.use();
		simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		//畫到frame buffer的深度貼圖裡
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		//glActiveTexture(GL_TEXTURE0);//啟用0號貼圖
		//glBindTexture(GL_TEXTURE_2D, woodTexture);
		renderShadowScene(simpleDepthShader);//給定scene中3D object所在的model矩陣
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderShadowScene(simpleDepthShader);
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
		//model = glm::scale(model, glm::vec3(1.0f));
		simpleDepthShader.setMat4("model", model);
		ourModel.Draw(simpleDepthShader);

		/*
		model = glm::translate(model, glm::vec3(0.5, 0.0, 0.0));
		simpleDepthShader.setMat4("model", model);
		ourModel.Draw(simpleDepthShader);
		*/
	



		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2.使用生成的shadow map來繪製scene場景
		glViewport(0, 0, SCR_width, SCR_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		{
			
			planeRender.use();
				{
					glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_width / (float)SCR_height, 0.1f, 100.0f);
					//glm::mat4 projection = glm::ortho(0, 1, 0, 1);
					glm::mat4 view = camera.GetViewMatrix();
					planeRender.setMat4("projection", projection);
					planeRender.setMat4("view", view);
					// set light uniforms
					planeRender.setVec3("viewPos", camera.Position);
					planeRender.setVec3("lightPos", lightPos);
					planeRender.setMat4("lightSpaceMatrix", lightSpaceMatrix);



					//在ourshader中配置貼圖
					glActiveTexture(GL_TEXTURE0);//啟用0號貼圖，當作木頭貼圖
					glBindTexture(GL_TEXTURE_2D, woodTexture);

					glActiveTexture(GL_TEXTURE1);//啟用1號貼圖給ourshader，當作shadow map貼圖
					glBindTexture(GL_TEXTURE_2D, shadowMap);

					glActiveTexture(GL_TEXTURE2);//啟用2號貼圖，當作環境貼圖
					glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

					//render地板
					glm::mat4 model = glm::mat4(1.0f);
					planeRender.setMat4("model", model);
					glBindVertexArray(planeVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					
					
					glBindVertexArray(0);

				}



				//使用ourshader的狀況
				ourshader.use();
					{
						glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_width / (float)SCR_height, 0.1f, 100.0f);
						//glm::mat4 projection = glm::ortho(0, 1, 0, 1);
						glm::mat4 view = camera.GetViewMatrix();
						ourshader.setMat4("projection", projection);
						ourshader.setVec3("cameraPos", camera.Position);
						ourshader.setMat4("view", view);
						// set light uniforms
						ourshader.setVec3("viewPos", camera.Position);
						ourshader.setVec3("lightPos", lightPos);
						ourshader.setMat4("lightSpaceMatrix", lightSpaceMatrix);


						//在ourshader中配置貼圖
						glActiveTexture(GL_TEXTURE0);//啟用0號貼圖，當作木頭貼圖
						glBindTexture(GL_TEXTURE_2D, woodTexture);

						glActiveTexture(GL_TEXTURE1);//啟用1號貼圖給ourshader，當作shadow map貼圖
						glBindTexture(GL_TEXTURE_2D, shadowMap);

						glActiveTexture(GL_TEXTURE2);//啟用2號貼圖，當作環境貼圖
						glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

						//renderShadowScene(ourshader);//全部的模型資料
						renderScene(ourshader);//只render cube
						glm::mat4 model(1.0f);
						//model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f)); // translate it down so it's at the center of the scene
						//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
						//model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
						ourshader.setMat4("model", model);
						ourModel.Draw(ourshader);

					    /*
						model = glm::translate(model, glm::vec3(0.5, 0.0, 0.0));
						ourshader.setMat4("model", model);
						ourModel.Draw(ourshader);
						*/
					}
		
					
					//畫出skybox
					glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
					skybox.use();
						{
							glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_width / (float)SCR_height, 0.1f, 100.0f);
							glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix 移除位移的部分，讓skybox不會跟著玩家一起動
							skybox.setMat4("view", view);
							skybox.setMat4("projection", projection);
							// skybox cube
							glBindVertexArray(skyboxVAO);
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
							glDrawArrays(GL_TRIANGLES, 0, 36);
							glBindVertexArray(0);
							glDepthFunc(GL_LESS); // set depth function back to default
						}


					}
		  
			//---------debug用
			// render Depth map to quad for visual debugging
			// ---------------------------------------------
			debugDepthQuad.use();
			debugDepthQuad.setFloat("near_plane", near_plane);
			debugDepthQuad.setFloat("far_plane", far_plane);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMap);
			
			//觀察shadow map
			//renderQuad();

			// glfw: swap buffers a
			


			//FPS compute
			
			float currentTime = (float)glfwGetTime();
			frameCount++;
			// If a second has passed.
			if (currentTime - previousTime >= 1.0)
			{
				// Display the frame count here any way you want.
				cout << "FPS : " << frameCount << endl;

				frameCount = 0;
				previousTime = currentTime;
			}

			//cout << "camera_position :(" << camera.Position.x<<","<< camera.Position.y<<","<< camera.Position.z<<")" << endl;

		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}


// renders the 3D scene
// --------------------
void renderShadowScene(const Shader &shader)
{
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// cubes
	/*
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	*/
	/*
	model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	*/
	/*
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
	*/
}
void renderScene(const Shader &shader)
{
	// floor
	/*	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	*/
	// cubes
	/*
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	*/
	/*
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	*/
	/*
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
	*/
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

//看有幾張skybox貼圖要load就一口氣load完並配置好
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
/*void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}*/