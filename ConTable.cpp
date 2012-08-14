#include "stdafx.h"
#include "ConTable.h"
//ConTable.CPP
#define ITEM_STATE_SELECT	1
#define TS_LASTCOL_AUTOSIZE	1

#define IDC_COPY_SELECT	0x3263
#define IDC_SELECT_ALL	0x3264
#define IDC_CLEAR_SELECT 0x3265
#define IDC_CLEAR_TEXT 0x3266
//header control height,this value is a const.
static int headerHeight = 37;
//ConTable
void ConTable::onCreate(){
	header.Create(_Handle);
	//set headerHeight const value.
	headerHeight = header.AdjustSize(_Handle);
	table.Create(_Handle);
}
void ConTable::onSize(base::Twain16 cs){
	table.SetDimension(0,headerHeight,cs.x,cs.y-headerHeight);
	if(_style&TS_LASTCOL_AUTOSIZE){
		int index = _colWidths.Count() - 1;
		if(index<0) return;
		int x = 0;
		for(int i=0;i<index;i++){
			x += _colWidths[i];
		}
		_colWidths[index] = cs.x - x;
		header.SetItemWidth(index,_colWidths[index]);
	}
}
void ConTable::onTableSize(base::Twain16 cs){
	refreshHeaderSize();
}
void ConTable::refreshHeaderSize(){
	base::Twain16 cs;
	table.GetClientSize(cs);
	base::Rect r;
	table.GetRect(r);
	int width = table.MinSize().x;
	if(width>cs.x)//have vertical scroll bar.
		width += r.Width()-cs.x;
	if(width<r.Width())
		width = r.Width();
	header.AdjustSize(_Handle,-table.Scr().x,0,width);
}
void ConTable::onDraw(base::gdi::DC* dc){
	base::Twain16 cs;
	table.GetClientSize(cs);
	//dc->SetBkColor(RGB(49,106,197));
	dc->SetBkTransparent();
	_Font.Select(dc->Handle());
	int i = 0;
	base::Twain scr = table.Scr();
	for(_line.First();;_line.Move(1),i++){
		_Line* line = _line.Element();
		if(line==0) break;
		if((i*_lineH+_lineH-scr.y)<0) continue;
		if((int)(i*_lineH-scr.y)>(int)cs.y) break;
		dc->SetTextColor(line->Color);
		int x = -scr.x;
		int y = i*_lineH - scr.y;
		for(uint j=0;j<line->Texts.Count();j++){
			base::Rect cr(x,y,x+_colWidths[j],y+_lineH);
			if(ITEM_STATE_SELECT&line->States[j]){
				::FillRect(*dc,cr,_selBkColor);
				//dc->SetTextColor()
			}
			::DrawText(dc->Handle(),line->Texts[j],line->Texts[j].Length(),cr,DT_SINGLELINE);
			x += _colWidths[j];
		}
	}
	_Font.Select(dc->Handle());
}
void ConTable::SetMaxCount(int lineCount){
	base::LocalCriticalSection lcs(cs);
	if(_maxCount==lineCount) return;
	_maxCount = lineCount;
	while((uint)_line.Count()>_maxCount)
	{
		_line.Delete(0);
	}
	table.Invalidate();
}
bool ConTable::SetColText(int index,LPCWSTR title){
	return header.SetItemText(index,title);
}
bool ConTable::SetColWidth(int index,int width){
	return header.SetItemWidth(index,width);
}
bool ConTable::SetColumn(int index,LPCWSTR title,int width){
	return header.SetItem(index,title,width);
}
bool shiftPress(){
	int state = (USHORT)GetAsyncKeyState(VK_SHIFT);
	return 0!=(state&0x8000);
}
void ConTable::clearSelState(){
	for(uint i=0;i<_selLines.Count();i++){
		_Line* line = _selLines.Element(i);
		for(uint j=0;j<line->States.Count();j++){
			line->States[j] |= ITEM_STATE_SELECT;
			line->States[j] -= ITEM_STATE_SELECT;
		}
	}
	_selLines.Clear();
}
void ConTable::onMouse(base::wnd::Mouse* m)
{
	switch(m->Event)
	{
	case WM_LBUTTONDOWN:
		{
			if(!shiftPress()) clearSelState();
			if(::GetFocus()!=_Handle) SetFocus(_Handle);
			uint index = (m->Y+table.Scr().y)/_lineH;
			if(index>=_line.Count()) break;

			_line.First();
			_line.Move(index);
			_Line* line = _line.Element();
			_selLines.Add(line);
			int x = table.Scr().x + m->X;
			int right = 0;
			for(uint i=0;i<_colWidths.Count();i++){
				right += _colWidths[i];
				if(x<=right){
					line->States[i] |= ITEM_STATE_SELECT;
					OnSelect(index,i);
					break;
				}
			}
			table.Invalidate();
		}
		break;
	case WM_RBUTTONUP:{
		POINT pt;
		pt.x = m->X;
		pt.y = m->Y;
		::ClientToScreen(table,&pt);
		_rMenu.Popup((short)pt.x,(short)pt.y,table);
					  }
					  break;
	}
}
void SetClipboardText(base::String& str)
{
	if(!OpenClipboard(0)) return;
	EmptyClipboard();
	HGLOBAL hM = GlobalAlloc(GMEM_MOVEABLE,str.Length()*2+2);//不能用GMEM_FIXED。
	wchar_t* hg = (wchar_t*)GlobalLock(hM);
	hg[str.Length()] = 0;
	memcpy(hg,str.Handle(),str.Length()*2);
	GlobalUnlock(hM);
	SetClipboardData(CF_UNICODETEXT,hM);
	GlobalFree(hM);
	CloseClipboard();
}
void ConTable::onCommand(WORD id)
{
	base::String str;
	switch(id)
	{
	case IDC_COPY_SELECT:{
		base::String str;
		for(int i=0;i<(int)_selLines.Count();i++){
			_Line* line = _selLines[i];
			bool newLine = true;
			base::String sLine;
			for(uint i=0;i<line->States.Count();i++){
				if(line->States[i]&ITEM_STATE_SELECT){
					LPCWSTR s;
					if(line->Texts.Count()>i) s = line->Texts[i];
					else s = L"";
					if(!sLine.IsNull())
						sLine += L"\t";
					sLine += s;
				}
			}
			if(!sLine.IsNull()){
				if(!str.IsNull())
					str += L"\r\n";
				str += sLine;
			}
		}
		SetClipboardText(str);
						 }
						 break;
	case IDC_CLEAR_TEXT:
		Clear();
		break;
	case IDC_CLEAR_SELECT:
		for(int i=0;i<(int)_selLines.Count();i++){
			_Line* line = _selLines[i];
			for(uint i=0;i<line->States.Count();i++){
				line->States[i] |= ITEM_STATE_SELECT;
				line->States[i] -= ITEM_STATE_SELECT;
			}
		}
		_selLines.Clear();
		table.Invalidate();
		break;
	case IDC_SELECT_ALL:
		_selLines.Clear();
		for(_line.First();;_line.Move(1)){
			_Line* line = _line.Element();
			if(line==0) break;
			for(uint i=0;i<line->States.Count();i++){
				line->States[i] |= ITEM_STATE_SELECT;
			}
			_selLines.Add(line);
		}
		table.Invalidate();
		break;
	}
}
void ConTable::onNotify(base::wnd::CtrlMsg* cm){
	NMHEADER* pnh = (NMHEADER*)cm->lParam;
	if(pnh->hdr.code==HDN_TRACK){
		_colWidths[pnh->iItem] = pnh->pitem->cxy;
		int width = 0;
		for(uint i=0;i<_colWidths.Count();i++){
			width += _colWidths[i];
		}
		table.SetMinSize(width,-1);
		refreshHeaderSize();
		table.Invalidate();
	}else if(pnh->hdr.code==HDN_ITEMCLICK){
		if(!shiftPress()) clearSelState();
		for(_line.First();;_line.Move(1)){
			_Line* line = _line.Element();
			if(line==0) break;
			line->States[pnh->iItem] |= ITEM_STATE_SELECT;
			_selLines.Add(line);
		}
		table.Invalidate();
	}
}
int ConTable::AddColumn(LPCWSTR title,int width,int index){
	base::LocalCriticalSection lcs(cs);
	index = header.AddItem(title,width,index);
	if(index<0) return -1;
	_colWidths.Add(width,index);
	for(_line.First();;_line.Move(1)){
		_Line* line = _line.Element();
		if(line==0) break;
		if((int)line->Texts.Count()>index)
			line->Texts.Add(0,index);
		line->States.Add(0,index);
	}
	width = 0;
	for(uint i=0;i<_colWidths.Count();i++){
		width += _colWidths[i];
	}
	table.SetMinSize(width,-1);
	refreshHeaderSize();		
	table.Invalidate();

	return index;
}
int ConTable::AddLine(int index){
	base::LocalCriticalSection lcs(cs);
	_Line* line;
	if(index==-1){
		line = _line.Add();
		index = _line.Count()-1;
	}else if(index==0){
		line = _line.Add(0);
		index = 0;
	}else{
		_line.First();
		_line.Move(index);
		line = _line.Add(1);
	}
	for(uint i=0;i<_colWidths.Count();i++)
		line->States.Add(0);
	if(_maxCount<(uint)_line.Count()) _line.Delete(0);
	table.SetMinSize(-1,_lineH*_line.Count());
	return index;
}
int ConTable::AddLine(LPCWSTR str,LPCWSTR sep,int index){
	base::LocalCriticalSection lcs(cs);
	_Line* line;
	if(index==-1){
		line = _line.Add();
		index = _line.Count()-1;
	}else if(index==0){
		line = _line.Add(0);
		index = 0;
	}else{
		_line.First();
		_line.Move(index);
		line = _line.Add(1);
	}
	for(uint i=0;i<_colWidths.Count();i++)
		line->States.Add(0);
	base::StringList sl;
	sl.SplitStandString(str,sep);
	for(uint i=0;i<sl.Count();i++){
		if(i>=_colWidths.Count()) break;
		if(line->Texts.Count()<=i)
			line->Texts.Add(sl[i]);
		else
			line->Texts[i] = sl[i];
	}
	if(_maxCount<(uint)_line.Count()) _line.Delete(0);
	table.SetMinSize(-1,_lineH*_line.Count());
	return index;
}
bool ConTable::SetLine(int iline,int column,LPCWSTR txt){
	base::LocalCriticalSection lcs(cs);
	if((int)_line.Count()<=iline) return false;
	int nCol = header.GetCount();
	if(column>=nCol) return false;
	_line.First();
	_line.Move(iline);
	_Line* line = _line.Element();
	while((int)line->Texts.Count()<=column){
		line->Texts.Add(0);
	}
	line->Texts.Element(column) = txt;
	return true;
}
void ConTable::Clear(){
	base::LocalCriticalSection lcs(cs);
	_selLines.Clear();
	_line.Clear();
	table.SetMinSize(-1,_lineH*_line.Count());
	table.Invalidate();
}
bool ConTable::DelColumn(int index){
	base::LocalCriticalSection lcs(cs);
	if(!header.DelItem(index)) return 0;
	_colWidths.Delete(index);
	for(_line.First();;_line.Move(1)){
		_Line* line = _line.Element();
		if(line==0) break;
		line->Texts.Delete(index);
		line->States.Delete(index);
	}
	refreshHeaderSize();
	table.Invalidate();
	return 1;
}
bool ConTable::DelLine(int index){
	base::LocalCriticalSection lcs(cs);
	if(index>=(int)_line.Count()) return 0;
	_line.First();
	_line.Move(index);
	_selLines.RemoveValue(_line.Element());
	_line.Delete();
	table.SetMinSize(-1,_lineH*_line.Count());
	table.Invalidate();
	return 1;
}
LPCWSTR ConTable::GetLine(int index,int col){
	base::LocalCriticalSection lcs(cs);
	if(index>=(int)_line.Count()) return 0;
	if(col>=(int)_colWidths.Count()) return 0;
	_line.First();
	_line.Move(index);
	_Line* line = _line.Element();
	return line->Texts[col];
}
void ConTable::onHScroll(base::wnd::Message* msg){
	header.SetLocation(-table.Scr().x,0);
}
void ConTable::SetFontSize(int size){
	_Font.LogFont()->lfHeight = size;
	_Font.Create();
	_lineH = size*4/3;
	table.SetMinSize(-1,_lineH*_line.Count());
}
void ConTable::ScrollToLine(int index){
	if(index<0){
		table.ScrollEnd(0,1);
		return;
	}
	base::Twain16 cs;
	table.GetClientSize(cs);
	int dy = (index+1)*_lineH - cs.y - table.Scr().y;
	table.Scroll(0,dy);
}
void ConTable::SetLineColor(int index,int color){
	_line.First();
	_line.Move(index);
	_Line* line = _line.Element();
	line->Color = color;
}
void ConTable::initMenu(){
	_rMenu.Create(1);
	_rMenu.AddItem(-1,L"复制选择文本",IDC_COPY_SELECT);
	_rMenu.AddSplit(-1);
	_rMenu.AddItem(-1,L"全选",IDC_SELECT_ALL);
	_rMenu.AddItem(-1,L"清除选择",IDC_CLEAR_SELECT);
	_rMenu.AddSplit(-1);
	_rMenu.AddItem(-1,L"清除所有文本",IDC_CLEAR_TEXT);
}

void ConTable::SetBkgndColor(COLORREF color,bool redraw){
	table.SetBkgndColor(color,redraw);
}
ConTable::ConTable():_maxCount(200),_style(TS_LASTCOL_AUTOSIZE){
	table.SetBkgndColor(0x99ddff);
	_Font.LogFont()->lfHeight = 12;
	_Font.Create();
	_lineH = _Font.LogFont()->lfHeight*4/3;
	_selBkColor.Create(RGB(51,153,255));

	Param->ClassStyle -= CS_DBLCLKS;
	//Param->AddStyle(WS_DLGFRAME);
	table.Param->ExStyle = (WS_EX_WINDOWEDGE);

	table.OnDraw.Add(this,&ConTable::onDraw);
	table.OnMouse.Add(this,&ConTable::onMouse);
	table.OnCommand.Add(this,&ConTable::onCommand);
	OnCreate.Add(this,&ConTable::onCreate);
	OnSize.Add(this,&ConTable::onSize);
	table.OnSize.Add(this,&ConTable::onTableSize);
	table.GetMsgHook(WM_HSCROLL)->Add(this,&ConTable::onHScroll);
	//header.OnNotify.Bind(this,&ConTable::onNotify);

	initMenu();
}
