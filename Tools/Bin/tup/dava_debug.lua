dava.log_level = 0

--
-- return directory path from file @path,
-- that is divided with separatot @sep
--
function dava_get_dir(path, sep)
    sep = sep or'/'
    return path:match("(.*"..sep..")")
end

-- 
-- prints @text with given log @level
-- text appers only if @level is less or equal
-- that global setting @dava.log_level
--
function dava_log(log_level, text)
    if level <= dava.debug.log_level then
        print("[DAVA] " .. text)
    end
end

--
-- prints given table @tbl into logs
--
function dava_dump_table(tbl, indent)
    if not indent then 
        indent = 0 
    end

    for key, val in pairs(tbl) do
        formatting = string.rep("  ", indent) .. key .. ": "
        if type(val) == "table" then
            print(formatting)
            dava_dump_table(val, indent+1)
        elseif type(val) == 'boolean' then
            print(formatting .. tostring(val))
        else
            print(formatting .. tostring(val))
        end
    end
end
