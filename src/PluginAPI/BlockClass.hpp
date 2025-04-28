#pragma once

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/BlockData.hpp"
#include "NodeModels/SceneModelManager.hpp"

#include "tl/expected.hpp"
#include <any>
#include <variant>
#include <atomic>

namespace opt
{
	struct DiffEquationWrapper;
	struct DiffEquationView;
	struct NLEquationWrapper;
	struct NLEquationView;
	struct NLStatefulEquationWrapper;
	struct NLStatefulEquationView;
	struct ObserverWrapper;
	struct ObserverView;
	struct SourceEqWrapper;
	struct SourceEqView;
}

namespace node
{

class BlockDialog;
class Scene;

enum class BlockType
{
	Source,
	Sink,
	Differential,
	Stateful,
	Stateless,
};

using BlockFunctor = typename std::variant<opt::DiffEquationWrapper, opt::NLEquationWrapper, 
	opt::NLStatefulEquationWrapper, opt::ObserverWrapper, opt::SourceEqWrapper>;

using BlockView = typename std::variant<opt::DiffEquationView, opt::NLEquationView,
	opt::NLStatefulEquationView, opt::ObserverView, opt::SourceEqView>;

struct BlockTemplate
{
	std::string category;
	std::string template_name;
	model::BlockData data;
	std::string styler_name;
	model::BlockStyleProperties style_properties;
};

class IBlockClass
{
public:
	using GetNameCallback = void(*)(void* context, std::string_view name);
	virtual void GetName(GetNameCallback cb, void* context) const = 0;

	using GetDescriptionCallback = void(*)(void* context, std::string_view description);
	virtual void GetDescription(GetDescriptionCallback cb, void* context) const = 0;

	using GetDefaultClassPropertiesCallback = void(*)(void* context, std::span<const model::BlockProperty> properties);
	virtual void GetDefaultClassProperties(GetDefaultClassPropertiesCallback cb, void* context) const = 0;

	class IValidatePropertiesNotifier
	{
	public:
		virtual void error(size_t property_index, const std::string_view& error_text) = 0;
	};
	virtual int ValidateClassProperties(std::span<const model::BlockProperty> properties, IValidatePropertiesNotifier& error_cb) const = 0;

	using CalculateSocketCallback = void(*)(void* context, std::span<const model::SocketType> sockets);
	virtual void CalculateSockets(std::span<const model::BlockProperty> properties, CalculateSocketCallback cb, void* context) const = 0;

	virtual BlockType GetBlockType(std::span<const model::BlockProperty> properties) const = 0;

	class IGetFunctorCallback
	{
	public:
		virtual void error(std::string_view error_text) = 0;
		virtual void call(std::span<BlockView> blocks) = 0;
	};
	virtual int GetFunctor(std::span<const model::BlockProperty> properties, IGetFunctorCallback& cb) const = 0;
	virtual void increment_ref() const = 0;
	virtual void decrement_ref() const = 0;
	virtual bool HasBlockDialog() const { return false; }
protected:
	virtual ~IBlockClass() = default;
};

struct ValidatePropertiesNotifier : public IBlockClass::IValidatePropertiesNotifier
{
	void error(size_t property_index, const std::string_view& error_text) override
	{
		errors.push_back({ property_index, std::string{error_text} });
	}

	struct PropertyError
	{
		size_t prop_idx;
		std::string error_text;
	};
	std::vector<PropertyError> errors;
};

struct LightValidatePropertiesNotifier : public IBlockClass::IValidatePropertiesNotifier
{
	void error(size_t property_index, const std::string_view& error_text) override
	{
		(void)(property_index);
		(void)(error_text);
		errored = true;
	}

	bool errored = false;
};

class RcBlockClass: public IBlockClass
{
public:
	void increment_ref() const override
	{
		m_refcount.fetch_add(1, std::memory_order_relaxed);
	}

	void decrement_ref() const override
	{
		if (m_refcount.fetch_add(-1, std::memory_order_release) == 1)
		{
			std::atomic_thread_fence(std::memory_order_acquire);
			delete this;
		}
	}

private:
	mutable std::atomic_int32_t m_refcount = 1;
};


class BlockClassPtr
{
public:
	struct new_reference_t {};
	struct steal_reference_t {};
	static constexpr new_reference_t new_reference{};
	static constexpr steal_reference_t steal_reference{};

	constexpr BlockClassPtr() = default;
	constexpr BlockClassPtr(nullptr_t) :BlockClassPtr{} {};
	constexpr BlockClassPtr& operator=(nullptr_t) {
		if (m_ptr)
		{
			m_ptr->decrement_ref();
		}
		m_ptr = nullptr;
		return *this;
	}
	constexpr BlockClassPtr(new_reference_t, IBlockClass* ptr)
		:m_ptr{ ptr } {
		if (m_ptr) { m_ptr->increment_ref(); }
	}
	constexpr BlockClassPtr(steal_reference_t, IBlockClass* ptr)
		:m_ptr{ ptr } { }
	
	constexpr IBlockClass* get() const { return m_ptr; }
	constexpr IBlockClass* release() { return std::exchange(m_ptr, nullptr); }
	constexpr ~BlockClassPtr() {
		if (m_ptr) { m_ptr->decrement_ref(); }
	}

	constexpr IBlockClass& operator*() const { return *m_ptr; }
	constexpr IBlockClass* operator->() const { return m_ptr; }

	constexpr BlockClassPtr(const BlockClassPtr& other)
		:m_ptr{other.m_ptr}
	{
		if (m_ptr) { m_ptr->increment_ref(); }
	}
	constexpr BlockClassPtr& operator=(const BlockClassPtr& other)
	{
		if (this == &other) { return *this; }
		if (m_ptr) { m_ptr->decrement_ref(); }
		m_ptr = other.m_ptr;
		if (m_ptr) { m_ptr->increment_ref(); }
		return *this;
	}

	constexpr BlockClassPtr(BlockClassPtr&& other) noexcept
		:m_ptr{ std::exchange(other.m_ptr, nullptr) }
	{
	}
	constexpr BlockClassPtr& operator=(BlockClassPtr&& other) noexcept
	{
		if (this == &other) { return *this; }
		m_ptr = std::exchange(other.m_ptr, nullptr);
		return *this;
	}

	constexpr friend bool operator==(const BlockClassPtr& lhs, const BlockClassPtr& rhs) { return lhs.m_ptr == rhs.m_ptr; }

	constexpr friend bool operator==(const BlockClassPtr& lhs, const IBlockClass* rhs) { return lhs.get() == rhs; }
	constexpr friend bool operator==(const IBlockClass* lhs, const BlockClassPtr& rhs) { return lhs == rhs.get(); }

	constexpr friend bool operator==(const BlockClassPtr& lhs, nullptr_t) { return lhs.get() == nullptr; }
	constexpr friend bool operator==(nullptr_t, const BlockClassPtr& rhs) { return nullptr == rhs.get(); }

	constexpr operator bool() const { return m_ptr; }
	constexpr operator IBlockClass* () const { return m_ptr; }
private:
	IBlockClass* m_ptr = nullptr;
};

template <typename T, typename...Args>
BlockClassPtr make_BlockClass(Args&&...args)
{
	return BlockClassPtr{ BlockClassPtr::steal_reference, new T{std::forward<Args>(args)...} };
}

class IBlockPropertiesUpdater
{
public:
	virtual std::optional<model::FunctionalBlockData> GetFunctionalBlockData() = 0;
	virtual tl::expected<std::monostate, std::vector<ValidatePropertiesNotifier::PropertyError>> UpdateBlockProperties(std::span<const model::BlockProperty> new_properties) = 0;
};

class BlockClass: public RcBlockClass
{
public:
	explicit BlockClass(std::string name);
	BlockClass(const BlockClass&) = delete;
	BlockClass& operator=(const BlockClass&) = delete;
	~BlockClass() override;

	void SetName(std::string name) { m_name = name; }
	void GetName(GetNameCallback cb, void* context) const override { cb(context, m_name.c_str()); }


	virtual std::unique_ptr<BlockDialog> CreateBlockDialog(Scene& scene, std::shared_ptr<IBlockPropertiesUpdater> model_updater, model::BlockModel& model,
		model::FunctionalBlockData& data, std::any& simulation_data);
private:
	std::string m_name;
};


int ValidateEqualPropertyTypes(std::span<const model::BlockProperty> properties, std::span<const model::BlockProperty> default_properties, IBlockClass::IValidatePropertiesNotifier& error_cb);

}
