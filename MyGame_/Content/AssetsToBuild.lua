--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
	shaders =
	{
		{ path = "shaders/vertex/vertexinputlayout_3dobject.shader", arguments = { "vertex" } },
		{ path = "shaders/vertex/standard.shader", arguments = { "vertex" } },
		{ path = "shaders/fragment/standard.shader", arguments = { "fragment" } },
		{ path = "shaders/fragment/blue.shader", arguments = { "fragment" } },
		{ path = "shaders/fragment/change_color.shader", arguments = { "fragment" } },
		{ path = "shaders/fragment/vertexColor.shader", arguments = { "fragment" } },
	},
	geometries = 
	{
		{ path = "geometries/objectRectangle.bin"},
		{ path = "geometries/object1.bin"},
		{ path = "geometries/object2.bin"},
		{ path = "geometries/object3.bin"},
		{ path = "geometries/objectTriangle.bin"},
	},
}

