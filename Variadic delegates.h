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

        Arguments(Args&&... _args) : args(std::forward_as_tuple(std::forward<Args>(_args)...)) {}

    public:
        
        std::tuple<Args&&...> args;

	};



	template<class... Args>
	class PArguments
	{
	public:

		std::tuple<Args*...> p_args;


		PArguments(Args*... ptr_args) : p_args(std::forward<ArgsToBind>(args)...) {}

	};



	class IDelegateData abstract
    { 
    public: 
        virtual void call(void*) abstract; 
            virtual void call_with_bound_args() abstract;
         virtual void bind_args(void*) abstract;
    };


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

        virtual void bind_args(void* argsToBind) override
        {
            if (argsToBind != m_bound_args)
            {
                delete m_bound_args;
                m_bound_args = argsToBind;
            }
        }

	private:

		O* pObj;
		M method;

        void* m_bound_args;


	public:


		template<class...ArgsToBind>
        DelegateData(O* ptrObj, M _method, ArgsToBind&&... argsToBind)
            : pObj(ptrObj), method(_method), m_bound_args(new Arguments<ArgsToBind&&...>(std::forward<ArgsToBind>(argsToBind)...)) {}



		void call_with_bound_args() override
		{
			invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), m_bound_args);
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

        virtual void bind_args(void* argsToBind) override
        {
            if (argsToBind != m_bound_args)
            {
                delete m_bound_args;
                m_bound_args = argsToBind;
            }
        }


	private:

		F pF;

        void* m_bound_args;

	public:


		template<class...ArgsToBind>
        DelegateData(F ptrF, ArgsToBind&&... argsToBind)
            : pF(ptrF), m_bound_args(new Arguments<ArgsToBind&&...>(std::forward<ArgsToBind>(argsToBind)...)) {}

		void call_with_bound_args() override
		{
			invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), m_bound_args);
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

		template<class R, class O, class...Args, class...ArgsToBind>
		explicit Delegate(std::string _name, O* pObj, R(O::*M)(Args...), ArgsToBind&&... argsToBind) : name(_name)
		{
            bind(pObj, M, std::forward<ArgsToBind>(argsToBind)...);
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

		template<class R, class O, class...Args, class...ArgsToBind>
        static Delegate Make(std::string name, O* pObj, R(O::*M)(Args...), ArgsToBind&&... argsToBind)
		{
			Delegate d(name);

            d.bind(pObj, M, std::forward<ArgsToBind>(argsToBind)...);

			return d;
		}


		// создать с именем поумолчанию
		template<class R, class O, class...Args, class...ArgsToBind>
        static Delegate Make(O* pObj, R(O::*M)(Args...), ArgsToBind&&... argsToBind)
		{
			Delegate d("");

            d.bind(pObj, M, std::forward<ArgsToBind>(argsToBind)...);

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

		template<class R, class...Args, class...ArgsToBind>
        static Delegate Make(std::string name, R(*F)(Args...), ArgsToBind&&... argsToBind)
		{
			Delegate d(name);

            d.bind(F, std::forward<ArgsToBind>(argsToBind)...);

			return d;
		}

		//создать с именем поумолчанию
		template<class R, class...Args, class...ArgsToBind>
        static Delegate Make(R(*F)(Args...), ArgsToBind&&... argsToBind)
		{
			Delegate d(string(""));

            d.bind(F, std::forward<ArgsToBind>(argsToBind)...);

			return d;
		}


		// Для методов

		template<class R, class O, class...Args>
		void bind(O* pObj, R(O::*M)(Args...))
		{
			idata = new DelegateData<R, O, R(Args...)>(pObj, M);
		}

		template<class R, class O, class...Args, class...ArgsToBind>
        void bind(O* pObj, R(O::*M)(Args...), ArgsToBind&&... argsToBind)
		{
            idata = new DelegateData<R, O, R(Args...)>(pObj, M, std::forward<ArgsToBind>(argsToBind)...);
		}



		// Для функций

		template<class R, class...Args>
		void bind(R(*F)(Args...))
		{
			idata = new DelegateData<R, R(*)(Args...)>(F);
		}

		template<class R, class...Args, class...ArgsToBind>
		void bind(R(*F)(Args...), ArgsToBind&&... args)
		{
			idata = new DelegateData<R, R(*)(Args...)>(F, std::forward<ArgsToBind>(args)...);
		}

        //
        // Общие методы
        //

		template<class...Args>
		void operator()(Args... args)
		{
			idata->call(new Arguments<Args...>(args...));
		}

		void call_with_bound_args()
		{
			idata->call_with_bound_args();
		}


        template<class... Args>
        void bind_args(Args&&... args)
        {
            idata->bind_args(new Arguments<Args...>(std::forward<args>(args)...));
        }



		std::string get_name()
		{
			return name;
		}


	private:

        std::string name;
        IDelegateData* idata;
        void* m_bound_args;
		


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

		template<class...Args, class... ArgsToBind>
		void add(Args&&... args, ArgsToBind&&... argsToBind)
		{
            delegates.emplace_back(std::forward<Args>(args)..., std::forward<ArgsToBind>(argsToBind)...);

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



