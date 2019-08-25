--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
	shaders =
	{
		{ path = "shaders/vertex/standard.shader", arguments = { "vertex" } },
		{ path = "shaders/fragment/standard.shader", arguments = { "fragment" } },

		{ path = "shaders/vertex/vertexinputlayout_3dobject.shader", arguments = { "vertex" } },
	},
}
