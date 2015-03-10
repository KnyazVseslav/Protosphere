#include "ShaderSystem.h"

#include "D3D.h"




using namespace std;
using namespace DirectX;
using namespace delegates;



namespace effects
{

    namespace shader_system
    {

        ///
        /// ------------- FX -----------------///
        ///

        Effects::Effects()
        {
            effects_collection.SetKeys(&names);
        }

        ShaderSystem& Effects::operator[](const std::string& key)
        {
            return effects_collection[key];
        }

        ShaderSystem& Effects::operator[](const uint& idx)
        {
            return effects_collection[idx];
        }

        void Effects::Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext) // использовать этот метод, когда объект будет применять эффекты, поставляемые ShaderSystem
        {
            this->d3dDevice = d3dDevice;
            this->d3dDeviceContext = d3dDeviceContext;
        }



        void Effects::add(ShaderSystem effect)
        {
            names.push_back(effect.get_name());

            effects_collection.push_back(effect);
        }

        void Effects::add(const std::string& name)
        {
            names.push_back(name);
            effects_collection.push_back(ShaderSystem(name, d3dDevice, d3dDeviceContext));
        }

        void Effects::add(const std::string& name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext)
        {
            names.push_back(name);
            effects_collection.push_back(ShaderSystem(name, d3dDevice, d3dDeviceContext));
        }

        void Effects::add(const std::string& name, const ShaderSystem& effect)
        {
            names.push_back(name);
            effects_collection.push_back(effect);
        }

        void Effects::add(const std::string& name, const ShaderSystem& effect, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext)
        {
            ShaderSystem fx_buf(name, d3dDevice, d3dDeviceContext);
            fx_buf.shaders = effect.shaders;

            effects_collection.push_back(fx_buf);
        }

        uint Effects::size()
        {
            return effects_collection.size();
        }





        ///
        /// ------------- ShaderSystem -----------------///
        ///


        ShaderSystem::ShaderSystem(std::string name) : VS_byte_code(0)
        {
            this->name = name;

            shaders.SetKeys(&shader_names); // map_vector<>::SetKeys() - передаём указатель на вектор имён шейдеров, вместо копирования;
            // далее контейнер map_vector будет использовать имена шейдеров для доступа к шейдерам по ключу
        }



        ShaderSystem::ShaderSystem() : VS_byte_code(0)
        {
            name = "";

            shaders.SetKeys(&shader_names); // map_vector<>::SetKeys() - передаём указатель на вектор имён шейдеров, вместо копирования;
            // далее контейнер map_vector будет использовать имена шейдеров для доступа к шейдерам по ключу
        }





        ShaderSystem::ShaderSystem(std::string name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext)
        {
            Init(name, d3dDevice, d3dDeviceContext);
        }


        void ShaderSystem::Init(std::string name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext) // использовать этот метод, когда объект будет применять эффекты, поставляемые ShaderSystem
        {
            this->name = name;

            this->d3dDevice = d3dDevice;
            this->d3dDeviceContext = d3dDeviceContext;
        }

        void ShaderSystem::AddShader(std::string shader_name, const ShaderTypes& ShaderType, char* szFileName,
            LPCSTR szEntryPoint, LPCSTR szShaderModel)
        {

            shader_names.push_back(shader_name);



            WCHAR* pwchar_str = 0;

            int chars = MultiByteToWideChar(CP_ACP, 0, szFileName, -1, NULL, 0);

            pwchar_str = new WCHAR[chars];

            MultiByteToWideChar(CP_ACP, 0, szFileName, -1, pwchar_str, chars);




            //shaders.push_back(Shader(shader_name, d3dDevice, d3dDeviceContext, ShaderType, pwchar_str, szEntryPoint, szShaderModel));
            shaders.emplace_back(shader_name, d3dDevice, d3dDeviceContext, ShaderType, pwchar_str, szEntryPoint, szShaderModel);

            //shaders.push_back(Shader(shader_name, d3dDevice, d3dDeviceContext, ShaderType, pchar_to_pwchar(szFileName), szEntryPoint, szShaderModel));

            //shaders.insert(make_pair(shader_name, Shader(shader_name)));


            VS_byte_code = (!VS_byte_code) && (VertexShader == ShaderType) ? shaders[shaders.size() - 1].get_byte_code() : VS_byte_code; // вернуть шейдерный 
            //байт-код, если шейдер 
            // является вершинным

        }



        void ShaderSystem::set()
        {
            for (uint i = 0; i < shaders.size(); i++)
                shaders[i].set();
        }


        ID3DBlob* ShaderSystem::get_VS_byte_code()
        {
            return VS_byte_code;
        }


        void ShaderSystem::begin()
        {
            begin_functions.launch();
        }

        void ShaderSystem::end()
        {
            end_functions.launch();
        }


        void ShaderSystem::AddBeginFunction(delegates::Delegate f)
        {
            begin_functions.add(f);
        }

        void ShaderSystem::AddEndFunction(delegates::Delegate f)
        {
            end_functions.add(f);
        }


        void ShaderSystem::set_device(ID3D11Device* d3dDevice)
        {
            this->d3dDevice = d3dDevice;
        }

        void ShaderSystem::set_device_context(ID3D11DeviceContext* d3dDeviceContext)
        {
            this->d3dDeviceContext = d3dDeviceContext;
        }


        std::string ShaderSystem::get_name()
        {
            return name;
        }


        ///
        /// ------------- Shader -----------------///
        ///


        Shader::Shader(std::string shader_name, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
            const ShaderTypes& ShaderType, WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel)
            : name(shader_name), pShaderCompilationBlob(0), rasterizer_state(0)
        {
            constant_buffers.SetKeys(&constant_buffers_names); // связываем адрес поля Shader::constant_buffers_names с map_vector::keys
            textures2D.SetKeys(&textures2D_names);


            this->d3dDevice = d3dDevice;
            this->d3dDeviceContext = d3dDeviceContext;
            this->ShaderType = ShaderType;

            create(szFileName, szEntryPoint, szShaderModel);

        };


        Shader::Shader(const Shader& rhs)
        {

            if (this != &rhs)
            {
                *this = rhs;
                constant_buffers.SetKeys(&constant_buffers_names);
                textures2D.SetKeys(&textures2D_names);
            }

            //cout<<"Shader::copy constructor()"<<"\сn\n";

        }



        


        void Shader::AddTexture2D(char* file_name, const uint& StartSlot, const uint& NumViews)
        {

            textures2D.push_back(Texture2D(file_name, StartSlot, NumViews, d3dDevice, setter));

        }

        void Shader::AddTexture2D(ID3D11ShaderResourceView* SRV, const uint& StartSlot, const uint& NumViews)
        {

            textures2D.push_back(Texture2D(SRV, StartSlot, NumViews, d3dDevice, setter));

        }


        HRESULT Shader::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel)
        {
            HRESULT hr = S_OK;

            if (pShaderCompilationBlob)
            {
                pShaderCompilationBlob->Release();
                pShaderCompilationBlob = 0;
            }

            DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
        #if defined( DEBUG ) || defined( _DEBUG )
            // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
            // Setting this flag improves the shader debugging experience, but still allows 
            // the shaders to be optimized and to run exactly the way they will run in 
            // the release configuration of this program.
            dwShaderFlags |= D3DCOMPILE_DEBUG;
        #endif

            ID3DBlob* pErrorBlob = 0;
            hr = D3DCompileFromFile(szFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
                dwShaderFlags, 0, &pShaderCompilationBlob, &pErrorBlob);

            if (FAILED(hr))
            {
                if (pErrorBlob != NULL)
                    OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
                if (pErrorBlob) pErrorBlob->Release();
                return hr;
            }
            if (pErrorBlob) pErrorBlob->Release();


            return S_OK;
        }


        void Shader::create(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel)
        {
            if (SUCCEEDED(CompileShaderFromFile(szFileName, szEntryPoint, szShaderModel)))


                // выбор установщика на основе типа шейдера;
                // поскольку тип шейдера определяет и тип констнантного буфера (devCon->XXSetConstantBuffers()),
                // то имеет смысл единожды произвести данное сопоставление и сохранить его;
                // именно поэтому интерфейс ISetter задаёт метод для установки шейдера и метод для установки КБ;
                // далее при создании КБ в качестве одного из параметров будет передан установщик уже выбранного
                // типа XXXShaderSetter, содержащий соответствующий оператор утсановки КБ (devCon->XXSetConstantBuffers()),
                // который будет выполнен во время вызова метода IConstantBuffer<T>::set()
                switch (this->ShaderType)
            {

                case VertexShader: d3dDevice->CreateVertexShader(pShaderCompilationBlob->GetBufferPointer(),
                    pShaderCompilationBlob->GetBufferSize(), NULL, &vs);

                    setter = new VertexShaderSetter(get_device_context(), vs);

                    break;

                case HullShader: d3dDevice->CreateHullShader(pShaderCompilationBlob->GetBufferPointer(),
                    pShaderCompilationBlob->GetBufferSize(), NULL, &hs);

                    setter = new HullShaderSetter(get_device_context(), hs);

                    break;

                case DomainShader: d3dDevice->CreateDomainShader(pShaderCompilationBlob->GetBufferPointer(),
                    pShaderCompilationBlob->GetBufferSize(), NULL, &ds);

                    setter = new DomainShaderSetter(get_device_context(), ds);

                    break;

                case GeometryShader: d3dDevice->CreateGeometryShader(pShaderCompilationBlob->GetBufferPointer(),
                    pShaderCompilationBlob->GetBufferSize(), NULL, &gs);

                    setter = new GeometryShaderSetter(get_device_context(), gs);

                    break;

                case PixelShader: d3dDevice->CreatePixelShader(pShaderCompilationBlob->GetBufferPointer(),
                    pShaderCompilationBlob->GetBufferSize(), NULL, &ps);

                    setter = new PixelShaderSetter(get_device_context(), ps);

                    break;

                case ComputeShader: d3dDevice->CreateComputeShader(pShaderCompilationBlob->GetBufferPointer(),
                    pShaderCompilationBlob->GetBufferSize(), NULL, &cs);

                    setter = new ComputeShaderSetter(get_device_context(), cs);

                    break;

            };

        }

        void Shader::set()
        {
            setter->set_shader(); // сперва устанавливается шейдер, т.к. это задаёт соответсвие для КБ, т.е. с каким шейдером будет связан КБ

            if (rasterizer_state)
            {
                d3dDeviceContext->RSSetState(rasterizer_state);
            }

            for (uint i = 0; i < constant_buffers.size(); i++)
                constant_buffers[i]->set();

            for (uint i = 0; i < textures2D.size(); i++)
                textures2D[i].set();

        }


        void Shader::set_Texture2D(ID3D11ShaderResourceView* const * SRV, const uint& StartSlot, const uint& NumViews)
        {
            setter->set_texture(StartSlot, NumViews, SRV);
        }



        std::string& Shader::get_name()
        {
            return name;
        }


        ID3D11DeviceContext* Shader::get_device_context()
        {
            return d3dDeviceContext;
        }

        ID3DBlob* Shader::get_byte_code()
        {
            return pShaderCompilationBlob;
        }

        ID3D11VertexShader* Shader::GetVS()
        {
            return vs;
        }

        ID3D11HullShader* Shader::GetHS()
        {
            return hs;
        }

        ID3D11DomainShader* Shader::GetDS()
        {
            return ds;
        }

        ID3D11GeometryShader* Shader::GetGS()
        {
            return gs;
        }

        ID3D11PixelShader* Shader::GetPS()
        {
            return ps;
        }

        ID3D11ComputeShader* Shader::GetCS()
        {
            return cs;
        }






        ///
        /// ------------- Texture2D ---------------///
        ///


        Specification::Specification(

            std::vector<std::string> _FX_names, 
            std::vector<std::string> _shader_names, 
            std::vector<uint> _StartSlots, 
            std::vector<uint> _NumViews
            
            ) 

            : FX_names( _FX_names ), shader_names( _shader_names ), StartSlots( _StartSlots ), NumViews( _NumViews )

        {

        }

        Texture2D::Texture2D(char* file_name)
        {
            
        }



        Texture2D::Texture2D(char* file_name, const uint& StartSlot, const uint& NumViews, ID3D11Device* d3dDevice, ISetter* setter)
        {
            this->StartSlot = StartSlot;
            this->NumViews = NumViews;
            this->d3dDevice = d3dDevice;

            this->setter = setter;

            // Преобразование из char* в WCHAR*

            const WCHAR* fn = 0;

            //fn = string_facilities::pchar_to_pwchar(file_name);


            WCHAR* pwchar_str = 0;

            int chars = MultiByteToWideChar(CP_ACP, 0, file_name, -1, NULL, 0);

            pwchar_str = new WCHAR[chars];

            MultiByteToWideChar(CP_ACP, 0, file_name, -1, pwchar_str, chars);


            fn = pwchar_str;


            HR(CreateDDSTextureFromFile(d3dDevice, fn, NULL, &SRV));

            //D3DX11CreateShaderResourceViewFromFile(d3dDevice, fn , 0, 0, &pDiffuseMap, 0); // D3DX  utility library is deprecated for Windows 8


        }

    


        Texture2D::Texture2D(ID3D11ShaderResourceView* SRV, const uint& StartSlot, const uint& NumViews, ID3D11Device* d3dDevice, ISetter* setter)
        {
            this->StartSlot = StartSlot;
            this->NumViews = NumViews;
            this->d3dDevice = d3dDevice;

            this->setter = setter;

            this->SRV = SRV;
        }


        Texture2D::Texture2D(char* file_name, const std::vector<Specification>& specifications)
        {

            // Преобразование из char* в WCHAR*

            const WCHAR* fn = 0;

            //fn = string_facilities::pchar_to_pwchar(file_name);


            WCHAR* pwchar_str = 0;

            int chars = MultiByteToWideChar(CP_ACP, 0, file_name, -1, NULL, 0);

            pwchar_str = new WCHAR[chars];

            MultiByteToWideChar(CP_ACP, 0, file_name, -1, pwchar_str, chars);


            fn = pwchar_str;


            HR(CreateDDSTextureFromFile(d3dDevice, fn, NULL, &SRV));

            for (const auto& s : specifications)			 // берётся i-ая спецификация
              for (const auto& effect_name : s.FX_names)     // у неё берётся i-ое имя эффекта
                 for (const auto&  fx_shn : s.shader_names)  // у i-го сопоставителя (детали) берётся i-ое имя шейдера 
                  {
                       uint i = 0;

                       // для итого имени шейдера берём соответствующие по индексу значения в коллекциях StartSlots и NumViews
                       _delegates.add(Delegate::Make(&((*FX_of_Group)[effect_name].shaders[fx_shn]), &Shader::set_Texture2D, &SRV,
                                                                &s.StartSlots[i], &s.NumViews[i]) );

                            i++;
                  }

                    

        }


        Texture2D::Texture2D(ID3D11ShaderResourceView* SRV, const std::vector<Specification>& specifications)
        {
            this->SRV = SRV;

            for (const auto& s : specifications)			 // берётся i-ая спецификация
             for (const auto& effect_name : s.FX_names)     // у неё берётся i-ое имя эффекта
              for (const auto& fx_shn : s.shader_names)  // у i-го сопоставителя (детали) берётся i-ое имя шейдера 
              {
                    uint i = 0;

                    // для итого имени шейдера берём соответствующие по индексу значения в коллекциях StartSlots и NumViews
                    _delegates.add(Delegate::Make(&((*FX_of_Group)[effect_name].shaders[fx_shn]), &Shader::set_Texture2D, &SRV,
                        &s.StartSlots[i], &s.NumViews[i]));

                    i++;
              }

        }




        void Texture2D::init(char* file_name, Specification const & specification)
        {
            this->d3dDevice = d3dDevice;

            this->file_name = file_name;
            this->specification = specification;


            initialized = true;

        }



        void Texture2D::assign()
        {
            
            // Преобразование из char* в WCHAR*

            const WCHAR* fn = 0;

            //fn = string_facilities::pchar_to_pwchar(file_name);


            WCHAR* pwchar_str = 0;

            int chars = MultiByteToWideChar(CP_ACP, 0, file_name, -1, NULL, 0);

            pwchar_str = new WCHAR[chars];

            MultiByteToWideChar(CP_ACP, 0, file_name, -1, pwchar_str, chars);


            fn = pwchar_str;


            HR(CreateDDSTextureFromFile(d3dDevice, fn, NULL, &SRV));
            //HR(CreateDDSTextureFromFile(d3dDevice, fn, nullptr, &SRV));



            const auto& s = specification;

            for (const auto& effect_name : s.FX_names)     // у спецификации берётся i-ое имя эффекта
              for (const auto& fx_shn : s.shader_names)  // у i-го сопоставителя (детали) берётся i-ое имя шейдера 
              {
                    uint i = 0;

                    // для i-го имени шейдера берём соответствующие по индексу значения в коллекциях StartSlots и NumViews
                    /*_delegates.add(Delegate::Make(&((*FX_of_Group)[effect_name].shaders[fx_shn]), &Shader::set_Texture2D, &SRV,
                        &s.StartSlots[i], &s.NumViews[i]));
                        */
                    
                    /*
                    auto* ptr_shader = &((*FX_of_Group)[effect_name].shaders[fx_shn]);

                    _delegates.add(Delegate::Make(ptr_shader, &Shader::set_Texture2D, &SRV,
                        &s.StartSlots[i], &s.NumViews[i]));
                        */

                    setters.push_back( (*FX_of_Group)[effect_name].shaders[fx_shn].get_setter() );

                    i++;
              }

        }


        


        Texture2D::Texture2D(ID3D11ShaderResourceView* SRV, Specification const & specification)
        {
            this->SRV = SRV;


            const auto& s = specification;

            for (const auto& effect_name : s.FX_names)     // у спецификации берётся i-ое имя эффекта
            for (const auto& fx_shn : s.shader_names)  // у i-го сопоставителя (детали) берётся i-ое имя шейдера 
            {
                uint i = 0;

                // для итого имени шейдера берём соответствующие по индексу значения в коллекциях StartSlots и NumViews
                _delegates.add(Delegate::Make(&((*FX_of_Group)[effect_name].shaders[fx_shn]), &Shader::set_Texture2D, &SRV,
                    &s.StartSlots[i], &s.NumViews[i]));

                i++;
            }

        }



        void Texture2D::set()
        {
            setter->set_texture(StartSlot, NumViews, &SRV); // привязать текстуру к шейдеру, задаваемому установщиком setter
        }


        void Texture2D::apply()
        {
            //_delegates.launch();

            for (uint i = 0; i < setters.size(); i++)
                setters[i]->set_texture(specification.StartSlots[i], specification.NumViews[i], &SRV);
        }



        void Texture2D::set_device(ID3D11Device* d3dDevice)
        {
            this->d3dDevice = d3dDevice;
        }


        void Texture2D::set_FX_of_Group(effects::shader_system::Effects* FX_of_Group)
        {
            this->FX_of_Group = FX_of_Group;
        }


        Specification Texture2D::get_specification() const
        {
            return specification;
        }
        
        char* Texture2D::get_file_name() const
        {
            return file_name;
        }



        ///
        /// ------------- Setters -----------------///
        ///


        /// VS


        VertexShaderSetter::VertexShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11VertexShader* VS)
        {
            this->d3dDeviceContext = d3dDeviceContext; this->VS = VS;
        }

        void VertexShaderSetter::set_shader()
        {
            d3dDeviceContext->VSSetShader(VS, NULL, 0);
        }

        void VertexShaderSetter::set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers)
        {
            d3dDeviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, &d3dBuf);
        }


        void VertexShaderSetter::set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV)
        {
            d3dDeviceContext->VSSetShaderResources(StartSlot, NumViews, SRV);
        }

        /// HS

        HullShaderSetter::HullShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11HullShader* HS)
        {
            this->d3dDeviceContext = d3dDeviceContext;
            this->HS = HS;
        }

        void HullShaderSetter::set_shader()
        {
            d3dDeviceContext->HSSetShader(HS, NULL, 0);
        }

        void HullShaderSetter::set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers)
        {
            d3dDeviceContext->HSSetConstantBuffers(StartSlot, NumBuffers, &d3dBuf);
        }


        void HullShaderSetter::set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV)
        {
            d3dDeviceContext->HSSetShaderResources(StartSlot, NumViews, SRV);
        }


        /// DS

        DomainShaderSetter::DomainShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11DomainShader* DS)
        {
            this->d3dDeviceContext = d3dDeviceContext; this->DS = DS;
        }

        void DomainShaderSetter::set_shader()
        {
            d3dDeviceContext->DSSetShader(DS, NULL, 0);
        }

        void DomainShaderSetter::set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers)
        {
            d3dDeviceContext->DSSetConstantBuffers(StartSlot, NumBuffers, &d3dBuf);
        }


        void DomainShaderSetter::set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV)
        {
            d3dDeviceContext->DSSetShaderResources(StartSlot, NumViews, SRV);
        }



        /// GS


        GeometryShaderSetter::GeometryShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11GeometryShader* GS)
        {
            this->d3dDeviceContext = d3dDeviceContext; this->GS = GS;
        }

        void GeometryShaderSetter::set_shader()
        {
            d3dDeviceContext->GSSetShader(GS, NULL, 0);
        }

        void GeometryShaderSetter::set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers)
        {
            d3dDeviceContext->GSSetConstantBuffers(StartSlot, NumBuffers, &d3dBuf);
        }


        void GeometryShaderSetter::set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV)
        {
            d3dDeviceContext->GSSetShaderResources(StartSlot, NumViews, SRV);
        }



        /// PS

        PixelShaderSetter::PixelShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11PixelShader* PS)
        {
            this->d3dDeviceContext = d3dDeviceContext; this->PS = PS;
        }

        void PixelShaderSetter::set_shader()
        {
            d3dDeviceContext->PSSetShader(PS, NULL, 0);
        }

        void PixelShaderSetter::set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers)
        {
            d3dDeviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, &d3dBuf);
        }


        void PixelShaderSetter::set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV)
        {			
                d3dDeviceContext->PSSetShaderResources(StartSlot, NumViews, SRV);			
        }

        /// CS

        ComputeShaderSetter::ComputeShaderSetter(ID3D11DeviceContext* d3dDeviceContext, ID3D11ComputeShader* CS)
        {
            this->d3dDeviceContext = d3dDeviceContext; this->CS = CS;
        }

        void ComputeShaderSetter::set_shader()
        {
            d3dDeviceContext->CSSetShader(CS, NULL, 0);
        }

        void ComputeShaderSetter::set_buffer(ID3D11Buffer* d3dBuf, const uint& StartSlot, const uint& NumBuffers)
        {
            d3dDeviceContext->CSSetConstantBuffers(StartSlot, NumBuffers, &d3dBuf);
        }


        void ComputeShaderSetter::set_texture(uint StartSlot, uint NumViews, ID3D11ShaderResourceView* const * SRV)
        {
            d3dDeviceContext->CSSetShaderResources(StartSlot, NumViews, SRV);
        }



    } // namespace shader_system






    ///
    /// ------------- Effects11 -----------------///
    ///




    void Effect11::Create_FX_and_Tech(char* fx_file_name, char* technique_name, ID3D11Device* device, ID3D11DeviceContext* DeviceContext)
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

            ID3DBlob* pCompiledFXBlob = 0;

            ID3DBlob* compilationErrors = 0;


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


            HR(D3DX11CreateEffectFromMemory(pCompiledFXBlob->GetBufferPointer(), pCompiledFXBlob->GetBufferSize(), 0, device, &fx));

            Technique = fx->GetTechniqueByName(technique_name);


            delete[] fn;

        }

        if ((this->technique_name != technique_name) && (fx_file_name == this->fx_file_name))
        {

            this->technique_name = technique_name;

            Technique = fx->GetTechniqueByName(technique_name);

        }



    }


    void Effect11::ConnectToShaderVariables()
    {

        fxLWVP = fx->GetVariableByName("LWVP")->AsMatrix();
        fxWV = fx->GetVariableByName("WV")->AsMatrix();
        fxView = fx->GetVariableByName("View")->AsMatrix();
        fxTexMtx = fx->GetVariableByName("TexMtx")->AsMatrix();

        fxDiffuseMapVar = fx->GetVariableByName("DiffuseMap")->AsShaderResource();
        fxSpecularMapVar = fx->GetVariableByName("SpecularMap")->AsShaderResource();
        fxOverlayMapVar = fx->GetVariableByName("OverlayMap")->AsShaderResource();

    }


    ID3DX11Effect* Effect11::GetEffect() const
    {

        if (fx) return fx;

        else return 0;

    }


    ID3DX11EffectTechnique* Effect11::GetTechnique() const
    {

        if (Technique) return Technique;

        else return 0;

    }


    void Effect11::SetEffect(ID3DX11Effect* fx)
    {
        this->fx = fx;
    }

    void Effect11::SetTechnique(ID3DX11EffectTechnique* Technique)
    {
        this->Technique = Technique;
    }


    void Effect11::set_device(ID3D11Device* d3dDevice)
    {
        this->d3dDevice = d3dDevice;
    }

    void Effect11::set_device_context(ID3D11DeviceContext* d3dDeviceContext)
    {
        this->d3dDeviceContext = d3dDeviceContext;
    }


    void Effect11::TextureDiffuseApply(char* filename)
    {
        if (filename != DiffuseTextureFileName)
        {

            DiffuseTextureFileName = filename;

            ReleaseCOM(pDiffuseMap);

            // Преобразование из char* в WCHAR*

            const WCHAR* fn = 0;

            fn = string_facilities::pchar_to_pwchar(filename);

            HR(CreateDDSTextureFromFile(d3dDevice, fn, NULL, &pDiffuseMap));

            //D3DX11CreateShaderResourceViewFromFile(d3dDevice, fn , 0, 0, &pDiffuseMap, 0); // D3DX  utility library is deprecated for Windows 8

        }

    }

    void Effect11::TextureDiffuseApply(ID3D11ShaderResourceView* SRV)
    {
        pDiffuseMap = SRV;
    }

    void Effect11::TextureSpecularApply(char* filename)
    {
        if (filename != SpecularTextureFileName)
        {

            SpecularTextureFileName = filename;

            ReleaseCOM(pSpecularMap);

            // Преобразование из char* в WCHAR*

            const WCHAR* fn = 0;

            fn = string_facilities::pchar_to_pwchar(filename);


            HR(CreateDDSTextureFromFile(d3dDevice, fn, NULL, &pSpecularMap));

            //D3DX11CreateShaderResourceViewFromFile(d3dDevice, fn , 0, 0, &pSpecularMap, 0); // D3DX  utility library is deprecated for Windows 8 // D3DX  utility library is deprecated for Windows 8

        }

    }


    void Effect11::TextureOverlayApply(char* filename)
    {
        if (filename != OverlayTextureFileName)
        {

            OverlayTextureFileName = filename;

            ReleaseCOM(pOverlayMap);

            // Преобразование из char* в WCHAR*

            const WCHAR* fn = 0;

            fn = string_facilities::pchar_to_pwchar(filename);

            HR(CreateDDSTextureFromFile(d3dDevice, fn, NULL, &pOverlayMap));

            //D3DX11CreateShaderResourceViewFromFile(d3dDevice, fn , 0, 0, &pOverlayMap, 0); // D3DX  utility library is deprecated for Windows 8 // D3DX  utility library is deprecated for Windows 8

        }

    }















 } // namespace effects



