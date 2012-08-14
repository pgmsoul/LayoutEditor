#include "stdafx.h"
#include "resource.h"
#include "LayoutWnd.h"
class TextDraw : public base::_class{
protected:
	base::Rect rect;
	base::String text;
public:
	TextDraw(){
		rect.SetValue(0,0,0,0);
	}
	void SetLocation(int x,int y){
		rect.SetLocation(x,y);
	}
	void SetText(LPCWSTR str){
		text = str;
	}
	void Draw(HDC hdc){
		::DrawText(hdc,text,text.Length(),rect,DT_SINGLELINE);
	}
	base::Twain GetRequestSize(HDC hdc){
		::DrawText(hdc,text,text.Length(),rect,DT_SINGLELINE|DT_CALCRECT);
		return base::Twain(rect.Width(),rect.Height());
	}
};
void rectangle(base::gdi::DC* dc,int x,int y,int cx,int cy){
	dc->MoveTo(x,y);
	dc->LineTo(x+cx,y);
	dc->LineTo(x+cx,y+cy);
	dc->LineTo(x,y+cy);
	dc->LineTo(x,y);
}
void LayoutWnd::onDraw(base::gdi::DC* dc){
	outLine.Select(*dc);
	_Font.Select(*dc);
	dc->SetBkTransparent();
	dc->SetTextColor(0xff0000);
	drawLayout(dc,&glLayout);
	if(selLay!=0){
		base::Twain16 lc = selLay->GetLocation(),
			sz = selLay->GetSize();
		selLine.Select(*dc);
		rectangle(dc,lc.x,lc.y,sz.x,sz.y);
		selLine.Select(*dc);
	}
	_Font.Select(*dc);
	outLine.Select(*dc);
}
void LayoutWnd::drawLayout(base::gdi::DC* dc,base::Layout* lay){
	base::Twain16 lc = lay->GetLocation(),
		sz = lay->GetSize();
	if(selLay!=lay) rectangle(dc,lc.x,lc.y,sz.x,sz.y);
	if(lay->Count()==0){
		LPCWSTR name = lay->Name;
		TextDraw td;
		td.SetText(name);
		base::Twain ds = td.GetRequestSize(*dc);
		td.SetLocation(lc.x+(sz.x-ds.x)/2,lc.y+(sz.y-ds.y)/2);
		if(selLay==lay) dc->SetTextColor(0xff);
		td.Draw(*dc);
		if(selLay==lay) dc->SetTextColor(0xff0000);
	}
	for(uint i=0;i<lay->Count();i++){
		drawLayout(dc,lay->Element(i));
	}
}
void LayoutWnd::onCreate(){
	base::wnd::SetWndIcon(_Handle,IDI_LAYOUT);
	//disable system close menu
	base::Menu sm;
	sm.Attach(::GetSystemMenu(_Handle,0));
	sm.SetItemDisable(SC_CLOSE,1,MF_BYCOMMAND);
	sm.Detach();
}
void LayoutWnd::onDrag(base::Layout*){
	Invalidate();
}
LayoutWnd::LayoutWnd():selLay(0){
	outLine.Create(0xff0000);
	selLine.Create(0xff,0,3);
	_Font.SetSize(12);
	_Font.Create();
	Param->Title = L"布局";
	Param->AddExStyle(WS_EX_COMPOSITED);
	OnCreate.Add(this,&LayoutWnd::onCreate);
	OnDraw.Add(this,&LayoutWnd::onDraw);
	glLayout.OnDrag.Bind(this,&LayoutWnd::onDrag);
	glLayout.SetContainer(this);
	base::wnd::WPRRegisterWnd(this,L"Layout_wnd");
}
//
#define USER_TYPE_XML L"usertype.xml"
#define defLen 7
inline bool isSpace(wchar_t ch){
	return ch==' '||ch=='\t'||ch=='\r'||ch=='\n';
}
int getNextSpace(LPCWSTR str){
	int n = 0;
	while(str[n]){
		if(isSpace(str[n])) return n;
		n++;
	}
	return n;
}
int getNextNotSpace(LPCWSTR str){
	int n = 0;
	while(str[n]){
		if(!isSpace(str[n])) return n;
		n++;
	}
	return n;
}
int checkLine(base::String& line){
	uint i = defLen;
	for(;i<line.Length();i++){
		if(isSpace(line[i])) continue;
		return i;
	}
	return 0;
}
void initListBoxByMap(base::wnd::ListBox& lb,base::Xml* map,DWORD s){
	if(map==0){
		_ASSERT(0);
		return;
	}
	for(int i=map->Count()-1;i>=0;i--){
		int style = 0;
		base::Xml* xml = map->Element(i);
		//control no WS_MAXIMIZEBOX style,it is equal width WS_TABSTOP
		//if(base::WcsEqual(L"WS_MAXIMIZEBOX",xml->GetName())) continue;
		xml->GetValueInt(style);
		if(style==0) continue;
		if((style&s)==style){
			s -= style;
			int index = lb.FindExactString(xml->GetName());
			if(index<0) lb.AddString(xml->GetName());
		}
	}
}
//
ControlProperty::ControlProperty(void){
	Param->Dialog = IDD_CONTROL_PROPERTY;
	base::wnd::WPRRegisterWnd(this,L"control_property");
	OnCreate.Add(this,&ControlProperty::onCreate);
	OnCommand.Add(this,&ControlProperty::onCommand);
	xmlUserType.Load(USER_TYPE_XML);
}

ControlProperty::~ControlProperty(void){
}

void ControlProperty::onCtrlTypeChanged(base::wnd::CtrlMsg* cm){
	if(cm->uMsg!=WM_COMMAND) return;
	WORD nMsg = HIWORD(cm->wParam);
	if(nMsg==LBN_SELCHANGE){
		if(cm->hWnd==ctrlTypeCombo){
			refreshCtrlStyle();
			if(xmlSelMap==0) return;
			int style = WS_VISIBLE|WS_CHILD;
			xmlSelMap->GetPropertyInt(L"style",style);
			int exStyle = 0;
			xmlSelMap->GetPropertyInt(L"exstyle",exStyle);
			initListBoxByMap(ctrlStyleList,xmlSelMap,style);
			initListBoxByMap(ctrlStyleList,xmlStyleMap,style);
			initListBoxByMap(ctrlExStyleList,xmlExStyleMap,exStyle);
		}else if(cm->hWnd==ctrlNameCombo){
			initCtrlStyleList();
		}
	}
}
void ControlProperty::initCtrlStyleList(){
	int index = ctrlNameCombo.GetCurSelect();
	base::Xml* xml = xmlUserType.Element(index);
	LPCWSTR strClass = xml->GetPropertyValue(L"Class");
	int iClass = ctrlTypeCombo.FindExactString(strClass);
	if(iClass<0){
		base::String msg;
		msg.Format(L"不支持的类名称:[%s]",strClass);
		::MessageBox(_Handle,msg,L"Error",MB_ICONERROR);
		return;
	}
	ctrlTypeCombo.SetCurSelect(iClass);
	refreshCtrlStyle();
	int style = 0;
	xml->GetPropertyInt(L"Style",style);
	int exStyle = 0;
	xml->GetPropertyInt(L"ExStyle",exStyle);
	initListBoxByMap(ctrlStyleList,xmlSelMap,style);
	initListBoxByMap(ctrlStyleList,xmlStyleMap,style);
	initListBoxByMap(ctrlExStyleList,xmlExStyleMap,exStyle);
}
void ControlProperty::refreshCtrlStyle(){
	int index = ctrlTypeCombo.GetCurSelect();
	ctrlNameCombo.SetText(L"<自定义>");
	if(index<=0){
		disableControl();
		return;
	}
	enableControl();
	base::String str(256);
	if(!ctrlTypeCombo.GetItemText(index,str)) return;
	xmlSelMap = xmlCtrlStyle->GetNode(str);
	if(xmlSelMap==0){
		//::MessageBox(_Handle,L"没有找到类型",str,MB_ICONERROR);
		return;
	}
	ctrlCSList.Clear();
	ctrlStyleList.Clear();
	ctrlExStyleList.Clear();
	for(uint i=0;i<xmlSelMap->Count();i++){
		ctrlCSList.AddString(xmlSelMap->Element(i)->GetName());
	}
}
void ControlProperty::getIdList(){
	base::FileStream fs;
	if(!fs.Create(residPath,OPEN_EXISTING)) return;
	base::String buf,name,val;
	fs.ReadString(buf);
	xmlIdMap.Clear();
	fs.Close();
	int fn = 0;
	while(1){
		fn = buf.FindNoCase(L"#define",fn);
		if(fn<0) break;
		fn += defLen;

		int offset = getNextNotSpace(buf+fn);
		fn += offset;
		int start = fn;
		int len = getNextSpace(buf+fn);
		fn += len;
		name = L"";
		name.CopyFrom(buf+start,len);

		offset = getNextNotSpace(buf+fn);
		fn += offset;
		start = fn;
		len = getNextSpace(buf+fn);
		fn += len;
		val = L"";
		val.CopyFrom(buf+start,len);

		xmlIdMap.AddNode(name,val);
	}
}
void ControlProperty::enableControl(){
	base::wnd::EditPtr titleBox = GetCtrlHandle(ID_EDIT_TEXT);
	titleBox.Enable();

	ctrlWSList.Enable();
	ctrlCSList.Enable();
	ctrlWSEXList.Enable();
	ctrlStyleList.Enable();
	ctrlExStyleList.Enable();

	base::wnd::ButtonPtr btn = GetCtrlHandle(IDC_ADD_WS);
	btn.Enable();
	btn = GetCtrlHandle(IDC_ADD_CS);
	btn.Enable();
	btn = GetCtrlHandle(IDC_ADD_ES);
	btn.Enable();
	btn = GetCtrlHandle(IDC_REMOVE_ES);
	btn.Enable();
	btn = GetCtrlHandle(IDC_REMOVE_WS);
	btn.Enable();
	btn = GetCtrlHandle(ID_SAVE_TYPE);
	btn.Enable();

	ctrlIdCombo.Enable();
}
void ControlProperty::disableControl(){
	base::wnd::EditPtr titleBox = GetCtrlHandle(ID_EDIT_TEXT);
	titleBox.Enable(0);

	ctrlWSList.Enable(0);
	ctrlCSList.Enable(0);
	ctrlWSEXList.Enable(0);
	ctrlStyleList.Enable(0);
	ctrlExStyleList.Enable(0);

	base::wnd::ButtonPtr btn = GetCtrlHandle(IDC_ADD_WS);
	btn.Enable(0);
	btn = GetCtrlHandle(IDC_ADD_CS);
	btn.Enable(0);
	btn = GetCtrlHandle(IDC_ADD_ES);
	btn.Enable(0);
	btn = GetCtrlHandle(IDC_REMOVE_ES);
	btn.Enable(0);
	btn = GetCtrlHandle(IDC_REMOVE_WS);
	btn.Enable(0);
	btn = GetCtrlHandle(ID_SAVE_TYPE);
	btn.Enable(0);

	ctrlIdCombo.Enable(0);
}
bool ControlProperty::loadWndConst(){
	if(xmlConst.Load(L"wnd_const.xml")) return 1;
	DWORD rs = 0;
	void* r = base::GetResource(IDR_XML_WND_CONST,L"RT_XML",&rs);
	if(r==0) return 0;
	base::File f;
	f.Create(L"wnd_const.xml");
	f.Write(r,rs);
	f.Close();
	return xmlConst.Load(L"wnd_const.xml");
}
void ControlProperty::initUI(){
	//加载常数
	xmlStyleMap = xmlConst.GetNode(L"WindowStyle");
	xmlExStyleMap = xmlConst.GetNode(L"WindowExStyle");

	//设置布局
	layout.SetContainer(this);
	layout.Load(IDR_CONTROL_LAYOUT);
	base::Layout* lay;
	//控件Stylel列表
	ctrlCSList.Param->AddStyle(LBS_SORT);
	ctrlCSList.Create(_Handle);
	lay = layout.GetElementByName(L"ListCtrlStyle");
	lay->SetControl(ctrlCSList);
	//控件名称Combo
	ctrlNameCombo.OnCtrlMsg.Bind(this,&ControlProperty::onCtrlTypeChanged);
	ctrlNameCombo.Create(_Handle);
	lay = layout.GetElementByName(L"ComboName");
	lay->SetControl(ctrlNameCombo);
	//控件类型（Class）列表
	ctrlTypeCombo.OnCtrlMsg.Bind(this,&ControlProperty::onCtrlTypeChanged);
	ctrlTypeCombo.Param->AddStyle(CBS_DROPDOWNLIST);
	ctrlTypeCombo.Create(_Handle);
	lay = layout.GetElementByName(L"ComboType");
	lay->SetControl(ctrlTypeCombo);
	xmlCtrlStyle = xmlConst.GetNode(L"ControlStyle");
	if(xmlCtrlStyle==NULL){
		::MessageBox(_Handle,L"No ControlStyle Node in Xml",L"Error",MB_ICONERROR);
		ExitProcess(0);
	}
	ctrlTypeCombo.AddString(L"NONE");
	for(uint i=0;i<xmlCtrlStyle->Count();i++){
		ctrlTypeCombo.AddString(xmlCtrlStyle->Element(i)->GetName());
	}
	ctrlTypeCombo.SetCurSelect(0);
	refreshCtrlStyle();
	//窗口Style类别 WS_
	ctrlWSList.Param->AddStyle(LBS_SORT);
	ctrlWSList.Create(_Handle);
	lay = layout.GetElementByName(L"ListStyle");
	lay->SetControl(ctrlWSList);
	base::Xml* wndStyleXml = xmlConst.GetNode(L"WindowStyle"); 
	if(wndStyleXml==NULL){
		::MessageBox(_Handle,L"No WindowStyle Node in Xml",L"Error",MB_ICONERROR);
		ExitProcess(0);
	}
	for(uint i=0;i<wndStyleXml->Count();i++){
		ctrlWSList.AddString(wndStyleXml->Element(i)->GetName());
	}
	//窗口额外风格列表，WS_EX_
	ctrlWSEXList.Param->AddStyle(LBS_SORT);
	ctrlWSEXList.Create(_Handle);
	lay = layout.GetElementByName(L"ListExStyle");
	lay->SetControl(ctrlWSEXList);
	base::Xml* wndExStyleXml = xmlConst.GetNode(L"WindowExStyle"); 
	if(wndExStyleXml==NULL){
		::MessageBox(_Handle,L"No WindowExStyle Node in Xml",L"Error",MB_ICONERROR);
		ExitProcess(0);
	}
	for(uint i=0;i<wndExStyleXml->Count();i++){
		ctrlWSEXList.AddString(wndExStyleXml->Element(i)->GetName());
	}
	//控件当前风格列表
	ctrlStyleList.Create(_Handle);
	lay = layout.GetElementByName(L"ControlStyle");
	lay->SetControl(ctrlStyleList);
	//控件当前ExStyle列表
	ctrlExStyleList.Create(_Handle);
	lay = layout.GetElementByName(L"ControlExStyle");
	lay->SetControl(ctrlExStyleList);
	//Id Combo
	ctrlIdCombo.Param->AddStyle(CBS_DROPDOWNLIST);
	ctrlIdCombo.Create(_Handle);
	lay = layout.GetElementByName(L"ComboID");
	lay->SetControl(ctrlIdCombo);
	getIdList();
	ctrlIdCombo.AddString(L"0");
	for(uint i=0;i<xmlIdMap.Count();i++){
		ctrlIdCombo.AddString(xmlIdMap.Element(i)->GetName());
	}
	ctrlIdCombo.SetCurSelect(0);

	lay = layout.GetElementByName(L"LabelName");
	lay->SetControl(L"Static",0,L"Control Name: ");

	lay = layout.GetElementByName(L"ButtonSaveType");
	lay->SetControl(L"Button",ID_SAVE_TYPE,L"Save As New Control Type");

	lay = layout.GetElementByName(L"LabelType");
	lay->SetControl(L"Static",0,L"Control Class: ",WS_VISIBLE|WS_CHILD,0);

	lay = layout.GetElementByName(L"LabelID");
	lay->SetControl(L"Static",0,L"Control ID: ",WS_VISIBLE|WS_CHILD,0);

	lay = layout.GetElementByName(L"LabelText");
	lay->SetControl(L"Static",0,L"Control Text: ",WS_VISIBLE|WS_CHILD,0);

	lay = layout.GetElementByName(L"BoxText");
	lay->SetControl(L"Edit",ID_EDIT_TEXT,L"",WS_VISIBLE|WS_CHILD|ES_MULTILINE|WS_VSCROLL|ES_AUTOVSCROLL|ES_WANTRETURN);

	lay = layout.GetElementByName(L"GroupStyle");
	lay->SetControl(L"Button",0,L"Window Style",WS_CHILD|WS_VISIBLE|BS_GROUPBOX);

	lay = layout.GetElementByName(L"ButtonAddStyle");
	lay->SetControl(L"Button",IDC_ADD_WS,L"Add To");

	lay = layout.GetElementByName(L"GroupCtrlStyle");
	lay->SetControl(L"Button",0,L"Control Style",WS_CHILD|WS_VISIBLE|BS_GROUPBOX);

	lay = layout.GetElementByName(L"ButtonAddCtrlStyle");
	lay->SetControl(L"Button",IDC_ADD_CS,L"Add To");

	lay = layout.GetElementByName(L"GroupControlStyle");
	lay->SetControl(L"Button",0,L"Control Style",WS_CHILD|WS_VISIBLE|BS_GROUPBOX);

	lay = layout.GetElementByName(L"ButtonDelStyle");
	lay->SetControl(L"Button",IDC_REMOVE_WS,L"Remove");

	lay = layout.GetElementByName(L"GroupExStyle");
	lay->SetControl(L"Button",0,L"Window ExStyle",WS_CHILD|WS_VISIBLE|BS_GROUPBOX);

	lay = layout.GetElementByName(L"ButtonAddExStyle");
	lay->SetControl(L"Button",IDC_ADD_ES,L"Add To");

	lay = layout.GetElementByName(L"GroupCtrlExStyle");
	lay->SetControl(L"Button",0,L"Control ExStyle",WS_CHILD|WS_VISIBLE|BS_GROUPBOX);

	lay = layout.GetElementByName(L"ButtonDelExStyle");
	lay->SetControl(L"Button",IDC_REMOVE_ES,L"Remove");

	lay = layout.GetElementByName(L"ButtonOK");
	lay->SetControl(L"Button",IDOK,L"OK");

	lay = layout.GetElementByName(L"ButtonCancel");
	lay->SetControl(L"Button",IDCANCEL,L"Cancel");

	layout.Refresh();
}
void ControlProperty::addStyle(base::wnd::ListBox& src,base::wnd::ListBox& dst){
	int index = src.GetSelItem();
	if(index<0){
		::MessageBox(_Handle,L"选定一个要添加的项",L"Warning",MB_ICONWARNING);
		return;
	}
	base::String str(256);
	src.GetText(index,str);
	str.Realize();
	if(-1!=dst.FindExactString(str)){
		::MessageBox(_Handle,L"这个项已经在列表中",L"Warning",MB_ICONWARNING);
		return;
	}
	dst.AddString(str);
}
void ControlProperty::delStyle(base::wnd::ListBox& src){
	int index = src.GetSelItem();
	if(index<0){
		::MessageBox(_Handle,L"选定一个要删除的项",L"Warning",MB_ICONWARNING);
		return;
	}
	src.DeleteString(index);
}
void ControlProperty::onCreate(void){
	base::wnd::SetWndIcon(_Handle,IDI_PROPERTY);
	if(!loadWndConst()){
		::MessageBox(_Handle,L"没有发现 wnd_const.xml 文件，无法加载控件常数",L"Error",MB_ICONERROR);	
		PostQuitMessage(0);
	}
	initUI();
	clickOk = false;
	initProperty();
}

void ControlProperty::setProperty(){
	clickOk = true;
	base::String str(256);
	//get class name
	int index = ctrlTypeCombo.GetCurSelect();
	if(index==0){
		CtrlParam.ClassName = L"";
		return;//no control
	}
	ctrlTypeCombo.GetItemText(index,str);
	CtrlParam.ClassName = str;
	//get id
	index = ctrlIdCombo.GetCurSelect();
	int id = 0;
	if(index>0){
		xmlIdMap.GetElement(index-1)->GetValueInt(id,10);
	}
	CtrlParam.Id = id;
	//get text
	str = L"";
	GetCtrlText(ID_EDIT_TEXT,str);
	CtrlParam.Text = str;
	//get Style
	CtrlParam.Style = getStyleFromList();
	//get ExStyle
	CtrlParam.ExStyle = getExStyleFromList();
}
DWORD ControlProperty::getStyleFromList(){
	base::String str(256);
	DWORD style = 0;
	int index = ctrlStyleList.GetCount();
	for(int i=0;i<index;i++){
		str = L"";
		ctrlStyleList.GetText(i,str);
		str.Realize();
		base::Xml* xmlV = xmlStyleMap->GetNode(str);
		if(xmlV==0) xmlV = xmlSelMap->GetNode(str);
		if(xmlV==0) continue;//这是不会发生的。
		int id = 0;
		if(!xmlV->GetValueInt(id)){
			_ASSERT(0);
		}
		style |= id;
	}
	return style;
}
DWORD ControlProperty::getExStyleFromList(){
	base::String str(256);
	DWORD style = 0;
	int index = ctrlExStyleList.GetCount();
	for(int i=0;i<index;i++){
		str = L"";
		ctrlExStyleList.GetText(i,str);
		str.Realize();
		base::Xml* xmlV = xmlExStyleMap->GetNode(str);
		if(xmlV==0) continue;//这是不会发生的。
		int id = 0;
		xmlV->GetValueInt(id);
		style |= id;
	}
	return style;
}
bool ControlProperty::Create(HWND parent,base::wnd::ICtrl* pc){
	if(pc==0){
		CtrlParam.ClassName = L"";
	}else{
		base::wnd::CreateStruct* pcs = pc->Param;
		CtrlParam.Style = pcs->Style;
		CtrlParam.ExStyle = pcs->ExStyle;
		CtrlParam.ClassName = pcs->ClassName;
		CtrlParam.Id = pcs->Identity;
		CtrlParam.Text = pcs->Title;
	}
	base::wnd::ModelDialog::Create(parent);
	return clickOk;
}
void ControlProperty::initProperty(){
	for(uint i=0;i<xmlUserType.Count();i++){
		ctrlNameCombo.AddString(xmlUserType.Element(i)->GetName());
	}
	if(CtrlParam.ClassName.IsNull()){
		disableControl();
		return;
	}
	//设置控件类
	int index = ctrlTypeCombo.FindExactString(CtrlParam.ClassName);
	if(index<=0){
		disableControl();
		return;
	}
	ctrlTypeCombo.SetCurSelect(index);
	refreshCtrlStyle();
	//在xmlIdMap里查找这个ID
	for(uint i=0;i<xmlIdMap.Count();i++){
		int id;
		xmlIdMap.Element(i)->GetValueInt(id,10);
		if((WORD)id==CtrlParam.Id){
			ctrlIdCombo.SetCurSelect(i+1);
			break;
		}
	}
	
	SetCtrlText(ID_EDIT_TEXT,CtrlParam.Text);
	//在 xmlSelMap 和 xmlStyleMap 里查找Style并且初始化在列表里;
	initListBoxByMap(ctrlStyleList,xmlSelMap,CtrlParam.Style);
	initListBoxByMap(ctrlStyleList,xmlStyleMap,CtrlParam.Style);
	initListBoxByMap(ctrlExStyleList,xmlExStyleMap,CtrlParam.ExStyle);
}
void ControlProperty::onCommand(WORD id){
	switch(id)
	{
	case IDOK:
		setProperty();
		Close();
		break;
	case IDCANCEL:
		Close();
		break;
	case IDC_ADD_WS:
		addStyle(ctrlWSList,ctrlStyleList);
		break;
	case IDC_ADD_ES:
		addStyle(ctrlWSEXList,ctrlExStyleList);
		break;
	case IDC_ADD_CS:
		addStyle(ctrlCSList,ctrlStyleList);
		break;
	case IDC_REMOVE_ES:
		delStyle(ctrlExStyleList);
		break;
	case IDC_REMOVE_WS:
		delStyle(ctrlStyleList);
		break;
	case ID_SAVE_TYPE:
		{
			base::String str(256);
			ctrlNameCombo.GetText(str);
			if(str.IsNull()){
				::MessageBox(_Handle,L"保存自定义控件类型需要指定一个名称",L"Error",MB_ICONERROR);
				::SetFocus(ctrlNameCombo);
				break;
			}
			if(str.Find(L"<")!=-1){
				::MessageBox(_Handle,L"名称不合法，请另指定一个名称",L"Error",MB_ICONERROR);
				::SetFocus(ctrlNameCombo);
				break;
			}
			bool newUserType = 1;
			if(ctrlNameCombo.FindExactString(str)!=-1){
				if(IDNO==::MessageBox(_Handle,L"此名称的自定义控件类型已经存在，你确定要改写它吗？",L"Warning",MB_ICONWARNING|MB_YESNO))
					break;
				else
					newUserType = 0;
			}
			int index = ctrlTypeCombo.GetCurSelect();
			if(index==0){
				::MessageBox(_Handle,L"控件类型为 None ，请指定一个有效的类名",L"Error",MB_ICONERROR);
				break;
			}
			if(newUserType) ctrlNameCombo.AddString(str);
			base::Xml* xml = xmlUserType.GetNode(str,1);
			ctrlTypeCombo.GetItemText(index,str);
			xml->SetProperty(L"Class",str);
			DWORD style = getStyleFromList();
			xml->SetPropertyInt(L"Style",style);
			DWORD exStyle = getExStyleFromList();
			xml->SetPropertyInt(L"ExStyle",exStyle);
			GetCtrlText(ID_EDIT_TEXT,str);
			xml->SetProperty(L"Text",str);
			xmlUserType.Save(USER_TYPE_XML);
		}
		break;
	}
}	
