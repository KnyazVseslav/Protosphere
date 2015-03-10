#pragma once
#ifndef VARIADIC_DELEGATES_H
#define VARIADIC_DELEGATES_H

#include <vector>
#include <tuple>
#include <functional>
#include <memory>
#include <type_traits>

#include "map_vector.h"

using namespace std;


namespace delegates
{


	template<int... Idcs> class Indices{};

	template<int N, int... Idcs> struct IndicesBuilder : IndicesBuilder<N - 1, N - 1, Idcs...> {};

	template<int... Idcs>
	struct IndicesBuilder<0, Idcs...>
	{
		typedef Indices<Idcs...> indices;
	};



	template<class... Args>
	class Arguments
	{
	public:

		std::tuple<Args...> args;


		Arguments(Args... _args) : args(_args...) {}

	};



	template<class... Args>
	class PArguments
	{
	public:

		std::tuple<Args*...> p_args;


		PArguments(Args*... ptr_args) : p_args(ptr_args...) {}

	};



	class IDelegateData{ public: virtual void call(void*) = 0; virtual void call_with_bound_args() = 0; };


	template<class...Args> class DelegateData : public IDelegateData {};


	// Данные для методов
	template<class R, class O, class... Args>
	class DelegateData<R, O, R(Args...)> : public IDelegateData
	{
	public:

		typedef R(O::*M)(Args...);

		DelegateData(O* ptrObj, M _method) : pObj(ptrObj), method(_method) {}

		template<class...Args>
		void operator()(Args... args)
		{
			(pObj->*method)(args...);
		}


		template<int...Idcs>
		void invoker(Indices<Idcs...>, void* p_args)
		{
			(pObj->*method)(std::get<Idcs>(static_cast<Arguments<Args...>*>(p_args)->args)...);
		}

		void call(void* p_args) override
		{
			invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), p_args);
		}

	private:

		O* pObj;
		M method;

		void* bound_ptr_args;
		void* bound_ref_args;


	public:


		template<class...PtrArgsToBind>
		DelegateData(O* ptrObj, M _method, PtrArgsToBind*... ptr_args)
			: pObj(ptrObj), method(_method), bound_ptr_args(new PArguments<PtrArgsToBind...>(ptr_args...)) {}

		template<int...Idcs>
		void dereferencing_invoker(Indices<Idcs...>, void* p_args)
		{
			(pObj->*method)(*std::get<Idcs>(static_cast<PArguments<Args...>*>(p_args)->p_args)...);
		}


		void call_with_bound_args() override
		{
			dereferencing_invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), bound_ptr_args);
		}



	};



	// Данные для функций
	template<class R, class... Args>
	class DelegateData<R, R(*)(Args...)> : public IDelegateData
	{
	public:

		typedef R(*F)(Args...);

		DelegateData(F ptrF) : pF(ptrF) {}

		template<class...Args>
		void operator()(Args... args)
		{
			pF(args...);
		}

		template<int...Idcs>
		void invoker(Indices<Idcs...>, void* p_args)
		{
			pF(std::get<Idcs>(static_cast<Arguments<Args...>*>(p_args)->args)...);
		}

		void call(void* p_args) override
		{
			invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), p_args);
		}


	private:

		F pF;

		void* bound_ptr_args;
		void* bound_ref_args;


	public:


		template<class...PtrArgsToBind>
		DelegateData(F ptrF, PtrArgsToBind*... ptr_args)
			: pF(ptrF), bound_ptr_args(new PArguments<PtrArgsToBind...>(ptr_args...)) {}

		template<int...Idcs>
		void dereferencing_invoker(Indices<Idcs...>, void* p_args)
		{
			pF(*std::get<Idcs>(static_cast<PArguments<Args...>*>(p_args)->p_args)...);
		}


		void call_with_bound_args() override
		{
			dereferencing_invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), bound_ptr_args);
		}


	};



	class Delegate
	{
	public:

		Delegate() = default;

		explicit Delegate(std::string _name) : name(_name) {}

		/*
		//
		// ДЛЯ МЕТОДОВ
		//
		*/

		template<class R, class O, class...Args>
		explicit Delegate(std::string _name, O* pObj, R(O::*M)(Args...)) : name(_name)
		{
			bind(pObj, M);
		}

		template<class R, class O, class...Args, class...PtrArgsToBind>
		explicit Delegate(std::string _name, O* pObj, R(O::*M)(Args...), PtrArgsToBind... ptr_args) : name(_name)
		{
			bind(pObj, M, ptr_args...);
		}

		template<class R, class O, class...Args>
		static Delegate Make(std::string name, O* pObj, R(O::*M)(Args...))
		{
			Delegate d(name);

			d.bind(pObj, M);

			return d;
		}

		// создать с именем поумолчанию
		template<class R, class O, class...Args>
		static Delegate Make(O* pObj, R(O::*M)(Args...))
		{
			Delegate d("");

			d.bind(pObj, M);

			return d;
		}



		//
		// С привязкой аргументов
		//

		template<class R, class O, class...Args, class...PtrArgsToBind>
		static Delegate Make(std::string name, O* pObj, R(O::*M)(Args...), PtrArgsToBind... ptr_args)
		{
			Delegate d(name);

			d.bind(pObj, M, ptr_args...);

			return d;
		}


		// создать с именем поумолчанию
		template<class R, class O, class...Args, class...PtrArgsToBind>
		static Delegate Make(O* pObj, R(O::*M)(Args...), PtrArgsToBind... ptr_args)
		{
			Delegate d("");

			d.bind(pObj, M, ptr_args...);

			return d;
		}




		/*
		//
		// ДЛЯ ФУНКЦИЙ
		//
		*/

		template<class R, class...Args>
		static Delegate Make(std::string name, R(*F)(Args...))
		{
			Delegate d(name);

			d.bind(F);

			return d;
		}

		// создать с именем поумолчанию
		template<class R, class...Args>
		static Delegate Make(R(*F)(Args...))
		{
			Delegate d(string(""));

			d.bind(F);

			return d;
		}



		//
		// С привязкой аргументов
		//

		template<class R, class...Args, class...PtrArgsToBind>
		static Delegate Make(std::string name, R(*F)(Args...), PtrArgsToBind...ptr_args)
		{
			Delegate d(name);

			d.bind(F, ptr_args...);

			return d;
		}

		//создать с именем поумолчанию
		template<class R, class...Args, class...PtrArgsToBind>
		static Delegate Make(R(*F)(Args...), PtrArgsToBind...ptr_args)
		{
			Delegate d(string(""));

			d.bind(F, ptr_args...);

			return d;
		}


	public:

		// Для методов

		template<class R, class O, class...Args>
		void bind(O* pObj, R(O::*M)(Args...))
		{
			idata = new DelegateData<R, O, R(Args...)>(pObj, M);
		}

		template<class R, class O, class...Args, class...PtrArgsToBind>
		void bind(O* pObj, R(O::*M)(Args...), PtrArgsToBind... ptr_args)
		{
			idata = new DelegateData<R, O, R(Args...)>(pObj, M, ptr_args...);
		}



		// Для функций

		template<class R, class...Args>
		void bind(R(*F)(Args...))
		{
			idata = new DelegateData<R, R(*)(Args...)>(F);
		}

		template<class R, class...Args, class...PtrArgsToBind>
		void bind(R(*F)(Args...), PtrArgsToBind...ptr_args)
		{
			idata = new DelegateData<R, R(*)(Args...)>(F, ptr_args...);
		}


		// Методы вызова делегатов

		template<class...Args>
		void operator()(Args... args)
		{
			idata->call(new Arguments<Args...>(args...));
		}

		void call_with_bound_args()
		{
			idata->call_with_bound_args();
		}


	public:

		std::string get_name()
		{
			return name;
		}


	private:

		IDelegateData* idata;

		std::string name;


	};



	class DelegatesSystem
	{
	public:

		DelegatesSystem()
		{
			delegates.SetKeys(&names);
		}


		template<class...Args>
		void add(Args&&... args)
		{
			delegates.emplace_back(std::forward<Args>(args)...);

			names.push_back(delegates.front().get_name());
		}

		template<class...Args, class...PtrArgsToBind>
		void add(Args&&... args, PtrArgsToBind... ptr_args)
		{
			delegates.emplace_back(std::forward<Args>(args)..., ptr_args...);

			names.push_back(delegates.front().get_name());
		}


		void launch()
		{
			for (auto& d : delegates)
				d.call_with_bound_args();
		}

	public:

		Delegate& operator[](uint idx)
		{
			return delegates[idx];
		}


		Delegate& operator[](std::string key)
		{
			return delegates[key];
		}

	private:

		std::vector<std::string> names;

		map_vector<std::string, Delegate> delegates;

	};



	// Для методов
	template<class R, class O, class... Args>
	DelegateData<R, O, R(Args...)> make_delegate(O* pObj, R(O::*M)(Args...))
	{
		DelegateData<R, O, R(Args...)> d(pObj, M);
		return d;
	}


	// Для функций
	template<class R, class... Args>
	DelegateData<R, R(*)(Args...)> make_delegate(R(*F)(Args...))
	{
		DelegateData<R, R(*)(Args...)> d(F);
		return d;
	}



} // namespace delegates


#endif



