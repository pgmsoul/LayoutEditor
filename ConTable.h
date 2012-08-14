//ConTable.H
#pragma once
class ConTable : public base::wnd::View{
protected:
	base::wnd::Header header;
	base::wnd::ScrollWnd table;
	typedef class _Line : public base::_class{
	public:
		base::StringList	Texts;
		base::IntegerList<int>	States;
		int				Color;
		_Line(){Color = 0;}
	};
	base::ObjectLink<_Line>		_line;
	base::Menu					_rMenu;
	base::IntegerList<_Line*>	_selLines;
	uint						_maxCount,_lineH;
	base::IntegerList<int>		_colWidths;
	base::gdi::Brush					_selBkColor;
	int							_style;//目前只支持TS_LASTCOL_AUTOSIZE

	base::CriticalSection cs;
	void onHScroll(base::wnd::Message* msg);
	void onNotify(base::wnd::CtrlMsg* cm);
	void onDraw(base::gdi::DC* dc);
	void onMouse(base::wnd::Mouse* m);
	void onCommand(WORD id);
	void onSize(base::Twain16 cs);
	void onTableSize(base::Twain16 cs);
	void onCreate();
	//calculate header control size and set it.
	void refreshHeaderSize();
	//clear all item's select flag
	void clearSelState();
	//init context menu.
	void initMenu();
public:
	base::Function<void,int,int> OnSelect;	//when a unit is click.
	//
	ConTable();
	//set a column width.
	bool SetColWidth(int index,int width);
	//set a column title.
	bool SetColText(int index,LPCWSTR title);
	//set a column title and width.
	bool SetColumn(int index,LPCWSTR title,int width);
	//add a column,return new column index.
	int AddColumn(LPCWSTR title,int width,int index = -1);
	//delete the column of index.
	bool DelColumn(int index);
	//add a new line,return new line index.
	int AddLine(int index = -1);
	//delete line of index;
	bool DelLine(int index);
	//set a unit text,return true or false if column or line is out bound of index.
	bool SetLine(int line,int column,LPCWSTR txt);
	//get a unit text,return null if column or line is not exist.
	LPCWSTR GetLine(int line,int column);
	//clear all line.
	void Clear();
	//return all line's count.
	int GetLineCount(){return _line.Count();}
	//ConTable control auto delete line in head,if add new line and all line count exceed this value.
	void SetMaxCount(int lineCount);
	//return the limit of line count.
	int GetMaxCount(){return _maxCount;}
	//scroll client to show line of index.
	void ScrollToLine(int index = -1);
	//set line text color.
	void SetLineColor(int index,int color);
	//set font size
	void SetFontSize(int size);
	//add a line and set text of column,param str is a string separate by param sep. 
	int AddLine(LPCWSTR str,LPCWSTR sep,int index = -1);
	//set background color
	void SetBkgndColor(COLORREF color,bool redraw = 0);
};
