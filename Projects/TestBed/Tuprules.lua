package.path = package.path .. ";../../Tools/Bin/tup/?.lua"
require("dava")

dava.add_pack_rule("pack1",
{
    { "Data",  ".*%.yaml" }
})

dava.add_pack_rule("pack2",
{
    { "Data", ".*" }
})
