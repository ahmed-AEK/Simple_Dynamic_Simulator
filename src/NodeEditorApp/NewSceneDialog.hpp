#pragma once

#include "toolgui/Dialog.hpp"

namespace node
{
class MainNodeScene;
class PropertyEditControl;

class OkCancelModalDialog : public Dialog
{
public:
	OkCancelModalDialog(std::string title, std::vector<std::string> content, 
		const WidgetSize& size, MainNodeScene* parent, bool hide_cancel = false);
	async::Future<bool> GetFuture();
protected:
	void OnOk() override;
	void OnClose() override;
private:
	async::ThreadedPromise<bool> m_promise;

};

class SingleEntryDialog: public OkCancelModalDialog
{
public:
	SingleEntryDialog(std::string title, std::vector<std::string> content, 
		std::string initial_value, const WidgetSize& size, MainNodeScene* parent);
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
	NewSceneDialog(const WidgetSize& size, MainNodeScene* parent);
protected:
	void OnOk() override;
};

class LoadSceneDialog : public SingleEntryDialog
{
public:
	LoadSceneDialog(const WidgetSize& size, MainNodeScene* parent);
protected:
	void OnOk() override;
};

class SaveSceneDialog : public SingleEntryDialog
{
public:
	SaveSceneDialog(const WidgetSize& size, MainNodeScene* parent);
protected:
	void OnOk() override;
};

}