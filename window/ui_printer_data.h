#pragma once
#include <string>
using std::string;
using std::wstring;

typedef struct
{
	wstring pid;
	wstring pname;
	wstring tid;
	wstring data;
} listdata;

#define WM_ADDDATA WM_APP