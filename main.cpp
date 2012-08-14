#include "stdafx.h"
#include "Consol.h"
#include "LayoutWnd.h"
#include "PropertyWnd.h"
#pragma usexpstyle

bool RegisterFileType(LPCTSTR ft,LPCWSTR shell,int icon = 0){
	if(ft==0) return 0;
	if(ft[0]!='.') return 0;
	base::String ftn;
	ftn = ft+1;
	ftn += L".Concise.File";

	base::Registry reg;
	if(base::Registry::REG_ERROR==reg.Create(ft,HKEY_CLASSES_ROOT)) return 0;
	reg.SetStrValue(0,ftn);
	reg.Close();

	base::String key,val;
	key.Format(L"HKEY_CLASSES_ROOT/%s/DefaultIcon",ftn.Handle());
	if(base::Registry::REG_ERROR==reg.Create(key,true)) return 0;
	val.Format(L"%s,%d",shell,icon);
	reg.SetStrValue(0,val);

	key.Format(L"HKEY_CLASSES_ROOT/%s/shell/open/command",ftn.Handle()); 
	if(base::Registry::REG_ERROR==reg.Create(key,true)) return 0;
	val.Format(L"\"%s\"",shell);
	val +=  L" \"%1\"";
	reg.SetStrValue(0,val);
	return true;
}
WINMAIN{
	base::String path;
	base::GetCurrentExeFileName(path);
	RegisterFileType(L".layout",path,1);
	PropertyWnd pw;
	base::MsgLoop ml;
	pw.Create(lpCmdLine,nCmdShow);
	return ml.Start();
};