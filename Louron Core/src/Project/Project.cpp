#include "Project.h"

// Louron Core Headers
#include "Project Serializer.h"
#include "../Core/Logging.h"
#include "../Renderer/RendererPipeline.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	// PROJECT

	/// <summary>
	/// Create a New Project Instance.
	/// Please Note: this does not set the Project to the active instance if there is an 
	/// existing instance. Where there is already an existing instance of project, this 
	/// needs to be set manually using SetActiveProject.
	/// </summary>
	/// <param name="projectPath">Path of where project will be saved. Please ensure the 
	/// path includes the file of the project. The filename of the project will become
	/// the name of the project. If a project already exists in this path
	/// this will call Project::LoadProject and return an instance the saved project.</param>
	std::shared_ptr<Project> Project::NewProject(const std::filesystem::path& projectFilePath) {

		std::filesystem::path outFilePath = projectFilePath;

		// Check if Project File Path is Empty.
		if (outFilePath.empty())
			outFilePath = "Untitled Project/Untitled Project.lproj";

		// Check if Project File Extension is Incompatible.
		if (outFilePath.extension() != ".lproj") {
			// replace this with error log opposed to assert - L_CORE_ASSERT(false, "Incompatible Project File Extension! Extension used: " + projectFilePath.extension().string() + ", Extension Required: .lproj");
			outFilePath.replace_extension();
			outFilePath = outFilePath.string() + ".lproj";
		}

		// Check if Project Has a Dedicated Parent Project Directory.
		if (!outFilePath.has_parent_path() || std::filesystem::absolute(outFilePath).parent_path() == std::filesystem::current_path())
			outFilePath = outFilePath.filename().replace_extension() / outFilePath;

		// Checks if Project Already Exists and Returns Deserialized Project Instance.
		if (std::filesystem::exists(outFilePath))
			return Project::LoadProject(outFilePath);
		
		// Initialise Project and Scene Instances.
		std::shared_ptr<Project> project = std::make_shared<Project>();

		// Define Project and Scene Config.
		std::filesystem::path projectDirectory = outFilePath.parent_path();

		project->m_Config.Name = outFilePath.filename().replace_extension().string();
		project->m_Config.StartScene = projectDirectory / "Scenes/Untitled Scene.lscene";
		project->m_Config.AssetDirectory = projectDirectory / "Assets/";

		project->m_ActiveScene = std::make_shared<Scene>(project->m_Config.StartScene);
		{
			SceneConfig config = project->m_ActiveScene->GetConfig();
			config.AssetDirectory = project->m_Config.AssetDirectory;
			project->m_ActiveScene->SetConfig(config);
		}

		project->m_ProjectFilePath = outFilePath;
		project->m_ProjectDirectory = projectDirectory;

		// Second Save Project Data
		ProjectSerializer serializer(project);
		if (!serializer.Serialize(outFilePath)) {
			L_CORE_ERROR("Project Could Not Be Saved");
			return nullptr;
		}

		L_CORE_INFO("Project Created: {0}", project->m_Config.Name);

		if (!s_ActiveProject)
			s_ActiveProject = project;

		return project;
	}

	/// <summary>
	/// Loads a Project File.
	/// Please Note: This updates the static Active Project with the loaded Project File
	/// and saves the current Project if valid.
	/// </summary>
	std::shared_ptr<Project> Project::LoadProject(const std::filesystem::path& projectFilePath) {

		if (projectFilePath.empty()) {
			L_CORE_ERROR("No Project File Path Provided");
			return nullptr;
		}

		if (projectFilePath.extension() != ".lproj") {
			L_CORE_ERROR("Incompatible Project File Extension! Extension Used: {0}, Extension Required : .lproj", projectFilePath.extension().string());
			return nullptr;
		}

		if (!std::filesystem::exists(projectFilePath)) {
			L_CORE_ERROR("Project File Path Does Not Exist");
			return nullptr;
		}

		if (s_ActiveProject) {
			SaveProject();
		}

		s_ActiveProject = (s_ActiveProject) ? s_ActiveProject : std::make_shared<Project>();

		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Deserialize(projectFilePath))
		{

			std::shared_ptr<Scene> scene = std::make_shared<Scene>(s_ActiveProject->m_Config.StartScene);

			s_ActiveProject->m_ProjectFilePath = projectFilePath;
			s_ActiveProject->m_ProjectDirectory = projectFilePath.parent_path();
			s_ActiveProject->m_ActiveScene = scene;

			L_CORE_INFO("Project Loaded: {0}", s_ActiveProject->m_Config.Name);
			L_CORE_INFO("Scene Loaded: {0}", scene->GetConfig().Name);
			return s_ActiveProject;
		}
		
		L_CORE_ERROR("Project Could Not Be Loaded");

		return nullptr;
	}

	/// <summary>
	/// Saves a Project File and Active Scene File.
	/// </summary>
	bool Project::SaveProject(const std::filesystem::path& projectFilePath) {

		// First Save Active Scene
		s_ActiveProject->SaveScene();

		// Second Save Project Data
		ProjectSerializer serializer(s_ActiveProject);
		const std::filesystem::path& path = (projectFilePath == "") ? s_ActiveProject->m_ProjectFilePath : projectFilePath;

		if (serializer.Serialize(path)) {
			s_ActiveProject->m_ProjectFilePath = path;
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			L_CORE_INFO("Project ({0}) Saved At: {1}", s_ActiveProject->m_Config.Name, path.string());
			return true;
		}
		
		L_CORE_ERROR("Project Could Not Be Saved");

		return false;
	}

	/// <summary>
	/// Updates Active Project Instance.
	/// </summary>
	void Project::SetActiveProject(std::shared_ptr<Project> project)
	{
		if (s_ActiveProject)
			s_ActiveProject->SaveProject();

		s_ActiveProject = project;
	}


	// SCENE

	/// <summary>
	/// Creates a New Scene.
	/// </summary>
	std::shared_ptr<Scene> Project::NewScene(const std::filesystem::path& sceneFilePath)
	{
		if (m_ActiveScene)
			SaveScene();

		m_ActiveScene = std::make_shared<Scene>(sceneFilePath.filename().string());
		L_CORE_INFO("Scene Created: {0}", sceneFilePath.filename().string());

		return m_ActiveScene;
	}

	/// <summary>
	/// Loads a Scene File.
	/// Please Note: this does not update the active scene. Please use Project::SetActiveScene
	/// to update the currently active scene.
	/// </summary>
	std::shared_ptr<Scene> Project::LoadScene(const std::filesystem::path& sceneFilePath) {
		std::shared_ptr<Scene> scene = std::make_shared<Scene>(sceneFilePath);
		L_CORE_INFO("Scene Loaded: {0}", sceneFilePath.filename().string());
		return scene;
	}

	/// <summary>
	/// Saves a Scene File.
	/// </summary>
	bool Project::SaveScene(const std::filesystem::path& sceneFilePath) {

		if (m_ActiveScene) {
			SceneSerializer sceneSerializer(m_ActiveScene);
			const std::filesystem::path& path = (sceneFilePath.empty()) ? m_ActiveScene->m_SceneFilePath : sceneFilePath;

			if (path.empty()) {
				L_CORE_ERROR("Cannot Save Scene, File Path Invalid : \'{0}\'", path.string());
				return false;
			}

			sceneSerializer.Serialize();
			L_CORE_INFO("Scene ({0}) Saved At: {1}", s_ActiveProject->m_Config.Name, path.string());
			return true;
		}

		return false;
	}

	/// <summary>
	/// Updates the Active Scene of the Active Project.
	/// </summary>
	void Project::SetActiveScene(std::shared_ptr<Scene> scene) {

		if (s_ActiveProject->m_ActiveScene)
			s_ActiveProject->SaveScene();

		s_ActiveProject->m_ActiveScene = scene;
	}

	/// <summary>
	/// Updates the Scene of the Project.
	/// </summary>
	/// <param name="scene"></param>
	void Project::SetScene(std::shared_ptr<Scene> scene) {

		if (m_ActiveScene)
			SaveScene();

		m_ActiveScene = scene;
	}
}