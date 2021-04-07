#pragma once
#include "workNode.h"
#include "boost/any.hpp"

namespace dataFlow
{
	template <typename valueType>
	class CBaseConsumer :virtual public CWorkNode
	{
	public:
		CBaseConsumer() = default;
		~CBaseConsumer() = default;

		CBaseConsumer(const CBaseConsumer&) = delete;

		virtual std::string inputType() const = 0;
		virtual void consumeInner(valueType const&) = 0;
	};
	
	using CAbstractAnyDataConsumer = CBaseConsumer<boost::any>;

	template <typename impl, typename Input>
	class CDataConsumer : public CAbstractAnyDataConsumer
	{
	public:

		virtual void consumeInner(boost::any const& input) override final //signal之后，调用的都是这个函数，无法被重写，boost::any保证了其通用性
		{
			static_cast<impl*>(this)->consume(boost::any_cast<Input>(input));
		}
	public:
		virtual std::string inputType() const override
		{
			return typeid(Input).name();
		}
	};

}