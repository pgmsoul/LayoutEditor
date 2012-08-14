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
	base::Xml	xmlConst,	//�ؼ���Style���������ڵ�Style��ExStyle����
		*xmlCtrlStyle,		//�ؼ���Style������
		*xmlSelMap,			//��ǰѡ���ؼ����͵�Style����
		*xmlStyleMap,		//����Style����WS
		*xmlExStyleMap,		//����ExStyle����WS_EX
		xmlIdMap,			//����resource�ļ���id����
		xmlUserType;		//�Զ������͡�
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
	void setProperty();			//�����õ�ֵ���ÿؼ��Ĳ���
	void getIdList();
	void onCtrlTypeChanged(base::wnd::CtrlMsg* cm);
	void refreshCtrlStyle();
	void initUI();
	void initProperty();	//�ӿؼ��Ĳ�����ʼ����ʾ
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
