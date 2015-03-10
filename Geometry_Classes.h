#pragma once
#ifndef GEOMETRY_CLASSES_H
#define GEOMETRY_CLASSES_H

#include "D3D.h"


#include "modules\Facilities_LIB\Facilities_LIB.h"


#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>


#include "Allocator.h"

#include "Vertex.h"

#include "ShaderSystem.h"


#include <list>
#include <map>
#include <set>
#include <deque>




const float Pi = 3.14159265358979323f;



namespace geometry_objects
{


	class AbstractGeometryObject
	{


	public:

		AbstractGeometryObject();
		~AbstractGeometryObject(){};

	public:


		DirectX::XMVECTOR get_Position();
		void set_Position(DirectX::FXMVECTOR new_Position);

		DirectX::XMVECTOR get_Right();
		void set_Right(DirectX::FXMVECTOR new_Right);

		DirectX::XMVECTOR get_Direction();
		void set_Direction(DirectX::FXMVECTOR new_Direction);

		DirectX::XMVECTOR get_Up();
		void set_Up(DirectX::FXMVECTOR new_Up);

	public:

		__declspec(property(get = get_Position, put = set_Position)) DirectX::XMVECTOR Position;
		//float pad1;      // not used


		__declspec(property(get = get_Right, put = set_Right)) DirectX::XMVECTOR Right;
		//float pad2;      // not used


		__declspec(property(get = get_Up, put = set_Up)) DirectX::XMVECTOR Up;
		//float pad3;      // not used


		__declspec(property(get = get_Direction, put = set_Direction)) DirectX::XMVECTOR Direction;
		//float pad4;      // not used


		DirectX::XMFLOAT4X4 ObjectSpace;


		void RotateAroundXAxis(float angle);
		void RotateAroundYAxis(float angle);
		void RotateAroundZAxis(float angle);
		void RotateAroundHorizontalAxis(float angle);
		void RotateAroundVerticalAxis(float angle);

		void RotateAroundAxis(DirectX::FXMVECTOR axis, float angle);

		void TranslateX(float dist);
		void TranslateY(float dist);
		void TranslateZ(float dist);


		virtual void RebuildBasis();

		virtual bool IsAbstract() = 0;

	public:

		DirectX::XMVECTOR fPosition;
		//float pad6;
		DirectX::XMVECTOR fRight;
		//float pad7;
		DirectX::XMVECTOR fUp;
		//float pad8;
		DirectX::XMVECTOR fDirection;
		//float pad9;


	public:

		DirectX::XMVECTOR changed;
		//float pad5;

	protected:

		typedef enum{ right, up, direction } Axes;

		void Rotate2AxesAroundAxis(DirectX::FXMVECTOR prev_axis, DirectX::FXMVECTOR new_axis, Axes holdAxis);

	};


	enum LightSourceTypes { Parallel, Point, Spot };

	class LightSource : public AbstractGeometryObject
	{

	public:

		LightSource();

		void Init(LightSourceTypes LightSourceType, ID3D11Device* device, ID3D11DeviceContext* DeviceContext,
			char* fx_file_name, char* technique_name);

		void InitFromExistingData(LightSourceTypes LightSourceType, ID3D11Device* device, ID3D11DeviceContext* DeviceContext,
			ID3DX11Effect* fx, ID3DX11EffectTechnique* Technique);


		void Create_FX_and_Tech(char* fx_file_name, char* technique_name);

		void ConnectToShaderVariables();

		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;

		DirectX::XMVECTOR att;
		float pad5;

		float spotPow;
		float range;
		float ConeMaxAngle; // theta in hlsl Light structure
		int LSType;

		ID3DX11Effect* GetEffect() const;
		ID3DX11EffectTechnique* GetTechnique() const;

		DirectX::XMMATRIX GetTransformToObjectSpace();


		void Cast(DirectX::FXMVECTOR* mEyePos);


		bool IsAbstract() override { return false; }


	protected:

		// Связь с эффектом и техникой
		ID3DX11Effect* fx;
		ID3DX11EffectTechnique* Technique;

		ID3DX11EffectVariable* fxLightVar;
		ID3DX11EffectVariable* fxEyePosVar;
		ID3DX11EffectScalarVariable* fxLightTypeVar;



		char* fx_file_name;
		char* technique_name;


		ID3D11Device* d3dDevice;
		ID3D11DeviceContext* d3dDeviceContext;

	};



	class Camera : public AbstractGeometryObject
	{

	public:

		Camera();
		~Camera();

		void RebuildBasis() override;


		DirectX::XMMATRIX* Observe(HWND RenderWindow, const int &aimX, const int &aimY,
			const int &screencenterX, const int &screencenterY, const float &observationVelocity);

		DirectX::XMMATRIX* Observe(HWND RenderWindow, const int &dx, const int &dy, const float &observationVelocity);


		bool IsAbstract() override { return false; }

	};


	

	typedef enum TBufferKind { VertexBuffer, IndexBuffer } TBufferKind;
	typedef enum TInfoKind { ObjectSpace, Normals, Position } TInfoKind;





	class InfoVectors
	{
	public:


		void Init(ID3D11Device* device, ID3D11DeviceContext* DeviceContext, char* fx_file_name, char* technique_name)
		{
			d3dDevice = device;
			d3dDeviceContext = DeviceContext;
			FX11.Create(device, DeviceContext, fx_file_name, technique_name);
			VertexInputLayoutCreate();
		}


		effects::Effect11 FX11;

		void VertexInputLayoutCreate();

		void SetVertices(const std::vector<VertexPosColor>& vertices);


		void Construct(const std::vector<VertexPosColor>& vertices, const std::vector<DWORD>& indices);

		void Render(DirectX::CXMMATRIX& WVP = DirectX::XMMatrixIdentity(),
			D3D11_FILL_MODE FillMode = D3D11_FILL_SOLID,
			BOOL isFrontCW = TRUE,
			D3D11_CULL_MODE CullingMode = D3D11_CULL_BACK,
			D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	private:

		virtual void BufferCreate(TBufferKind BufferKind, D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags); // виртуальными сделал на будущее

		virtual void VertexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags);
		virtual void IndexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags);

		ID3D11Device* d3dDevice;
		ID3D11DeviceContext* d3dDeviceContext;

		ID3D11InputLayout* VertexLayout;

		std::vector<VertexPosColor> vertices;
		std::vector<DWORD> indices;

		ID3D11Buffer* VB;
		ID3D11Buffer* IB;

		//Параметры растеризации
		ID3D11RasterizerState* RState;

	};



	typedef enum StateAfterRender { obj_changed = 1, obj_not_changed = 0 } StateAfterRender;
	//typedef enum Effects_Type {et_Effect11 = 0, et_ShaderSystem = 1} Effects_Type;
	typedef allocator_aligned<VertexPosNormVelocityTex, 16> a16VPNVT;



	


	class VisualGeometryObject : public AbstractGeometryObject
	{

	public:

		DirectX::XMMATRIX TextureTransformationMatrix;

		VisualGeometryObject()
		{			

			name = "";

			VB = 0;
			IB = 0;
			effects_type = et_Effect11;
			NumVertices = 0;
			NumFaces = 0;
			scale = 1;
			m = 1;
			Deformable = false;
			Kd = 0.15;
			VertexLayout = 0;
			counter = 0;
			g = 1.0 / 9;
			KarmanLine = 500;
			MetalCore = -500;
			inCollision = false;
			Kv = 1;			


			TextureTransformationMatrix = DirectX::XMMatrixIdentity();
			XMStoreFloat4x4(&ScaleMatrix, DirectX::XMMatrixIdentity());
			TranslationMatrix = DirectX::XMMatrixIdentity();



			pos_prev = DirectX::XMVectorSet(0, 0, 0, 0);
			PositionW_prev = DirectX::XMVectorSet(0, 0, 0, 0);
			velocity = DirectX::XMVectorSet(0, 0, 0, 0);
			velocity_prev = DirectX::XMVectorSet(0, 0, 0, 0);
			Force = DirectX::XMVectorSet(-1e-1, 1e-1, 0, 0);
			Force_total = DirectX::XMVectorSet(0, -0.5, 0, 0);

		}

		

		VisualGeometryObject(std::string name);



		VisualGeometryObject(std::string name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type);


		//virtual void ConstructGeometry(IArguments*) = 0;


		~VisualGeometryObject()
		{
			ReleaseCOM(VB);
			ReleaseCOM(IB);
		}

		// использовать этот метод, когда каждый объект будет иметь свой набор эффектов
		void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, char* VS_file_name, char* VS_EntryPoint, char* VS_ShaderModel);

		void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type);

		void Init(std::string name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type);


		// использовать этот метод, когда все
		// объекты сцены разделяют однин и тот же набор эффектов
		void InitFromExistingData(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
			const effects::shader_system::Effects& effects, ID3DBlob* VS_byte_code);



		void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, char* fx_file_name, char* technique_name)
		{
			this->d3dDevice = d3dDevice;
			this->d3dDeviceContext = d3dDeviceContext;

			effects_type = et_Effect11;

			FX11.Create_FX_and_Tech(fx_file_name, technique_name, d3dDevice, d3dDeviceContext);
			FX11.ConnectToShaderVariables();
			VertexInputLayoutCreate();
		}


		void InitFromExistingData(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, ID3DX11Effect* fx, ID3DX11EffectTechnique* Technique)
		{
			this->d3dDevice = d3dDevice;
			this->d3dDeviceContext = d3dDeviceContext;

			effects_type = et_Effect11;


			ReleaseCOM(FX11.fx);
			FX11.fx = fx;

			DeleteCOM(FX11.Technique);
			FX11.Technique = Technique;

			FX11.set_device(d3dDevice);
			FX11.set_device_context(d3dDeviceContext);

			FX11.ConnectToShaderVariables();
			VertexInputLayoutCreate();
		}


		void VertexInputLayoutCreate(ID3DBlob* VS_byte_code);
		void VertexInputLayoutCreate();

		//void SetPosition(const float& x, const float& y, const float& z) { pos_prev = DirectX::XMVectorSet(x, y, z, 0);}

		void SetPosition(const float& x, const float& y, const float& z) { Position = DirectX::XMVectorSet(x, y, z, 0); }



		/*
		void TranslateX(float dist) {Position += dist*Right; velocity= Position-pos_prev;}
		void TranslateY(float dist) {Position += dist*Up; velocity= Position-pos_prev;}
		void TranslateZ(float dist) {Position += dist*Direction; velocity= Position-pos_prev;}
		*/
		/*
		void TranslateX(float dist) {Position += dist*Right; velocity_prev= Position-pos_prev;}
		void TranslateY(float dist) {Position += dist*Up; velocity_prev= Position-pos_prev;}
		void TranslateZ(float dist) {Position += dist*Direction; velocity_prev= Position-pos_prev;}
		*/



		void TranslateX(float dist);
		void TranslateY(float dist);
		void TranslateZ(float dist);

		void ScaleObject(const float& scale)
		{


			ScaleMatrix._11 = scale;
			ScaleMatrix._22 = scale;
			ScaleMatrix._33 = scale;

			this->scale = scale;

		}


		virtual void CalculateVertexNormals();


		void RebuildBasis() override;




		// индивидуальная отрисовка
		void Render(DirectX::CXMMATRIX WVP = DirectX::XMMatrixIdentity(),
			D3D11_FILL_MODE FillMode = D3D11_FILL_SOLID,
			BOOL isFrontCW = TRUE,
			D3D11_CULL_MODE CullingMode = D3D11_CULL_BACK,
			D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			StateAfterRender isChanged = obj_not_changed);


		void RenderInGroup(); // использовать для отрисовки, когда объект является частью объектной группы



		DirectX::XMMATRIX WV; //Object space of a camera
		DirectX::XMMATRIX WVP;

		bool isChangedAfterRender;

		delegates::Delegate Construct_Geometry;


		D3D11_RASTERIZER_DESC rasterisation_state;


		effects::shader_system::Texture2D DiffuseTexture;

		effects::shader_system::Effects* FX_of_Group;


		effects::shader_system::Effects FX;
		effects::Effect11 FX11;

		Effects_Type effects_type;



		int counter;

		std::vector<float> angles;
		std::vector<float> Kv_coefs;
		std::vector<DirectX::XMVECTOR> F_ball;
		std::vector<DirectX::XMVECTOR> F_cloth;
		std::vector<DirectX::XMVECTOR> F;

		DirectX::XMVECTOR velocity_prev;


		DirectX::XMFLOAT4X4 ScaleMatrix;
		DirectX::XMMATRIX TranslationMatrix;

		DWORD NumVertices;
		DWORD NumFaces;

		float Kv;

		bool inCollision;

		float scale;

		float m; // масса объекта

		double Kd; // stiffness coefficient

		float r; // радиус объекта
		float GetRadius() const { return r; }

		float g;

		float KarmanLine;
		float MetalCore;

		bool Deformable; // деформируемость

		DirectX::XMVECTOR pos_prev; // для недеформируемых объектов

		//void AddVelocity(DirectX::FXMVECTOR v, const float& dt) {velocity += v; velocity.y += -m*1; a += v/dt;} 

		void AddVelocity(DirectX::FXMVECTOR v, const float& dt);
		void SetVelocity(DirectX::FXMVECTOR v, const float& dt);
		DirectX::XMVECTOR GetVelocity() const { return velocity; }

		/*void AddVelocity(DirectX::FXMVECTOR v, const float& dt) {velocity_prev += v + XMVectorZero(); a += v/dt;}
		void SetVelocity(DirectX::FXMVECTOR v, const float& dt) {velocity_prev = v; a = v/dt;}
		DirectX::XMVECTOR GetVelocity() const {return velocity_prev;}*/

		DirectX::XMVECTOR GetAcceleration() const { return a; }


		DirectX::XMVECTOR Force;
		DirectX::XMVECTOR Force_total;

		DirectX::XMVECTOR PositionW;

		//void AddForce(DirectX::FXMVECTOR F) {Force += F;} 
		//DirectX::FXMVECTOR GetForce() const {return Force;}

		void AddForce(DirectX::FXMVECTOR F);

		DirectX::XMVECTOR GetForce() const { return Force_total; }

		void ApplyForce(float dt, const bool& isApplyGravity);




		std::vector<VertexPosNormVelocityTex, a16VPNVT> vertices;
		std::vector<VertexPosNormVelocityTex, a16VPNVT> vertices_old;
		std::vector<DWORD> indices;

		/*
		std::vector<VertexPosNormVelocityTex> vertices;
		std::vector<VertexPosNormVelocityTex> vertices_old;
		*/


		bool IsAbstract() override  { return false; }


		void InfoVectorsConstruct(TInfoKind InfoKind, const std::vector<VertexPosNormVelocityTex>* const geometry);
		void InfoVectorsShow();


		std::string name;

		

	protected:

		ID3D11Device* d3dDevice;
		ID3D11DeviceContext* d3dDeviceContext;

		DirectX::XMVECTOR a; // ускорение
		DirectX::XMVECTOR velocity;
		//DirectX::XMVECTOR velocity_prev;

		DirectX::XMVECTOR PositionW_prev;

		ID3D11Buffer* VB;
		ID3D11Buffer* IB;

		virtual void BufferCreate(TBufferKind BufferKind, D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags); // виртуальными сделал на будущее

		virtual void VertexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags);
		virtual void IndexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags);

		//---Для отображения инфовекторов	

		std::vector<VisualGeometryObject> InfoVectors;


		/*
		ID3D11InputLayout* VertexLayoutInfovectors;
		std::vector<VertexPosColor> vertices_info;
		std::vector<DWORD> indices_info;

		ID3D11Buffer* VB_infovectors;
		ID3D11Buffer* IB_infovectors;

		DWORD NumInfovectorsVertices;

		ID3DX11EffectTechnique* TechniqueInfovectors;
		*/
		//---



		//Тип вершин
		ID3D11InputLayout* VertexLayout;




		//Параметры растеризации
		ID3D11RasterizerState* RState;

	};




	class Sphere : public VisualGeometryObject
	{

	public:


		Sphere(std::string name);


		Sphere() : VisualGeometryObject() 
		{
			static int id = 1;

				name = "Sphere" + std::to_string(id);

			id++;
		}

		void ConstructGeometry(const float r = 1, const float h = 2, const DWORD vqty = 100, const DWORD cqty = 100);

		bool IsAbstract() override { return false; }

		void CalculateVertexNormals() override;


	private:


		DWORD vqty, cqty;
		//float r, h;
		float h;

	};



	class Cube : public VisualGeometryObject
	{

	public:

		
		Cube()
		{
		
			static int id = 1;

				name = "Cube" + std::to_string(id);

			id++;

		}


		Cube(std::string name);

		bool IsAbstract() override { return false; }


		void ConstructGeometry(float scale = 1);


	};


	class Mesh : public VisualGeometryObject
	{

	public:

		Mesh(std::string name);


		Mesh() :NumSubObjects(0)
		{
			static int id = 1;

				name = "Mesh" + std::to_string(id);

			id++;
		}

		bool IsAbstract() override { return false; }

		


		std::string Object_Name;

		std::vector<Mesh> SubObjects;

		DWORD NumSubObjects;


		bool LoadMesh(const char *FileName);


	protected:

		virtual void VertexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags);
		virtual void IndexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags);


	};


	class Grid : public VisualGeometryObject
	{

	public:

		Grid(std::string name);


		Grid()
		{
			static int id = 1;

				name = "Grid" + std::to_string(id);

			id++;
		}

		bool IsAbstract() override { return false; }

		void ConstructGeometry(DWORD m, DWORD n, float dx);

	};



	class Cloth : public VisualGeometryObject
	{

	public:

		bool IsAbstract() override { return false; }

		Cloth(std::string name);


		Cloth() : indices_vpatterns(0), NumPatternIndices(0), MaxVertices(1e2 * 3) 
		{ 
			static int id = 1;

				name = "Cloth" + std::to_string(id);

			id++;

			Deformable = true; m = 1; Kd = 1e-1; r = 10; 
		
		}
		//Cloth(): indices_vpatterns(0), NumPatternIndices(0), MaxVertices(1e2*3) {Deformable = true; m = 1; Kd = 1; r = 10;}

		void ConstructGeometry(DWORD m, DWORD n, float dx);

		void SimulateClothDynamics(const float& dt, DirectX::FXMVECTOR Force);

		DWORD  NumRows,
			NumCols;




	protected:

		std::deque<float> radii; // радиусы, соответствующие группам в паттернах	

		DWORD MaxVertices;


		// --- PATTERNS

		void BuildIndices_vpatterns(DWORD& m, DWORD& n, float& d);
		DWORD NumPatternIndices;
		std::deque<DWORD> indices_vpatterns; // indices for vertices' patterns


	};


	class Landscape : public Grid
	{

	public:

		//bool IsAbstract() override {return false;}

		float getHeight(float x, float z)const;

		void ConstructGeometry(ID3D11Device* device, DWORD m, DWORD n, float dx);


	};


} // namespace geometry_objects


#endif




