#include "Louron.h"
#include "Core/EngineStart.h"

#include "Editor Layer/Editor Layer.h"

class EditorApplication : public Louron::Engine {

public:
	EditorApplication(const Louron::EngineSpecification& specification) : Louron::Engine(specification) {
		PushLayer(new EditorLayer());
	}

};

Louron::Engine* Louron::CreateEngine(Louron::EngineCommandLineArgs args) {
	EngineSpecification spec;
	spec.Name = "Louron Engine - Editor";
	spec.WorkingDirectory = "Louron Editor";
	spec.CommandLineArgs = args;

	return new EditorApplication(spec);
}