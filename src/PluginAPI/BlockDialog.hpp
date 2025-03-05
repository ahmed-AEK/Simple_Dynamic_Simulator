#pragma once

#include "toolgui/Dialog.hpp"
#include <any>

namespace node
{

class BlockDialog : public Dialog
{
public:
	using Dialog::Dialog;
	virtual void UpdateResults(std::any new_result) { UNUSED_PARAM(new_result); };
};

}
