package.path = package.path .. ";../../Tools/Bin/tup/?.lua"
require("dava")

dava.config.output_dir = ".tupOutput"
dava.config.packlist_output_dir = ".tupOutput/packlists"

dava.add_pack_rule("vpack",
{
    depends = { "pack1", "pack2" }
})

dava.add_pack_rule("pack1",
{
    exclusive = true,
    { "Data",  "%.aaa" }
})

dava.add_pack_rule("pack2",
{
    exclusive = true,
    { "Data", "%.sc2" }
})

dava.add_pack_rule("pack3",
{
    { "Data", "" }
})
