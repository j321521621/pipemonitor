#include "stdafx.h"
#include "common.h"
#include <tlhelp32.h>
#include <windows.h>
#include <sstream>
using std::wstringstream;

vector<vector<wstring>> common::proc_snapshot()
{
	vector<vector<wstring>> ret;
	HANDLE hProcs=0;
	if ( hProcs = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) )
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof( PROCESSENTRY32 );
		BOOL ok = Process32First(hProcs, &pe32);
		while (ok)
		{
			vector<wstring> line;
			wstringstream ss;
			ss<<pe32.th32ProcessID;
			line.push_back(ss.str());
			line.push_back(pe32.szExeFile);
			ret.push_back(line);
			ok = Process32Next(hProcs, &pe32);
		}
		CloseHandle(hProcs);
	}
	return ret;
};
