#pragma once

class Scene {

public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void Update() { }
	virtual void UpdateGUI() { }

private:
	virtual void Draw() { }

};