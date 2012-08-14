//LayoutParam
#include "stdafx.h"
#include "Layout.h"
void LayoutParam::Reset(){
	::ZeroMemory(this,sizeof(*this));
	Weight = 1;
}
void LayoutParam::SetDimen(int width,int height,float weight){
	Width = width;
	Height = height;
	Weight = weight;
}
void LayoutParam::SetMargin(int left,int top,int right,int bottom){
	Margin.SetValue(left,top,right,bottom);
}
LayoutParam::LayoutParam(){
	::ZeroMemory(this,sizeof(*this));
	Weight = 1;
}
//Layout Object use for View layout,but pure Layout object not bind a View object.
void Layout::_setRect(int left,int top,int width,int height){
	_location.x = (short)left;
	_location.y = (short)top;
	_size.x = (short)width;
	_size.y = (short)height;
	if(_view){
		_view->SetLocation(left,top);
		_view->SetSize(width,height);
	}
	if(_param.Orientation==LayoutParam::VERTICAL)
		_verRect(left,top,width,height);
	else if(_param.Orientation==LayoutParam::HORIZONTAL)
		_horRect(left,top,width,height);
}
void Layout::_horRect(int left,int top,int width,int height){
	float sumWeight = 0;
	int sumDim = 0;
	for(uint i=0;i<_childs.Count();i++){
		LayoutParam& lp = _childs[i]._param;
		sumWeight += lp.Weight;
		sumDim += lp.Width + lp.Margin.left + lp.Margin.right;
	}
	int rest = width - sumDim;
	float x = (float)left;
	for(uint i=0;i<_childs.Count();i++){
		Layout& lay = _childs[i];
		LayoutParam& lp = lay._param;
		float rationW;
		if((sumWeight<0.1f)&&(sumWeight>-0.1f))
			rationW = (float)lp.Width;
		else
			rationW = lp.Width + rest*lp.Weight/sumWeight;
		x += lp.Margin.left;
		lay._setRect((int)x,top+lp.Margin.top,(int)(x+rationW)-(int)x,height-lp.Margin.top-lp.Margin.bottom);
		x += rationW + lp.Margin.right;
	}
}
void Layout::_verRect(int left,int top,int width,int height){
	float sumWeight = 0;
	int sumDim = 0;
	for(uint i=0;i<_childs.Count();i++){
		LayoutParam& lp = _childs[i]._param;
		sumWeight += lp.Weight;
		sumDim += lp.Height + lp.Margin.top + lp.Margin.bottom;
	}
	int rest = height - sumDim;
	float y = (float)top;
	for(uint i=0;i<_childs.Count();i++){
		Layout& lay = _childs[i];
		LayoutParam& lp = lay._param;
		float rationH;
		if((sumWeight<0.1f)&&(sumWeight>-0.1f))
			rationH = (float)lp.Height;
		else
			rationH = lp.Height + rest*lp.Weight/sumWeight;
		y += lp.Margin.top;
		lay._setRect(left+lp.Margin.left,(int)y,width-lp.Margin.left-lp.Margin.right,(int)(y+rationH)-(int)y/*消除舍1导致的一个像素的间距*/);
		y += rationH + lp.Margin.bottom;
	}
}
Layout::Layout(){
	_size.SetValue(0);
	_location.SetValue(0);
	_parent = 0;
	_view = 0;
}
//set LayoutParam to this object,this will re Layout object.
void Layout::SetLayoutParam(LayoutParam lp){
	_param = lp;
	if(_parent!=NULL) _parent->_refresh();
	else _refresh();
}
//add a new Layout and set LayoutParam.
Layout* Layout::AddLayout(LayoutParam lp,int index){
	Layout* layout = _childs.Add(index);
	layout->_parent = this;
	layout->SetLayoutParam(lp);
	return layout;
}
//add a exist Layout object
int Layout::AddLayout(Layout* lay,int index){
	index = _childs.In(lay,index);
	lay->_parent = this;
	_refresh();
	return index;
}
//delete a Layout object from child list.
bool Layout::DeleteLayout(int index){
	if(1!=_childs.Delete(index)) return false;
	_refresh();
	return true;
}
//delete a Layout object from child list.
bool Layout::DeleteLayout(Layout* lay){
	if(!_childs.Delete(lay)) return false;
	_refresh();
	return true;
}
//remove a Layout object from child list but not delete it.
Layout* Layout::RemoveLayout(int index){
	Layout* lay = _childs.Element(index);
	if(lay==0) return 0;
	_childs.Out(index);
	_refresh();
	return lay;
}
//remove a Layout object from child list but not delete it.
bool Layout::RemoveLayout(Layout* lay){
	int index = _childs.IndexOf(lay);
	if(index<0) return false;
	_childs.Out(index);
	_refresh();
	return true;
}
void Layout::SetVertical(){
	_param.Orientation = LayoutParam::VERTICAL;
	_refresh();
}
void Layout::SetHorizontal(){
	_param.Orientation = LayoutParam::HORIZONTAL;
	_refresh();
}
void Layout::SetDimen(int width,int height,float weight){
	_param.Width = width;
	_param.Height = height;
	_param.Weight = weight;
	_refresh();
}
void Layout::SetMargin(int left,int top,int right,int bottom){
	_param.Margin.SetValue(left,top,right,bottom);
	_refresh();
}

void Layout::_LayoutToXml(base::Xml* xn,Layout* lay){
	xn->SetName(lay->_name);
	xn->SetPropertyInt(L"Orientation",lay->_param.Orientation);
	xn->SetPropertyDouble(L"Weight",lay->_param.Weight);
	xn->SetPropertyInt(L"Width",lay->_param.Width);
	xn->SetPropertyInt(L"Height",lay->_param.Height);
	xn->SetPropertyInt(L"Margin_Left",lay->_param.Margin.left);
	xn->SetPropertyInt(L"Margin_Top",lay->_param.Margin.top);
	xn->SetPropertyInt(L"Margin_Right",lay->_param.Margin.right);
	xn->SetPropertyInt(L"Margin_Bottom",lay->_param.Margin.bottom);
	for(uint i=0;i<lay->_childs.Count();i++){
		_LayoutToXml(xn->AddNode(),&lay->_childs[i]);
	}
}
bool Layout::_XmlToLayout(Layout* lay,base::Xml* xn){
	LayoutParam param;
	if(!xn->GetPropertyInt(L"Orientation",param.Orientation)) return false;
	double v = 1.0;
	if(!xn->GetPropertyDouble(L"Weight",v)) return false;
	param.Weight = (float)v;
	if(!xn->GetPropertyInt(L"Width",param.Width)) return false;
	if(!xn->GetPropertyInt(L"Height",param.Height)) return false;
	if(!xn->GetPropertyInt(L"Margin_Left",param.Margin.left)) return false;
	if(!xn->GetPropertyInt(L"Margin_Top",param.Margin.top)) return false;
	if(!xn->GetPropertyInt(L"Margin_Right",param.Margin.right)) return false;
	if(!xn->GetPropertyInt(L"Margin_Bottom",param.Margin.bottom)) return false;
	lay->_param = param;
	lay->_name = xn->GetName();
	for(uint i=0;i<xn->GetNodeCount();i++){
		Layout* newLay = new Layout();
		lay->AddLayout(newLay);
		if(!_XmlToLayout(newLay,xn->GetNode(i))) return false;
	}
	return true;
}
bool Layout::Save(LPCWSTR file){
	base::Xml xml;
	_LayoutToXml(&xml,this);
	return xml.Save(file);
}
bool Layout::Load(LPCWSTR file){
	base::Xml xml;
	if(!xml.Load(file,L"UTF-8")) return false;
	_param.Reset();
	_name = L"";
	_childs.Clear();
	return _XmlToLayout(this,&xml);
}
bool Layout::Load(base::ResID res,HINSTANCE hInst){
	_param.Reset();
	_name = L"";
	_childs.Clear();
	if(hInst) hInst = base::Instance();
	DWORD size = 0;
	void* pm = base::GetResource(res,L"LAYOUT",0,&size);
	base::String str;
	str.FromMultiByte((LPCSTR)pm,size,CP_UTF8);
	base::Xml xml;
	if(!xml.FromString(str)) return false;
	return _XmlToLayout(this,&xml);
}
Layout* findLayoutByName(Layout* lay,LPCWSTR name){
	if(base::WcsEqual(lay->GetName(),name)) return lay;
	for(uint i=0;i<lay->GetCount();i++){
		Layout* fLay = findLayoutByName(lay->GetLayout(i),name);
		if(fLay) return fLay;
	}
	return 0;
}
Layout* Layout::GetLayout(LPCWSTR name){
	return findLayoutByName(this,name);
}
void TopLayout::_onSize(base::Twain16 cs){
	int w = base::Maximum(_view->MinSize().x,(int)cs.x);
	int h = base::Maximum(_view->MinSize().y,(int)cs.y);
	_setRect(_param.Margin.left-_view->Scr().x,_param.Margin.top-_view->Scr().y,
		w-_param.Margin.left-_param.Margin.right,
		h-_param.Margin.top-_param.Margin.bottom);
	_view->Invalidate();
}
TopLayout::TopLayout():_view(0){
	_name = L"Root";
	_param.Margin.SetValue(10,10,10,10);
}
//this can be set before view is created.
bool TopLayout::Attach(base::View* view){
	if(_view!=NULL){
		PrintD(L"%s",L"TopLayout.Attach: this object have attach to a view");
		return false;
	}
	_view = view;
	view->OnSize.Add(this,&TopLayout::_onSize);
	base::Twain16 cs;
	if(view->GetClientSize(cs)){
		_setRect(0,0,cs.x,cs.y);
	}
	return true;
}
bool TopLayout::Detach(){
	if(_view==NULL) return false;
	_view->OnSize.Delete(this,&TopLayout::_onSize);
	_view = NULL;
	return true;
}
