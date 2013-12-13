#include <windows.h>

typedef
	int
	WINAPI
	FUN_MessageBoxW(
	__in_opt HWND hWnd,
	__in_opt LPCWSTR lpText,
	__in_opt LPCWSTR lpCaption,
	__in UINT uType);

static FUN_MessageBoxW *OLD_MessageBoxW=MessageBoxW;  

int
	WINAPI
	NEW_MessageBoxW(
	__in_opt HWND hWnd,
	__in_opt LPCWSTR lpText,
	__in_opt LPCWSTR lpCaption,
	__in UINT uType)
{  

	//修改输入参数，调用原函数  
	int ret=OLD_MessageBoxW(hWnd,L"输入参数已修改",L"[测试]",uType);  
	return ret;  
}