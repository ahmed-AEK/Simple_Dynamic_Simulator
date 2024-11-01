#pragma once

#include "toolgui/Dialog.hpp"

namespace node
{
class MainNodeScene;
class PropertyEditControl;

class OkCancelModalDialog : public Dialog
{
public:
	OkCancelModalDialog(std::string title, std::vector<std::string> content, const SDL_FRect& rect, MainNodeScene* parent, bool hide_cancel = false);
protected:
	void OnOk() override;
	void OnClose() override;
private:

};

class SingleEntryDialog: public OkCancelModalDialog
{
public:
	SingleEntryDialog(std::string title, std::vector<std::string> content, std::string initial_value, const SDL_FRect& rect, MainNodeScene* parent);
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
	NewSceneDialog(const SDL_FRect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
};

class LoadSceneDialog : public SingleEntryDialog
{
public:
	LoadSceneDialog(const SDL_FRect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
};

class SaveSceneDialog : public SingleEntryDialog
{
public:
	SaveSceneDialog(const SDL_FRect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
};

class ConfirmOverwriteSaveSceneDialog : public OkCancelModalDialog
{
public:
	ConfirmOverwriteSaveSceneDialog(std::string name,const SDL_FRect& rect, MainNodeScene* parent);
protected:
	void OnOk() override;
private:
	std::string m_name;
};

}