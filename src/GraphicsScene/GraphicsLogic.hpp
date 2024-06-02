#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "toolgui/MouseInteractable.hpp"

namespace node
{

    
    class GraphicsScene;

    class GRAPHICSSCENE_API GraphicsLogic
    {
    public:
        GraphicsLogic(GraphicsScene* scene);
        virtual ~GraphicsLogic();

        void setScene(GraphicsScene* scene);
        const GraphicsScene* GetScene() const { return m_pScene; }
        GraphicsScene* GetScene() { return m_pScene; }

        void MouseOut();
        void MouseIn();
        void MouseMove(const SDL_Point& current_mouse_point);
        MI::ClickEvent LMBDown(const SDL_Point& current_mouse_point);
        MI::ClickEvent LMBUp(const SDL_Point& current_mouse_point);
        MI::ClickEvent RMBDown(const SDL_Point& current_mouse_point);
        MI::ClickEvent RMBUp(const SDL_Point& current_mouse_point);
        void Cancel();
    protected:
        virtual void OnMouseOut();
        virtual void OnMouseIn();
        virtual void OnMouseMove(const SDL_Point& current_mouse_point);
        virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point);
        virtual MI::ClickEvent OnRMBDown(const SDL_Point& current_mouse_point);
        virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point);
        virtual MI::ClickEvent OnRMBUp(const SDL_Point& current_mouse_point);
        virtual void OnCancel();
        void SetDone(bool value = true);
    private:
        GraphicsScene* m_pScene;
        bool b_done = false;
    };

}