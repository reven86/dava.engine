tup.initdava("../../../Tools/Bin/lua/Tupdava.lua")

tupState = TupState.New({
    outputDir = "../.Assets",
    superpackDir = "../.AssetsSuperpack",
    intermediateDir = "../.tmp",
    superpack = true
})

tup.include("Tuprules.inc.lua")

tupState:AddPacks {
    {
        name = "allpacks",
        depends = { "pack1", "pack2", "packgpu" }
    },
    {
        name = "pack1",
        is_base = true,
        compression = "none",
        rules = {
            { "^$",  ".*" }
        },
    },
    lowercase_rule,
    {
        name = "packgpu",
        is_gpu = true,
        rules = {
            { "", ".*{gpu}.*"}
        }
    },
    {
        name = "pack2",
        rules = {
            { "", ".*" }
        },
    }
}
