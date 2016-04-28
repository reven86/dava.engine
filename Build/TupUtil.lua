debugger = require("debugger")
-- debugger = function() end

function UtilDumpTable(table, indent)
    indent = indent or 0 
    for key, val in pairs(table) do
        formatting = string.rep("  ", indent) .. key .. ": "
        if type(val) == "table" then
            print(formatting)
            UtilDumpTable(val, indent + 1)
        elseif type(val) == "boolean" then
            print(formatting .. tostring(val))
        else
            print(formatting .. tostring(val))
        end
    end
end 

function UtilConvertToPlatformPath(platform, p)
    -- convert only unix pathes into win pathes
    if platform ~= "win32" then
        return
    end
    
    if type(p) == "string" then
        -- if p is string just convert it 
        return p:gsub("/", "\\")        
    elseif type(p) == "table" then
        -- if p is table - convert evry
        -- string in that table
        for k,v in pairs(p) do
            if type(v) == "string" then
                p[k] = v:gsub("/", "\\")
            end
        end        
    end
end

function UtilIterateTable(table, count)
    local totalCount = #table
    local i = 0
    return function()
        if i < totalCount then
            local part = { }

            for j = 1, math.min(totalCount - i, count) do
                part[j] = table[i + j]
            end

            i = i + count
            return (i / count), part 
        end
    end
end
