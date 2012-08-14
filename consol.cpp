#include "stdafx.h"
#include "consol.h"
using namespace base;
#define ITEM_STATE_SELECT	1
#define TS_LASTCOL_AUTOSIZE	1

#define IDC_COPY_SELECT		0x3263
#define IDC_SELECT_ALL		0x3264
#define IDC_CLEAR_SELECT	0x3265
#define IDC_CLEAR_TEXT		0x3266

Table::Table():_maxCount(-1),_style(TS_LASTCOL_AUTOSIZE){
	//header control height,this value is a const.
	_headerHeight = 37;
	_hMargin = 5;
	_vMargin = 5;
	_gridColor.Create(RGB(60,60,60));

	_grid.OnDraw.Add(this,&Table::onDraw);

	_grid.SetBkgndColor(0);
	_Font.LogFont()->lfHeight = 12;
	_Font.Create();
	_textH = _Font.LogFont()->lfHeight;
	_vMargin = _textH/3;

	Param->ClassStyle -= CS_DBLCLKS;
	OnCreate.Add(this,&Table::onCreate);
	OnSize.Add(this,&Table::onSize);

	//table.OnMouse.Add(this,&ConTable::onMouse);
	//table.OnCommand.Add(this,&ConTable::onCommand);
	_grid.GetMsgHook(WM_HSCROLL)->Add(this,&Table::onHScroll);
	_header.OnCtrlMsg.Bind(this,&Table::onNotify);
}
void Table::onCreate(){
	_header.Create(_Handle);
	//set headerHeight const value.
	_headerHeight = _header.AdjustSize(_Handle);
	_grid.Create(_Handle);
}
void Table::onHScroll(base::wnd::Message* msg){
	_header.SetLocation(-_grid.Scr().x,0);
}
void Table::onNotify(base::wnd::CtrlMsg* cm){
	if(cm->uMsg!=WM_NOTIFY) return;
	NMHEADER* pnh = (NMHEADER*)cm->lParam;
	//base::Print(L"%d",pnh->hdr.code);
	if(pnh->hdr.code==HDN_TRACK){
		_colWidths[pnh->iItem] = pnh->pitem->cxy;
		int width = 0;
		for(uint i=0;i<_colWidths.Count();i++){
			width += _colWidths[i];
		}
		_newMinWidth = width;
	}else if(pnh->hdr.code==HDN_ITEMCLICK){//-322
		/*if(!shiftPress()) clearSelState();
		for(_line.First();;_line.Move(1)){
			_Line* line = _line.Element();
			if(line==0) break;
			line->States[pnh->iItem] |= ITEM_STATE_SELECT;
			_selLines.Add(line);
		}
		table.Invalidate();*/
	}else if(pnh->hdr.code==HDN_BEGINTRACK){
		int width = 0;
		for(uint i=0;i<_colWidths.Count();i++){
			width += _colWidths[i];
		}
		_newMinWidth = width;
	}else if(pnh->hdr.code==HDN_ENDTRACK){
		_grid.SetMinSize(_newMinWidth,-1);
		refreshHeaderSize();
		_grid.Invalidate();
	}
}
void Table::onSize(base::Twain16 cs){
	_grid.SetDimension(0,_headerHeight,cs.x,cs.y-_headerHeight);
	refreshHeaderSize();
	/*if(_style&TS_LASTCOL_AUTOSIZE){
		int index = _colWidths.Count() - 1;
		if(index<0) return;
		int x = 0;
		for(int i=0;i<index;i++){
			x += _colWidths[i];
		}
		_colWidths[index] = cs.x - x;
		_header.SetItemWidth(index,_colWidths[index]);
	}*/
}
void Table::refreshHeaderSize(){
	base::Twain16 cs;
	_grid.GetClientSize(cs);
	/*base::Rect r;
	_grid.GetRect(r);*/
	int width = _grid.MinSize().x;
	/*if(width>cs.x)//have horizontal scroll bar.
		width += r.Width()-cs.x;*/
	if(width<cs.x) width = cs.x;
	_header.AdjustSize(_Handle,-_grid.Scr().x,0,width);
}

void Table::onDraw(base::gdi::DC* dc){
	base::Twain16 cs;
	_grid.GetClientSize(cs);
	//dc->SetBkColor(RGB(49,106,197));
	dc->SetBkTransparent();
	_Font.Select(dc->Handle());
	int i = 0;
	base::Twain scr = _grid.Scr();
	int lineH = _vMargin*2 + _textH;
	_gridColor.Select(*dc);
	uint tw = 0;
	for(uint k=0;k<_colWidths.Count();k++){
		tw += _colWidths[k];
	}
	for(_line.First();;_line.Move(1),i++){
		_Line* line = _line.Element();
		if(line==0) break;
		if((i*lineH+_vMargin+lineH-scr.y)<0) continue;
		if((int)(i*lineH+_vMargin-scr.y)>(int)cs.y) break;
		int x = -scr.x,x1;
		int y = i*lineH - scr.y;
		for(uint j=0;j<line->units.Count();j++){
			Unit& unit = line->units[j];
			dc->SetTextColor(unit.Color);
			x1 = x+_colWidths[j]-_hMargin;
			base::Rect cr(x+_hMargin,y+_vMargin,x1,y+_textH+_vMargin);
			/*if(ITEM_STATE_SELECT&unit.State){
				::FillRect(*dc,cr,_selBkColor);
				//dc->SetTextColor()
			}*/
			bool bDraw = 1;
			if(OnDrawUnit.NotNull()){
				bDraw = OnDrawUnit(dc,cr,unit,i,j);
			}
			if(bDraw) ::DrawText(dc->Handle(),unit.Text,unit.Text.Length(),cr,DT_SINGLELINE);
			x += _colWidths[j];
		}
		if(i<(int)_line.Count()){
			dc->MoveTo(-scr.x,y+lineH);
			dc->LineTo(tw-scr.x,y+lineH);
		}
	}
	uint w = -scr.x;
	for(uint i=0;i<_colWidths.Count();i++){
		w += _colWidths[i];
		dc->MoveTo(w,0);
		dc->LineTo(w,lineH*_line.Count()-scr.y);
	}
/*
	dc->MoveTo(0,0);
	dc->LineTo(cs.x,0);
	dc->LineTo(cs.x,cs.y);
	dc->LineTo(0,cs.y);
	dc->LineTo(0,0);
*/
	_gridColor.Select(*dc);
	_Font.Select(dc->Handle());
}
void Table::refreshMinSize(){
	uint width = 0;
	for(uint i=0;i<_colWidths.Count();i++){
		width += _colWidths[i];
	}
	width += 2*_hMargin;
	int lineH = _vMargin*2 + _textH;
	_grid.SetMinSize(width,lineH*_line.Count());
	refreshHeaderSize();
}
void Table::SetMaxCount(uint lineCount){
	base::LocalCriticalSection lcs(_cs);
	if(_maxCount==lineCount) return;
	_maxCount = lineCount;
	int delCount = _line.Count() - _maxCount;
	while((uint)_line.Count()>_maxCount){
		_line.Delete(0);
	}
	if(delCount>0) _grid.Invalidate();
}
int Table::AddColumn(LPCWSTR title,uint width,uint index){
	base::LocalCriticalSection lcs(_cs);
	if(width>0xffff) return -1;
	width += 2*_hMargin;
	index = _header.AddItem(title,width,index);
	if(index<0) return -1;
	_colWidths.Add(width,index);
	for(_line.First();;_line.Move(1)){
		_Line* line = _line.Element();
		if(line==0) break;
		if(line->units.Count()>index)
			line->units.Add(index);
	}
	_grid.SetMinSize(width,-1);
	refreshMinSize();
	refreshHeaderSize();		
	_grid.Invalidate();

	return index;
}
bool Table::SetColWidth(uint index,uint width){
	if(width>0xffff) return 0;
	if(index>=_colWidths.Count()) return 0;
	_colWidths[index] = width + _hMargin;
	_grid.Invalidate();
	return 1;
}
int Table::AddLine(uint index){
	base::LocalCriticalSection lcs(_cs);
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
	if(_maxCount<(uint)_line.Count()) _line.Delete(0);
	refreshMinSize();
	_grid.Invalidate();
	return index;
}
int Table::AddLine(LPCWSTR str,LPCWSTR sep,uint index){
	base::LocalCriticalSection lcs(_cs);
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
	base::StringList sl;
	sl.SplitStandString(str,sep);
	for(uint i=0;i<sl.Count();i++){
		if(i>=_colWidths.Count()) break;
		Unit* pu;
		if(line->units.Count()<=i)
			pu = line->units.Add();
		else
			pu = &line->units[i];
		pu->Text = sl[i];
	}
	if(_maxCount<(uint)_line.Count()) _line.Delete(0);
	refreshMinSize();
	_grid.Invalidate();
	return index;
}
bool Table::SetUnit(uint iline,uint iColumn,LPCWSTR txt,int color,int state){
	base::LocalCriticalSection lcs(_cs);
	if(_line.Count()<=iline||_colWidths.Count()<=iColumn) return false;
	_line.First();
	_line.Move(iline);
	_Line* line = _line.Element();
	while(line->units.Count()<=iColumn){
		line->units.Add();
	}
	Unit& unit = line->units[iColumn];
	if(txt!=INVALID_HANDLE_VALUE) unit.Text = txt;
	if(color!=-1) unit.Color = color;
	if(state!=-1) unit.State = state;
	_grid.Invalidate();
	return true;
}
void Table::Clear(){
	base::LocalCriticalSection lcs(_cs);
	_line.Clear();
	refreshMinSize();
	_grid.Invalidate();
}
bool Table::DelColumn(uint index){
	base::LocalCriticalSection lcs(_cs);
	_colWidths.Delete(index);
	for(_line.First();;_line.Move(1)){
		_Line* line = _line.Element();
		if(line==0) break;
		line->units.Delete(index);
	}
	_grid.Invalidate();
	return 1;
}
bool Table::DelLine(uint index){
	base::LocalCriticalSection lcs(_cs);
	if(index>=_line.Count()) return 0;
	_line.First();
	_line.Move(index);
	_line.Delete();
	refreshMinSize();
	_grid.Invalidate();
	return 1;
}
Table::Unit* Table::GetUnit(uint index,uint col){
	base::LocalCriticalSection lcs(_cs);
	if(index>=_line.Count()) return 0;
	if(col>=_colWidths.Count()) return 0;
	_line.First();
	_line.Move(index);
	_Line* line = _line.Element();
	return &line->units[col];
}
void Table::SetFontSize(uint size){
	_Font.LogFont()->lfHeight = size;
	_Font.Create();
	base::gdi::DC dc;
	dc.GetDC(_Handle);
	Rect cr(0,0,0,0);
	_Font.Select(dc);
	::DrawText(dc,L"a",1,cr,DT_CALCRECT|DT_SINGLELINE);
	_Font.Select(dc);
	_textH = cr.Height();
	_vMargin = size/3;
	refreshMinSize();
	_grid.Invalidate();
}
void Table::SetUnitSpacing(uint hSpace,uint vSpace){
	if(hSpace!=-1) _hMargin = hSpace;
	if(vSpace!=-1) _vMargin = vSpace;
	refreshMinSize();
	_grid.Invalidate();
}
void Table::ScrollToLine(uint index){
	if(index<0){
		_grid.ScrollEnd(0,1);
		return;
	}
	base::Twain16 cs;
	GetClientSize(cs);
	int lineH = _vMargin*2 + _textH;
	int dy = (index+1)*lineH + _vMargin - cs.y - Scr().y;
	_grid.Scroll(0,dy);
}
int Table::AddMsg(LPCWSTR msg,int color){
	int i = AddLine();
	base::SystemTime st;
	st.GetLocalTime();
	base::String tf;
	tf.Format(L"%02d:%02d:%02d",st.Hour,st.Minute,st.Second);
	SetUnit(i,0,tf,color);
	SetUnit(i,1,msg,color);
	return i;
}
int Table::AddOkMsg(LPCWSTR msg){
	int i = AddMsg(msg,0xff00);
	return i;
}
int Table::AddErrMsg(LPCWSTR msg){
	int i = AddMsg(msg,0xff);
	return i;
}
int Table::AddInfMsg(LPCWSTR msg){
	int i = AddMsg(msg,0xffffff);
	return i;
}
inline bool shiftPress(){
	int state = (USHORT)GetAsyncKeyState(VK_SHIFT);
	return 0!=(state&0x8000);
}
/*void Table::clearSelState(){
for(uint i=0;i<_selLines.Count();i++){
_Line* line = _selLines.Element(i);
for(uint j=0;j<line->States.Count();j++){
line->States[j] |= ITEM_STATE_SELECT;
line->States[j] -= ITEM_STATE_SELECT;
}
}
_selLines.Clear();
}
*/
