--package.path = package.path .. ";../../../../dava.framework/Build/?.lua"
require("TupState")

tupState = TupState.New({
    outputDir = "../.output",
    outputDbName = "",
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
            { "",  "%.aaa" } -- "" - dir_mask, "%." - only . in regexp in lua, - file has ".aaa" substr
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

