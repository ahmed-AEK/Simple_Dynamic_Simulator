#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "toolgui/MouseInteractable.hpp"
#include "NodeModels/Utils.hpp"

namespace node
{

    
    class GraphicsScene;

    namespace logic
    {

    class GRAPHICSSCENE_API GraphicsLogic
    {
    public:
        explicit GraphicsLogic(GraphicsScene* scene);
        virtual ~GraphicsLogic();

        void setScene(GraphicsScene* scene);
        GraphicsScene* GetScene() const { return m_pScene; }

        void MouseOut();
        void MouseIn();
        void MouseMove(const model::Point& current_mouse_point);
        MI::ClickEvent LMBUp(const model::Point& current_mouse_point);
        void Cancel();
        bool IsDone() const { return b_done; }
        void SetDone(bool value = true) { b_done = value; }
    protected:
        virtual void OnMouseOut();
        virtual void OnMouseIn();
        virtual void OnMouseMove(const model::Point& current_mouse_point);
        virtual MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point);
        virtual void OnCancel();
    private:
        GraphicsScene* m_pScene;
        bool b_done = false;
    };

    }
}