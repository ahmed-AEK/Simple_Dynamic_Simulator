#include "ToolbarScene.h"
#include "toolgui/NodeMacros.h"

ToolbarScene::ToolbarScene(): m_new_button("assets/new_file.png"), m_save_button("assets/save_file.png")
{
    //m_new_button.loadimage();
    //m_save_button.loadimage();
    m_new_button.Bind([](){SDL_Log("new\n");});
    m_save_button.Bind([](){SDL_Log("save\n");});
    m_vec.push_back(&m_new_button);
    m_vec.push_back(&m_save_button);
}

void ToolbarScene::Draw(SDL_Renderer* renderer)
{
    for (auto& ptr:m_vec)
    {
        ptr->Draw(renderer);
    }
}

const SDL_Rect &ToolbarScene::GetRect() const noexcept
{
    return this->m_rect;
}

void ToolbarScene::SetRect(const SDL_Rect &rect)
{
    this->m_rect = rect;
    int height = std::min(m_rect.h, 100);
    int index = 0;
    for (auto iter:m_vec)
    {
        iter->SetRect({m_rect.x + height  * index, m_rect.y, height, height});
        index++;
    }

}

std::span<ImageButton *> ToolbarScene::GetButtons()
{
    return m_vec;
}

void ToolbarScene::InsertButton(ImageButton button, int index)
{
    UNUSED_PARAM(button);
    UNUSED_PARAM(index);
    //m_owned_vec.emplace_back(std::move(button));
    m_vec.insert(m_vec.begin()+index, &(*m_owned_vec.rbegin()));
}

void ToolbarScene::RemoveButton(int index)
{
    SDL_assert(static_cast<std::size_t>(index) < m_vec.size());
    ImageButton* ptr = m_vec[index];
    m_vec.erase(m_vec.begin()+index);
    if (m_owned_vec.size() > 0 && ptr >= &m_owned_vec[0] && ptr <= &m_owned_vec[m_owned_vec.size()-1])
    {
        auto it = std::find_if(m_owned_vec.begin(), m_owned_vec.end(), [&](const ImageButton& btn){return &btn != ptr;});
        m_owned_vec.erase(it);
    }
}

ToolbarScene* ToolbarScene::OnGetInteractableAtPoint(const SDL_Point &point)
{
    UNUSED_PARAM(point);
    return this;
}
