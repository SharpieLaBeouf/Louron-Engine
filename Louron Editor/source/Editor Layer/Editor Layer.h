#pragma once

#include "Louron.h"
#include "Core/Layer.h"

class EditorLayer : public Louron::Layer {
public:

	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate() override;
	virtual void OnGuiRender() override;

private:

};