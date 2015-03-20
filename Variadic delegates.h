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

		Arguments(Args&&... args) : m_args(std::forward_as_tuple(std::forward<Args>(args)...)) {}

    public:
        
        std::tuple<Args&&...> m_args;

	};

	class IDelegateData abstract
    { 
    public: 
        virtual void call(void*) abstract; 
            virtual void call_with_bound_args() abstract;
         virtual void bind_args(void*) abstract;
	protected:
		void* m_pBound_args;
    };


	template<class...Args> class DelegateData : public IDelegateData {};


	// Данные для методов
	template<class R, class O, class... Args>
	class DelegateData<R, O, R(Args...)> : public IDelegateData
	{
	public:

		typedef R(O::*M)(Args...);

		DelegateData(O* pObj, M pMethod) : m_pObj(pObj), m_pMethod(pMethod) {}

		template<class...Args>
		void operator()(Args... args)
		{
			(m_pObj->*m_pMethod)(std::forward<Args>(args)...);
		}

		void call(void* pArgs) override
		{
			invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), pArgs);
		}

		template<int...Idcs>
		void invoker(Indices<Idcs...>, void* pArgs)
		{
			auto pArguments = static_cast<Arguments<Args...>*>(pArgs);
			(m_pObj->*m_pMethod)(std::get<Idcs>(pArguments->m_args)...);
		}

	public:

		template<class...ArgsToBind>
		DelegateData(O* pObj, M pMethod, ArgsToBind&&... argsToBind) : m_pObj(pObj), m_pMethod(pMethod)
		{
			bind_args(new Arguments<ArgsToBind&&...>(std::forward<ArgsToBind>(argsToBind)...));
		}

		virtual void bind_args(void* argsToBind) override
		{
			if (argsToBind != m_pBound_args)
			{
				delete m_pBound_args;
				m_pBound_args = argsToBind;
			}
		}

		void call_with_bound_args() override
		{
			invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), m_pBound_args);
		}


	private:

		O* m_pObj;
		M  m_pMethod;

	};


	// Данные для функций
	template<class R, class... Args>
	class DelegateData<R, R(*)(Args...)> : public IDelegateData
	{
	public:

		typedef R(*F)(Args...);

		DelegateData(F pF) : m_pF(pF) {}

		template<class...Args>
		void operator()(Args... args)
		{
			m_pF(args...);
		}

		void call(void* pArgs) override
		{
			invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), pArgs);
		}

		template<int...Idcs>
		void invoker(Indices<Idcs...>, void* pArgs)
		{
			auto pArguments = static_cast<Arguments<Args...>*>(pArgs);
			m_pF(std::get<Idcs>(pArguments->m_args)...);
		}

	public:

		template<class...ArgsToBind>
		DelegateData(F pF, ArgsToBind&&... argsToBind) : m_pF(pF)
		{
			bind_args(new Arguments<ArgsToBind&&...>(std::forward<ArgsToBind>(argsToBind)...));
		}

		virtual void bind_args(void* argsToBind) override
		{
			if (argsToBind != m_pBound_args)
			{
				delete m_pBound_args;
				m_pBound_args = argsToBind;
			}
		}

		void call_with_bound_args() override
		{
			invoker(typename IndicesBuilder<sizeof...(Args)>::indices(), m_pBound_args);
		}


	private:

		F m_pF;

	};



	class Delegate
	{
	public:

		Delegate() = default;

		explicit Delegate(std::string name) : m_name(name) {}

		/*
		//
		// ДЛЯ МЕТОДОВ
		//
		*/

		template<class R, class O, class...Args>
		explicit Delegate(std::string name, O* pObj, R(O::*M)(Args...)) : m_name(name)
		{
			bind(pObj, M);
		}

		template<class R, class O, class...Args, class...ArgsToBind>
		explicit Delegate(std::string name, O* pObj, R(O::*M)(Args...), ArgsToBind&&... argsToBind) : m_name(name)
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
		explicit Delegate(std::string name, R(*F)(Args...)) : m_name(name)
        {
            bind(F);
        }

        template<class R, class...Args, class...ArgsToBind>
		explicit Delegate(std::string name, R(*F)(Args...), ArgsToBind&&... argsToBind) : m_name(name)
        {
            bind(F, std::forward<ArgsToBind>(argsToBind)...);
        }


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


		template<class R, class O, class...Args>
		void bind(O* pObj, R(O::*M)(Args...))
		{
			m_data = new DelegateData<R, O, R(Args...)>(pObj, M);
		}

		template<class R, class O, class...Args, class...ArgsToBind>
        void bind(O* pObj, R(O::*M)(Args...), ArgsToBind&&... argsToBind)
		{
            m_data = new DelegateData<R, O, R(Args...)>(pObj, M, std::forward<ArgsToBind>(argsToBind)...);
		}

		template<class R, class...Args>
		void bind(R(*F)(Args...))
		{
			m_data = new DelegateData<R, R(*)(Args...)>(F);
		}

		template<class R, class...Args, class...ArgsToBind>
		void bind(R(*F)(Args...), ArgsToBind&&... args)
		{
			m_data = new DelegateData<R, R(*)(Args...)>(F, std::forward<ArgsToBind>(args)...);
		}

        //
        // Общие методы
        //

		template<class...Args>
		void operator()(Args&&... args)
		{
            m_data->call(new Arguments<Args...>(std::forward<Args>(args)...));
		}

		void call_with_bound_args()
		{
			m_data->call_with_bound_args();
		}


        template<class... Args>
        void bind_args(Args&&... args)
        {
            m_data->bind_args(new Arguments<Args...>(std::forward<Args>(args)...));
        }



		std::string get_name()
		{
			return m_name;
		}


	private:

        std::string m_name;
        IDelegateData* m_data;

	};



	class DelegatesSystem
	{
	public:

		DelegatesSystem()
		{
			m_delegates.SetKeys(&m_names);
		}


		template<class...Args>
        void add(Args&&... delegateCtorArgs)
		{
            m_delegates.emplace_back(std::forward<Args>(delegateCtorArgs)...);

			m_names.push_back(m_delegates.front().get_name());
		}

		void launch()
		{
			for (auto& d : m_delegates)
				d.call_with_bound_args();
		}

	public:

		Delegate& operator[](uint idx)
		{
			return m_delegates[idx];
		}


		Delegate& operator[](std::string key)
		{
			return m_delegates[key];
		}

	private:

		std::vector<std::string> m_names;

		map_vector<std::string, Delegate> m_delegates;

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



