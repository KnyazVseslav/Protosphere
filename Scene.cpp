#include "Scene.h"

using namespace geometry_objects;
using namespace effects::shader_system;
using namespace delegates;



_Scene::_Scene()
{
	vg_objects_groups.SetKeys(&vgo_groups_names);
}


void _Scene::Render()
{
	for (auto& VGOs_group : vg_objects_groups)
		VGOs_group.Render();
}




VisualGeometryObjectsGroup::VisualGeometryObjectsGroup(std::string _name) : name(_name), objects(VisualGeometryObjectsCollection(&effects))
{
	
}

void VisualGeometryObjectsGroup::Render()
{
	
	for (auto& shader_system : effects)			   // дл€ каждого эффекта в коллекции
	{

		shader_system.set();						// установить набор шейдеров


			shader_system.begin();					// пакетный вызов функций, предвар€ющих отрисовку сцены

				for (auto& object : objects)
					object->RenderInGroup();		// отрисовка объектов сцены

			shader_system.end();				    // пакетный вызов функций, следующих за отрисовкой сцены

	}

}


///
/// IMPLEMENTATION OF VisualGeometryObjectsCollection NONTEMPLATE METHODS
///

VisualGeometryObjectsCollection::VisualGeometryObjectsCollection(effects::shader_system::Effects* FX_of_Group)
{
	vg_objects.SetKeys(&vgo_names);

	this->FX_of_Group = FX_of_Group;
}

VisualGeometryObjectsCollection::VisualGeometryObjectsCollection(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
																		const Effects_Type& effects_type)
{
	vg_objects.SetKeys(&vgo_names);

		Init(d3dDevice, d3dDeviceContext, effects_type);
}




void VisualGeometryObjectsCollection::Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type)
{
	this->d3dDevice = d3dDevice;
	this->d3dDeviceContext = d3dDeviceContext;

	this->effects_type = effects_type;
}



geometry_objects::VisualGeometryObject* VisualGeometryObjectsCollection::operator[](std::string name)
{
	return vg_objects[name];
}


geometry_objects::VisualGeometryObject* VisualGeometryObjectsCollection::operator[](uint idx)
{
	return vg_objects[idx];
}


