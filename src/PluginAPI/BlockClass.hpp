#pragma once

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/BlockData.hpp"

#include <any>
#include <variant>

namespace opt
{
	struct DiffEquationWrapper;
	struct NLEquationWrapper;
	struct NLStatefulEquationWrapper;
	struct ObserverWrapper;
	struct SourceEqWrapper;
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

	virtual std::string_view GetName() const = 0;
	virtual std::string_view GetDescription() const = 0;

	virtual std::span<const model::BlockProperty> GetDefaultClassProperties() const = 0;
	virtual bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) const = 0;

	virtual std::vector<model::SocketType>
		CalculateSockets(const std::vector<model::BlockProperty>& properties) const = 0;
	virtual BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) const = 0;

	using GetFunctorResult = std::variant<BlockFunctor, std::vector<BlockFunctor>>;
	virtual GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const = 0;
	virtual void increment_ref() const = 0;
	virtual void decrement_ref() const = 0;
	virtual bool HasBlockDialog() const { return false; }
protected:
	virtual ~IBlockClass() = default;
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
	static constexpr new_reference_t new_reference;
	static constexpr steal_reference_t steal_reference;

	constexpr BlockClassPtr() = default;
	constexpr BlockClassPtr(nullptr_t) :BlockClassPtr{} {};
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

	constexpr friend bool operator==(const BlockClassPtr& lhs, IBlockClass* rhs) { return lhs.get() == rhs; }
	constexpr friend bool operator==(IBlockClass* lhs, const BlockClassPtr& rhs) { return lhs == rhs.get(); }

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

class BlockClass: public RcBlockClass
{
public:
	BlockClass(std::string name);
	BlockClass(const BlockClass&) = delete;
	BlockClass& operator=(const BlockClass&) = delete;
	virtual ~BlockClass();

	void SetName(std::string name) { m_name = name; }
	std::string_view GetName() const override { return m_name; }


	virtual std::unique_ptr<BlockDialog> CreateBlockDialog(Scene& scene, model::BlockModel& model,
		model::FunctionalBlockData& data, std::any& simulation_data);
private:
	std::string m_name;
};

}
