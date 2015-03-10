#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "Geometry_Classes.h"
#include "Variadic Delegates.h"
#include "ShaderSystem.h"
#include <memory>

//#include "D3D.h"
#include "map_vector.h"


//template<class T> using sptr_of = std::shared_ptr<T>;

//using namespace std;
//using namespace effects::shader_system;



class VisualGeometryObjectsGroup;




class _Scene
{
public:

	_Scene();

	void Render();

	map_vector<std::string, VisualGeometryObjectsGroup> vg_objects_groups;

private:

	std::vector<std::string> vgo_groups_names;


};



///
/// VisualGeometryObjectsCollection
///



// Класс-контейнер полиморфных объектов,
// базовым классом которых является geometry_objects::VisualGeometryObject;
// 1) можно сперва вызвать Init и установить общие для всей группы d3dDevice, d3dDeviceContext и effects_type,
// а затем использовать add для rvalue-значений, передавая пустой ктор или ктор без уже установленных Init параметров (напр., ктор с параметром name)
// 2) или можно использовать move, передавая туда объект с предварительно заданным именем
class VisualGeometryObjectsCollection
{

public:

	VisualGeometryObjectsCollection(effects::shader_system::Effects* FX_of_Group);
	VisualGeometryObjectsCollection(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type);
	
	// см. 1) , использовать совместно с rvalue add
	
	void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type);

	// см. 1) , использовать совместно с Init;
	// для перемещения rvalue-значений;
	// если добавляемый объект должен иметь параметры, совпадающие с уже заданными через Init параметрами группы;	
	// предполагается использовать данный метод только с кторами передаваемых объектов;
	// для упрощённого добавления объектов: чтобы не заполнять передаваемый ктор одними и теми же параметрами каждый раз при добавлении объекта;
	// просто передаётся ктор без параметров или с отличными от параметров метода Init (см. выше)
	template<class VGODerivedType>
	void add(VGODerivedType&& vg_obj);


	// для копирования lvalue-объектов;
	// внутри метода полям объекта ничего не присваивается
	template<class VGODerivedType>
	void add(VGODerivedType& vg_obj);
	
	

	/// Следующие методы предназначены для перемещения объектов
	
	// для перемещения rvalue-значений;
	// внутри метода объекту ничего не присваивается (подразумевается передача инициализированного объекта)
	template<class VGODerivedType>
	void move(VGODerivedType&& vg_obj);
	

	// для перемещения lvalue-значений;
	// внутри метода объекту ничего не присваивается;
	// предполагается использовать с локальными переменными
	template<class VGODerivedType>
	void move(VGODerivedType& vg_obj);  



	

private:

	std::vector<std::string> vgo_names;
	map_vector< std::string, geometry_objects::VisualGeometryObject* > vg_objects;


public:

	geometry_objects::VisualGeometryObject* operator[](std::string name);
	geometry_objects::VisualGeometryObject* operator[](uint idx);

	auto begin()->decltype(vg_objects.begin())
	{
		return vg_objects.begin();
	}

	auto end()->decltype(vg_objects.end())
	{
		return vg_objects.end();
	}


private:


	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;

	effects::shader_system::Effects* FX_of_Group;



	Effects_Type effects_type;

};


///
/// VisualGeometryObjectsGroup
///

class VisualGeometryObjectsGroup
{
public:

	explicit VisualGeometryObjectsGroup(std::string _name); // связывает objects.FX_of_Group c &effects

	void Render();


	VisualGeometryObjectsCollection objects;
	effects::shader_system::Effects effects;


private:

	std::string name;

};


///
/// IMPLEMENTATION OF VisualGeometryObjectsCollection TEMPLATE METHODS
///






// для перемещения rvalue-значений и одновременного заполнения их полей значениями общими для всей группы объектов
// предполагается использовать данный метод только с кторами
template<class VGODerivedType>
void VisualGeometryObjectsCollection::add(VGODerivedType&& vg_obj)
{
	if (dynamic_cast<geometry_objects::VisualGeometryObject*>(&vg_obj))
	{

		vg_objects.SetKeys(&vgo_names);

			vg_obj.Init(d3dDevice, d3dDeviceContext, effects_type);


			vgo_names.push_back(vg_obj.name);

		//vg_objects.push_back(std::shared_ptr<geometry_objects::VisualGeometryObject>(new VGODerivedType(std::move(vg_obj)))); // создание полиморфного объекта,
																									// инициализированного rvalue-значением 
																									// vg_obj, переданным в move-ctor 
																									// производного класса

			vg_objects.emplace_back(new VGODerivedType(vg_obj));

		//vg_objects.back()->Construct_Geometry = Delegate::Make(dynamic_pointer_cast<VGODerivedType>(vg_objects.back()).get(), &VGODerivedType::ConstructGeometry);
		vg_objects.back()->Construct_Geometry = Delegate::Make(dynamic_cast<VGODerivedType*>(vg_objects.back()), &VGODerivedType::ConstructGeometry);
		vg_objects.back()->FX_of_Group = FX_of_Group;
	}
}


// для копирования lvalue-значений
template<class VGODerivedType>
void VisualGeometryObjectsCollection::add(VGODerivedType& vg_obj)
{

	if (dynamic_cast<geometry_objects::VisualGeometryObject*>(&vg_obj))
	{

		vg_objects.SetKeys(&vgo_names);

	   	   vgo_names.push_back(vg_obj.name);

		//vg_objects.push_back(std::shared_ptr<geometry_objects::VisualGeometryObject>(new VGODerivedType(vg_obj))); // создание полиморфного объекта,
																						 // инициализированного lvalue-значением 
																						 // vg_obj, переданным в copy-ctor 
																						 // производного класса
		   vg_objects.emplace_back(new VGODerivedType(vg_obj));


		auto& added_VGO = vg_objects.back();

		added_VGO->Construct_Geometry = Delegate::Make(dynamic_cast<VGODerivedType*>(added_VGO), &VGODerivedType::ConstructGeometry);
		
		added_VGO->FX_of_Group = FX_of_Group;
		added_VGO->DiffuseTexture.set_FX_of_Group(FX_of_Group);


		if (added_VGO->DiffuseTexture.initialized) added_VGO->DiffuseTexture.assign();
	}

}



// для перемещения rvalue-значений (подразумевается передача инициализированного объекта)
template<class VGODerivedType>
void VisualGeometryObjectsCollection::move(VGODerivedType&& vg_obj)
{
	if (dynamic_cast<geometry_objects::VisualGeometryObject*>(&vg_obj))
	{
		vg_objects.SetKeys(&vgo_names);
		
		   vgo_names.push_back(vg_obj.name);

		   vg_objects.emplace_back(new VGODerivedType(vg_obj));

		   vg_objects.back()->Construct_Geometry = Delegate::Make(dynamic_cast<VGODerivedType*>(vg_objects.back()), &VGODerivedType::ConstructGeometry);

		vg_objects.back()->FX_of_Group = FX_of_Group;
	}
}


// для перемещения lvalue-значений
template<class VGODerivedType>
void VisualGeometryObjectsCollection::move(VGODerivedType& vg_obj)
{
	if (dynamic_cast<geometry_objects::VisualGeometryObject*>(&vg_obj))
	{
		vg_objects.SetKeys(&vgo_names);

			 vgo_names.push_back(vg_obj.name);
			
			 vg_objects.emplace_back(new VGODerivedType(vg_obj));

			 vg_objects.back()->Construct_Geometry = Delegate::Make(dynamic_cast<VGODerivedType*>(vg_objects.back()), &VGODerivedType::ConstructGeometry);

		vg_objects.back()->FX_of_Group = FX_of_Group;
	}
}




#endif



