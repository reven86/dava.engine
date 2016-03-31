package.path = package.path .. ";../../Tools/Bin/tup/?.lua"
require("dava")

print("set config")
dava.config.output_dir = ".tupOutput"
dava.config.packlist_output_dir = ".tupOutput/packlists"

dava.add_pack_rule("pack1",
{
    { "Data",  ".*%.yaml" }
})

dava.add_pack_rule("pack2",
{
    { "Data", "%.sc2" }
})
