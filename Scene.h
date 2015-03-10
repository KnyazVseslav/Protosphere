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



// �����-��������� ����������� ��������,
// ������� ������� ������� �������� geometry_objects::VisualGeometryObject;
// 1) ����� ������ ������� Init � ���������� ����� ��� ���� ������ d3dDevice, d3dDeviceContext � effects_type,
// � ����� ������������ add ��� rvalue-��������, ��������� ������ ���� ��� ���� ��� ��� ������������� Init ���������� (����., ���� � ���������� name)
// 2) ��� ����� ������������ move, ��������� ���� ������ � �������������� �������� ������
class VisualGeometryObjectsCollection
{

public:

	VisualGeometryObjectsCollection(effects::shader_system::Effects* FX_of_Group);
	VisualGeometryObjectsCollection(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type);
	
	// ��. 1) , ������������ ��������� � rvalue add
	
	void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type);

	// ��. 1) , ������������ ��������� � Init;
	// ��� ����������� rvalue-��������;
	// ���� ����������� ������ ������ ����� ���������, ����������� � ��� ��������� ����� Init ����������� ������;	
	// �������������� ������������ ������ ����� ������ � ������� ������������ ��������;
	// ��� ����������� ���������� ��������: ����� �� ��������� ������������ ���� ������ � ���� �� ����������� ������ ��� ��� ���������� �������;
	// ������ ��������� ���� ��� ���������� ��� � ��������� �� ���������� ������ Init (��. ����)
	template<class VGODerivedType>
	void add(VGODerivedType&& vg_obj);


	// ��� ����������� lvalue-��������;
	// ������ ������ ����� ������� ������ �� �������������
	template<class VGODerivedType>
	void add(VGODerivedType& vg_obj);
	
	

	/// ��������� ������ ������������� ��� ����������� ��������
	
	// ��� ����������� rvalue-��������;
	// ������ ������ ������� ������ �� ������������� (��������������� �������� ������������������� �������)
	template<class VGODerivedType>
	void move(VGODerivedType&& vg_obj);
	

	// ��� ����������� lvalue-��������;
	// ������ ������ ������� ������ �� �������������;
	// �������������� ������������ � ���������� �����������
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

	explicit VisualGeometryObjectsGroup(std::string _name); // ��������� objects.FX_of_Group c &effects

	void Render();


	VisualGeometryObjectsCollection objects;
	effects::shader_system::Effects effects;


private:

	std::string name;

};


///
/// IMPLEMENTATION OF VisualGeometryObjectsCollection TEMPLATE METHODS
///






// ��� ����������� rvalue-�������� � �������������� ���������� �� ����� ���������� ������ ��� ���� ������ ��������
// �������������� ������������ ������ ����� ������ � �������
template<class VGODerivedType>
void VisualGeometryObjectsCollection::add(VGODerivedType&& vg_obj)
{
	if (dynamic_cast<geometry_objects::VisualGeometryObject*>(&vg_obj))
	{

		vg_objects.SetKeys(&vgo_names);

			vg_obj.Init(d3dDevice, d3dDeviceContext, effects_type);


			vgo_names.push_back(vg_obj.name);

		//vg_objects.push_back(std::shared_ptr<geometry_objects::VisualGeometryObject>(new VGODerivedType(std::move(vg_obj)))); // �������� ������������ �������,
																									// ������������������� rvalue-��������� 
																									// vg_obj, ���������� � move-ctor 
																									// ������������ ������

			vg_objects.emplace_back(new VGODerivedType(vg_obj));

		//vg_objects.back()->Construct_Geometry = Delegate::Make(dynamic_pointer_cast<VGODerivedType>(vg_objects.back()).get(), &VGODerivedType::ConstructGeometry);
		vg_objects.back()->Construct_Geometry = Delegate::Make(dynamic_cast<VGODerivedType*>(vg_objects.back()), &VGODerivedType::ConstructGeometry);
		vg_objects.back()->FX_of_Group = FX_of_Group;
	}
}


// ��� ����������� lvalue-��������
template<class VGODerivedType>
void VisualGeometryObjectsCollection::add(VGODerivedType& vg_obj)
{

	if (dynamic_cast<geometry_objects::VisualGeometryObject*>(&vg_obj))
	{

		vg_objects.SetKeys(&vgo_names);

	   	   vgo_names.push_back(vg_obj.name);

		//vg_objects.push_back(std::shared_ptr<geometry_objects::VisualGeometryObject>(new VGODerivedType(vg_obj))); // �������� ������������ �������,
																						 // ������������������� lvalue-��������� 
																						 // vg_obj, ���������� � copy-ctor 
																						 // ������������ ������
		   vg_objects.emplace_back(new VGODerivedType(vg_obj));


		auto& added_VGO = vg_objects.back();

		added_VGO->Construct_Geometry = Delegate::Make(dynamic_cast<VGODerivedType*>(added_VGO), &VGODerivedType::ConstructGeometry);
		
		added_VGO->FX_of_Group = FX_of_Group;
		added_VGO->DiffuseTexture.set_FX_of_Group(FX_of_Group);


		if (added_VGO->DiffuseTexture.initialized) added_VGO->DiffuseTexture.assign();
	}

}



// ��� ����������� rvalue-�������� (��������������� �������� ������������������� �������)
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


// ��� ����������� lvalue-��������
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



