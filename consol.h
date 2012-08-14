#pragma once
using namespace base;
class Table : public wnd::View{
public:
	typedef struct Unit : public base::_class{
		base::String Text;
		uint	Color,State;
		Unit(){Color=0xffffff;State=0;}
	};
protected:
	base::wnd::Header	_header;
	int					_headerHeight;
	int					_style;//目前只支持TS_LASTCOL_AUTOSIZE
	int					_newMinWidth;
	base::wnd::ScrollWnd	_grid;
	uint _hMargin,_vMargin;
	gdi::Pen		_gridColor;
	typedef class _Line : public base::_class{
	public:
		base::ObjectList<Unit> units;
	};
	base::ObjectLink<_Line>		_line;
	base::IntegerList<uint>		_colWidths;
	uint						_maxCount,_textH;
	base::CriticalSection		_cs;

	void onHScroll(base::wnd::Message* msg);
	void onNotify(base::wnd::CtrlMsg* cm);
	void onDraw(base::gdi::DC* dc);
	void onCreate();
	void onSize(base::Twain16 cs);
	//刷新最小尺寸
	void refreshMinSize();
	//刷新头控件尺寸
	void refreshHeaderSize();
public:
	base::Function<bool,base::gdi::DC*,base::Rect&,Unit&,int,int> OnDrawUnit;	//绘制一个单元

	Table();
	//set a column width.
	bool SetColWidth(uint index,uint width);
	//add a column,return new column index.
	int AddColumn(LPCWSTR title,uint width,uint index = -1);
	//delete the column of index.
	bool DelColumn(uint index);
	//add a new line,return new line index.
	int AddLine(uint index = -1);
	//delete line of index;
	bool DelLine(uint index);
	//set a unit text,return true or false if column or line is out bound of index; txt, color and state if -1, value is not changed.
	bool SetUnit(uint line,uint column,LPCWSTR txt,int color = -1,int state = -1);
	//return the Unit pointer or null if not exist.
	Unit* GetUnit(uint line,uint col);
	//clear all line.
	void Clear();
	//return all line's count.
	int GetLineCount(){return _line.Count();}
	//ConTable control auto delete line in head,if add new line and all line count exceed this value.
	void SetMaxCount(uint lineCount);
	//return the limit of line count.
	int GetMaxCount(){return _maxCount;}
	//scroll client to show line of index.
	void ScrollToLine(uint index = -1);
	//set font size
	void SetFontSize(uint size);
	//设置行距, 注意: 调用了设置字体函数, 行距会设成字高的1/3, 这个值会被覆盖, 必须重新设置, if -1 not change.
	void SetUnitSpacing(uint hSpace,uint vSpace);
	//add a line and set text of column,param str is a string separate by param sep. 
	int AddLine(LPCWSTR str,LPCWSTR sep,uint index = -1);
	//set text background color.
	void SetBkgndColor(int color){_grid.SetBkgndColor(color);}
	//msg function
	int AddMsg(LPCWSTR msg,int color = -1);
	int AddOkMsg(LPCWSTR msg);
	int AddErrMsg(LPCWSTR msg);
	int AddInfMsg(LPCWSTR msg);
};
