--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
	effects =
	{
		{path = "effects/effectA.bin"},
		{path = "effects/effectB.bin"},
		{path = "effects/effectC.bin"},
	},

	shaders =
	{
		{ path = "shaders/vertex/vertexinputlayout_3dobject.shader", arguments = { "vertex" } },
	},
	geometries = 
	{
		{ path = "geometries/objectRectangle.bin"},
		{ path = "geometries/objectCube1.bin"},
		{ path = "geometries/objectCube2.bin"},
		{ path = "geometries/objectCube3.bin"},
		{ path = "geometries/object1.bin"},
		{ path = "geometries/object2.bin"},
		{ path = "geometries/object3.bin"},
		{ path = "geometries/objectTriangle.bin"},
	},
}

