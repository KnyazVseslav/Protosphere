#pragma once

#ifndef D3DApplication_H
#define D3DApplication_H

#include "D3D.h"
#include "stdafx.h"
#include "resources\значки\Resource.h"
#include "GameTimer.h"


#define MAX_LOADSTRING 100


class D3DApplication
{

public:

	D3DApplication();
	~D3DApplication();


D3D *d3d;

TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HINSTANCE hInst;								// current instance
HWND hWnd;										// Main window handle

UINT ClientWidth;
UINT ClientHeight;

	bool mAppPaused;
	bool mMinimized;
	bool mMaximized;
	bool mResizing;

	GameTimer mTimer;


	bool isAppPaused;


HINSTANCE getInstance(){return hInst;}

ATOM RegisterWindowClass();
ATOM RegisterWindowClassEx();

//static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

virtual void UpdateScene(float dt);

virtual void RenderScene();

virtual void OnResize();

void Reset_RTV_DSV_VP(UINT ClientWidth, UINT ClientHeight);

BOOL Init(HINSTANCE, int);
BOOL InitInstance(int);

int MessageLoop();

static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);


};

#endif

