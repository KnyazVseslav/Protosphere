
#pragma once

#ifndef SHADER_SYSTEM_H
#define SHADER_SYSTEM_H

#include "D3D.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Variadic Delegates.h"
#include "map_vector.h"

#include <map>



#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <d3d11.h>




namespace effects
{

namespace shader_system
{



//typedef unsigned int uint;


class Shader;


/*
template<class Key, class Val>
class map_vector: public std::vector<Val>
{
public:

	Val& operator[](const Key& idx);
	Val& operator[](const uint& idx);

	void SetKeys(std::vector<Key>* keys); // сохранить указатель на вектор ключей, чтобы избежать дублирования данных
	//void SetKeys(std::vector<Key>::iterator keys);


private:

	uint index_of(const std::vector<Key>& key_vector, const Key& search_val);

	std::vector<Key>* keys; // храним указатель на вектор ключей, чтобы избежать дублирования данных

};
*/

class ShaderSystem;




class Effects
{
public:

	Effects();


	void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext);

	void add(ShaderSystem effect);

	void add(const std::string& name); // используется когда у всех эффектов одни и те же d3dDevice и d3dDeviceContext
	void add(const std::string& name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext); // если d3dDevice и d3dDeviceContext
																											 // у каждого эффекта свои

	void add(const std::string& name, const ShaderSystem& effect); // если нужно добавить готовый эффект
	void add(const std::string& name, const ShaderSystem& effect,
		               ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext); // если нужно добавить готовый эффект с новыми
																						// d3dDevice и d3dDeviceContext

	uint size(); // количество эффектов в коллекции

	

private:

  map_vector<std::string, ShaderSystem> effects_collection;	

	
public:

	ShaderSystem& operator[](const std::string& key);
	ShaderSystem& operator[](const uint& idx);

	auto begin()->decltype(effects_collection.begin())
	{
		return effects_collection.begin();
	}

	auto end()->decltype(effects_collection.end())
	{
		return effects_collection.end();
	}


private:

	std::vector<std::string> names;




	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;


};



enum ShaderTypes{VertexShader, HullShader, DomainShader, GeometryShader, PixelShader, ComputeShader};




class ShaderSystem
{

public:

	ShaderSystem(std::string name);
	
	ShaderSystem();	
	//{
	//	shaders.SetKeys(&shader_names); // map_vector<>::SetKeys() - передаём указатель на вектор имён шейдеров, вместо копирования;
										// далее контейнер map_vector будет использовать имена шейдеров для доступа к шейдерам по ключу
	//}

	ShaderSystem(std::string name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext);
	

	void Init(std::string name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext); // использовать этот метод, когда объект будет применять эффекты, поставляемые ShaderSystem
	

	void AddShader(std::string shader_name,				   
				   const ShaderTypes& ShaderType,
				   char* szFileName, 
				   LPCSTR szEntryPoint, 
				   LPCSTR szShaderModel);

	void set();



	void begin();
	void end();


	void AddBeginFunction(delegates::Delegate f);
	void AddEndFunction(delegates::Delegate f);

	template<class... Args>
	void AddBeginFunction(Args&&... args);

	template<class... Args>
	void AddEndFunction(Args&&... args);

	template<class... Args, class...PtrArgsToBind>
	void AddBeginFunction(Args&&... args, PtrArgsToBind... ptr_args);

	template<class... Args, class...PtrArgsToBind>
	void AddEndFunction(Args&&... args, PtrArgsToBind... ptr_args);

	/*
	template<class... Args>
	void ShaderSystem::AddBeginFunction(Args&&... args)
	{

	}

	template<class... Args>
	void ShaderSystem::AddEndFunction(Args&&... args)
	{

	}
	*/

	delegates::DelegatesSystem begin_functions;
	delegates::DelegatesSystem end_functions;


	typedef map_vector<std::string, Shader> ShadersCollection;
	
	ShadersCollection shaders; // коллекция шейдеров


	  ID3DBlob* get_VS_byte_code();

	  void set_device(ID3D11Device* d3dDevice);
	  void set_device_context(ID3D11DeviceContext* d3dDeviceContext);


public:

	std::string get_name();

private:

	std::string name;

	std::vector<std::string> shader_names;	
	
	ID3DBlob* VS_byte_code;	

	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;	

};

template<class... Args>
void ShaderSystem::AddBeginFunction(Args&&... args)
{
	begin_functions.add(std::forward<Args>(args)...);
}

template<class... Args>
void ShaderSystem::AddEndFunction(Args&&... args)
{
	end_functions.add(std::forward<Args>(args)...);
}

template<class... Args, class...PtrArgsToBind>
void ShaderSystem::AddBeginFunction(Args&&... args, PtrArgsToBind... ptr_args)
{
	begin_functions.add(std::forward<Args>(args)..., ptr_args...);
}

template<class... Args, class...PtrArgsToBind>
void ShaderSystem::AddEndFunction(Args&&... args, PtrArgsToBind... ptr_args)
{
	end_functions.add(std::forward<Args>(args)..., ptr_args...);
}


class IConstantBuffer;

template<class T> class ConstantBuffer;





	class ISetter
	{
	public:

		virtual void set_shader() = 0;

		virtual void set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers) = 0;

		virtual void set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV) = 0;

	protected:

		ID3D11DeviceContext* d3dDeviceContext;

	};


	class VertexShaderSetter: public ISetter
	{
	public:
		
		VertexShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11VertexShader* VS);

		void set_shader();

		void set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers);

		void set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV);
	
	private:

		ID3D11VertexShader* VS;

	};


	class HullShaderSetter: public ISetter
	{
	public:

		HullShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11HullShader* HS);
	
		void set_shader();

		void set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers);

		void set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV);
	
	private:

		ID3D11HullShader* HS;

	};


	class DomainShaderSetter: public ISetter
	{
	public:

		DomainShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11DomainShader* DS);
		
		void set_shader();

		void set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers);

		void set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV);
	
	private:

		ID3D11DomainShader* DS;

	};


	class GeometryShaderSetter: public ISetter
	{
	public:

		GeometryShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11GeometryShader* GS);

		void set_shader();
		
		void set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers);

		void set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV);
		
	private:

		ID3D11GeometryShader* GS;

	};
	

	class PixelShaderSetter: public ISetter
	{
	public:

		PixelShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11PixelShader* PS);

		void set_shader();
		
		void set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers);

		void set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV);
		
	private:

		ID3D11PixelShader* PS;

	};


	class ComputeShaderSetter: public ISetter
	{
	public:
		
		ComputeShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11ComputeShader* CS);

		void set_shader();
		
		void set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers);

		void set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV);
		
	
	private:

		ID3D11ComputeShader* CS;

	};



class Texture2D;


class Shader
{

public:

	Shader() = default;

	Shader(std::string shader_name,
		   ID3D11Device* d3dDevice,	
		   ID3D11DeviceContext* d3dDeviceContext, 
		   const ShaderTypes& ShaderType,
		   WCHAR* szFileName, 
		   LPCSTR szEntryPoint, 
		   LPCSTR szShaderModel);


	Shader(const Shader& rhs);

	
	std::string& get_name();

	template<class T> void AddConstantBuffer(const std::string& buffer_name,
										     const uint& StartSlot,
											 const T* content = NULL,
										     const uint& NumBuffers = 1,
										     D3D11_USAGE Usage = D3D11_USAGE_DYNAMIC, 
										     UINT CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, 
										     UINT MiscFlags = 0);

	
	void AddTexture2D(char* file_name, const uint& StartSlot, const uint& NumViews = 1);
	void AddTexture2D(ID3D11ShaderResourceView* SRV, const uint& StartSlot, const uint& NumViews = 1);

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel);

	void create(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel); // происходит компиляция шейдера и выбор установщика на основе 
																			   // типа шейдера
	//void create(

	void set();

	void set_Texture2D(ID3D11ShaderResourceView* const * SRV, const uint& StartSlot, const uint& NumViews);



	typedef map_vector<std::string, IConstantBuffer*> ConstantBuffersCollection; // интерфейс IConstantBuffer нужен для того, чтобы можно было
																		    // создать коллекцию КБ, не указывая параметр шаблона ConstantBuffer<T>,
																			// т.к. мы не знаем заранее какой будет тип содержимого КБ
	
		 ConstantBuffersCollection constant_buffers; // коллекция КБ


	typedef map_vector<std::string, Texture2D> Textures2DCollection;

	   Textures2DCollection textures2D;




	   ID3D11RasterizerState* rasterizer_state;


	/*
	ID3D11DeviceContext** get_device_context()
	{
		return &d3dDeviceContext;
	}
	*/

	ID3D11DeviceContext* get_device_context();

	ID3DBlob* get_byte_code();

	ID3D11VertexShader* GetVS();

	ID3D11HullShader* GetHS();

	ID3D11DomainShader* GetDS();

	ID3D11GeometryShader* GetGS();

	ID3D11PixelShader* GetPS();

	ID3D11ComputeShader* GetCS();
	
//protected:
private:

	std::string name;

	std::vector<std::string> constant_buffers_names;

	std::vector<std::string> textures2D_names;


	ShaderTypes ShaderType;

	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;


	ID3DBlob* pShaderCompilationBlob; // будет содержать шейдерный байт-код после компиляции шейдера


	// каким шейдером может стать:
	ID3D11VertexShader* vs;
	ID3D11HullShader* hs;
	ID3D11DomainShader* ds;
	ID3D11GeometryShader* gs;
	ID3D11PixelShader* ps;	
	ID3D11ComputeShader* cs;	


	ISetter* setter; // поле для хранения полиморфного объекта, тип которого будет определён во время выполнения метода create(...)	

public:

	ISetter* get_setter() { return setter; }


};


template<class T>
void Shader::AddConstantBuffer(const std::string& buffer_name,
	const uint& StartSlot,
	const T* content,
	const uint& NumBuffers,
	D3D11_USAGE Usage,
	UINT CPUAccessFlags,
	UINT MiscFlags)
{
	constant_buffers.push_back(new ConstantBuffer<T>(buffer_name, StartSlot, content, NumBuffers, d3dDevice, d3dDeviceContext,
		setter, Usage, CPUAccessFlags, MiscFlags)); // создаётся полиморфный объект
	// IConstantBuffer* == ConstantBuffer<T> и добавляется в коллекцию КБ
	// map_vector<std::string, IConstantBuffer*> constant_buffers

	constant_buffers_names.push_back(buffer_name);
	//ConstantBuffers.insert(make_pair("name", ConstantBuffer<T>));
}


// Класс, описывающий спецификацию
// 
class Specification
{
public:

	Specification() = default;

	Specification(std::vector<std::string> _FX_names, std::vector<std::string> _shader_names, std::vector<uint> _StartSlots, 
						std::vector<uint> _NumViews = { 1 });
	

public:


	std::vector<std::string> FX_names;        // на i-ое имя эффекта

	std::vector<std::string> shader_names;   // приходится коллекция имён шейдеров
	
	std::vector<uint> StartSlots;			 // на каждое имя шейдера приходится одно значение из коллекции StartSlots
	std::vector<uint> NumViews;				 // а также - одно значение из коллекции NumViews

};



class Texture2D
{

public:

	Texture2D() : initialized(false) {}

	Texture2D(char* file_name);

	Texture2D(char* file_name, const uint& StartSlot, const uint& NumViews, ID3D11Device* d3dDevice, ISetter* setter);

	Texture2D(ID3D11ShaderResourceView* SRV, const uint& StartSlot, const uint& NumViews, ID3D11Device* d3dDevice, ISetter* setter);

	Texture2D(char* file_name, const std::vector<Specification>& specifications);
	Texture2D(ID3D11ShaderResourceView* SRV, const std::vector<Specification>& specifications);

	
	void init(char* file_name, Specification const & specification);
	void assign();


	Texture2D(ID3D11ShaderResourceView* SRV, Specification const & specification);

	void set();

	void apply(); // для пакетного запуска делегатов



public:

	void set_device(ID3D11Device* d3dDevice);

	void set_FX_of_Group(effects::shader_system::Effects* FX_of_Group);

	Specification get_specification() const;
	char* get_file_name() const;

	bool initialized;


private:

ID3D11ShaderResourceView* SRV;

effects::shader_system::Effects* FX_of_Group; // эффекты группы выставляются при добавлении объекта в группу

std::vector<Specification> specifications;

delegates::DelegatesSystem _delegates; // контейнер для методов void Shader::set_Texture2D(SRV, StartSlot, NumViews)


char* file_name;

Specification specification;

uint StartSlot;
uint NumViews;


ID3D11Device* d3dDevice;

ISetter* setter;

std::vector<ISetter*> setters;



};


/*
class Textures2DSystem
{
public:

	void add(char* file_name, Specification const & specification);
	void add(ID3D11ShaderResourceView* SRV, Specification const & specification);



private:

	map_vector<std::string, Texture2D> textures;

};
*/

class IConstantBuffer
{

public:

	virtual std::string get_name() const = 0;
	virtual void set() = 0;
	virtual void* operator=(void* cb) = 0;
	virtual ID3D11Buffer* GetD3DBuffer() const = 0;
};


template<class T>
class ConstantBuffer: public IConstantBuffer
{

public:

	ConstantBuffer(const std::string& name, 
				   const uint& StartSlot, 
				   const T* content,
				   const uint& NumBuffers, 
				   ID3D11Device* d3dDevice,
				   ID3D11DeviceContext* d3dDeviceContext, 
				   ISetter* setter,  
				   D3D11_USAGE Usage = D3D11_USAGE_DYNAMIC, 
				   UINT CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, 
				   UINT MiscFlags = 0);



	virtual std::string get_name() const; // перегрузка метода IConstantBuffer
	



	virtual void* operator=(void* cb); // перегрузка метода IConstantBuffer
	


	void create(D3D11_USAGE Usage = D3D11_USAGE_DYNAMIC, UINT CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, UINT MiscFlags = 0);	


	void update();


	void set();  // перегрузка метода IConstantBuffer
	


	T& get_content();

	void set_content(const T& rhs);


	    __declspec(property(get = get_content, put = set_content)) T Content;



	ID3D11Buffer* GetD3DBuffer() const; // перегрузка метода IConstantBuffer
	
	
private:

	std::string name;

	typedef typename T type;

	T content;

	bool isSet;

	ISetter* setter;

	uint StartSlot; // индекс регистра cbuffer в HLSL (b#, cb#)
	uint NumBuffers;

	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;

	ID3D11Buffer* d3dConstantBuffer;

};



template<class T> T& access(IConstantBuffer* buf);



/*
*
*	IMPLEMENTATION
*
*/



///
/// ------------- ConstantBuffer -----------------///
///



template<class T>
ConstantBuffer<T>::ConstantBuffer(const std::string& name,
	const uint& StartSlot,
	const T* content,
	const uint& NumBuffers,
	ID3D11Device* d3dDevice,
	ID3D11DeviceContext* d3dDeviceContext,
	ISetter* setter,
	D3D11_USAGE Usage = D3D11_USAGE_DYNAMIC,
	UINT CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	UINT MiscFlags = 0)
{
	this->name = name;
	this->setter = setter;
	this->StartSlot = StartSlot;

	if (content)
	{
		this->content = *content; // на случай необходимости инициализировать КБ
		isSet = true;
	}
	else
	{
		isSet = false;
	}

	this->NumBuffers = NumBuffers;
	this->d3dDevice = d3dDevice;
	this->d3dDeviceContext = d3dDeviceContext;

	if (this->d3dDevice)

		create(Usage, CPUAccessFlags, MiscFlags);
}

template<class T>
std::string ConstantBuffer<T>::get_name() const // перегрузка метода IConstantBuffer
{
	return name;
}



template<class T>
void* ConstantBuffer<T>::operator=(void* cb) // перегрузка метода IConstantBuffer
{
	content = *static_cast<T*>(cb);
	return static_cast<void*>(&content);
}

template<class T>
void ConstantBuffer<T>::create(D3D11_USAGE Usage = D3D11_USAGE_DYNAMIC, UINT CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, UINT MiscFlags = 0)
{
	D3D11_BUFFER_DESC bdesc;

	bdesc.Usage = Usage;
	bdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdesc.CPUAccessFlags = CPUAccessFlags;
	bdesc.MiscFlags = MiscFlags;
	bdesc.ByteWidth = sizeof(T);

	if (isSet)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &content;

		d3dDevice->CreateBuffer(&bdesc, &InitData, &d3dConstantBuffer);
	}

	else

		d3dDevice->CreateBuffer(&bdesc, NULL, &d3dConstantBuffer);
}


template<class T>
void ConstantBuffer<T>::update()
{
	D3D11_MAPPED_SUBRESOURCE bufferMapping;

	d3dDeviceContext->Map(d3dConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferMapping);

	T* joinning_ptr = (T*)bufferMapping.pData;

	*joinning_ptr = content; // сопоставить данные content с буфером d3dConstantBuffer; дальнейшее сопоставление с шейдером будет задано, тем какой шейдер будет установлен перед установкой данного КБ

	d3dDeviceContext->Unmap(d3dConstantBuffer, 0);
}


template<class T>
void ConstantBuffer<T>::set()  // перегрузка метода IConstantBuffer
{
	update();

	setter->set_buffer(d3dConstantBuffer, StartSlot, NumBuffers); // StartSlot - индекс регистра cbuffer в HLSL (b#, cb#)
}


template<class T>
T& ConstantBuffer<T>::get_content()
{
	return content;
}

template<class T>
void ConstantBuffer<T>::set_content(const T& rhs)
{
	content = rhs;
}


template<class T>
ID3D11Buffer* ConstantBuffer<T>::GetD3DBuffer() const // перегрузка метода IConstantBuffer
{
	return d3dConstantBuffer;
}


template<class T> T& access(IConstantBuffer* buf)
{
	return static_cast<ConstantBuffer<T>*>(buf)->Content;
}





} // namespace shader_system


class Effect11
{

public:

	Effect11() : d3dDevice(0),
		d3dDeviceContext(0),
		fx_file_name(0),
		technique_name(0),
		fx(0),
		Technique(0),
		pDiffuseMap(0),
		pSpecularMap(0),
		pOverlayMap(0),
		fxLWVP(0),
		fxDiffuseMapVar(0),
		fxSpecularMapVar(0),
		fxOverlayMapVar(0) {};

	void Create(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, char* fx_file_name, char* technique_name)
	{
		this->d3dDevice = d3dDevice;
		this->d3dDeviceContext = d3dDeviceContext;


		DiffuseTextureFileName = "";
		SpecularTextureFileName = "";
		OverlayTextureFileName = "";

		Create_FX_and_Tech(fx_file_name, technique_name, d3dDevice, d3dDeviceContext);
	}

	void Create_FX_and_Tech(char* fx_file_name, char* technique_name, ID3D11Device* device, ID3D11DeviceContext* DeviceContext);

	void ConnectToShaderVariables();



	void TextureDiffuseApply(char* filename);

	void TextureDiffuseApply(ID3D11ShaderResourceView* SRV);

	void TextureSpecularApply(char* filename);

	void TextureOverlayApply(char* filename);



	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechnique() const;

	void SetEffect(ID3DX11Effect* fx);
	void SetTechnique(ID3DX11EffectTechnique* Technique);

	void set_device(ID3D11Device* d3dDevice);
	void set_device_context(ID3D11DeviceContext* d3dDeviceContext);


	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;


	// Связь с эффектом и техникой
	ID3DX11Effect* fx;
	ID3DX11EffectTechnique* Technique;


	//Связь с переменными КБ
	ID3DX11EffectMatrixVariable* fxLWVP;
	ID3DX11EffectMatrixVariable* fxWV;
	ID3DX11EffectMatrixVariable* fxView;
	ID3DX11EffectMatrixVariable* fxTexMtx;
	ID3DX11EffectMatrixVariable* fxTranslationMtx;


	ID3DX11EffectShaderResourceVariable* fxDiffuseMapVar;
	ID3DX11EffectShaderResourceVariable* fxSpecularMapVar;
	ID3DX11EffectShaderResourceVariable* fxOverlayMapVar;


	//Текстуры
	char* DiffuseTextureFileName;
	char* SpecularTextureFileName;
	char* OverlayTextureFileName;

	ID3D11ShaderResourceView* pDiffuseMap;
	ID3D11ShaderResourceView* pSpecularMap;
	ID3D11ShaderResourceView* pOverlayMap;



protected:



	char* fx_file_name;
	char* technique_name;




};




} // namespace effects


#endif




