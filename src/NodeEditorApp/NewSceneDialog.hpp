#pragma once

#include "toolgui/Dialog.hpp"

namespace node
{
class MainNodeScene;
class PropertyEditControl;

class OkCancelModalDialog : public Dialog
{
public:
	OkCancelModalDialog(std::string title, std::string content, const SDL_Rect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
	void OnClose() override;
private:

};

class SingleEntryDialog: public OkCancelModalDialog
{
public:
	SingleEntryDialog(std::string title, std::string content, std::string initial_value, const SDL_Rect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
	void OnClose() override;
	std::string GetValue();
private:
	PropertyEditControl* m_edit;
};

class NewSceneDialog : public OkCancelModalDialog
{
public:
	NewSceneDialog(const SDL_Rect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
};

class LoadSceneDialog : public SingleEntryDialog
{
public:
	LoadSceneDialog(const SDL_Rect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
};

class SaveSceneDialog : public SingleEntryDialog
{
public:
	SaveSceneDialog(const SDL_Rect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
};

}