#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Louron.h"

class Scene3 {

private:

	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

public:

	Scene3() :
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary())
	{			
		std::cout << "[L20] Opening Scene 3..." << std::endl;

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_ShaderLib.LoadShader("assets/Shaders/Basic/basic_texture.glsl");
			
		m_ShaderLib.GetShader("basic_texture")->Bind();
		m_ShaderLib.GetShader("basic_texture")->SetInt("ourTexture", 0);
		m_ShaderLib.GetShader("basic_texture")->UnBind();

		m_TextureLib.loadTexture("assets/Images/cube_texture.png");
	}
	~Scene3()
	{
		std::cout << "[L20] Closing Scene 3..." << std::endl;
		glDisable(GL_DEPTH_TEST);
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}


	//Private Scene Variables
private:
	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
	int speed = 0;

	Louron::TransformComponent trans;

	glm::vec4 back_colour = glm::vec4(0.674f, 0.992f, 0.588f, 1.0f);
	glm::vec4 fore_colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//Public Functions
public:

	void Update() {
		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Draw();
	}

	void UpdateGUI() {

		ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

		ImGui::InputInt("Speed", &speed);
		ImGui::DragFloat3("Translate", glm::value_ptr(trans.position), 0.01f, 0, 0, "%.2f");
		ImGui::DragFloat3("Rotate", glm::value_ptr(trans.rotation), 1.0f, 0, 0, "%.2f");
		ImGui::DragFloat3("Scale", glm::value_ptr(trans.scale), 0.01f, 0, 0, "%.2f");

		ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

		ImGui::Text("F11 = Toggle Fullscreen");

		static bool wireFrame = false;
		ImGui::Checkbox("Wireframe Mode", &wireFrame);

		wireFrame ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ImGui::Separator();

		ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
		ImGui::ColorPicker4("Triangles", glm::value_ptr(fore_colour));

		ImGui::End();
	}

	//Private Functions
private:

	void Draw() {

		glEnable(GL_DEPTH_TEST);
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		trans.rotation.z += deltaTime * speed;

		Louron::Shader* shader = m_ShaderLib.GetShader("basic_texture");
		if (shader)
		{
			glBindVertexArray(VAO);

			shader->Bind();
			shader->SetMat4("proj", glm::perspective(glm::radians(60.0f), (float)Louron::Engine::Get().GetWindow().GetWidth() / (float)Louron::Engine::Get().GetWindow().GetHeight(), 0.1f, 100.0f));
			shader->SetMat4("view", glm::lookAt(glm::vec3(0.0f, 1.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
			shader->SetMat4("model", trans.getTransform());
			shader->SetVec4("ourColour", fore_colour);

			m_TextureLib.GetTexture("cube_texture")->Bind();
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			m_TextureLib.UnBind();
		}
		shader->UnBind();
		glDisable(GL_DEPTH_TEST);
	}

	unsigned int VAO = NULL;
	unsigned int VBO = NULL;
	unsigned int EBO = NULL;
	float vertices[120]{
			0.5f,  0.5f, 0.5f,  1.0f, 1.0f,   // front top right
			0.5f, -0.5f, 0.5f,  1.0f, 0.0f,   // front bottom right
		-0.5f, -0.5f, 0.5f,  0.0f, 0.0f,   // front bottom left
		-0.5f,  0.5f, 0.5f,  0.0f, 1.0f,   // front top left

			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // back top right
			0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // back bottom right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // back bottom left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // back top left

			0.5f,  0.5f,  0.5f, 1.0f, 1.0f,   // right top right
			0.5f,  0.5f, -0.5f, 1.0f, 0.0f,   // right bottom right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f,   // right bottom left
			0.5f, -0.5f,  0.5f, 0.0f, 1.0f,   // right top left 

		-0.5f,  0.5f,  0.5f, 1.0f, 1.0f,   // left top right
		-0.5f,  0.5f, -0.5f, 1.0f, 0.0f,   // left bottom right
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,   // left bottom left
		-0.5f, -0.5f,  0.5f, 0.0f, 1.0f,   // left top left 

			0.5f,  0.5f,  0.5f, 1.0f, 1.0f,   // top top right
			0.5f,  0.5f, -0.5f, 1.0f, 0.0f,   // top bottom right
		-0.5f,  0.5f, -0.5f, 0.0f, 0.0f,   // top bottom left
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,   // top top left 

			0.5f, -0.5f,  0.5f, 1.0f, 1.0f,   // bottom top right
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f,   // bottom bottom right
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,   // bottom bottom left
		-0.5f, -0.5f,  0.5f, 0.0f, 1.0f    // bottom top left 
	};
	unsigned int indices[36] = {
		0, 1, 3,
		1, 2, 3,

		4, 5, 7,
		5, 6, 7,

		8, 9, 11,
		9, 10, 11,

		12, 13, 15,
		13, 14, 15,

		16, 17, 19,
		17, 18, 19,

		20, 21, 23,
		21, 22, 23
	};

};