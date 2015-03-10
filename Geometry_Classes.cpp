#include "Geometry_Classes.h"


#include "IncludeHLSL.h"

//For Mesh --------------------------------//
#include <fstream>
#include <io.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//-------------------------------------//


//#include <d3dcompiler.h>



#include "MathFunctions.h"
//#include <array>

#include <tuple>
#include <stddef.h>


using std::vector;
using std::string;

using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace effects;
using namespace effects::shader_system;


#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)
#define my_sizeof(type_val) (char*)(&type_val+1)-(char*)(&type_val)

bool operator==(FXMVECTOR v1, FXMVECTOR v2)
{
	/*
	uint32_t comparison_serialized_result;

	XMVectorEqualR(&comparison_serialized_result, v1, v2);

	return XMComparisonAllTrue(comparison_serialized_result);
	*/


	uint16_t bit_field = 0;

	bit_field |= (v1.m128_f32[0] == v2.m128_f32[0]) << 0;
	bit_field |= (v1.m128_f32[1] == v2.m128_f32[1]) << 1;
	bit_field |= (v1.m128_f32[2] == v2.m128_f32[2]) << 2;
	bit_field |= (v1.m128_f32[3] == v2.m128_f32[3]) << 3;


	return (bit_field == 0xF);

}

bool operator!=(FXMVECTOR v1, FXMVECTOR v2)
{

	uint16_t bit_field = 0;

	bit_field |= (v1.m128_f32[0] == v2.m128_f32[0]) << 0;
	bit_field |= (v1.m128_f32[1] == v2.m128_f32[1]) << 1;
	bit_field |= (v1.m128_f32[2] == v2.m128_f32[2]) << 2;
	bit_field |= (v1.m128_f32[3] == v2.m128_f32[3]) << 3;


	return (bit_field != 0xF);

}


namespace geometry_objects
{


	AbstractGeometryObject::AbstractGeometryObject()
	{

		XMStoreFloat4x4(&ObjectSpace, DirectX::XMMatrixIdentity());

		fPosition = XMVectorZero();
		fUp = XMVectorSet(0, 1, 0, 0);
		fDirection = XMVectorSet(0, 0, 1, 0);
		fRight = XMVectorSet(1, 0, 0, 0);


		changed = XMVectorSet(1, 0, 0, 0);

	}


	void AbstractGeometryObject::Rotate2AxesAroundAxis(FXMVECTOR prev_axis, FXMVECTOR new_axis, Axes holdAxis)
	{

		XMVECTOR new_axis_ort;

		new_axis_ort = XMVector3Normalize(new_axis);

		if (new_axis_ort != prev_axis)
		{

			XMVECTOR axis;

			axis = XMVector3Cross(prev_axis, new_axis_ort); // в зависимости от контекста - новый x,y или z

			axis = XMVector3Normalize(axis);


			switch (holdAxis)
			{

			case Axes::right:

				fRight = new_axis_ort;
				fDirection = XMVector3Normalize(XMVector3Cross(fRight, XMVectorSet(0, 1, 0, 0)));
				fUp = XMVector3Normalize(XMVector3Cross(fRight, fDirection));

				break;

			case Axes::up:

				fUp = new_axis_ort;
				fRight = XMVector3Normalize(XMVector3Cross(fUp, XMVectorSet(0, 1, 0, 0)));
				fDirection = XMVector3Normalize(XMVector3Cross(fUp, fRight));

				break;

			case Axes::direction:

				fDirection = new_axis_ort;

				XMVECTOR v = XMVectorSet(0, 1, 0, 0),
					nv = XMVectorSet(0, -1, 0, 0);

				if (v == fDirection || nv == fDirection) v = XMVectorSet(1, 0, 0, 0);

				fRight = XMVector3Normalize(XMVector3Cross(v, fDirection));
				//fRight= XMVector3Normalize(XMVector3Cross(fDirection, v));
				fUp = XMVector3Normalize(XMVector3Cross(fDirection, fRight));
				//fUp= XMVector3Normalize(XMVector3Cross(fRight, fDirection));

				break;

			}

			changed.m128_f32[0] = 1;


		}// if
	}



	XMVECTOR AbstractGeometryObject::get_Position()
	{
		return fPosition;
	}


	void AbstractGeometryObject::set_Position(FXMVECTOR new_Position)
	{
		if (new_Position != fPosition)
		{
			fPosition = new_Position;

			changed.m128_f32[0] = 1;
		}
	}


	XMVECTOR AbstractGeometryObject::get_Right()
	{
		return fRight;
	}


	void AbstractGeometryObject::set_Right(FXMVECTOR new_Right)
	{
		Rotate2AxesAroundAxis(fRight, new_Right, Axes::right);

		//XMVector3Normalize(fRight, &new_Right);
	}

	XMVECTOR AbstractGeometryObject::get_Direction()
	{
		return fDirection;
	}


	void AbstractGeometryObject::set_Direction(FXMVECTOR new_Direction)
	{
		Rotate2AxesAroundAxis(fDirection, new_Direction, Axes::direction);

		//XMVector3Normalize(fDirection, &new_Direction);
	}

	XMVECTOR AbstractGeometryObject::get_Up()
	{
		return fUp;
	}


	void AbstractGeometryObject::set_Up(FXMVECTOR new_Up)
	{
		Rotate2AxesAroundAxis(fUp, new_Up, Axes::up);

		//XMVector3Normalize(fUp, &new_Up);
	}






	void AbstractGeometryObject::TranslateX(float dist)
	{

		fPosition += dist*fRight;

		changed.m128_f32[0] = 1;

	}


	void AbstractGeometryObject::TranslateY(float dist)
	{

		fPosition += dist*fUp;

		changed.m128_f32[0] = 1;

	}


	void AbstractGeometryObject::TranslateZ(float dist)
	{

		fPosition += dist*fDirection;

		changed.m128_f32[0] = 1;

	}


	void AbstractGeometryObject::RotateAroundAxis(FXMVECTOR axis, float angle)
	{
		XMMATRIX R;
		R = XMMatrixRotationAxis(axis, angle);

		fRight = XMVector3TransformNormal(fRight, R);
		fUp = XMVector3TransformNormal(fUp, R);
		fDirection = XMVector3TransformNormal(fDirection, R);

		changed.m128_f32[0] = 1;

	}



	void AbstractGeometryObject::RotateAroundHorizontalAxis(float angle)
	{

		XMMATRIX R;
		R = XMMatrixRotationAxis(fRight, angle);

		fUp = XMVector3TransformNormal(fUp, R);
		fDirection = XMVector3TransformNormal(fDirection, R);

		changed.m128_f32[0] = 1;

	}


	void AbstractGeometryObject::RotateAroundVerticalAxis(float angle)
	{


		XMMATRIX R;
		R = XMMatrixRotationAxis(fUp, angle);

		fRight = XMVector3TransformNormal(fRight, R);
		fDirection = XMVector3TransformNormal(fDirection, R);

		changed.m128_f32[0] = 1;
	}


	void AbstractGeometryObject::RotateAroundXAxis(float angle)
	{

		XMMATRIX R;
		R = XMMatrixRotationX(angle);

		fRight = XMVector3TransformNormal(fRight, R);
		fUp = XMVector3TransformNormal(fUp, R);
		fDirection = XMVector3TransformNormal(fDirection, R);


		changed.m128_f32[0] = 1;

	}


	void AbstractGeometryObject::RotateAroundYAxis(float angle)
	{

		XMMATRIX R;
		R = XMMatrixRotationY(angle);

		fRight = XMVector3TransformNormal(fRight, R);
		fUp = XMVector3TransformNormal(fUp, R);
		fDirection = XMVector3TransformNormal(fDirection, R);

		changed.m128_f32[0] = 1;
	}


	void AbstractGeometryObject::RotateAroundZAxis(float angle)
	{

		XMMATRIX R;
		R = XMMatrixRotationZ(angle);

		fRight = XMVector3TransformNormal(fRight, R);
		fUp = XMVector3TransformNormal(fUp, R);
		fDirection = XMVector3TransformNormal(fDirection, R);

		changed.m128_f32[0] = 1;

	}

	void AbstractGeometryObject::RebuildBasis()
	{

		// Получить матрицу перехода от объектного пространства к мировому пространству

		float x = XMVectorGetByIndex(XMVector3Dot(fPosition, fRight), 0);
		float y = XMVectorGetByIndex(XMVector3Dot(fPosition, fUp), 0);
		float z = XMVectorGetByIndex(XMVector3Dot(fPosition, fDirection), 0);

		XMStoreFloat4x4(&ObjectSpace, XMMatrixSet

			(
			fRight.m128_f32[0], fRight.m128_f32[1], fRight.m128_f32[2], 0,
			fUp.m128_f32[0], fUp.m128_f32[1], fUp.m128_f32[2], 0,
			fDirection.m128_f32[0], fDirection.m128_f32[1], fDirection.m128_f32[2], 0,
			x, y, z, 1
			));



	}


	LightSource::LightSource() : fx_file_name(0), technique_name(0)
	{

		ConeMaxAngle = Pi / 4;

	}

	void LightSource::Init(LightSourceTypes LightSourceType, ID3D11Device* device, ID3D11DeviceContext* DeviceContext,
		char* fx_file_name, char* technique_name)
	{

		d3dDevice = device;
		d3dDeviceContext = DeviceContext;

		Create_FX_and_Tech(fx_file_name, technique_name);

		switch (LightSourceType)
		{

		case Parallel:
		{
							// Parallel light.

							LSType = 1;

							Direction = XMVectorSet(0.57735f, -0.57735f, 0.57735f, 0.0f);
							//ambient = XMVectorSet(0.4f, 0.4f, 0.4f, 1.0f);
							//ambient = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
							ambient = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
							diffuse = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
							specular = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
							break;
		}

		case Point:
		{
						// Pointlight--position is changed every frame to animate.

						LSType = 2;

						ambient = XMVectorSet(0.4f, 0.4f, 0.4f, 1.0f);
						diffuse = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
						specular = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
						att.m128_f32[0] = 0.0f;
						att.m128_f32[1] = 0.1f;
						att.m128_f32[2] = 0.0f;
						range = 50.0f;
						break;
		}

		case Spot:
		{

						// Spotlight--position and direction changed every frame to animate.

						LSType = 3;

						ambient = XMVectorSet(0.4f, 0.4f, 0.4f, 1.0f);
						diffuse = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
						specular = XMVectorSet(1.0f, 1.0f, 1.0f, 2.0f);
						att.m128_f32[0] = 1.0f;
						att.m128_f32[1] = 0.0f;
						att.m128_f32[2] = 0.0f;
						spotPow = 128;
						range = 1e4;
						break;
		}

		}


	}

	void LightSource::InitFromExistingData(LightSourceTypes LightSourceType, ID3D11Device* device, ID3D11DeviceContext* DeviceContext,
		ID3DX11Effect* fx, ID3DX11EffectTechnique* Technique)
	{

		d3dDevice = device;
		d3dDeviceContext = DeviceContext;


		ReleaseCOM(this->fx);
		this->fx = fx;

		DeleteCOM(this->Technique);
		this->Technique = Technique;

		ConnectToShaderVariables();


		switch (LightSourceType)
		{

		case Parallel:
		{
							// Parallel light.

							LSType = 1;

							Direction = XMVectorSet(0.57735f, -0.57735f, 0.57735f, 0.0f);
							//ambient = XMVectorSet(0.4f, 0.4f, 0.4f, 1.0f);
							//ambient = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
							ambient = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
							diffuse = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
							specular = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
							break;
		}

		case Point:
		{
						// Pointlight--position is changed every frame to animate.


						LSType = 2;

						ambient = XMVectorSet(0.4f, 0.4f, 0.4f, 1.0f);
						diffuse = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
						specular = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
						att.m128_f32[0] = 0.0f;
						att.m128_f32[1] = 0.1f;
						att.m128_f32[2] = 0.0f;
						range = 50.0f;
						break;
		}

		case Spot:
		{

						// Spotlight--position and direction changed every frame to animate.

						LSType = 3;

						ambient = XMVectorSet(0.4f, 0.4f, 0.4f, 1.0f);
						diffuse = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
						specular = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
						att.m128_f32[0] = 1.0f;
						att.m128_f32[1] = 0.0f;
						att.m128_f32[2] = 0.0f;
						spotPow = 128;
						range = 1e4;
						ConeMaxAngle = Pi / 4;
						break;
		}

		}


	}


	void LightSource::Create_FX_and_Tech(char* fx_file_name, char* technique_name)
	{

		if (this->fx_file_name != fx_file_name)
		{

			this->fx_file_name = fx_file_name;
			this->technique_name = technique_name;

			ReleaseCOM(fx);
			if (Technique) { delete Technique; Technique = 0; }


			DWORD shaderFlags = 0;

	#if defined(DEBUG) || defined(_DEBUG)
			shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif

	#ifndef DEBUG && _DEBUG
			shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
	#endif

			//ID3DBlob* // for use with D3DCompileFromFile() API after unplugging legacy SDK

			ID3D10Blob* pCompiledFXBlob = 0;

			ID3D10Blob* compilationErrors = 0;


			// Преобразование из char* в WCHAR*

			const WCHAR* fn = 0;

			fn = string_facilities::pchar_to_pwchar(fx_file_name);


			// Создание экземпляра класса IncludeHLSL, реализующего пользовательский механизм подключения HLSL-модулей
			//IncludeHLSL include_hlsl;

			//HRESULT hr=	D3DCompileFromFile(fn, NULL, &include_hlsl, NULL, "fx_5_0", shaderFlags, 0, &pCompiledFXBlob, &compilationErrors);

			HRESULT hr = D3DCompileFromFile(fn, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, "fx_5_0", shaderFlags, 0, &pCompiledFXBlob, &compilationErrors);

			//HRESULT hr=	D3DX11CompileFromFile(fxfn, NULL, NULL, NULL, "fx_5_0", shaderFlags, 0, NULL, &pCompiledFXBlob, &compilationErrors, NULL);


			//Если возникли ошбки во время компиляции шейдеров
			if (compilationErrors)
			{
				string s = (char*)compilationErrors->GetBufferPointer();

				if (s.find("warning") == string::npos)
					MessageBoxA(0, s.c_str(), "Shader compile error(s)", 0);

				ReleaseCOM(compilationErrors);
			}

			//Если были другие ошибки
			if (FAILED(hr))
			{
				DXTrace(__FILEW__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", TRUE);
			}


			HR(D3DX11CreateEffectFromMemory(pCompiledFXBlob->GetBufferPointer(), pCompiledFXBlob->GetBufferSize(), 0, d3dDevice, &fx));

			Technique = fx->GetTechniqueByName(technique_name);

			delete[] fn;

		}

		if ((this->technique_name != technique_name) && (fx_file_name == this->fx_file_name))
		{

			this->technique_name = technique_name;

			Technique = fx->GetTechniqueByName(technique_name);

		}

		fxLightVar = fx->GetVariableByName("light");
		fxEyePosVar = fx->GetVariableByName("EyePosW");

		fxLightTypeVar = fx->GetVariableByName("LightSourceType")->AsScalar();

	}

	void LightSource::ConnectToShaderVariables()
	{

		fxLightVar = fx->GetVariableByName("light");
		fxEyePosVar = fx->GetVariableByName("EyePosW");

		fxLightTypeVar = fx->GetVariableByName("LightSourceType")->AsScalar();

	}


	ID3DX11Effect* LightSource::GetEffect() const
	{


		if (fx) return fx;

		else return 0;


	}


	ID3DX11EffectTechnique* LightSource::GetTechnique() const
	{


		if (Technique) return Technique;

		else return 0;


	}


	XMMATRIX LightSource::GetTransformToObjectSpace()
	{
		// Получить матрицу перехода от мирового пространства к объектному пространству (ВП) ИС

		float x = -XMVectorGetByIndex(XMVector3Dot(fPosition, fRight), 0);
		float y = -XMVectorGetByIndex(XMVector3Dot(fPosition, fUp), 0);
		float z = -XMVectorGetByIndex(XMVector3Dot(fPosition, fDirection), 0);

		XMMATRIX TransformToObjectSpace;

		TransformToObjectSpace = XMMatrixSet

			(
				fRight.m128_f32[0], fUp.m128_f32[0], fDirection.m128_f32[0], 0,
				fRight.m128_f32[1], fUp.m128_f32[1], fDirection.m128_f32[1], 0,
				fRight.m128_f32[2], fUp.m128_f32[2], fDirection.m128_f32[2], 0,
				x, y, z, 1
			);



		/*
		XMMATRIX TransformToObjectSpace;


		XMMatrixInverse(&TransformToObjectSpace, 0, &ObjectSpace);
		*/
		return TransformToObjectSpace;

	}


	void LightSource::Cast(FXMVECTOR* mEyePos)
	{
		/*
		fxEyePosVar->SetRawValue(&mEyePos, 0, sizeof(XMVECTOR));
		fxLightVar->SetRawValue(this, 0, sizeof(LightSource));
		*/
		fxLightTypeVar->SetInt(LSType);

	}


	Camera::Camera()
	{

		XMStoreFloat4x4(&ObjectSpace, DirectX::XMMatrixIdentity());

		Position = XMVectorSet(0, 0, -1.5, 0);
		Up = XMVectorSet(0, 1, 0, 0);
		Direction = XMVectorSet(0, 0, 1, 0);
		Right = XMVectorSet(1, 0, 0, 0);

	}


	Camera::~Camera()
	{


	}

	void Camera::RebuildBasis()
	{
		// Получить матрицу перехода от мирового пространства к объектному пространству (ВП) камеры

		float x = -XMVectorGetByIndex(XMVector3Dot(Position, Right), 0);
		float y = -XMVectorGetByIndex(XMVector3Dot(Position, Up), 0);
		float z = -XMVectorGetByIndex(XMVector3Dot(Position, Direction), 0);



		XMStoreFloat4x4(&ObjectSpace, XMMatrixSet

			(
			fRight.m128_f32[0], fUp.m128_f32[0], fDirection.m128_f32[0], 0,
			fRight.m128_f32[1], fUp.m128_f32[1], fDirection.m128_f32[1], 0,
			fRight.m128_f32[2], fUp.m128_f32[2], fDirection.m128_f32[2], 0,
			x, y, z, 1
			));

		/*
		ObjectSpace(0,0) = Right.x;
		ObjectSpace(1,0) = Right.y;
		ObjectSpace(2,0) = Right.z;
		ObjectSpace(3,0) = x;

		ObjectSpace(0,1) = Up.x;
		ObjectSpace(1,1) = Up.y;
		ObjectSpace(2,1) = Up.z;
		ObjectSpace(3,1) = y;

		ObjectSpace(0,2) = Direction.x;
		ObjectSpace(1,2) = Direction.y;
		ObjectSpace(2,2) = Direction.z;
		ObjectSpace(3,2) = z;

		ObjectSpace(0,3) = 0.0f;
		ObjectSpace(1,3) = 0.0f;
		ObjectSpace(2,3) = 0.0f;
		ObjectSpace(3,3) = 1.0f;
		*/

	}


	XMMATRIX* Camera::Observe(HWND RenderWindow, const int &aimX, const int &aimY,
		const int &screencenterX, const int &screencenterY, const float &observationVelocity)
	{

		ShowCursor(FALSE);

		short int dx = aimX - screencenterX;
		short int dy = screencenterY - aimY;


		if (dx != 0) RotateAroundYAxis(dx*observationVelocity);
		if (dy != 0) RotateAroundHorizontalAxis(-dy*observationVelocity);


		POINT p;
		p.x = screencenterX; p.y = screencenterY;

		ClientToScreen(RenderWindow, &p); // Изначально координаты вычисляются относительно клиентской части окна
		// поэтому переводим их в координаты экрана

		SetCursorPos(p.x, p.y); // После преобразования координат устанавливаем курсор в заданную точку, в центр экрана


		RebuildBasis();
		


		return &XMLoadFloat4x4(&ObjectSpace);

	}

	XMMATRIX* Camera::Observe(HWND RenderWindow, const int &dx, const int &dy, const float &observationVelocity)
	{

		if (dx != 0) RotateAroundYAxis(dx*observationVelocity);
		if (dy != 0) RotateAroundHorizontalAxis(dy*observationVelocity);

		SetCapture(RenderWindow);

		RebuildBasis();


		return &XMLoadFloat4x4(&ObjectSpace);

	}



	




	void InfoVectors::VertexInputLayoutCreate()
	{
		D3DX11_PASS_DESC PassDesc;

		FX11.GetTechnique()->GetPassByIndex(0)->GetDesc(&PassDesc);

		D3D11_INPUT_ELEMENT_DESC vertexformat[] =
		{

			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		};

		// при добавлении новой семантики менять 2й параметр на новое количество семантик
		d3dDevice->CreateInputLayout(vertexformat, 2, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &VertexLayout);
	}


	void InfoVectors::BufferCreate(TBufferKind BufferKind, D3D11_USAGE Usage,
		UINT BindFlags, UINT CPUAccessFlags)
	{
		D3D11_BUFFER_DESC bdesc;
		D3D11_SUBRESOURCE_DATA buf_data;

		bdesc.Usage = Usage;
		bdesc.BindFlags = BindFlags;
		bdesc.CPUAccessFlags = CPUAccessFlags;
		bdesc.MiscFlags = 0;

		if (TBufferKind::VertexBuffer == BufferKind)
		{
			bdesc.ByteWidth = sizeof(VertexPosColor)*vertices.size();
			buf_data.pSysMem = &vertices[0];
			d3dDevice->CreateBuffer(&bdesc, &buf_data, &VB);
		}
		else
		{
			bdesc.ByteWidth = sizeof(DWORD)*indices.size();
			buf_data.pSysMem = &indices[0];
			d3dDevice->CreateBuffer(&bdesc, &buf_data, &IB);
		}

	}

	void InfoVectors::VertexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags)
	{
		BufferCreate(TBufferKind::VertexBuffer, Usage, BindFlags, CPUAccessFlags);
	}


	void InfoVectors::IndexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags)
	{
		BufferCreate(TBufferKind::IndexBuffer, Usage, BindFlags, CPUAccessFlags);
	}


	void InfoVectors::SetVertices(const vector<VertexPosColor>& vertices)
	{
		this->vertices = vertices;
	}

	void InfoVectors::Construct(const vector<VertexPosColor>& vertices, const vector<DWORD>& indices)
	{
		this->vertices = vertices;
		this->indices = indices;

		VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);
		IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);

	}



	void InfoVectors::Render(DirectX::CXMMATRIX& WVP, D3D11_FILL_MODE FillMode, BOOL isFrontCW,
		D3D11_CULL_MODE CullingMode, D3D11_PRIMITIVE_TOPOLOGY Topology)
	{

		d3dDeviceContext->IASetInputLayout(VertexLayout);
		d3dDeviceContext->IASetPrimitiveTopology(Topology);


		FX11.GetEffect()->GetVariableByName("WVP")->AsMatrix()->SetMatrix((float*)&WVP);



		//Обновляем вершинный буфер

		VertexPosColor* v = 0;

		D3D11_MAPPED_SUBRESOURCE bufferMapping;

		d3dDeviceContext->Map(VB, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferMapping);

		v = (VertexPosColor*)bufferMapping.pData;

		for (DWORD i = 0; i < vertices.size(); i++)
		{
			v[i] = vertices[i];
			v[i].pos.m128_f32[3] = 1.0f;
		}

		d3dDeviceContext->Unmap(VB, 0);



		/*

		VB->Map(D3D11_MAP_WRITE_DISCARD, 0, (void**) &v);

		for(DWORD i = 0; i < vertices.size(); i++)
		{

		v[i] = vertices[i];

		}

		VB->Unmap();

		*/



		//------------------------Параметры растеризации---------------------//

		D3D11_RASTERIZER_DESC rstatedesc;
		ZeroMemory(&rstatedesc, sizeof(D3D11_RASTERIZER_DESC));

		rstatedesc.FillMode = FillMode;
		rstatedesc.FrontCounterClockwise = !isFrontCW;
		rstatedesc.CullMode = CullingMode;

		d3dDevice->CreateRasterizerState(&rstatedesc, &RState);

		d3dDeviceContext->RSSetState(RState);


		//---------------------- Отрисовка -- DrawIndexed() ---------------------//


		UINT stride = sizeof(VertexPosColor);

		UINT offset = 0;


		D3DX11_TECHNIQUE_DESC d3dtechdesc;

		FX11.GetTechnique()->GetDesc(&d3dtechdesc);

		for (UINT i = 0; i < d3dtechdesc.Passes; ++i)
		{

			FX11.GetTechnique()->GetPassByIndex(i)->Apply(0, d3dDeviceContext);

			d3dDeviceContext->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
			d3dDeviceContext->IASetIndexBuffer(IB, DXGI_FORMAT_R32_UINT, 0);

			d3dDeviceContext->DrawIndexed(indices.size(), 0, 0);

		}

	}


	VisualGeometryObject::VisualGeometryObject(std::string name) : VisualGeometryObject()
	{
		this->name = name;
	}


	VisualGeometryObject::VisualGeometryObject(std::string name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
		const Effects_Type& effects_type) : VisualGeometryObject(name)
	{
		Init(d3dDevice, d3dDeviceContext, effects_type);
	}


	// использовать этот метод, когда каждый объект будет иметь свой набор шейдеров
	void VisualGeometryObject::Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, char* VS_file_name,
		char* VS_EntryPoint, char* VS_ShaderModel)
	{
		this->d3dDevice = d3dDevice;
		this->d3dDeviceContext = d3dDeviceContext;

		effects_type = et_ShaderSystem;


		//Effects.AddShader("VS", effects::shader_system::ShaderTypes::VertexShader, VS_file_name, VS_EntryPoint, VS_ShaderModel);


		VertexInputLayoutCreate();
	}


	void VisualGeometryObject::Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type)
	{
		this->d3dDevice = d3dDevice;
		this->d3dDeviceContext = d3dDeviceContext;

		this->effects_type = effects_type;

		DiffuseTexture.set_device(d3dDevice);
	}


	void VisualGeometryObject::Init(std::string name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const Effects_Type& effects_type)
	{
		this->name = name;

		this->d3dDevice = d3dDevice;
		this->d3dDeviceContext = d3dDeviceContext;

		this->effects_type = effects_type;

		DiffuseTexture.set_device(d3dDevice);
	}


	// использовать этот метод, когда объект будет применять эффекты, поставляемые заранее предустановленным ShaderSystem; т.е. когда все
	// объекты сцены разделяют однин и тот же набор шейдеров
	void VisualGeometryObject::InitFromExistingData(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
		const effects::shader_system::Effects& effects, ID3DBlob* VS_byte_code)
	{
		this->d3dDevice = d3dDevice;
		this->d3dDeviceContext = d3dDeviceContext;

		effects_type = et_ShaderSystem;

		FX = effects;


		VertexInputLayoutCreate(VS_byte_code);
	}



	void VisualGeometryObject::VertexInputLayoutCreate(ID3DBlob* VS_byte_code)
	{

		D3D11_INPUT_ELEMENT_DESC vertexformat[] =
		{

			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 4 + 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 4 + 4 * 4 + 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 }

		};


		// при добавлении новой семантики менять 2й параметр на новое количество семантик
		d3dDevice->CreateInputLayout(vertexformat, 4, VS_byte_code->GetBufferPointer(), VS_byte_code->GetBufferSize(), &VertexLayout);
	}



	void VisualGeometryObject::VertexInputLayoutCreate()
	{

		D3D11_INPUT_ELEMENT_DESC vertexformat[] =
		{

			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 4 + 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 4 + 4 * 4 + 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 }

		};


		D3DX11_PASS_DESC PassDesc;

		FX11.Technique->GetPassByIndex(0)->GetDesc(&PassDesc);

		// при добавлении новой семантики менять 2й параметр на новое количество семантик
		d3dDevice->CreateInputLayout(vertexformat, 4, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &VertexLayout);

	}



	void VisualGeometryObject::InfoVectorsConstruct(TInfoKind InfoKind, const vector<VertexPosNormVelocityTex>* const geometry)
	{

		if (TInfoKind::ObjectSpace == InfoKind && nullptr == geometry)
		{

			InfoVectors.resize(1);

			InfoVectors[0].d3dDevice = d3dDevice;

			InfoVectors[0].FX11.SetEffect(FX11.fx);
			InfoVectors[0].FX11.SetTechnique(FX11.Technique);

			InfoVectors[0].VertexLayout = VertexLayout;


			InfoVectors[0].vertices.push_back(VertexPosNormVelocityTex(fPosition.m128_f32[0], fPosition.m128_f32[1], fPosition.m128_f32[2], 0, 0, 0, 0, 0, 0, 0, 0));
			InfoVectors[0].vertices.push_back(VertexPosNormVelocityTex(fRight.m128_f32[0] + fPosition.m128_f32[0],
				fRight.m128_f32[1] + fPosition.m128_f32[1],
				fRight.m128_f32[2] + fPosition.m128_f32[2], 0, 0, 0, 0, 0, 0, 0, 0));

			//InfoVectors[0].vertices.push_back(VertexPosNormVelocityTex(fPosition.m128_f32[0], fPosition.m128_f32[1], fPosition.m128_f32[2], 0, 0, 0,  0, 0, 0,  0, 0));
			InfoVectors[0].vertices.push_back(VertexPosNormVelocityTex(fUp.m128_f32[0] + fPosition.m128_f32[0],
				fUp.m128_f32[1] + fPosition.m128_f32[1],
				fUp.m128_f32[2] + fPosition.m128_f32[2], 0, 0, 0, 0, 0, 0, 0, 0));

			//InfoVectors[0].vertices.push_back(VertexPosNormVelocityTex(fPosition.m128_f32[0], fPosition.m128_f32[1], fPosition.m128_f32[2], 0, 0, 0,  0, 0, 0,  0, 0));
			InfoVectors[0].vertices.push_back(VertexPosNormVelocityTex(fDirection.m128_f32[0] + fPosition.m128_f32[0],
				fDirection.m128_f32[1] + fPosition.m128_f32[1],
				fDirection.m128_f32[2] + fPosition.m128_f32[2], 0, 0, 0, 0, 0, 0, 0, 0));


			vector<DWORD> indcs;


			indcs.push_back(0); indcs.push_back(1); // ось X

			indcs.push_back(0); indcs.push_back(2); // ось Y

			indcs.push_back(0); indcs.push_back(3); // ось Z


			InfoVectors[0].indices = indcs;


			InfoVectors[0].VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);

			InfoVectors[0].IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);


		}

	}


	void VisualGeometryObject::InfoVectorsShow()
	{



	}


	void VisualGeometryObject::BufferCreate(TBufferKind BufferKind, D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags)
	{

		D3D11_BUFFER_DESC bdesc;
		D3D11_SUBRESOURCE_DATA buf_data;


		bdesc.Usage = Usage;
		bdesc.BindFlags = BindFlags;
		bdesc.CPUAccessFlags = CPUAccessFlags;
		bdesc.MiscFlags = 0;



		if (TBufferKind::VertexBuffer == BufferKind)
		{
			bdesc.ByteWidth = sizeof(VertexPosNormVelocityTex)*NumVertices;


			buf_data.pSysMem = &vertices[0];


			d3dDevice->CreateBuffer(&bdesc, &buf_data, &VB);


		}
		else
		{

			bdesc.ByteWidth = sizeof(DWORD)*NumFaces * 3;


			buf_data.pSysMem = &indices[0];


			d3dDevice->CreateBuffer(&bdesc, &buf_data, &IB);
		}



	}

	void VisualGeometryObject::VertexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags)
	{
		/*
		D3D11_BUFFER_DESC vbdesc;

		vbdesc.ByteWidth= sizeof(VertexPosNormVelocityTex)*NumVertices;
		vbdesc.Usage= D3D11_USAGE_IMMUTABLE;
		vbdesc.BindFlags= D3D11_BIND_VERTEX_BUFFER;
		vbdesc.CPUAccessFlags= 0;
		vbdesc.MiscFlags= 0;
		*/

		/*
		D3D11_BUFFER_DESC vbdesc;

		vbdesc.ByteWidth= sizeof(VertexPosNormVelocityTex)*NumVertices;
		vbdesc.Usage= D3D11_USAGE_DYNAMIC;
		vbdesc.BindFlags= D3D11_BIND_VERTEX_BUFFER;
		vbdesc.CPUAccessFlags= D3D11_CPU_ACCESS_WRITE;
		vbdesc.MiscFlags= 0;

		D3D11_SUBRESOURCE_DATA vsbd;

		vsbd.pSysMem= &vertices[0];
		d3dDevice->CreateBuffer(&vbdesc, &vsbd, &VB);
		*/

		BufferCreate(TBufferKind::VertexBuffer, Usage, BindFlags, CPUAccessFlags);

	}


	void VisualGeometryObject::IndexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags)
	{

		/*
		D3D11_BUFFER_DESC ibdesc;

		ibdesc.ByteWidth= sizeof(DWORD)*NumFaces*3;
		ibdesc.Usage= D3D11_USAGE_IMMUTABLE;
		ibdesc.BindFlags= D3D11_BIND_INDEX_BUFFER;
		ibdesc.CPUAccessFlags= 0;
		ibdesc.MiscFlags= 0;

		D3D11_SUBRESOURCE_DATA isbd;

		isbd.pSysMem= &indices[0];
		d3dDevice->CreateBuffer(&ibdesc, &isbd, &IB);
		*/


		BufferCreate(TBufferKind::IndexBuffer, Usage, BindFlags, CPUAccessFlags);

	}

	void VisualGeometryObject::CalculateVertexNormals()
	{

		DWORD i;

		for (i = 0; i < NumFaces; i++)
		{

			DWORD i0, i1, i2;

			i0 = indices[i * 3];
			i1 = indices[i * 3 + 1];
			i2 = indices[i * 3 + 2];

			XMVECTOR v0, v1, v2, a, b, n, dir;

			v0 = vertices[i0].pos;
			v1 = vertices[i1].pos;
			v2 = vertices[i2].pos;

			a = v1 - v0;
			b = v2 - v0;

			n = XMVector3Cross(a, b);

			vertices[i0].normal += n;
			vertices[i1].normal += n;
			vertices[i2].normal += n;

		}


		for (i = 0; i < NumVertices; i++)
			vertices[i].normal = XMVector3Normalize(vertices[i].normal);


	}



	void VisualGeometryObject::RebuildBasis()
	{

		// Получить матрицу перехода от объектного пространства к мировому пространству

		PositionW.m128_f32[0] = XMVectorGetByIndex(XMVector3Dot(Position, Right), 0);
		PositionW.m128_f32[1] = XMVectorGetByIndex(XMVector3Dot(Position, Up), 0);
		PositionW.m128_f32[2] = XMVectorGetByIndex(XMVector3Dot(Position, Direction), 0);

		XMVECTOR dPosW = PositionW - PositionW_prev;

		PositionW_prev = PositionW;



		XMStoreFloat4x4(&ObjectSpace, XMMatrixSet

			(
			fRight.m128_f32[0], fRight.m128_f32[1], fRight.m128_f32[2], 0,
			fUp.m128_f32[0], fUp.m128_f32[1], fUp.m128_f32[2], 0,
			fDirection.m128_f32[0], fDirection.m128_f32[1], fDirection.m128_f32[2], 0,
			dPosW.m128_f32[0], dPosW.m128_f32[1], dPosW.m128_f32[2], 1
			));


		/*
		ObjectSpace(0,0) = Right.x;
		ObjectSpace(1,0) = Up.x;
		ObjectSpace(2,0) = Direction.x;
		ObjectSpace(3,0) = dPosW.x;

		ObjectSpace(0,1) = Right.y;
		ObjectSpace(1,1) = Up.y;
		ObjectSpace(2,1) = Direction.y;
		ObjectSpace(3,1) = dPosW.y;

		ObjectSpace(0,2) = Right.z;
		ObjectSpace(1,2) = Up.z;
		ObjectSpace(2,2) = Direction.z;
		ObjectSpace(3,2) = dPosW.z;

		ObjectSpace(0,3) = 0.0f;
		ObjectSpace(1,3) = 0.0f;
		ObjectSpace(2,3) = 0.0f;
		ObjectSpace(3,3) = 1.0f;
		*/


	}

	void VisualGeometryObject::TranslateX(float dist) { Position += dist*Right; }
	void VisualGeometryObject::TranslateY(float dist) { Position += dist*Up; Force.m128_f32[1] = -m*g + (m*g / KarmanLine)*Position.m128_f32[1]; }
	void VisualGeometryObject::TranslateZ(float dist) { Position += dist*Direction; }


	void VisualGeometryObject::AddVelocity(FXMVECTOR v, const float& dt)
	{
		velocity += v; a += v / dt;
	}

	void VisualGeometryObject::SetVelocity(FXMVECTOR v, const float& dt)
	{
		velocity = v; a = v / dt;
	}


	void VisualGeometryObject::AddForce(FXMVECTOR F)
	{
		Force_total += F;
	}

	void VisualGeometryObject::ApplyForce(float dt, const bool& isApplyGravity)
	{

		//
		// --- Adding new force and changing position
		//



		XMVECTOR Fg = XMVectorZero();       // gravity force
		//XMVECTOR Fg (0, -m*g, 0);       // gravity force
		//XMVECTOR Fg (0, -m*g*1e1, 0);       // gravity force

		XMVECTOR temp;

		DWORD i = 0;

		if (Deformable)
		{

			for (i = 0; i < NumVertices - 1; i++)
			{

				vertices[i].vel += Fg / m*dt;

				temp = vertices[i].pos;

				vertices[i].pos = vertices[i].pos + (vertices[i].pos - vertices_old[i].pos)*(1 - Kd) + vertices[i].vel*dt;

				vertices_old[i].pos = temp;

				vertices[i].vel = XMVectorZero();

			}

		}


		if (!Deformable)
		{

			velocity_prev = velocity;

			float k = 1;


			if (isApplyGravity)
			{
				Fg.m128_f32[1] = -m*g + (m*g / KarmanLine)*fPosition.m128_f32[1];
			}


			velocity = (Force_total / m)*dt;

			Position += velocity*dt;


			float dV = XMVectorGetByIndex(XMVector3Length((velocity - velocity_prev)), 0);



			Force *= dV;

			Force_total = Force + Fg;

			//Force_total = (Force + Fg)*Kv;


			/*
			if (inCollision)
			{

			Force = XMVectorZero();
			velocity = XMVectorZero();
			}
			*/

			if (fPosition.m128_f32[1] < MetalCore) fPosition.m128_f32[1] = MetalCore;




		}

		inCollision = false;


	}



	void VisualGeometryObject::Render(DirectX::CXMMATRIX WVP, D3D11_FILL_MODE FillMode, BOOL isFrontCW,
		D3D11_CULL_MODE CullingMode, D3D11_PRIMITIVE_TOPOLOGY Topology, StateAfterRender isChanged)
	{



		//----------Обновление вершинного буфера

		if (1 == changed.m128_f32[0] || inCollision)
		{

			RebuildBasis();


			VertexPosNormVelocityTex* v = 0;
			//VertexPosNormVelocityTex* v= new VertexPosNormVelocityTex[vertices.size()];
			XMVECTOR p;

			D3D11_MAPPED_SUBRESOURCE bufferMapping;


			d3dDeviceContext->Map(VB, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferMapping);

			v = (VertexPosNormVelocityTex*)bufferMapping.pData;


			XMMATRIX m = XMMatrixMultiply(XMLoadFloat4x4(&ScaleMatrix), XMLoadFloat4x4(&ObjectSpace));


			for (DWORD i = 0; i < NumVertices; i++)
			{

				//vertices[i].pos = XMVector3Transform(vertices[i].pos, ObjectSpace);
				//p= vertices[i].pos;
				//p = XMVector3Transform(p, ScaleMatrix*ObjectSpace);

				vertices[i].pos = XMVector3Transform(vertices[i].pos, m);
				vertices[i].pos.m128_f32[3] = 1.0f;

				vertices[i].normal.m128_f32[3] = 0.0f;


				v[i] = vertices[i];

			}

			d3dDeviceContext->Unmap(VB, 0);

			//delete[] v;


			CalculateVertexNormals();

			changed.m128_f32[0] = isChanged;

		} //if (1 == changed.m128_f32[0] || inCollision)


		//
		//----------------------------------------Отрисовка сцены--------------------------------------
		//

		UINT stride = sizeof(VertexPosNormVelocityTex);
		UINT offset = 0;


		if (et_ShaderSystem == effects_type)
		{

			for (USHORT i = 0; i < FX.size(); i++)
			{

				d3dDeviceContext->OMSetDepthStencilState(0, 0);

				float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
				d3dDeviceContext->OMSetBlendState(0, blendFactors, 0xffffffff);

				d3dDeviceContext->IASetInputLayout(VertexLayout);

				d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


				FX[i].set();

				//------------------Установка параметров растеризации

				D3D11_RASTERIZER_DESC rstatedesc;
				ZeroMemory(&rstatedesc, sizeof(D3D11_RASTERIZER_DESC));

				rstatedesc.FillMode = FillMode;
				rstatedesc.FrontCounterClockwise = !isFrontCW;
				rstatedesc.CullMode = CullingMode;

				d3dDevice->CreateRasterizerState(&rstatedesc, &RState);

				d3dDeviceContext->RSSetState(RState);


				//------------------- Отрисовка -- DrawIndexed()


				d3dDeviceContext->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
				d3dDeviceContext->IASetIndexBuffer(IB, DXGI_FORMAT_R32_UINT, 0);


				FX[i].begin(); // отработают все begin-функции


				d3dDeviceContext->DrawIndexed(NumFaces * 3, 0, 0);


				FX[i].end(); // отработают все end-функции


			} // for	


		}
		else
		{

			d3dDeviceContext->OMSetDepthStencilState(0, 0);

			float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			d3dDeviceContext->OMSetBlendState(0, blendFactors, 0xffffffff);

			d3dDeviceContext->IASetInputLayout(VertexLayout);

			d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


			//------Установка параметров растеризации

			D3D11_RASTERIZER_DESC rstatedesc;
			ZeroMemory(&rstatedesc, sizeof(D3D11_RASTERIZER_DESC));

			rstatedesc.FillMode = FillMode;
			rstatedesc.FrontCounterClockwise = !isFrontCW;
			rstatedesc.CullMode = CullingMode;

			d3dDevice->CreateRasterizerState(&rstatedesc, &RState);

			d3dDeviceContext->RSSetState(RState);


			//------------Установка КБ и шейдерных переменных

			//Set matrices and other values
			FX11.fxLWVP->SetMatrix((float*)&WVP);
			FX11.fxWV->SetMatrix((float*)&WV);
			FX11.fxTexMtx->SetMatrix((float*)&TextureTransformationMatrix);

			//Set texture RVs
			FX11.fxDiffuseMapVar->SetResource(FX11.pDiffuseMap);
			FX11.fxOverlayMapVar->SetResource(FX11.pOverlayMap);
			FX11.fxSpecularMapVar->SetResource(FX11.pSpecularMap);



			//------------------- Отрисовка -- DrawIndexed()


			D3DX11_TECHNIQUE_DESC d3dtechdesc;

			FX11.Technique->GetDesc(&d3dtechdesc);

			for (UINT i = 0; i < d3dtechdesc.Passes; ++i)
			{

				FX11.Technique->GetPassByIndex(i)->Apply(0, d3dDeviceContext);

				d3dDeviceContext->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
				d3dDeviceContext->IASetIndexBuffer(IB, DXGI_FORMAT_R32_UINT, 0);

				d3dDeviceContext->DrawIndexed(NumFaces * 3, 0, 0);

			}

		}// else	 





	}


	void VisualGeometryObject::RenderInGroup()
	{
		static uint idx = 0;
		idx++;




		//----------Обновление вершинного буфера

		if (1 == changed.m128_f32[0] || inCollision)
		{

			RebuildBasis();


			VertexPosNormVelocityTex* v = 0;
			//VertexPosNormVelocityTex* v= new VertexPosNormVelocityTex[vertices.size()];
			XMVECTOR p;

			D3D11_MAPPED_SUBRESOURCE bufferMapping;


			d3dDeviceContext->Map(VB, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferMapping);

			v = (VertexPosNormVelocityTex*)bufferMapping.pData;

			

			XMMATRIX m = XMMatrixMultiply(XMLoadFloat4x4(&ScaleMatrix), XMLoadFloat4x4(&ObjectSpace));			
				


				for (DWORD i = 0; i < NumVertices; i++)
				{
					vertices[i].pos = XMVector3Transform(vertices[i].pos, m);
					vertices[i].pos.m128_f32[3] = 1.0f;

					v[i] = vertices[i];					
				}

			

			d3dDeviceContext->Unmap(VB, 0);

			//delete[] v;


			//CalculateVertexNormals();

			changed.m128_f32[0] = isChangedAfterRender;

		} //if (1 == changed.m128_f32[0] || inCollision)

		CalculateVertexNormals();

		//
		//----------------------------------------Отрисовка сцены--------------------------------------
		//

		UINT stride = sizeof(VertexPosNormVelocityTex);
		UINT offset = 0;


		if (et_ShaderSystem == effects_type)
		{



			d3dDeviceContext->OMSetDepthStencilState(0, 0);

			float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			d3dDeviceContext->OMSetBlendState(0, blendFactors, 0xffffffff);

			d3dDeviceContext->IASetInputLayout(VertexLayout);

			d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


			//------------------ Установка параметров растеризации

			D3D11_RASTERIZER_DESC rstatedesc;
			ZeroMemory(&rstatedesc, sizeof(D3D11_RASTERIZER_DESC));

			/*
			rstatedesc.FillMode = rasterisation_state.FillMode;
			rstatedesc.FrontCounterClockwise = rasterisation_state.FrontCounterClockwise;
			rstatedesc.CullMode = rasterisation_state.CullMode;
			*/

			rstatedesc.FillMode = rasterisation_state.FillMode;
			rstatedesc.FrontCounterClockwise = TRUE;
			rstatedesc.CullMode = D3D11_CULL_NONE;


			d3dDevice->CreateRasterizerState(&rstatedesc, &RState);

			d3dDeviceContext->RSSetState(RState);

			//-------------------- Установка текстур


			DiffuseTexture.apply();



			//------------------- Отрисовка -- DrawIndexed()


			d3dDeviceContext->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
			d3dDeviceContext->IASetIndexBuffer(IB, DXGI_FORMAT_R32_UINT, 0);


			d3dDeviceContext->DrawIndexed(NumFaces * 3, 0, 0);


		}
		else
		{

			d3dDeviceContext->OMSetDepthStencilState(0, 0);

			float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			d3dDeviceContext->OMSetBlendState(0, blendFactors, 0xffffffff);

			d3dDeviceContext->IASetInputLayout(VertexLayout);

			d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


			//------Установка параметров растеризации

			D3D11_RASTERIZER_DESC rstatedesc;
			ZeroMemory(&rstatedesc, sizeof(D3D11_RASTERIZER_DESC));

			rstatedesc.FillMode = rasterisation_state.FillMode;
			rstatedesc.FrontCounterClockwise = rasterisation_state.FrontCounterClockwise;
			rstatedesc.CullMode = rasterisation_state.CullMode;

			d3dDevice->CreateRasterizerState(&rstatedesc, &RState);

			d3dDeviceContext->RSSetState(RState);


			//------------Установка КБ и шейдерных переменных

			//Set matrices and other values
			FX11.fxLWVP->SetMatrix((float*)&WVP);
			FX11.fxWV->SetMatrix((float*)&WV);
			FX11.fxTexMtx->SetMatrix((float*)&TextureTransformationMatrix);

			//Set texture RVs
			FX11.fxDiffuseMapVar->SetResource(FX11.pDiffuseMap);
			FX11.fxOverlayMapVar->SetResource(FX11.pOverlayMap);
			FX11.fxSpecularMapVar->SetResource(FX11.pSpecularMap);



			//------------------- Отрисовка -- DrawIndexed()


			D3DX11_TECHNIQUE_DESC d3dtechdesc;

			FX11.Technique->GetDesc(&d3dtechdesc);

			for (UINT i = 0; i < d3dtechdesc.Passes; ++i)
			{

				FX11.Technique->GetPassByIndex(i)->Apply(0, d3dDeviceContext);

				d3dDeviceContext->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
				d3dDeviceContext->IASetIndexBuffer(IB, DXGI_FORMAT_R32_UINT, 0);

				d3dDeviceContext->DrawIndexed(NumFaces * 3, 0, 0);

			}

		}// else


	}



	Sphere::Sphere(std::string name)
	{
		this->name = name;
	}


	void Sphere::ConstructGeometry(const float r, const float h, const DWORD vqty, const DWORD cqty)
	{



		if ((this->cqty != cqty) || (this->vqty != vqty) || (this->r != r) || (this->h != h))
		{

			vertices.clear();
			ReleaseCOM(VB); ReleaseCOM(IB);

		}
		else return;


		NumVertices = (cqty + 1)*(vqty + 1);

		vertices = vector<VertexPosNormVelocityTex, a16VPNVT>(NumVertices);
		//vertices= vector<VertexPosNormVelocityTex> (NumVertices);


		DWORD i, j;
		float a, b, x, y, z;

		this->vqty = vqty;
		this->cqty = cqty;
		this->r = r;
		this->h = h;


		float du = 1.0f / (vqty + 1);
		float dv = 1.0f / (cqty + 1);
		float u, v;


		for (i = 0; i < cqty + 1; i++)
		{


			for (j = 0; j < vqty + 1; j++)
			{


				a = Pi / cqty*i;
				b = (float)2 * Pi / vqty*j;

				x = r*sin(a)*cos(b);
				y = h / 2 * cos(a);
				z = r*sin(a)*sin(b);


				vertices[i*(vqty + 1) + j].pos.m128_f32[0] = x;
				vertices[i*(vqty + 1) + j].pos.m128_f32[1] = y;
				vertices[i*(vqty + 1) + j].pos.m128_f32[2] = z;


				vertices[i*(vqty + 1) + j].normal = XMVector3Normalize(XMVectorSet(x, y, z, 0));


				// Создаём текстурные координаты для сферы

				u = du*j;
				v = dv*i;


				vertices[i*(vqty + 1) + j].texC.m128_f32[0] = u;
				vertices[i*(vqty + 1) + j].texC.m128_f32[1] = v;


			}
		}




		VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);


		vertices_old = vertices;



		//Расчёт индексов

		NumFaces = (((vqty + 1) - 1)*((cqty + 1) - 1)) * 2;

		indices = vector<DWORD>(NumFaces * 3);

		DWORD k = 0;

		// Boundaries of the cycles do not come to the total vertices count by 1 point
		// in order to get around the last quad's left upper vertex


		DWORD index1, index2, index3, index4, index5, index6;


		for (i = 0; i < cqty; i++)
		for (j = 0; j < vqty; j++)
		{

			index1 = i*(vqty + 1) + j;
			index2 = i*(vqty + 1) + j + 1;
			index3 = (i + 1)*(vqty + 1) + j + 1;
			index4 = (i + 1)*(vqty + 1) + j + 1;
			index5 = (i + 1)*(vqty + 1) + j;
			index6 = i*(vqty + 1) + j;


			indices[k] = index1;
			indices[k + 1] = index2;
			indices[k + 2] = index3;
			indices[k + 3] = index4;
			indices[k + 4] = index5;
			indices[k + 5] = index6;

			k += 6;

		}



		IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);



	}

	void Sphere::CalculateVertexNormals()
	{

		//VisualGeometryObject::CalculateVertexNormals();

		/*
		XMVECTOR n;

		for (uint i = 0; i < NumVertices; i++)
		{

			n = XMVector3Normalize(vertices[i].pos - PositionW);
			
			vertices[i].normal = n;
			//vertices[i].normal = XMVector3Normalize((vertices[i].pos - PositionW));

		}
		*/


	}


	Cube::Cube(std::string name)
	{
		this->name = name;
	}


	void Cube::ConstructGeometry(float scale)
	{

		//VisualGeometryObject::ConstructGeometry();

		if (scale != this->scale)
		{
			this->scale = scale;

			ReleaseCOM(VB); ReleaseCOM(IB);
		}

		NumVertices = 24;
		NumFaces = 12;


		vertices.resize(24);

		// Fill in the front face vertex data.
		vertices[0] = VertexPosNormVelocityTex(-1, -1, -1, 0, 0, -1, 0, 0, 0, 0, 1);
		vertices[1] = VertexPosNormVelocityTex(-1, 1, -1, 0, 0, -1, 0, 0, 0, 0, 0);
		vertices[2] = VertexPosNormVelocityTex(1, 1, -1, 0, 0, -1, 0, 0, 0, 1, 0);
		vertices[3] = VertexPosNormVelocityTex(1, -1, -1, 0, 0, -1, 0, 0, 0, 1, 1);


		// Fill in the back face VertexPosNormVelocityTex data.
		vertices[4] = VertexPosNormVelocityTex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0, 0, 0, 1.0f, 1.0f);
		vertices[5] = VertexPosNormVelocityTex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0, 0, 0, 0.0f, 1.0f);
		vertices[6] = VertexPosNormVelocityTex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0, 0, 0, 0.0f, 0.0f);
		vertices[7] = VertexPosNormVelocityTex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0, 0, 0, 1.0f, 0.0f);

		// Fill in the top face VertexPosNormVelocityTex data.
		vertices[8] = VertexPosNormVelocityTex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0, 0, 0, 0.0f, 1.0f);
		vertices[9] = VertexPosNormVelocityTex(-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0, 0, 0.0f, 0.0f);
		vertices[10] = VertexPosNormVelocityTex(1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0, 0, 1.0f, 0.0f);
		vertices[11] = VertexPosNormVelocityTex(1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0, 0, 0, 1.0f, 1.0f);

		// Fill in the bottom face VertexPosNormVelocityTex data.
		vertices[12] = VertexPosNormVelocityTex(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0, 0, 0, 1.0f, 1.0f);
		vertices[13] = VertexPosNormVelocityTex(1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0, 0, 0, 0.0f, 1.0f);
		vertices[14] = VertexPosNormVelocityTex(1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0, 0, 0, 0.0f, 0.0f);
		vertices[15] = VertexPosNormVelocityTex(-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0, 0, 0, 1.0f, 0.0f);

		// Fill in the left face VertexPosNormVelocityTex data.
		vertices[16] = VertexPosNormVelocityTex(-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f, 1.0f);
		vertices[17] = VertexPosNormVelocityTex(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f, 0.0f);
		vertices[18] = VertexPosNormVelocityTex(-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0, 0, 0, 1.0f, 0.0f);
		vertices[19] = VertexPosNormVelocityTex(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0, 0, 0, 1.0f, 1.0f);

		// Fill in the right face VertexPosNormVelocityTex data.
		vertices[20] = VertexPosNormVelocityTex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f, 1.0f);
		vertices[21] = VertexPosNormVelocityTex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f, 0.0f);
		vertices[22] = VertexPosNormVelocityTex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0, 0, 0, 1.0f, 0.0f);
		vertices[23] = VertexPosNormVelocityTex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0, 0, 0, 1.0f, 1.0f);


		for (int i = 0; i<NumVertices; i++)
			vertices[i].pos *= scale;


		VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);

		//DWORD indices[36];
		indices.resize(NumFaces * 3);

		// Fill in the front face index data
		indices[0] = 0; indices[1] = 1; indices[2] = 2;
		indices[3] = 0; indices[4] = 2; indices[5] = 3;

		// Fill in the back face index data
		indices[6] = 4; indices[7] = 5; indices[8] = 6;
		indices[9] = 4; indices[10] = 6; indices[11] = 7;

		// Fill in the top face index data
		indices[12] = 8; indices[13] = 9; indices[14] = 10;
		indices[15] = 8; indices[16] = 10; indices[17] = 11;

		// Fill in the bottom face index data
		indices[18] = 12; indices[19] = 13; indices[20] = 14;
		indices[21] = 12; indices[22] = 14; indices[23] = 15;

		// Fill in the left face index data
		indices[24] = 16; indices[25] = 17; indices[26] = 18;
		indices[27] = 16; indices[28] = 18; indices[29] = 19;

		// Fill in the right face index data
		indices[30] = 20; indices[31] = 21; indices[32] = 22;
		indices[33] = 20; indices[34] = 22; indices[35] = 23;



		IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);

	}


	#pragma warning( disable : 4996);

	//71


	Mesh::Mesh(std::string name)
	{
		this->name = name;
	}


	bool Mesh::LoadMesh(const char *FileName)
	{

		unsigned short chunk_id; // двух-байтовый заголовок
		unsigned int chunk_length_info; // информация о длине чанка (4 байта)
		char name_char; // сюда считывается очередной символ из имени объекта (блок 4000 - Object block)
		unsigned int i, j, subobjectindex = -1;
		unsigned short scounter = 0;

		Object_Name = "Root";


		srand(time(NULL));



		FILE *i3DSFile;

		//fopen_s(&i3DSFile, FileName, "r");
		if ((i3DSFile = fopen(FileName, "rb")) == NULL) return false;

		//if(NULL == i3DSFile) return false;




		while (ftell(i3DSFile) < _filelength(_fileno(i3DSFile)))
		{
			// Каждая запись в 3DS файле занимает 6 байт,
			// поэтому будем считывать по 6 байт
			fread(&chunk_id, 2, 1, i3DSFile);
			fread(&chunk_length_info, 4, 1, i3DSFile);



			switch (chunk_id)
			{

			case 0x4d4d: // Main chunk
				break;

			case 0x3d3d: // 3D Editor block
				break;

			case 0x4000: // Object block


				subobjectindex++;

				SubObjects.reserve(SubObjects.size() + 1);
				SubObjects.resize(SubObjects.size() + 1);


				SubObjects[subobjectindex].InitFromExistingData(d3dDevice, d3dDeviceContext, FX11.fx, FX11.Technique);
				SubObjects[subobjectindex].Object_Name = "";


				do
				{
					fread(&name_char, 1, 1, i3DSFile);

					if (name_char != '\0') SubObjects[subobjectindex].Object_Name += name_char;

				} while (name_char != '\0');


				printf(SubObjects[subobjectindex].Object_Name.c_str());
				printf("\n\n");


				break;

			case 0x4100: // Triangular mesh
				break;


			case 0x4110:  // Vertices list

				fread(&SubObjects[subobjectindex].NumVertices, 2, 1, i3DSFile);//sizeof(unsigned short)); //считываем два байта

				SubObjects[subobjectindex].vertices.resize(SubObjects[subobjectindex].NumVertices);


				for (i = 0; i < SubObjects[subobjectindex].NumVertices; i++)
				{

					//Position
					fread(&SubObjects[subobjectindex].vertices[i].pos.m128_f32[0], sizeof(float), 1, i3DSFile);
					fread(&SubObjects[subobjectindex].vertices[i].pos.m128_f32[2], sizeof(float), 1, i3DSFile);
					fread(&SubObjects[subobjectindex].vertices[i].pos.m128_f32[1], sizeof(float), 1, i3DSFile);


					//Normal
					SubObjects[subobjectindex].vertices[i].normal.m128_f32[0] = 0;
					SubObjects[subobjectindex].vertices[i].normal.m128_f32[1] = 0;
					SubObjects[subobjectindex].vertices[i].normal.m128_f32[2] = 0;

					//texC
					SubObjects[subobjectindex].vertices[i].texC.m128_f32[0] = (float)(rand() % 1001) / 1000;
					SubObjects[subobjectindex].vertices[i].texC.m128_f32[1] = (float)(rand() % 1001) / 1000;

				}

				break;


			case 0x4120:  // Faces description

				fread(&SubObjects[subobjectindex].NumFaces, 2, 1, i3DSFile);

				SubObjects[subobjectindex].indices.resize(SubObjects[subobjectindex].NumFaces * 3);


				for (i = 0; i < SubObjects[subobjectindex].NumFaces; i++)
				{

					for (j = 0; j < 3; j++)
					{

						fread(&SubObjects[subobjectindex].indices[i * 3 + j], sizeof(unsigned short), 1, i3DSFile);

					}

					fseek(i3DSFile, sizeof(unsigned short), SEEK_CUR); // Пропускаем face flag

				} // for i



				NumSubObjects++;
				SubObjects[subobjectindex].VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);
				SubObjects[subobjectindex].IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);


				break;


			default:


				fseek(i3DSFile, chunk_length_info - 6, SEEK_CUR); // Смещаем указатель позиции потока файла на n+m (в начало следующего чанка)

				break;



			}//switch

		}//while



		fclose(i3DSFile);



		if (NumSubObjects > 0)
		{

			DWORD i, j, N = 0;

			for (i = 0; i < NumSubObjects; i++)
			{

				NumVertices += SubObjects[i].NumVertices;
				NumFaces += SubObjects[i].NumFaces;

			}


			vertices.resize(NumVertices);
			indices.resize(NumFaces * 3);

			for (i = 0; i < NumSubObjects; i++)
			{


				for (j = 0; j < SubObjects[i].NumVertices; j++)

					vertices[N + j] = SubObjects[i].vertices[j];


				N += SubObjects[i].NumVertices;

			}


			N = 0;


			for (i = 0; i < NumSubObjects; i++)
			{


				for (j = 0; j < SubObjects[i].NumFaces * 3; j++)

					indices[N + j] = SubObjects[i].indices[j];


				N += SubObjects[i].NumFaces * 3;

			}


			CalculateVertexNormals();

			VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);
			IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);



		}




		return true;

	}


	void Mesh::VertexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags)
	{

		if (NumVertices > 0)

			VisualGeometryObject::VertexBufferCreate(Usage, BindFlags, CPUAccessFlags);


	}


	void Mesh::IndexBufferCreate(D3D11_USAGE Usage, UINT BindFlags, UINT CPUAccessFlags)
	{


		if (NumVertices > 0)

			VisualGeometryObject::IndexBufferCreate(Usage, BindFlags, CPUAccessFlags);


	}





	Grid::Grid(std::string name)
	{
		this->name = name;
	}


	void Grid::ConstructGeometry(DWORD m, DWORD n, float dx)
	{

		float NumRows = m;
		float NumCols = n;

		NumVertices = m*n;
		NumFaces = (m - 1)*(n - 1) * 2;

		vertices.resize(NumVertices);

		float halfWidth = (n - 1)*dx*0.5f;
		float halfDepth = (m - 1)*dx*0.5f;

		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);


		for (DWORD i = 0; i < m; ++i)
		{
			float z = halfDepth - i*dx;
			for (DWORD j = 0; j < n; ++j)
			{
				float x = -halfWidth + j*dx;

				float y = 0;

				vertices[i*n + j].pos = XMVectorSet(x, y, z, 0);

				vertices[i*n + j].texC.m128_f32[0] = j*du;
				vertices[i*n + j].texC.m128_f32[1] = i*dv;


				XMVECTOR normal;

			}
		}


		VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);



		indices.resize(NumFaces * 3);


		int k = 0;

		for (DWORD i = 0; i < m - 1; ++i)
		{
			for (DWORD j = 0; j < n - 1; ++j)
			{
				indices[k] = i*n + j;
				indices[k + 1] = i*n + j + 1;
				indices[k + 2] = (i + 1)*n + j;

				indices[k + 3] = (i + 1)*n + j;
				indices[k + 4] = i*n + j + 1;
				indices[k + 5] = (i + 1)*n + j + 1;

				k += 6; // next quad
			}
		}


		IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);


		CalculateVertexNormals();


	}

	/*
	void Cloth::CalculateNormals()
	{


	XMVECTOR v1v0, v2v0, normal;
	DWORD i0, i1, i2;


	for(DWORD i = 0; i < NumFaces ; i++ )
	{


	i0= indices[i*3];		//
	i1= indices[i*3+1];   // для каждого треугольника
	i2= indices[i*3+2];   //



	v1v0= vertices[i1].pos-vertices[i0].pos;
	v2v0= vertices[i2].pos-vertices[i0].pos;


	XMVector3Cross(&normal, &v1v0, &v2v0);

	XMVector3Normalize(&normal, &normal);


	vertices[i0].normal= normal;
	vertices[i1].normal= normal;
	vertices[i2].normal= normal;


	}


	}

	*/


	Cloth::Cloth(std::string name)
	{
		this->name = name;
	}

	void Cloth::ConstructGeometry(DWORD m, DWORD n, float dx)
	{

		NumRows = m;
		NumCols = n;

		NumVertices = m*n;
		NumFaces = (m - 1)*(n - 1) * 2;

		vertices.resize(NumVertices);

		float halfWidth = (n - 1)*dx*0.5f;
		float halfDepth = (m - 1)*dx*0.5f;

		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);


		for (DWORD i = 0; i < m; ++i)
		{
			float z = halfDepth - i*dx;
			for (DWORD j = 0; j < n; ++j)
			{
				float x = -halfWidth + j*dx;

				float y = 0;

				vertices[i*n + j].pos = XMVectorSet(x, y, z, 0);

				vertices[i*n + j].vel = XMVectorZero();

				vertices[i*n + j].texC.m128_f32[0] = j*du;
				vertices[i*n + j].texC.m128_f32[1] = i*dv;

			}
		}




		VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);

		//BuildVertexBuffer_render();



		indices.resize(NumFaces * 3);

		int k = 0;
		for (DWORD i = 0; i < m - 1; ++i)
		{
			for (DWORD j = 0; j < n - 1; ++j)
			{
				indices[k] = i*n + j;
				indices[k + 1] = i*n + j + 1;
				indices[k + 2] = (i + 1)*n + j;

				indices[k + 3] = (i + 1)*n + j;
				indices[k + 4] = i*n + j + 1;
				indices[k + 5] = (i + 1)*n + j + 1;

				k += 6; // next quad
			}
		}


		IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);



		// Построить нормали
		CalculateVertexNormals();


		vertices_old = vertices;



		//-----------------------------------------------------//

		BuildIndices_vpatterns(m, n, dx); // индексы для передачи паттернов



	}


	void Cloth::BuildIndices_vpatterns(DWORD& m, DWORD& n, float& d)
	{

		//indices_vpatterns.reserve(NumVertices*1e1);

		vector<DWORD> maxIndex;

		maxIndex.resize(m);


		for (DWORD i = 0; i < m; i++)

			maxIndex[i] = i*n + n - 1;



		//float r[3] = {d, d*sqrt(2), 2*d}; // массив радиусов для построения паттернов
		float r[3] = { 1, sqrt(2), 2 }; // массив радиусов для построения паттернов




		struct round_functor
		{

			double operator()(double a)
			{

				double c = ceil(a);

				return (c - a) < 1e-1 ? c : a;

			}


		} round;


		//DWORD d_qty= round(r[1]/sqrt(2)/d);
		DWORD d_qty = round(r[1] / sqrt(2) / 1);


		XMVECTOR origin = XMVectorSet(-1, -1, 0, 0); // индексы точки, которая будет служить центром поиска точек паттерна
		double a = Pi / 4, alpha;

		// сформировать массив индексов точек, образующих паттерны

		double i = 0, j = 0;

		for (DWORD k = 0; k < m; k++) // пройти по всем строкам сетки
		{

			origin.m128_f32[1] = k;

			for (DWORD l = 0; l < n; l++) // пройти по всем столбцам сетки
			{

				origin.m128_f32[0] = l;

				DWORD index_origin = origin.m128_f32[1] * n + origin.m128_f32[0];

				for (DWORD v = 0; v < 3; v++) // перебрать 3 радиуса, при помощи которых будут строиться паттерны точек
				{

					if (m - 1 == k && 1 == v) continue;

					for (DWORD w = 0; w < 8; w++) // пройти по 8 направлениям, используя 3 радиуса
					{



						alpha = a*w;

						if ((v != 1) && ((w % 2) != 0)) continue;

						if ((1 == v) && ((w % 2) == 0)) continue;

						if ((1 == v) && ((d_qty > (maxIndex[k] - index_origin)) && (Pi / 4 == alpha))) continue;

						//---------- Для радиусов r[0..2] == d, d*sqrt(2), 2d :

						//jj= floor(r[v]*cos(a*w)+origin.m128_f32[0]); // x
						//ii= ceil(r[v]*sin(a*w)+origin.m128_f32[1]); // y

						j = round(r[v] * cos(alpha) + origin.m128_f32[0]); // x

						i = round(r[v] * sin(alpha) + origin.m128_f32[1]); // y


						DWORD j_dw = j, i_dw = i;

						if ((j < 0) || (i < 0) || (j - j_dw > 1e-3) || (i - i_dw > 1e-3)) continue; // если индексы меньше нуля или дробные


						// получаем одномерный индекс для добавления его в индексный буфер паттернов:

						double index_inferred = round(i*n + j);

						if ((0 == alpha || Pi == alpha) && index_inferred > maxIndex[k]) continue;

						if ((Pi / 2 == alpha || 3 * Pi / 2 == alpha) && index_inferred > maxIndex[m - 1]) continue;

						//if( (Pi/4 == alpha || 3*Pi/4 == alpha || 5*Pi/4 == alpha || 7*Pi/4 == alpha) && index_inferred > maxIndex[m-1] ) continue;


						DWORD index_inferred_dw = index_inferred;

						if ((index_inferred - index_inferred_dw) < 1e-1)
						{

							// если полученный индекс находится в диапазоне индексов сетки и больше предыдущего (по отношению к себе самому), то
							// добавляем индекс центра поиска и полученный индекс в массив индексов паттернов
							if (index_inferred_dw > index_origin)
							{

								indices_vpatterns.push_back(index_origin);
								indices_vpatterns.push_back(index_inferred_dw);

								radii.push_back(r[v]);

								NumPatternIndices += 2;

							}


						}


					}// w


				}// v


			}// l

		}//k





	}



	void Cloth::SimulateClothDynamics(const float& dt, FXMVECTOR Force)
	{


		if (Kd > 1 || Kd < 0) Kd = 1e-1;


		std::set<DWORD> corner_pts_indcs;

		corner_pts_indcs.insert(0);
		corner_pts_indcs.insert(NumCols - 1);
		corner_pts_indcs.insert(NumRows*NumCols - NumCols);
		corner_pts_indcs.insert(NumRows*NumCols - 1);

		struct functor
		{

			bool operator()(std::set<DWORD>& s, DWORD& elem)
			{

				if (s.find(elem) == s.end()) return false;

				return true;

			}


		} InSet;



		float pq_length;

		UINT k = 0;

		XMVECTOR dir, velocity, moveToEquillibriumVector, moveToEquillibriumVector_half;


		VertexPosNormVelocityTex vi, vi2;

		DWORD i, i2;


		//
		// --- Satisfy constraints:
		//


		//for(DWORD u= 0; u < 2; u++) // релаксация
		//{

		for (DWORD t = 0; t < NumPatternIndices - 1;)
		{


			i = indices_vpatterns[t];
			i2 = indices_vpatterns[t + 1];


			dir = vertices[i2].pos - vertices[i].pos;
			pq_length = XMVectorGetByIndex(XMVector3Length(dir), 0);  // current distance
			moveToEquillibriumVector = dir*(1 - radii[k] / pq_length);
			moveToEquillibriumVector_half = moveToEquillibriumVector*0.5;

			k++; // индекс для выбора радиуса


			// Вернуть P[i] в состояние с минимальной энергией

			if (!InSet(corner_pts_indcs, i))
			{

				vertices[i].pos += moveToEquillibriumVector_half;

			}


			// Вернуть Q[i] в состояние с минимальной энергией

			if (!InSet(corner_pts_indcs, i2))
			{

				vertices[i2].pos -= moveToEquillibriumVector_half;

			}

			t += 2;

		}

		//k= 0;

		//}



		//
		// --- Adding new force and changing position
		//



		float g = 1;


		XMVECTOR Fg = XMVectorSet(0, -m*g, 0, 0);       // gravity force
		//XMVECTOR Fg (0, -m*g*1e1, 0);       // gravity force

		XMVECTOR temp;

		for (i = 0; i < NumVertices - 1; i++)
		{

			vertices[i].vel += 0 * Fg / m*dt + Force / m;


			if (!InSet(corner_pts_indcs, i))
			{

				temp = vertices[i].pos;

				vertices[i].pos = vertices[i].pos + (vertices[i].pos - vertices_old[i].pos)*(1 - Kd) + vertices[i].vel*dt;

				vertices_old[i].pos = temp;

				vertices[i].vel = XMVectorZero();

			}


		}


	}


	float Landscape::getHeight(float x, float z)const
	{
		return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
	}



	void Landscape::ConstructGeometry(ID3D11Device* device, DWORD m, DWORD n, float dx)
	{

		float NumRows = m;
		float NumCols = n;

		NumVertices = m*n;
		NumFaces = (m - 1)*(n - 1) * 2;

		vertices.resize(NumVertices);

		float halfWidth = (n - 1)*dx*0.5f;
		float halfDepth = (m - 1)*dx*0.5f;

		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);


		for (DWORD i = 0; i < m; ++i)
		{
			float z = halfDepth - i*dx;
			for (DWORD j = 0; j < n; ++j)
			{
				float x = -halfWidth + j*dx;

				float y = getHeight(x, z);

				vertices[i*n + j].pos = XMVectorSet(x, y, z, 0);

				vertices[i*n + j].texC.m128_f32[0] = j*du;
				vertices[i*n + j].texC.m128_f32[1] = i*dv;


				XMVECTOR normal;

				normal.m128_f32[0] = -0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z);
				normal.m128_f32[1] = 1.0f;
				normal.m128_f32[2] = -0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z);
				vertices[i*n + j].normal = XMVector3Normalize(normal);

			}
		}


		VertexBufferCreate(D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);


		vertices_old = vertices;



		indices.resize(NumFaces * 3);

		int k = 0;
		for (DWORD i = 0; i < m - 1; ++i)
		{
			for (DWORD j = 0; j < n - 1; ++j)
			{
				indices[k] = i*n + j;
				indices[k + 1] = i*n + j + 1;
				indices[k + 2] = (i + 1)*n + j;

				indices[k + 3] = (i + 1)*n + j;
				indices[k + 4] = i*n + j + 1;
				indices[k + 5] = (i + 1)*n + j + 1;

				k += 6; // next quad
			}
		}


		IndexBufferCreate(D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);


	}




} // namespace geometry_objects







