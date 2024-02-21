#include "Editor Layer.h"

EditorLayer::EditorLayer() {

}


void EditorLayer::OnAttach() {
	std::cout << "\n[L20] The Editor Application Has Been Started, Press Escape to Exit..." << std::endl;
}

void EditorLayer::OnDetach() {

}

void EditorLayer::OnUpdate() {

	if(Louron::Engine::Get().GetInput().GetKeyDown(GLFW_KEY_ESCAPE))
		Louron::Engine::Get().Close();
}

void EditorLayer::OnGuiRender() {

}