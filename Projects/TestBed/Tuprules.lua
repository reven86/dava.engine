package.path = package.path .. ";../../../dava.framework/Build/?.lua"
require("TupState")

tupState = TupState.New({
    outputDir = ".output",
    outputDbName = "testbed"
})

tupState:AddPacks {
    {
        name = "vpack",
        depends = { "pack1", "pack2" }
    },
    {
        exclusive = true,
        name = "pack1",
        rules = {
            { "Data",  "%.aaa" }
        },
    },
    {
        exclusive = true,
        name = "pack2",
        rules = {
            { "Data", "%.sc2" }
        },
    },
    { 
         name = "gpupack",
         gpu = true,
         rules = {
            { "Data", "%.{gpu}" },
            { "Data", "%.sc3" }
         }
    },
}

-- UtilDumpTable(tupState)

