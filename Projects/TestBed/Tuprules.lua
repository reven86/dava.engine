package.path = package.path .. ";../../../dava.framework/?.lua"
require("DavaBuild")

dava.config.output_dir = ".tupOutput"
dava.config.packlist_output_dir = ".tupOutput/packlists"

dava_add_packs
{
    vpack = {
        depends = { "pack1", "pack2" }
    },

    pack1 = {
        exclusive = true,
        { "Data",  "%.aaa" }
    },

    pack2 = {
        exclusive = true,
        { "Data", "%.sc2" }
    },

    pack3 = {
        { "Data", "%.*" }
    },
}
