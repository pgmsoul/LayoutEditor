Layout布局系统使用说明
============

    Layout 布局系统的原理和使用都非常简单，它的基本思路就是按照权重和指定的尺寸，自动分配对象的大小，无需精细调整位置大小，大大简化了界面设计的工作量。通常的拖放控件的方法只适用于顺序的放置控件，如果想回头改变某个控件的大小，那么可能牵扯到其它控件的位置，而且拖放往往很难精确控制位置和对齐。

    布局对象 Layout 是一个树形结构的对象，最上级只能有一个根对象，它布满整个窗口的客户区，但是可以有 4 个边距属性 Margin_Left、Margin_Top、Margin_Right、Margin_Bottom 流出的空白。Margin属性的意思就是和父对象的边距。

    每一级的 Layout 对象包含边距区的话，都是布满整个窗口，只有一种特例就是所有的 Weight 属性值都是 0 的时候，Layout 的大小完全由 Width 和 Height 属性来决定大小，但是这种情况只是特例，不是设计 Layout 系统的目的。

    LayoutParam 对象有 9 个属性：

    Name	//这个对象实例的名称，应该保持唯一性，因为使用 Layout 绑定控件的时候需要这个值来确定哪个 Layout 。
    Orientation	//方向，子 Layout 排列的方向，只有 2 个值 0 和 1，分别代表垂直和水平两个方向。
    Weight	//这是个非常重要的属性，它是一个 float 类型的值，代表着布局对象的尺寸分配权重。
    Width	//固定宽度，Layout 的实际大小，是这个值加上由权重分别的剩余大小，注意，剩余大小可能为负值。
    Height	//
    Margin
    
    
    
    
    
    
    
    
    
    
    
    