--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
	audiosources = 
    {
		{path = "audiosources/a.wav"},
		{path = "audiosources/b.wav"},
	},
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
		{ path = "geometries/plane.bin"},
		{ path = "geometries/sphereOrigin.bin"},
	},
	colliders = 
	{
		{ path = "colliders/objectRectangle.bin"},
		{ path = "colliders/objectCube1.bin"},
		{ path = "colliders/objectCube2.bin"},
		{ path = "colliders/objectCube3.bin"},
		{ path = "colliders/object1.bin"},
		{ path = "colliders/object2.bin"},
		{ path = "colliders/object3.bin"},
		{ path = "colliders/objectTriangle.bin"},
		{ path = "colliders/plane.bin"},
		{ path = "colliders/sphereOrigin.bin"},
	}
}

