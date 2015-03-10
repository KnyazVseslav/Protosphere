#pragma once

#ifndef MAP_VECTOR_H
#define MAP_VECTOR_H

typedef unsigned int uint;
#include <vector>

template<class Key, class Val>
class map_vector : public std::vector<Val>
{
public:

	void add(Val&& val); // для перемещения rvalue-значений
	void add(const Val& val); // для копирования lvalue-значений
	void add(Key name); // для создания дефолтного объекта с заданным именем

public:

	Val& operator[](const Key& idx);
	Val& operator[](const uint& idx);

public:

	void SetKeys(std::vector<Key>* keys); // сохранить указатель на вектор ключей, чтобы избежать дублирования данных
	//void SetKeys(std::vector<Key>::iterator keys);


private:

	uint index_of(const std::vector<Key>& key_vector, const Key& search_val);

	std::vector<Key>* keys; // храним указатель на вектор ключей, чтобы избежать дублирования данных

};



// для перемещения rvalue-значений
template<class Key, class Val>
void map_vector<Key, Val>::add(Val&& val)
{	
	try
	{
		keys->push_back(val.name);
		push_back(std::move(val));
	}
	catch (...)
	{
		MessageBoxA(0, "value operated on has no member \"name\"", "Time to debug ))", MB_OK);
	}
}


// для копирования lvalue-значений
template<class Key, class Val>
void map_vector<Key, Val>::add(const Val& val)
{
	try
	{
		keys->push_back(val.name);
		push_back(val);
	}
	catch (...)
	{
		MessageBoxA(0, "value operated on has no member \"name\"", "Time to debug ))", MB_OK);
	}
}


// для создания дефолтного объекта с заданным именем
template<class Key, class Val>
void map_vector<Key, Val>::add(Key name)
{
	try
	{
	//push_back(std::move(Val(name)));
	emplace_back(name);
	keys->push_back(name);
	}
	catch (...)
	{
		MessageBoxA(0, "value operated on has no member \"name\"", "Time to debug ))", MB_OK);
	}
}







// Метод для определения индекса заданного ключа в векторе ключей;
// нужно для реализации ассоциативного доступа к вектору (подобно доступу к отображению)
template<class Key, class Val>
uint map_vector<Key, Val>::index_of(const std::vector<Key>& key_vector, const Key& search_val)
{

	std::vector<Key>::const_iterator it =
		search_n(key_vector.begin(), key_vector.end(), 1, search_val);

	return it - key_vector.cbegin();

	//return 0;

}


// Перегрузка оператора индекса для доступа по ассоциативной семантике
template<class Key, class Val>
Val& map_vector<Key, Val>::operator[](const Key& idx)
{
	return std::vector<Val>::operator[](index_of(*keys, idx)); // поиск заданного ключа idx в векторе ключей std::vector<Key>* keys и возвращение по индексу 
	// данного ключа, связанного с idx значением
}


// Перегрузка оператора индекса также и для обычного доступа по индексной семантике, т.к. появление ассоциативной перегрузки скрывало данную
template<class Key, class Val>
Val& map_vector<Key, Val>::operator[](const uint& idx)
{
	return std::vector<Val>::operator[](idx);
	
	/*
	Val& elem = std::vector<Val>::operator[](idx);
		//Val& elem = operator[](idx);

	return elem;
	*/

}


// сохранить указатель на вектор ключей, чтобы избежать дублирования данных
template<class Key, class Val>
void map_vector<Key, Val>::SetKeys(std::vector<Key>* keys)
{
	this->keys = keys;
}





#endif

