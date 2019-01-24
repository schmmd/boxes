#include <windows.h>

#include "app.h"
#include "procedure.h"
#include "resource.h"

LRESULT CALLBACK CClass::Procedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	char * szClassName = "CWindow";
	char * szCaption   = "Boxes";

	CClass windowclass (szClassName, hInstance);
	windowclass.SetMenu(IDR_MENU);
	windowclass.SetIcon(IDI_ICON);
	windowclass.Register();

	CProcedure proc;

	CWindow window(szCaption, szClassName, hInstance);
	window.SetDimentions(30 * 20, 20 * 20 + 40); // + 40 for top and bottom
	window.Create(&proc);
	window.AttachAccelerator(IDR_ACCELERATOR);
	window.Show();

	while(window.ProcessMessage())
	{

	}

	return 0;
}
