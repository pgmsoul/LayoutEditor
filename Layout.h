#pragma once

//LayoutParam
struct LayoutParam : public base::_struct{
	enum{
		VERTICAL = 0,
		HORIZONTAL = 1
	};
	base::Rect	Margin;
	float		Weight;	
	int			Orientation;
	int			Width,Height;
	void SetVertical(){
		Orientation = VERTICAL;
	}
	void SetHorizontal(){
		Orientation = HORIZONTAL;
	}
	void Reset();
	void SetDimen(int width,int height,float weight);
	void SetMargin(int left,int top,int right,int bottom);
	LayoutParam();
};
//Layout Object use for View layout,but pure Layout object not bind a View object.
class Layout : public base::_class{
protected:
	LayoutParam		_param;
	base::Twain16	_size;
	base::Twain16	_location;
	Layout*			_parent;
	base::IWindow*	_view;
	base::String	_name;
	base::ObjectList<Layout> _childs;

	void _LayoutToXml(base::Xml* xn,Layout* lay);
	bool _XmlToLayout(Layout*,base::Xml* xn);
	//set realize dimension of this object
	void _setRect(int left,int top,int width,int height);
	void _horRect(int left,int top,int width,int height);
	void _verRect(int left,int top,int width,int height);
	//refresh the object when _param changed.
	void _refresh(){
		_setRect(_location.x,_location.y,_size.x,_size.y);
	}
public:
	Layout();
	//set the name of this layout
	void SetName(LPCWSTR name){_name = name;}
	//get the name of this layout
	LPCWSTR GetName(){return _name;}
	//get realize location of object
	base::Twain16 GetLocation(){return _location;}
	//get realize size of object
	base::Twain16 GetSize(){return _size;}
	//get LayoutParam value of this object
	LayoutParam GetLayoutParam(){return _param;}
	//return parent layout.
	Layout* GetParent(){return _parent;}
	//set LayoutParam to this object,this will re-Layout object.
	void SetLayoutParam(LayoutParam lp);
	//return child layout count not include sonson layout.
	uint GetCount(){return _childs.Count();}
	//get a Layout object in child list.
	Layout* GetLayout(int index){return _childs.Element(index);}
	//index of child layout object in child list.
	int IndexOf(Layout* child){return _childs.IndexOf(child);}
	//add a new Layout and set LayoutParam.
	Layout* AddLayout(LayoutParam lp,int index = -1);
	//add a exist Layout object
	int AddLayout(Layout* lay,int index = -1);
	//delete a Layout object from child list.
	bool DeleteLayout(int index);
	//delete a Layout object from child list.
	bool DeleteLayout(Layout* lay);
	//remove a Layout object from child list but not delete it.
	Layout* RemoveLayout(int index);
	//remove a Layout object from child list but not delete it.
	bool RemoveLayout(Layout* lay);
	//bind a view so the view can change size width this object.
	void Bind(base::IWindow* view){_view = view;}
	//unbind the view
	void Unbind(){_view = 0;}
	//return view;
	base::IWindow* GetView(){return _view;}
	//set orientation is vertical.
	void SetVertical();
	//set orientation is horizontal.
	void SetHorizontal();
	//set dimen
	void SetDimen(int width,int height,float weight);
	//set margin,out border dimen.
	void SetMargin(int left,int top,int right,int bottom);
	//save to a file normaly a .layout file
	bool Save(LPCWSTR file);
	//load from a .layout file.
	bool Load(LPCWSTR file);
	//load from resource, hInst = 0 is load from current module.
	bool Load(base::ResID res,HINSTANCE hInst = 0);
	//return a layout that name is required.
	Layout* GetLayout(LPCWSTR name);
};
class TopLayout : public Layout{
protected:
	base::View* _view;

	void _onSize(base::Twain16 cs);
public:
	TopLayout();
	//this can be set before view is created.
	bool Attach(base::View* view);
	bool Detach();
};
