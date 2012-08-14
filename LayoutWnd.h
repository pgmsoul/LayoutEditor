#pragma once
class LayoutWnd : public base::wnd::Frame{
protected:
	base::gdi::Pen outLine,selLine;

	void onDraw(base::gdi::DC* dc);
	void drawLayout(base::gdi::DC* dc,base::Layout* lay);
	void onCreate();
	void onDrag(base::Layout* lay);
public:
	base::DragLayout			glLayout;
	base::Layout*				selLay;
	LayoutWnd();
};
typedef struct CTRL_PARAM : public base::_struct{
	WORD Id;
	DWORD Style,ExStyle;
	base::String ClassName,
		Text;
	int TextColor,BkColor;
}CTRL_PARAM;
class ControlProperty :	public base::wnd::ModelDialog{
protected:
	bool			clickOk;
	base::TopLayout layout;
	base::Xml	xmlConst,	//控件的Style常数，窗口的Style和ExStyle常数
		*xmlCtrlStyle,		//控件的Style常数树
		*xmlSelMap,			//当前选定控件类型的Style常数
		*xmlStyleMap,		//窗口Style常数WS
		*xmlExStyleMap,		//窗口ExStyle常数WS_EX
		xmlIdMap,			//程序resource文件的id定义
		xmlUserType;		//自定义类型。
	base::wnd::ComboBox		ctrlTypeCombo,ctrlNameCombo,ctrlIdCombo;
	base::wnd::ListBox		ctrlWSList,
		ctrlCSList,
		ctrlWSEXList,
		ctrlStyleList,
		ctrlExStyleList;

	bool loadWndConst();
	//set dialog can control
	void enableControl();
	//set dialog can't control
	void disableControl();
	DWORD getStyleFromList();
	DWORD getExStyleFromList();
	void initCtrlStyleList();
	void setProperty();			//从设置的值设置控件的参数
	void getIdList();
	void onCtrlTypeChanged(base::wnd::CtrlMsg* cm);
	void refreshCtrlStyle();
	void initUI();
	void initProperty();	//从控件的参数初始化显示
	void addStyle(base::wnd::ListBox& src,base::wnd::ListBox& dst);
	void delStyle(base::wnd::ListBox& src);
	void onCreate(void);
	void onCommand(WORD id);
public:
	base::String	residPath;
	CTRL_PARAM		CtrlParam;

	bool Create(HWND parent,base::wnd::ICtrl* pc);
	ControlProperty(void);
	~ControlProperty(void);
};
