#include "dialogsettings.h"
#include "resource.h"

#include <stdio.h>

BOOL CALLBACK SettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

volatile int g_iBeastDelay = 45, g_iBeasts = 5, g_iDensity = 30;

bool SettingsDialog(HWND hwnd, int & iBeastDelay, int & iBeasts, int & iDensity)
{
	if (DialogBox (GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), hwnd, &SettingsProc) == -1)
		return false;

	iBeastDelay = g_iBeastDelay;
	iBeasts     = g_iBeasts;
	iDensity    = g_iDensity;

	return true;
}

BOOL CALLBACK SettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szBuffer[16];
	RECT rect, rect2;

	switch (message)
	{
		case WM_INITDIALOG:
			GetWindowRect(GetParent(hWnd), &rect);
			GetWindowRect(hWnd , &rect2);
			SetWindowPos (hWnd, HWND_TOP, ((rect.left + rect.right) / 2) - ((rect2.right - rect2.left) / 2), ((rect.top + rect.bottom) / 2) - ((rect2.bottom - rect2.top) / 2), 0, 0, SWP_NOSIZE);

			sprintf(szBuffer, "%i", g_iBeasts);
			SetWindowText(GetDlgItem(hWnd, IDC_BEASTS), szBuffer);
			sprintf(szBuffer, "%i", g_iBeastDelay);
			SetWindowText(GetDlgItem(hWnd, IDC_BEASTDELAY), szBuffer);
			sprintf(szBuffer, "%i", g_iDensity);
			SetWindowText(GetDlgItem(hWnd, IDC_DENSITY), szBuffer);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					EndDialog(hWnd, -1);
					return 1;
				case IDOK:
					GetWindowText(GetDlgItem(hWnd, IDC_BEASTS), szBuffer, 16);
					g_iBeasts = atoi(szBuffer);

					GetWindowText(GetDlgItem(hWnd, IDC_BEASTDELAY), szBuffer, 16);
					g_iBeastDelay = atoi(szBuffer);

					GetWindowText(GetDlgItem(hWnd, IDC_DENSITY), szBuffer, 16);
					g_iDensity = atoi(szBuffer);

					SendMessage(hWnd, WM_DESTROY, 0, 0);
					return 1;
			}
		break;

		case WM_DESTROY:
			EndDialog(hWnd, 0);
			return 1;
	}

	return 0;
}