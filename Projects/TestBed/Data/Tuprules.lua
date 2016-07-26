require("TupState")

tupState = TupState.New({
    outputDir = "../.Assets",
    superpackDir = "../.AssetsSuperpack",
    intermediateDir = "../.tmp",
    superpack = true
})

tupState:AddPacks {
    {
        name = "allpacks",
        depends = { "pack1", "pack2", "packgpu" }
    },
    {
        name = "pack1",
        is_base = true,
        exclusive = true,
        compression = "none",
        rules = {
            { "^$",  ".*" }
        },
    },
    {
        name = "testlower",
        exclusive = true,
        is_base = true,
        --is_lowercase = true,
        rules = {
            "UI/Test/scrollscreen.yaml"
        }
    },
    {
        name = "packgpu",
        is_gpu = true,
        exclusive = true,
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
