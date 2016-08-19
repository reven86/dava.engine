package.path = package.path .. ";../../../../dava.framework/Build/?.lua"
require("TupState")

tupState = TupState.New({
    outputDir = "../.output",
    outputDbName = "testbed",
    intermediateDir = "../.tmp"
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
            { "",  "%.aaa" }
        },
    },
    {
        exclusive = true,
        name = "pack2",
        rules = {
            { "", "%.sc2" }
        },
    },
    { 
         name = "gpupack",
         gpu = true,
         rules = {
            { "", "%.{gpu}" },
            { "", "%.sc3" }
         }
    },
}

-- UtilDumpTable(tupState)

