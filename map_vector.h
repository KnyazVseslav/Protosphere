#pragma once

#ifndef MAP_VECTOR_H
#define MAP_VECTOR_H

typedef unsigned int uint;
#include <vector>

template<class Key, class Val>
class map_vector : public std::vector<Val>
{
public:

	void add(Val&& val); // ��� ����������� rvalue-��������
	void add(const Val& val); // ��� ����������� lvalue-��������
	void add(Key name); // ��� �������� ���������� ������� � �������� ������

public:

	Val& operator[](const Key& idx);
	Val& operator[](const uint& idx);

public:

	void SetKeys(std::vector<Key>* keys); // ��������� ��������� �� ������ ������, ����� �������� ������������ ������
	//void SetKeys(std::vector<Key>::iterator keys);


private:

	uint index_of(const std::vector<Key>& key_vector, const Key& search_val);

	std::vector<Key>* keys; // ������ ��������� �� ������ ������, ����� �������� ������������ ������

};



// ��� ����������� rvalue-��������
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


// ��� ����������� lvalue-��������
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


// ��� �������� ���������� ������� � �������� ������
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







// ����� ��� ����������� ������� ��������� ����� � ������� ������;
// ����� ��� ���������� �������������� ������� � ������� (������� ������� � �����������)
template<class Key, class Val>
uint map_vector<Key, Val>::index_of(const std::vector<Key>& key_vector, const Key& search_val)
{

	std::vector<Key>::const_iterator it =
		search_n(key_vector.begin(), key_vector.end(), 1, search_val);

	return it - key_vector.cbegin();

	//return 0;

}


// ���������� ��������� ������� ��� ������� �� ������������� ���������
template<class Key, class Val>
Val& map_vector<Key, Val>::operator[](const Key& idx)
{
	return std::vector<Val>::operator[](index_of(*keys, idx)); // ����� ��������� ����� idx � ������� ������ std::vector<Key>* keys � ����������� �� ������� 
	// ������� �����, ���������� � idx ���������
}


// ���������� ��������� ������� ����� � ��� �������� ������� �� ��������� ���������, �.�. ��������� ������������� ���������� �������� ������
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


// ��������� ��������� �� ������ ������, ����� �������� ������������ ������
template<class Key, class Val>
void map_vector<Key, Val>::SetKeys(std::vector<Key>* keys)
{
	this->keys = keys;
}





#endif

