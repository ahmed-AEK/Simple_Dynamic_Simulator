#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "toolgui/MouseInteractable.hpp"

namespace node
{

    
    class GraphicsScene;

    class GRAPHICSSCENE_API GraphicsLogic
    {
    public:
        explicit GraphicsLogic(GraphicsScene* scene);
        virtual ~GraphicsLogic();

        void setScene(GraphicsScene* scene);
        GraphicsScene* GetScene() const { return m_pScene; }

        void MouseOut();
        void MouseIn();
        void MouseMove(const SDL_Point& current_mouse_point);
        MI::ClickEvent LMBDown(const SDL_Point& current_mouse_point);
        MI::ClickEvent LMBUp(const SDL_Point& current_mouse_point);
        MI::ClickEvent RMBDown(const SDL_Point& current_mouse_point);
        MI::ClickEvent RMBUp(const SDL_Point& current_mouse_point);
        void Cancel();
        bool IsDone() const { return b_done; }
    protected:
        virtual void OnMouseOut();
        virtual void OnMouseIn();
        virtual void OnMouseMove(const SDL_Point& current_mouse_point);
        virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point);
        virtual MI::ClickEvent OnRMBDown(const SDL_Point& current_mouse_point);
        virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point);
        virtual MI::ClickEvent OnRMBUp(const SDL_Point& current_mouse_point);
        virtual void OnCancel();
        void SetDone(bool value = true) { b_done = value; }
    private:
        GraphicsScene* m_pScene;
        bool b_done = false;
    };

}