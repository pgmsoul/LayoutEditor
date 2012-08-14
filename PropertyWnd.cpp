#include "stdafx.h"
#include "resource.h"
#include "Consol.h"
#include "LayoutWnd.h"
#include "PropertyWnd.h"
PropertyEditor::PropertyEditor(){
	editing = 0;
	Param->AddStyle(WS_DISABLED);
	_grid.SetBkgndColor(0xffffff);
	OnCreate.Add(this,&PropertyEditor::onCreate);
	_grid.OnMouse.Add(this,&PropertyEditor::onMouse);
	selector.OnCtrlMsg.Bind(this,&PropertyEditor::onLostFocus);
	editor.OnCtrlMsg.Bind(this,&PropertyEditor::onLostFocus);
}
void PropertyEditor::onCreate(){
	AddColumn(L"Name",120);
	AddColumn(L"Value",80);

	AddLine(L"Name||",L"|");
	AddLine(L"Style|Vertical|",L"|");
	AddLine(L"Margin_Left||",L"|");
	AddLine(L"Margin_Top||",L"|");
	AddLine(L"Margin_Right||",L"|");
	AddLine(L"Margin_Bottom||",L"|");
	AddLine(L"Length||",L"|");
	AddLine(L"Weight||",L"|");
	AddLine(L"Control|NONE|",L"|");
	for(int i=0;i<GetLineCount();i++)
		SetUnit(i,0,(LPCWSTR)INVALID_HANDLE_VALUE);

	selector.Param->RemoveStyle(WS_VISIBLE);
	//selector.Param->AddStyle(CBS_DROPDOWNLIST|CBS_DROPDOWN);
	selector.Create(_grid);
	selector.GetFont()->SetSize(16);
	selector.SetFont();
	selector.AddString(L"Vertical");
	selector.AddString(L"Horizontal");
	selector.AddString(L"Vert|Drag");
	selector.AddString(L"Hor|Drag");

	editor.Param->RemoveStyle(WS_VISIBLE);
	editor.Create(_grid);
	editor.GetFont()->SetSize(16);
	editor.SetFont();
}
void PropertyEditor::onMouse(base::wnd::Mouse* m){
	switch(m->Event)
	{
	case WM_LBUTTONDOWN:
		{
			//if(!shiftPress()) clearSelState();
			if(::GetFocus()!=_grid) SetFocus(_grid);
			uint index = (m->Y+_grid.Scr().y)/(_textH+2*_vMargin);
			if(index>=_line.Count()) break;

			_line.First();
			_line.Move(index);
			_Line* line = _line.Element();
			//_selLines.Add(line);
			int x = _grid.Scr().x + m->X;
			int right = 0;
			for(uint i=0;i<_colWidths.Count();i++){
				right += _colWidths[i];
				if(x<=right){
					//line->States[i] |= ITEM_STATE_SELECT;
					onSelect(index,i);
					break;
				}
			}
			_grid.Invalidate();
		}
		break;
	case WM_RBUTTONUP:
		{
			/*POINT pt;
			pt.x = m->X;
			pt.y = m->Y;
			::ClientToScreen(_grid,&pt);
			_rMenu.Popup((short)pt.x,(short)pt.y,_grid);*/
		}
		break;
	}
}

void PropertyEditor::onSelect(int line,int col){
	if(col!=1) return;//col = 1;
	int lineH = _textH + 2*_hMargin;
	int y = lineH*line - _grid.Scr().y;
	int x = -_grid.Scr().x;
	for(int i=0;i<col;i++){
		x += _colWidths[i];
	}
	LPCWSTR str;
	Unit* pu = GetUnit(line,col);
	if(pu) str = pu->Text;
	else str = 0;
	editLine = line;
	editing = true;
	if(line==1){
		selector.SetLocation(x,y);
		selector.SetSize(_colWidths[col],lineH);
		selector.SelectString(str);
		ShowWindow(selector,SW_SHOW);
		SetFocus(selector);
	}else if(line==8){
		OnChanged(8,0,0);
	}else{
		editor.SetLocation(x,y);
		editor.SetSize(_colWidths[col],lineH);
		editor.SetText(str);
		base::Twain16 r(0,-1);
		editor.SetSelect(r);
		ShowWindow(editor,SW_SHOW);
		SetFocus(editor);
	}
	//PrintD(L"%d,%d",line,col);
}
void PropertyEditor::RefreshValue(){
	if(!editing) return;
	base::String str;
	if(editLine==1){
		selector.GetText(str);
		if(OnChanged(editLine,str,selector.GetCurSelect())){
			ShowWindow(selector,SW_HIDE);
			editing = false;
		}else{
			SetFocus(selector);
		}
	}else if(editLine==8){
		editing = false;
	}else{
		editor.GetText(str);
		if(OnChanged(editLine,str,-1)){
			ShowWindow(editor,SW_HIDE);
			editing = false;
		}else{
			SetFocus(editor);
		}
	}
}
void PropertyEditor::onLostFocus(base::wnd::CtrlMsg* cm){
	if(cm->uMsg!=WM_COMMAND) return;
	if(!editing) return;
	WORD id = HIWORD(cm->wParam);
	base::String str;
	if(id==EN_KILLFOCUS){
		editor.GetText(str);
		if(OnChanged(editLine,str,-1)){
			ShowWindow(editor,SW_HIDE);
		}else{
			SetFocus(editor);
		}
		editing = false;
	}else if(id==CBN_KILLFOCUS){
		selector.GetText(str);
		if(OnChanged(editLine,str,selector.GetCurSelect())){
			ShowWindow(selector,SW_HIDE);
		}else{
			SetFocus(selector);
		}
		editing = false;
	}
}
bool PropertyEditor::SetProperty(LPCWSTR name,LPCWSTR value){
	int i = 0;
	for(_line.First();;_line.Move(1),i++){
		_Line* line = _line.Element();
		if(line==0) break;
		if(line->units[0].Text==name){
			SetUnit(i,1,value);
			return true;
		}
	}
	return false;
}
bool PropertyEditor::SetUnit(uint line,uint column,LPCWSTR txt){
	int color;
	if(column==0)
		color = 0xff0000;
	else
		color = 0xff00ff;
	return Table::SetUnit(line,column,txt,color);
}

LPCWSTR PropertyEditor::GetProperty(LPCWSTR name){
	int i = 0;
	for(_line.First();;_line.Move(1),i++){
		_Line* line = _line.Element();
		if(line==0) break;
		if(line->units[0].Text==name){
			return GetUnit(i,1)->Text;
		}
	}
	return 0;
}
bool proptWndGetSaveFileName(HWND hwnd,base::String& str,bool save){
	base::wnd::FileDialog fd;
	if(save){
		if(!fd.ShowSave(hwnd)) return false;
	}else{
		if(!fd.ShowOpen(hwnd)) return false;
	}
	str = fd.GetFileName();
	return true;
}
void PropertyWnd::onCommand(WORD id){
	switch(id)
	{
	case ID_LAYOUT_ADD:
		addItem();
		break;
	case ID_LAYOUT_DEL:
		delItem();
		break;
	case ID_LAYOUT_LEFT:
		moveToLeft();
		break;
	case ID_LAYOUT_RIGHT:
		moveToRight();
		break;
	case ID_LAYOUT_UP:
		moveToUp();
		break;
	case ID_LAYOUT_DOWN:
		moveToDown();
		break;
	case ID_MENU_SAVE:
		{
			if(editFileName.IsNull()){
				base::wnd::FileDialog fd;
				fd.AddFilter(L"布局文件(layout)",L"*.layout");
				fd.SetFilterIndex(0);
				fd.DefaultExt = L"layout";
				if(!fd.ShowSave(_Handle)) break;
				editFileName = fd.GetFileName();
			}
			if(!layWnd.glLayout.Save(editFileName)){
				base::Warning(L"save");
			}
			base::String fp = editFileName;
			base::String fn = editFileName;
			base::FPToFileName(fn);
			fn.Insert(L"布局视图——",0);
			layWnd.SetText(fn);
			base::FPToParent(fp);
			base::FPLinkPath(fp,L"resource.h");
			ctrlWnd.residPath = fp;
			return;
		}
	case ID_MENU_OPEN:
		{
			base::wnd::FileDialog fd;
			fd.AddFilter(L"布局文件(layout)",L"*.layout");
			fd.SetFilterIndex(0);
			if(!fd.ShowOpen(_Handle)) break;
			loadFile(fd.GetFileName());
			break;
		}
	case ID_HELP_ABOUT:
		ShellExecute(0,L"open",L"http://www.laibacom.com",0,0,1);
		break;
	case ID_HELP_SHOW:
		ShellExecute(0,L"open",L"notepad.exe",L"readme.txt",0,1);
		break;
	case ID_MENU_EXIT:
		if(!close())
			Quit();
		return;
	case ID_MENU_NEW:
		newLayout();
		break;
	case ID_LAYOUT_COPY:
		copyToClipboard();
		break;
	case ID_LAYOUT_PASTE:
		pasteFromClipboard();
		break;
	default:
		return;
	}
	layWnd.glLayout.Refresh();
	layWnd.Invalidate();
}
#define CF_LAYOUT L"C014B7B0-F64E-45dc-9C27-79D3E784E64E"
void PropertyWnd::copyToClipboard(){
	if(0==getSelectItem()) return;
	if(!OpenClipboard(0)) return;
	base::Xml xml;
	selLay->ToXml(&xml);
	base::String str;
	xml.ToString(str);
	UINT cf_layout = ::RegisterClipboardFormat(CF_LAYOUT);
	EmptyClipboard();
	HGLOBAL hM = GlobalAlloc(GMEM_MOVEABLE,str.Length()*2+2);//不能用GMEM_FIXED。
	wchar_t* hg = (wchar_t*)GlobalLock(hM);
	hg[str.Length()] = 0;
	memcpy(hg,str.Handle(),str.Length()*2);
	GlobalUnlock(hM);
	SetClipboardData(cf_layout,hM);
	GlobalFree(hM);
	CloseClipboard();
}
void PropertyWnd::pasteFromClipboard(){
	if(0==getSelectItem()) return;
	if(!OpenClipboard(0)) return;
	UINT cf_layout = ::RegisterClipboardFormat(CF_LAYOUT);
	HGLOBAL hM = ::GetClipboardData(cf_layout);
	if(hM==NULL){
		PrintD(L"%s",L"hM is NULL");
		return;
	}
	size_t sz = GlobalSize(hM);
	void* mem = GlobalLock(hM);
	base::String str;
	str = (LPCWSTR)mem;
	GlobalUnlock(mem);
	CloseClipboard();
	base::Xml xml;
	xml.FromString(str);
	base::Layout* lay = new base::Layout;
	lay->FromXml(&xml);
	addItem(lay);
	//PrintD(L"%s",str.Handle());
}
void PropertyWnd::loadFile(LPCWSTR file){
	if(!layWnd.glLayout.Load(file)){
		::MessageBox(_Handle,L"读取布局文件过程中出现错误",file,MB_ICONERROR);
		editFileName = L"";
	}else
		editFileName = file;
	layWnd.glLayout.Refresh();
	tree.Clear();
	createTreeFromLayout(&layWnd.glLayout,TVI_ROOT);
	layWnd.Invalidate();
	base::String fp = editFileName;
	base::String fn = editFileName;
	base::FPToFileName(fn);
	fn.Insert(L"布局视图——",0);
	layWnd.SetText(fn);
	base::FPToParent(fp);
	fp += L"resource.h";
	ctrlWnd.residPath = fp;
}
void PropertyWnd::createTreeFromLayout(base::Layout* lay,HTREEITEM parent,HTREEITEM insertAfter){
	HTREEITEM item = tree.AddItem(parent,lay->Name,0,0,insertAfter);
	tree.SetItemData(item,(LPARAM)lay);
	for(uint i=0;i<lay->Count();i++){
		createTreeFromLayout(lay->Element(i),item);
	}
}
void PropertyWnd::getUniqName(base::String& name){
	name = L"Layout";
	int i = 1;
	while(!checkNameUniq(&layWnd.glLayout,name)){
		name.Format(L"Layout%d",i);
		i++;
	}
}
bool PropertyWnd::checkNameUniq(base::Layout* lay,LPCWSTR name){
	if(base::WcsEqual(lay->Name,name)) return false;
	for(uint i=0;i<lay->Count();i++){
		if(!checkNameUniq(lay->Element(i),name)) return false;
	}
	return true;
}
void PropertyWnd::moveTreeItem(HTREEITEM item,HTREEITEM parent,HTREEITEM pos){
	HTREEITEM newItem = copyTreeItem(item,parent,pos);

	base::Layout* lay = 0;
	tree.GetItemData(item,(LPARAM*)&lay);
	base::Layout* oldParent = lay->GetParent();
	int oldIndex = oldParent->IndexOf(lay);
	base::Layout* newParent = 0;
	tree.GetItemData(parent,(LPARAM*)&newParent);
	oldParent->RemoveOut(oldIndex);
	HTREEITEM sItem = tree.GetItem(TVGN_CHILD,parent);
	int index = 0;
	while(sItem!=newItem){
		sItem = tree.GetItem(TVGN_NEXT,sItem);
		index++;
	}
	//同一级调整位置向下时，因为移除了上面的项，index需要调整 1 .
	if(newParent==oldParent&&oldIndex<index){
		index--;
	}
	newParent->AddIn(lay,index);

	tree.SetItemData(newItem,(LPARAM)lay);
	copyChildItem(item,newItem);
	tree.SetSelectItem(newItem);
	tree.DeleteItem(item);
	tree.Expand(newItem,TVE_EXPAND);
}
void PropertyWnd::copyChildItem(HTREEITEM item,HTREEITEM newItem){
	HTREEITEM child = tree.GetItem(TVGN_CHILD,item);
	while(child){
		HTREEITEM cItem = copyTreeItem(child,newItem,TVI_LAST);
		copyChildItem(child,cItem);
		child = tree.GetItem(TVGN_NEXT,child);
	}
}
HTREEITEM PropertyWnd::copyTreeItem(HTREEITEM item,HTREEITEM parent,HTREEITEM pos){
	base::String str(256);
	tree.GetItemText(item,str);
	LPARAM param = 0;
	tree.GetItemData(item,&param);
	HTREEITEM addItem = tree.AddItem(parent,str,0,0,pos);
	tree.SetItemData(addItem,param);
	return addItem;
}
HTREEITEM PropertyWnd::getSelectItem(){
	HTREEITEM item = tree.GetSelectItem();
	if(item==0){
		::MessageBox(_Handle,L"需要选定插入点或操做项",L"Warn",MB_ICONERROR);
		return 0;
	}
	return item;
}
void PropertyWnd::renameLayout(base::Layout* lay,LPCWSTR suffix){
	base::String name = lay->Name;
	name += suffix;
	lay->Name = name;
	for(uint i=0;i<lay->Count();i++){
		renameLayout(lay->Element(i),suffix);
	}
}
void PropertyWnd::addItem(base::Layout* addLay){
	HTREEITEM item = getSelectItem();
	if(!item) return;

	renameLayout(addLay,L"副本");
	base::String name;
	getUniqName(name);
	HTREEITEM insertAfter,parent = tree.GetParent(item);
	int index;
	base::Layout* parentLay = 0;
	if(parent==0){
		parent = item;
		insertAfter = TVI_FIRST;
		index = 0;
		tree.GetItemData(parent,(LPARAM*)&parentLay);
	}else{
		insertAfter = item;
		tree.GetItemData(parent,(LPARAM*)&parentLay);
		base::Layout* afterLay = 0;
		tree.GetItemData(insertAfter,(LPARAM*)&afterLay);
		index = parentLay->IndexOf(afterLay);
		index++;
	}
	parentLay->AddIn(addLay,index);
	createTreeFromLayout(addLay,parent,insertAfter);

	layWnd.Invalidate();
}
void PropertyWnd::addItem(){
	HTREEITEM item = getSelectItem();
	if(!item) return;

	HTREEITEM addItem;
	base::String name;
	getUniqName(name);
	HTREEITEM parent = tree.GetParent(item);
	HTREEITEM insertAfter = TVI_FIRST;
	if(parent==0) parent = item;
	else insertAfter = item;
	addItem = tree.AddItem(parent,name,0,0,insertAfter);

	base::Layout* lay = 0;
	tree.GetItemData(item,(LPARAM*)&lay);
	base::Layout* parentLay = lay->GetParent();
	int index = 0;
	if(parentLay==0)
		parentLay = lay;
	else
		index = parentLay->IndexOf(lay) + 1;
	lay = parentLay->Add(index);
	lay->Name = name;

	tree.SetItemData(addItem,(LPARAM)lay);
	tree.SetSelectItem(addItem);
}
void PropertyWnd::delItem(){
	HTREEITEM item = getSelectItem();
	if(item==0) return;
	HTREEITEM parent = tree.GetParent(item);
	if(parent==0){
		::MessageBox(_Handle,L"不能删除根节点",L"Warn",MB_ICONERROR);
		return;
	}

	base::Layout* lay = 0;
	tree.GetItemData(item,(LPARAM*)&lay);
	base::Layout* parentLay = lay->GetParent();
	parentLay->Delete(lay);

	tree.DeleteItem(item);
}
void PropertyWnd::moveToLeft(){
	HTREEITEM item = getSelectItem();
	if(!item) return;

	HTREEITEM parent = tree.GetParent(item);
	if(parent==0) return;
	HTREEITEM grand = tree.GetParent(parent);
	if(grand==0){
		::MessageBox(_Handle,L"顶级只能有一个布局",L"Warn",MB_ICONERROR);
		return;
	}
	moveTreeItem(item,grand,parent);
}
void PropertyWnd::moveToRight(){
	HTREEITEM item = getSelectItem();
	if(!item) return;

	HTREEITEM prev = tree.GetItem(TVGN_PREVIOUS,item);
	if(prev!=0)
		moveTreeItem(item,prev,TVI_LAST);
}
void PropertyWnd::moveToDown(){
	HTREEITEM item = getSelectItem();
	if(!item) return;

	HTREEITEM next = tree.GetItem(TVGN_NEXT,item);
	if(next!=0)
		moveTreeItem(item,tree.GetParent(next),next);
}
void PropertyWnd::moveToUp(){
	HTREEITEM item = getSelectItem();
	if(!item) return;

	HTREEITEM prev = tree.GetItem(TVGN_PREVIOUS,item);
	HTREEITEM insert = tree.GetItem(TVGN_PREVIOUS,prev);
	if(insert==0) insert = TVI_FIRST;
	if(prev!=0)
		moveTreeItem(item,tree.GetParent(prev),insert);
}
void PropertyWnd::createImageButton(base::wnd::Button& btn,int id,int res){
	base::gdi::Icon icon;
	btn.Param->AddStyle(BS_ICON);
	btn.Param->Identity = id;
	btn.Create(_Handle);
	icon.LoadIcon(res,base::Instance());
	btn.SendMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)icon.Detach());
}
void PropertyWnd::onCreate(){
	base::wnd::SetWndIcon(_Handle,IDI_EDIT);

	editor.Create(_Handle);
	editor.SetFontSize(16);

	btnAdd.Param->Title = L"插入";
	btnAdd.Param->Identity = ID_LAYOUT_ADD;
	btnAdd.Create(_Handle);
	
	btnDel.Param->Title = L"删除";
	btnDel.Param->Identity = ID_LAYOUT_DEL;
	btnDel.Create(_Handle);

	btnCopy.Param->Title = L"复制";
	btnCopy.Param->Identity = ID_LAYOUT_COPY;
	btnCopy.Create(_Handle);

	btnPaste.Param->Title = L"粘贴";
	btnPaste.Param->Identity = ID_LAYOUT_PASTE;
	btnPaste.Create(_Handle);

	createImageButton(arrowBtn[0],ID_LAYOUT_LEFT,IDI_LEFT);
	createImageButton(arrowBtn[1],ID_LAYOUT_RIGHT,IDI_RIGHT);
	createImageButton(arrowBtn[2],ID_LAYOUT_UP,IDI_UP);
	createImageButton(arrowBtn[3],ID_LAYOUT_DOWN,IDI_DOWN);

	base::Layout* lay;
	panel.Param.SetHorizontal();
	//add right base::Layout
	base::Layout* rlay = new base::Layout();
	rlay->Param.SetVertical();
	rlay->Param.SetDimen(230,0);
	rlay->Param.SetMargin(0,10,10,10);
	panel.AddIn(rlay);
	//add right top base::Layout
	base::Layout* topLay = new base::Layout();
	topLay->Param.SetHorizontal();
	topLay->Param.SetDimen(50,0);
	rlay->AddIn(topLay);

	lay = new base::Layout();
	lay->Param.SetMargin(0,0,0,10);
	topLay->AddIn(lay);
	lay->SetControl(btnAdd);

	lay = new base::Layout();
	lay->Param.SetMargin(10,0,0,10);
	topLay->AddIn(lay);
	lay->SetControl(btnDel);
	//copy button
	lay = new base::Layout();
	lay->Param.SetMargin(10,0,0,10);
	topLay->AddIn(lay);
	lay->SetControl(btnCopy);
	//paste button
	lay = new base::Layout();
	lay->Param.SetMargin(10,0,0,10);
	topLay->AddIn(lay);
	lay->SetControl(btnPaste);

	base::Layout* midLay = new base::Layout();
	midLay->Param.SetHorizontal();
	midLay->Param.SetDimen(42,0);
	rlay->AddIn(midLay);

	lay = new base::Layout();
	lay->Param.SetMargin(0,0,10,0);
	midLay->AddIn(lay);
	lay->SetControl(*arrowBtn);

	lay = new base::Layout();
	lay->Param.SetMargin(0,0,10,0);
	midLay->AddIn(lay);
	lay->SetControl(*(arrowBtn+1));

	lay = new base::Layout();
	lay->Param.SetMargin(0,0,10,0);
	midLay->AddIn(lay);
	lay->SetControl(*(arrowBtn+2));

	lay = new base::Layout();
	lay->Param.SetMargin(0,0,0,0);
	midLay->AddIn(lay);
	lay->SetControl(*(arrowBtn+3));

	lay = new base::Layout();
	lay->Param.SetMargin(0,10,0,0);
	rlay->AddIn(lay);
	lay->SetControl(editor);

	layWnd.Create();

	treeIL.Create(32,32,ILC_COLOR32);
	base::gdi::Icon icon;
	icon.LoadIcon(IDI_LAYOUT,base::Instance());
	treeIL.AddIcon(icon);
	tree.Param->AddStyle(TVS_SHOWSELALWAYS);
	tree.Create(_Handle);
	tree.SetImageList(treeIL);
	LPCWSTR name = layWnd.glLayout.Name;
	HTREEITEM ht = tree.AddItem(0,name,0,0);
	tree.SetItemData(ht,(LPARAM)&layWnd.glLayout);
	tree.SetSelectItem(ht);
	//add left base::Layout
	lay = new base::Layout();
	lay->Param.SetMargin(10,10,10,10);
	lay->SetControl(tree);
	panel.AddIn(lay,0);
}
void PropertyWnd::onTreeNotify(base::wnd::CtrlMsg* cm){
	if(cm->uMsg!=WM_NOTIFY) return;
	LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)cm->lParam;
	switch(pnmtv->hdr.code)
	{
	case TVN_SELCHANGED:
		{
			editor.RefreshValue();
			base::Layout* lay = 0;
			tree.GetItemData(pnmtv->itemNew.hItem,(LPARAM*)&lay);
			selTree = pnmtv->itemNew.hItem;
			selLay = lay;
			layWnd.selLay = lay;
			if(lay==0) return;
			EnableWindow(editor,1);

			editor.SetUnit(0,1,lay->Name);
			base::LayoutParam& lp = lay->Param;
			base::String num;
			if(lp.IsHorizontal()){
				if(lp.CanDrag())
					num = L"Hor|Drag";
				else
					num = L"Horizontal";
			}else{
				if(lp.CanDrag())
					num = L"Vert|Drag";
				else
					num = L"Vertical";
			}
			editor.SetUnit(1,1,num);
			num.Format(L"%d",lp.Margin.left);
			editor.SetUnit(2,1,num);
			num.Format(L"%d",lp.Margin.top);
			editor.SetUnit(3,1,num);
			num.Format(L"%d",lp.Margin.right);
			editor.SetUnit(4,1,num);
			num.Format(L"%d",lp.Margin.bottom);
			editor.SetUnit(5,1,num);
			num.Format(L"%d",lp.Length);
			editor.SetUnit(6,1,num);
			num.Format(L"%f",lp.Weight);
			editor.SetUnit(7,1,num);
			base::wnd::ICtrl* ctrl = lay->GetControl();
			if(ctrl==0)
				num = L"None";
			else
				num = ctrl->Param->ClassName;
			editor.SetUnit(8,1,num);
			editor.Invalidate();
			layWnd.Invalidate();
			break;
		}
	}
}
void setIntValue(int& setm,base::String& str){
	setm = str.ToInt32(10);
	str.Format(L"%d",setm);
}
void setFloatValue(float& setm,base::String& str){
	setm = (float)str.ToFloat();
	str.Format(L"%f",setm);
}
bool PropertyWnd::onValueRefresh(int line,LPCWSTR val,int index){
	if(selLay==0) return 0;
	base::String str = val;
	base::LayoutParam& lp = selLay->Param;
	switch(line)
	{
	case 0:
		selLay->Name = str;
		tree.SetItemText(selTree,str);
		break;
	case 1:
		switch(index)
		{
		case 0:
			lp.SetVertical();
			lp.SetCanDrag(false);
			break;
		case 1: 
			lp.SetHorizontal();
			lp.SetCanDrag(false);
			break;
		case 2:
			lp.SetVertical();
			lp.SetCanDrag();
			break;
		case 3:
			lp.SetHorizontal();
			lp.SetCanDrag();
		break;
		}
		break;
	case 2:
		setIntValue(lp.Margin.left,str);
		break;
	case 3:
		setIntValue(lp.Margin.top,str);
		break;
	case 4:
		setIntValue(lp.Margin.right,str);
		break;
	case 5:
		setIntValue(lp.Margin.bottom,str);
		break;
	case 6:
		setIntValue(lp.Length,str);
		break;
	case 7:
		setFloatValue(lp.Weight,str);
		break;
	case 8://control type
		if(!ctrlWnd.Create(_Handle,selLay->GetControl())) return false;
		if(ctrlWnd.CtrlParam.ClassName.IsNull())
			str = L"NONE";
		else
			str = ctrlWnd.CtrlParam.ClassName;
		selLay->SetControl(ctrlWnd.CtrlParam.ClassName,ctrlWnd.CtrlParam.Id,ctrlWnd.CtrlParam.Text,ctrlWnd.CtrlParam.Style,ctrlWnd.CtrlParam.ExStyle);
		break;
	}
	selLay->Refresh(layWnd);
	editor.SetUnit(line,1,str);
	layWnd.Invalidate();
	return true;
}
void PropertyWnd::setControlParam(int line,base::String& str,int type){
	/*WORD id;
	if(line==9){
		id = LOWORD(str.ToInt32(10));
	}else{
		base::String num;
		num = editor.GetLine(9,1);
		id = LOWORD(num.ToInt32(10));
	}
	LPCWSTR text;
	if(line==10){
		text = str;
	}else{
		text = editor.GetLine(10,1);
	}
	selLay->SetControlParam((base::ControlParam::ControlType)type,id,text);*/
}
void PropertyWnd::onClose(base::wnd::Message* msg){
	if(close()) msg->Result = 0;
}
bool PropertyWnd::close(){
	if(editFileName.IsNull()){
		/*int r = ::MessageBox(_Handle,L"文件还未保存，要保存文件吗？",L"Warn",MB_ICONWARNING|MB_YESNOCANCEL);
		if(r==IDYES){
			onCommand(ID_MENU_SAVE);
		}else if(r==IDCANCEL){
			return true;
		}*/
	}else{
		layWnd.glLayout.Save(editFileName);
	}
	return false;
}
void PropertyWnd::newLayout(){
	if(editFileName.IsNull()){
		int r = ::MessageBox(_Handle,L"文件还未保存，要保存文件吗？",L"Warn",MB_ICONWARNING|MB_YESNOCANCEL);
		if(r==IDYES){
			onCommand(ID_MENU_SAVE);
		}else if(r==IDCANCEL){
			return ;
		}
	}else{
		layWnd.glLayout.Save(editFileName);
	}
	layWnd.glLayout.Reset();
	editFileName = L"";
	EnableWindow(editor,0);
	tree.Clear();
	createTreeFromLayout(&layWnd.glLayout,TVI_ROOT);
	HTREEITEM root = tree.GetRoot();
	if(root) tree.SetSelectItem(root);
	layWnd.Invalidate();
	base::String fn = L"布局视图——";
	layWnd.SetText(fn);
}
void PropertyWnd::Create(LPCWSTR param,int nCmdShow){
	base::wnd::Frame::Create(0,nCmdShow);
	base::String layf = param,rf;
	if(layf.IsNull()) return;
	for(uint i=0;i<layf.Length();i++){
		if(layf[i]=='\"'||layf[i]=='\'') continue;
		rf.Insert(layf[i]);
	}
	if(rf.IsNull()) return;
	loadFile(rf);
}
PropertyWnd::PropertyWnd():selLay(0),selTree(0){
	//Param->AddExStyle(WS_EX_COMPOSITED);
	Param->Title = L"布局编辑器 1.0";
	base::Menu	menu;
	menu.Load(IDR_MENU_MAIN);
	Param->Menu = menu.Detach();
	OnCreate.Add(this,&PropertyWnd::onCreate);
	OnCommand.Add(this,&PropertyWnd::onCommand);
	GetMsgHook(WM_CLOSE)->Add(this,&PropertyWnd::onClose);
	tree.OnCtrlMsg.Bind(this,&PropertyWnd::onTreeNotify);
	editor.OnChanged.Bind(this,&PropertyWnd::onValueRefresh);

	panel.SetContainer(this);

	base::wnd::WPRRegisterWnd(this,L"property_wnd");
}
