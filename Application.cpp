#include "Протосфера.h"
#include "Geometry_Classes.h"
#include "Scene.h"

#include "Physics.h"

#include "ShaderSystem.h"

#include <sstream>
#include <stdio.h>

#include "ShadowMap.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>





using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace geometry_objects;

using namespace effects;
using namespace effects::shader_system;

using namespace delegates;




class Application: public Protosphere
{

public:

    Application();

            void InitScene();

            void OnResize();


            virtual void UpdateScene(float dt);

            virtual void RenderScene();


        LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);


    XMMATRIX World, View, Proj, WVP, TexMtx;
    

    POINT aim;
    POINT mOldMousePos;

    string stats;


    float theta;
    float phi;

    XMVECTOR EyePos;
    float radius;

        
};

XMVECTOR 
RED= XMVectorSet(1, 0, 0, 0),
GREEN= XMVectorSet(0, 1, 0, 0),
BLUE= XMVectorSet(0, 0, 1, 0);



ID3DX11EffectVectorVariable* fxLightDir;





float forceY= -1e-2;

float h = 0;

D3D11_FILL_MODE fill_mode = D3D11_FILL_SOLID;

bool isBounce = true;

bool isApplyGravity = false;


XMMATRIX mLookAt;





namespace scene
{

// для наложения теней
namespace shadow_mapping
{

    ShadowMap * depthMap;

    XMMATRIX LightVolumeProj;
    XMMATRIX LightWVP; 

    

}

namespace fx
{
    
    ShaderSystem RenderFX;
    ShaderSystem DepthMapBuildFX;
    
    Effect11 TexFX;
    Effect11 ShadowFX;
    
}

namespace light_sources
{

    LightSource Spot;

}

namespace objects
{

  Mesh model1;

     Camera camera;

 Cube box;
 

   InfoVectors infoV;

     Sphere sphere2("sphere2");

     Cloth cloth2("cloth2");


}


    namespace reflector_types
    {

        namespace VS_render
        {
            struct VS_cbPerScene
            {
                XMMATRIX WV;
                XMMATRIX View;

                XMMATRIX LightWVP; // for shadow mapping
            };


            struct VS_cbPerObject   
            {
                XMMATRIX World;
                XMMATRIX LWVP;

                XMMATRIX TexMtx;
            };


        }


        namespace PS_render
        {

            struct PS_cbPerFrame
            {	
                XMMATRIX WV;
    
                XMVECTOR LightDir;
                XMVECTOR LightPos;
                XMVECTOR pos_test2;

                XMVECTOR L_ambient;
                XMVECTOR L_diffuse;
                XMVECTOR L_specular;

                int LightSourceType; 
                LightSource light;
                LightSource light2;
                XMVECTOR EyePosW;
            };

        }


        namespace VS_build_depthmap
        {
            struct VS_build_depthmap_cbPerFrame    
            {
                XMMATRIX LightWVP;
            };
        }


    } // namespace reflector_types




} // namespace scene




using scene::reflector_types::VS_render::VS_cbPerScene;
using scene::reflector_types::VS_render::VS_cbPerObject;
using scene::reflector_types::PS_render::PS_cbPerFrame;
using scene::reflector_types::VS_build_depthmap::VS_build_depthmap_cbPerFrame;

using scene::fx::RenderFX;
using scene::fx::DepthMapBuildFX;

using namespace scene::objects;





Application::Application(): theta(0.0f*Pi), phi(0.0f*Pi), radius(20.0f)
{

        View= DirectX::XMMatrixIdentity();
        Proj= DirectX::XMMatrixIdentity();
        WVP= DirectX::XMMatrixIdentity();
        TexMtx= DirectX::XMMatrixIdentity();

}


void Application::InitScene()
{
    

    //------ Описание depth map

    float depth_map_width  = 2048.0f;
    float depth_map_height = depth_map_width;

    scene::shadow_mapping::depthMap = new ShadowMap;
    scene::shadow_mapping::depthMap->Initialize(d3d->Device, d3d->DeviceContext);
    scene::shadow_mapping::depthMap->create_DSV_SRV_VP_for_depthMap(depth_map_width, depth_map_height);
    
    
    float aspect_ratio = depth_map_width / depth_map_height;
    scene::shadow_mapping::LightVolumeProj= XMMatrixPerspectiveFovLH(0.25*Pi, aspect_ratio, 0.5f, 40.0f);


    


    //
    // Шейдеры общие для всех объектов сцены
    //



    // DepthMapBuildFX

        DepthMapBuildFX.Init("DepthMapBuildFX", d3d->Device, d3d->DeviceContext);

        // VS
        DepthMapBuildFX.AddShader("VS", VertexShader, "shaders\\HLSL\\build_depthmap_VS.hlsl", "VS", "vs_5_0");
    
        DepthMapBuildFX.shaders["VS"].AddConstantBuffer<VS_build_depthmap_cbPerFrame>("vs_build_depthmap_cbPerFrame", 0); // b0

        // PS
        DepthMapBuildFX.AddShader("PS", PixelShader, "shaders\\HLSL\\build_depthmap_PS.hlsl", "PS", "ps_5_0");


        // callback-функции (выполняются до рендера и после него)

        DepthMapBuildFX.AddBeginFunction("OMbind_DSV_nullRTV_VP", scene::shadow_mapping::depthMap, &ShadowMap::OMbind_DSV_nullRTV_VP);
        DepthMapBuildFX.AddEndFunction("Reset_RTV_DSV_VP", d3d, &D3D::Reset_RTV_DSV_VP, &ClientWidth, &ClientHeight);



    // RenderFX

        RenderFX.Init("RenderFX", d3d->Device, d3d->DeviceContext);

    
        // VS
        RenderFX.AddShader("VS", VertexShader, "shaders\\HLSL\\render_VS.hlsl", "VS", "vs_5_0");
        
        RenderFX.shaders["VS"].AddConstantBuffer<VS_cbPerScene>("vs_cbPerScene", 0); // b0
        RenderFX.shaders["VS"].AddConstantBuffer<VS_cbPerObject>("vs_cbPerObject", 1); // b1


        // PS
        RenderFX.AddShader("PS", PixelShader, "shaders\\HLSL\\render_PS.hlsl", "PS", "ps_5_0");

        RenderFX.shaders["PS"].AddConstantBuffer<PS_cbPerFrame>("ps_cbPerFrame", 0); // b0
        RenderFX.shaders["PS"].AddTexture2D(scene::shadow_mapping::depthMap->Get_SRV(), 0);		


//		
//Сцена
//


    // ткань

        scene::objects::cloth2.Init("cloth2", d3d->Device, d3d->DeviceContext, et_ShaderSystem);
        scene::objects::cloth2.ConstructGeometry(10, 10, 1);		
        scene::objects::cloth2.m = 3;
        
        scene::objects::cloth2.DiffuseTexture.init("resources\\textures\\cloth.DDS", { { "RenderFX" }, { "PS" }, { 1 }, { 1 } });

        scene::objects::cloth2.VertexInputLayoutCreate(RenderFX.shaders["VS"].get_byte_code());
        
        scene::objects::cloth2.Position= XMVectorSet(0, 0, 0, 0);


    // сфера

    scene::objects::sphere2.Init("sphere2", d3d->Device, d3d->DeviceContext, et_ShaderSystem);	
    scene::objects::sphere2.ConstructGeometry(2, 4, 40, 40);		
                                                                                                                    //t1   1 SRV
    scene::objects::sphere2.DiffuseTexture.init("resources\\textures\\Protosphere.DDS", { { "RenderFX" }, { "PS" }, { 1 }, { 1 } });

    scene::objects::sphere2.VertexInputLayoutCreate(RenderFX.shaders["VS"].get_byte_code());

    scene::objects::sphere2.Position= XMVectorSet(0, 3, 0, 0);


    auto& v0 = sphere2.vertices[0];
    auto& v1 = sphere2.vertices[1];
    auto& vn = sphere2.vertices.back();


    
    // Наполнение группы сцены объектами и эффектами
    
    Scene.vg_objects_groups.add("Sphere_Plane_Group");

    Scene.vg_objects_groups[0].effects.add(DepthMapBuildFX);
    Scene.vg_objects_groups[0].effects.add(RenderFX);
    

    
    Scene.vg_objects_groups[0].objects.add(cloth2);
    Scene.vg_objects_groups[0].objects.add(sphere2);
    
    


    // Effect11 для инфовекторов

    scene::fx::TexFX.Create(d3d->Device, d3d->DeviceContext, "shaders\\fx\\tex.fx", "TexTech");	
    

    // источник света

    scene::light_sources::Spot.InitFromExistingData(Spot, d3d->Device, d3d->DeviceContext, scene::fx::TexFX.GetEffect(), scene::fx::TexFX.GetTechnique());
    scene::light_sources::Spot.Position= XMVectorZero();


    // камера

    scene::objects::camera.Position= XMVectorSet(0, 10, -10, 0);
    

    // куб для визуализации источника света

    scene::objects::box.InitFromExistingData(d3d->Device, d3d->DeviceContext, scene::fx::TexFX.GetEffect(), scene::fx::TexFX.GetTechnique());

    scene::objects::box.InfoVectorsConstruct(ObjectSpace, NULL);

    scene::objects::box.ConstructGeometry();
    scene::objects::box.Position= XMVectorSet(0, 0, 10, 0);


    // инфовекторы

    scene::objects::infoV.Init(d3d->Device, d3d->DeviceContext, "shaders\\fx\\InfoVectors.fx", "InfoTech");

    vector<VertexPosColor> v;
    v.push_back(VertexPosColor(scene::objects::box.Position, RED)); //0
    v.push_back(VertexPosColor(scene::objects::box.Right, RED)); //1

    v.push_back(VertexPosColor(scene::objects::box.Position, GREEN)); //2
    v.push_back(VertexPosColor(scene::objects::box.Up, GREEN)); //3

    v.push_back(VertexPosColor(scene::objects::box.Position, BLUE)); //4
    v.push_back(VertexPosColor(scene::objects::box.Direction, BLUE)); //5

    vector<DWORD> indcs;
    indcs.push_back(0);
    indcs.push_back(1);
    indcs.push_back(2);
    indcs.push_back(3);
    indcs.push_back(4);
    indcs.push_back(5);
    
    
    scene::objects::infoV.Construct(v, indcs);	


}


void Application::UpdateScene(float dt)
{

    D3DApplication::UpdateScene(dt);



    // камера

    scene::objects::camera.Observe(hWnd, aim.x, aim.y, ClientWidth/2, ClientHeight/2, 1e-3*1.8);


    float p;

    static float vel = 1e2 / 5; // velocity factor 
    static float ls_rotation_vel = 1e-2;

    if (GetAsyncKeyState('W') & 0x8000) scene::objects::camera.TranslateZ(10e-3/5*vel);
    if (GetAsyncKeyState('S') & 0x8000) scene::objects::camera.TranslateZ(-10e-3/5*vel);
    if (GetAsyncKeyState('A') & 0x8000) scene::objects::camera.TranslateX(-10e-3/5*vel);
    if (GetAsyncKeyState('D') & 0x8000) scene::objects::camera.TranslateX(10e-3/5*vel);


    // изменение скоростей
    p = 1e-2;

    if (GetAsyncKeyState(VK_ADD) & 0x8000)	{ vel += 1e-2;  ls_rotation_vel += 1e-5; };
    if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000)	{ vel -= 1e-2;  ls_rotation_vel -= 1e-5; };


    p = 1e1;

    if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)	Scene.vg_objects_groups[0].objects["sphere2"]->TranslateZ(1e-3*vel / 1.85f);
    if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)	Scene.vg_objects_groups[0].objects["sphere2"]->TranslateZ(-1e-3*vel / 1.85f);
    if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)	Scene.vg_objects_groups[0].objects["sphere2"]->TranslateX(-1e-3*vel / 1.85f);
    if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)	Scene.vg_objects_groups[0].objects["sphere2"]->TranslateX(1e-3*vel / 1.85f);

    p= 1e1*2;

    if(GetAsyncKeyState(VK_NUMPAD7) & 0x8000)	Scene.vg_objects_groups[0].objects["sphere2"]->TranslateY((1e-2)/1.4*p);

    p= 1e0;

    if(GetAsyncKeyState(VK_NUMPAD9) & 0x8000)	Scene.vg_objects_groups[0].objects["sphere2"]->TranslateY(-1e-3*p);

    if(GetAsyncKeyState(VK_NUMPAD5) & 0x8000)	
    {
            Scene.vg_objects_groups[0].objects["sphere2"]->SetPosition(0, h, 0);
            Scene.vg_objects_groups[0].objects["sphere2"]->SetVelocity(XMVectorZero(), 1);
            Scene.vg_objects_groups[0].objects["sphere2"]->Force = XMVectorZero();
    }


    // изменение массы сферы

    p= 1e-2;

    if (GetAsyncKeyState(VK_ADD) & 0x8000)	Scene.vg_objects_groups[0].objects["sphere2"]->m += p;
    if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000)	Scene.vg_objects_groups[0].objects["sphere2"]->m -= p;

    
    // изменение жёсткости ткани

    p= 1e-2*5;
    
    if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)	    Scene.vg_objects_groups[0].objects["cloth2"]->Kd -= p;
    if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)	Scene.vg_objects_groups[0].objects["cloth2"]->Kd += p;


    // вкл/выкл реакцию сферы на столкновение с тканью
    if(GetAsyncKeyState(VK_DIVIDE) & 0x8000)   
    {
            isBounce = !isBounce;
    }


    // вкл/выкл действие гравитации на сферу
    if(GetAsyncKeyState(VK_MULTIPLY) & 0x8000) 
    {
            isApplyGravity = !isApplyGravity;
    }


    // Управление источником света
    
    // вращение вокруг вертикальной оси
    if (GetAsyncKeyState('G') & 0x8000) theta -= ls_rotation_vel/3;
    if (GetAsyncKeyState('J') & 0x8000) theta += ls_rotation_vel/3;

    // вращение вокруг горизонтальной оси
    if (GetAsyncKeyState('Y') & 0x8000) phi -= ls_rotation_vel/3;
    if (GetAsyncKeyState('H') & 0x8000) phi += ls_rotation_vel/3;


    p= 1e-2;

    // отдалить приблизить источник света
    if(GetAsyncKeyState(VK_UP) & 0x8000)	radius+= p;
    if(GetAsyncKeyState(VK_DOWN) & 0x8000)	radius-= p;

    
    
    EyePos.m128_f32[0] =  radius*sinf(phi)*sinf(theta);
    EyePos.m128_f32[2] = -radius*sinf(phi)*cosf(theta);
    EyePos.m128_f32[1] =  radius*cosf(phi);
    
    scene::objects::box.Position= EyePos;


    mLookAt= XMMatrixLookAtLH(EyePos,
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

    
}


void Application::OnResize()
{

    D3DApplication::OnResize();

    float aspectratio = (float)ClientWidth/ClientHeight;

    Proj= XMMatrixPerspectiveFovLH(0.25*Pi, aspectratio, 0.5f, 1e3f);

}


void Application::RenderScene()
{

    D3DApplication::RenderScene();

    


    //
    // Установка параметров сцены
    //


    View = XMLoadFloat4x4(&scene::objects::camera.ObjectSpace);
        
        WVP = View*Proj;



    // Обновление константных буферов

        VS_cbPerScene vs_cbPerScene;

        vs_cbPerScene.WV= View;
        vs_cbPerScene.View= View;
        vs_cbPerScene.LightWVP= scene::shadow_mapping::LightWVP;


        VS_cbPerObject vs_cbPerObject;

        vs_cbPerObject.LWVP= WVP;
        vs_cbPerObject.World= XMMatrixIdentity();
        vs_cbPerObject.TexMtx= XMMatrixIdentity();		



        PS_cbPerFrame ps_cbPerFrame;

        ps_cbPerFrame.WV= View;
        ps_cbPerFrame.LightDir= scene::light_sources::Spot.Direction;
        ps_cbPerFrame.LightPos= scene::light_sources::Spot.Position;
        ps_cbPerFrame.pos_test2= scene::light_sources::Spot.Position;
        ps_cbPerFrame.light= scene::light_sources::Spot;
        ps_cbPerFrame.light2= scene::light_sources::Spot;
        ps_cbPerFrame.EyePosW= scene::objects::camera.Position;
        ps_cbPerFrame.L_ambient= scene::light_sources::Spot.ambient;
        ps_cbPerFrame.L_diffuse= scene::light_sources::Spot.diffuse;
        ps_cbPerFrame.L_specular= scene::light_sources::Spot.specular;
        ps_cbPerFrame.LightSourceType= scene::light_sources::Spot.LSType;

    

        *Scene.vg_objects_groups["Sphere_Plane_Group"].effects["RenderFX"].shaders["VS"].constant_buffers["vs_cbPerScene"]= &vs_cbPerScene;
        *Scene.vg_objects_groups["Sphere_Plane_Group"].effects["RenderFX"].shaders["VS"].constant_buffers["vs_cbPerObject"]= &vs_cbPerObject;
        *Scene.vg_objects_groups["Sphere_Plane_Group"].effects["RenderFX"].shaders["PS"].constant_buffers["ps_cbPerFrame"]= &ps_cbPerFrame;

        
        VS_build_depthmap_cbPerFrame vs_build_depthmap_cbPerFrame;
        vs_build_depthmap_cbPerFrame.LightWVP= scene::light_sources::Spot.GetTransformToObjectSpace()*scene::shadow_mapping::LightVolumeProj;

        *Scene.vg_objects_groups["Sphere_Plane_Group"].effects["DepthMapBuildFX"].shaders["VS"].constant_buffers["vs_build_depthmap_cbPerFrame"] = &vs_build_depthmap_cbPerFrame;


        Scene.vg_objects_groups[0].objects[0]->rasterisation_state.FillMode = fill_mode;
        Scene.vg_objects_groups[0].objects[1]->rasterisation_state.FillMode = fill_mode;
        

        // работа с источником света

        scene::light_sources::Spot.Position = scene::objects::box.Position;
        scene::light_sources::Spot.Direction = -scene::objects::box.Position;

        scene::shadow_mapping::LightWVP = scene::light_sources::Spot.GetTransformToObjectSpace()*scene::shadow_mapping::LightVolumeProj;



    // симуляция ткани

        XMVECTOR Force = XMVectorSet(0, 0, 0, 0);

        isBounce = false;
        isApplyGravity = true;

        float dt = 1;


        CollideClothWithBall(Scene.vg_objects_groups[0].objects["cloth2"], Scene.vg_objects_groups[0].objects["sphere2"], isApplyGravity, dt);



    // отрисовка основной сцены

        Scene.Render();



    // вывод инфографики

         scene::objects::box.Render(WVP, D3D11_FILL_WIREFRAME, 1, D3D11_CULL_NONE);

             vector<VertexPosColor> v;
             XMVECTOR x, y, z;
            
             
             // создание геометрии инфовекторов - базис ИС
             x= scene::objects::box.Position+XMVectorSet(mLookAt.r[0].m128_f32[0], mLookAt.r[1].m128_f32[0], mLookAt.r[2].m128_f32[0], 0);

                 v.push_back(VertexPosColor(scene::objects::box.Position, RED));
                 v.push_back(VertexPosColor(x, RED));

             y= scene::objects::box.Position+XMVectorSet(mLookAt.r[0].m128_f32[1], mLookAt.r[1].m128_f32[1], mLookAt.r[2].m128_f32[1], 0);

                 v.push_back(VertexPosColor(scene::objects::box.Position, GREEN));
                 v.push_back(VertexPosColor(y, GREEN));

             z= scene::objects::box.Position+XMVectorSet(mLookAt.r[0].m128_f32[2], mLookAt.r[1].m128_f32[2], mLookAt.r[2].m128_f32[2], 0);

                 v.push_back(VertexPosColor(scene::objects::box.Position, BLUE));
                 v.push_back(VertexPosColor(z, BLUE));
                 

                 // установка геометрии инфовекторов
                 scene::objects::infoV.SetVertices(v); 

                 // отрисовка базиса ИС
                 scene::objects::infoV.Render(WVP, D3D11_FILL_SOLID, 1, D3D11_CULL_NONE, D3D11_PRIMITIVE_TOPOLOGY_LINELIST); 			


    d3d->SwapChain->Present(0, 0); 	 

}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    

    Application d3dapplication;


    if(!d3dapplication.Init(hInstance, SW_SHOWMAXIMIZED)) return 0;

            d3dapplication.InitScene();	
                
                    return  d3dapplication.MessageLoop();

    
}


LRESULT Application::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{

    POINT mousePos;
    int dx = 0;
    int dy = 0;

    switch(msg)
    {

        case WM_LBUTTONDOWN:
        if( wParam & MK_LBUTTON )
        {
            SetCapture(hWnd);

            mOldMousePos.x = LOWORD(lParam);
            mOldMousePos.y = HIWORD(lParam);
        }
        return 0;

    case WM_LBUTTONUP:
        ReleaseCapture();
        return 0;	

    case WM_MOUSEMOVE:

        {
            aim.x= GET_X_LPARAM(lParam);
            aim.y= GET_Y_LPARAM(lParam);
            

            if( wParam & MK_LBUTTON )
            {

                mousePos.x = (int)LOWORD(lParam); 
                mousePos.y = (int)HIWORD(lParam); 

                dx = mousePos.x - mOldMousePos.x;
                dy = mousePos.y - mOldMousePos.y;
                         

                mOldMousePos = mousePos;

            }

        return 0;
        
        
        }

        case WM_KEYDOWN:

            switch(wParam)
            {

                case VK_ESCAPE: DestroyWindow(hWnd);
                      
                                 return 0;

                case VK_SPACE:
                    
                    switch(fill_mode)		
                    {
                        
                        case D3D11_FILL_SOLID: fill_mode= D3D11_FILL_WIREFRAME;
                                    
                                 return 0;

                        case D3D11_FILL_WIREFRAME: fill_mode= D3D11_FILL_SOLID;												   

                    }

                            return 0;
                               
                                 
            }


        
        return 0;


        default: 
            return D3DApplication::WndProc(msg, wParam, lParam);	

    }


}

