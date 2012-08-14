#pragma once
//控件类型枚举。
class PropertyEditor : public Table{
protected:
	base::wnd::Edit	editor;
	base::wnd::ComboBox	selector;
	int editLine;
	bool	editing;

	void onMouse(base::wnd::Mouse* m);
	void onCreate();
	void onLostFocus(base::wnd::CtrlMsg* cm);
	void onSelect(int line,int col);
public:
	base::Function<bool,int/*line*/,LPCWSTR/*value*/,int> OnChanged;
	PropertyEditor();
	//if value editing,then refresh to object,or no do nothing
	void RefreshValue();
	bool SetProperty(LPCWSTR name,LPCWSTR value);
	LPCWSTR GetProperty(LPCWSTR name);
	bool SetUnit(uint line,uint column,LPCWSTR txt);
};

class PropertyWnd : public base::wnd::Frame{
protected:
	ControlProperty ctrlWnd;
	base::String	editFileName;
	base::wnd::ImageList treeIL;
	base::wnd::TreeView	tree;
	base::wnd::Button btnAdd,btnDel,btnCopy,btnPaste;
	base::wnd::Button arrowBtn[4];
	PropertyEditor editor;
	base::TopLayout panel;
	base::Layout*			selLay;
	HTREEITEM		selTree;
	LayoutWnd			layWnd;

	void setControlParam(int line,base::String& str,int type);
	void onCommand(WORD id);
	void getUniqName(base::String& name);
	bool checkNameUniq(base::Layout* lay,LPCWSTR name);
	void createTreeFromLayout(base::Layout* lay,HTREEITEM parent,HTREEITEM insertAfter = TVI_LAST);
	void moveTreeItem(HTREEITEM item,HTREEITEM parent,HTREEITEM pos);
	void copyChildItem(HTREEITEM item,HTREEITEM newItem);
	HTREEITEM copyTreeItem(HTREEITEM item,HTREEITEM parent,HTREEITEM pos);
	HTREEITEM getSelectItem();
	void addItem(base::Layout* lay);
	void addItem();
	void delItem();
	//move a item to his parent degree list
	void moveToLeft();
	void moveToRight();
	void moveToDown();
	void moveToUp();
	void createImageButton(base::wnd::Button& btn,int id,int res);
	void onCreate();
	void onTreeNotify(base::wnd::CtrlMsg* cm);
	bool onValueRefresh(int line,LPCWSTR str,int index);
	void onClose(base::wnd::Message* msg);
	bool close();//cancel return true
	void newLayout();
	void refreshEnvirement();	//call this when layout file changed 
	void loadFile(LPCWSTR file);
	void copyToClipboard();
	void pasteFromClipboard();
	void renameLayout(base::Layout* lay,LPCWSTR suffix);
public:
	PropertyWnd();
	void Create(LPCWSTR param,int nCmdShow);
};
