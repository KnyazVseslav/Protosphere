#pragma once

#ifndef Протосфера_H
#define Протосфера_H


#include <windowsx.h>


#include "D3DApplication.h"
#include <DirectXMath.h>

#include "Scene.h"


class Protosphere: public D3DApplication
{

public:
	
	   Protosphere();

	     DirectX::XMMATRIX WorldBasis;

			  void InitScene(){};

		 		 void OnResize(){};

		   virtual void UpdateScene(float dt){};

		virtual void RenderScene(){};


public:

	_Scene Scene;


};






#endif

