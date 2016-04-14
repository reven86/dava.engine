
dava = {}
dava.im = { }
dava.initialized = false;

require("dava_debug")

-- 
-- this can be used in user rules
--
dava.platform = tup.getconfig("TUP_PLATFORM");
dava.project_dir = tup.getcwd()
dava.current_dir = tup.getrelativedir(dava.project_dir)

-- 
-- this can be configuren by user
--
dava.config = { }
dava.config.output_dir = "Output"
dava.config.output_db = "packs.db"
dava.config.packlist_output_dir = "Output/packlist"
dava.config.packlist_ext = "list"
dava.config.packlist_default = "__else__"

-- 
-- this function will be automatically called
--
dava.init = function()
    if dava.initialized ~= true then
        dava.im.output_dir = dava.project_dir .. "/" .. dava.config.output_dir
        dava.im.packs_db_output = dava.im.output_dir .. "/" .. dava.config.output_db
        dava.im.packlist_output_dir = dava.project_dir .. "/" .. dava.config.packlist_output_dir
        dava.im.packlist_ext = dava.config.packlist_ext
        dava.im.packlist_default = dava.config.packlist_default
        
        dava.im.packs = { 
            { 
                name = dava.im.packlist_default,
                rules = {" "}
            }
        }

        -- get directory path where this dava.lua file is located 
        local info = debug.getinfo(1, 'S');
        local fwpath = dava_get_dir(info.short_src, "/")
        if fwpath == nil then
            fwpath = dava_get_dir(info.short_src, "\\")
        end

        -- if got dir isn't absolute we should concat
        -- it with project dir
        if fwpath[1] ~= "/" and fwpath:match(":\\") == nil then
            fwpath = dava.project_dir .. "/" .. fwpath
        end

        -- commands
        dava.im.cmd_cp = "cp"
        dava.im.cmd_cat = "cat"
        dava.im.cmd_fwdep = fwpath .. "dep"
        dava.im.cmd_fwzip = fwpath .. "../7za"
        dava.im.cmd_fwsql = fwpath .. "../sqlite3"

        -- edit commands for win32
        if dava.platform == "win32" then
            dava.im.cmd_cat = "type 2> nul"
            dava.im.cmd_fwzip = fwpath .. "../7z"

            -- make slashes in commands win compatible
            for k,v in pairs(dava.im) do
                if type(v) == "string" then
                    dava.im[k] = v:gsub("/", "\\")
                end
            end
        end

        -- mark initialized
        dava.initialized = true
    end
end

-- 
-- add pack rule
-- @packname should be string
-- @pack_rules should be a table with rows:
--   * table with 2 string: first is lua-match rule for directory, 
--     second is lua-match rule for file
--   * function witch takes 2 args: directory and file and sould return true
--     if that pair is matching specified pack name
--
dava.add_pack_rule = function(pack_name, pack_rules)
    dava.init()

	assert(type(pack_name) == "string", "Pack name should be a string")
	assert(type(pack_rules) == "table", "Pack rules should be table")
	assert(dava.im.packs[pack_name] == nil, "Pack '" .. pack_name .. "' is already defined")

	for k, v in pairs(pack_rules) do
        if type(k) ~= "number" then
            assert(type(k) == "string" and (k == "depends" or k == "exclusive"), "Pack dependencies should be declared with 'depends' key.")
            if k == "depends" then
                assert(type(v) == "table", "Pack dependencies should be declared as array")
            end
        elseif type(v) ~= "string" then
            if type(v) == "table" then
                assert(#v >= 2 and type(v[1]) == "string" and type(v[2]) == "string",
                    "Pack rule #" .. k .. " table should be defined as { 'dir pattern', 'file pattern', exclusive = false }")
            elseif type(v) ~= "function" then
                assert(false, "Pack rule #" .. k .. " can be either string, table or function")
            end
        end
	end

	dava.im.packs[#dava.im.packs + 1] = { name = pack_name, rules = pack_rules }
end


dava.create_lists = function()
    dava.init()

    local cur_dir = dava.current_dir
	local files = tup.glob("*")

	affected_packs = {}

    -- local function to append values
    -- to @affected packs table
    local add_to_affected_packs = function(pack_name, file)
        if affected_packs[pack_name] == nil then
            affected_packs[pack_name] = {}
        end

        sz = #affected_packs[pack_name]
        affected_packs[pack_name][sz + 1] = file
        return true
    end

    -- go throught all files in current dir
    for k, file in pairs(files) do
        local file_full_path = cur_dir .. "/" .. file
        local pack_found = false

        -- go through pack
        for pai, pack in pairs(dava.im.packs) do
            local is_exclusive = false

            -- each pack has multiple rules
            for kk, rule in pairs(pack.rules) do

                -- dependency specification
                if kk == "depends" then

                    -- nothing to do here

                elseif kk == "exclusive" then
                    -- mark that we found exclusive pack
                    is_exclusive = true

                -- when pack rule is a simple string
                -- we should just compare it to match
                elseif type(rule) == "string" then

                    if pack.rules == file_full_path then
                        pack_found = add_to_affected_packs(pack.name, file)
                    end

                -- when pack rule is a table
                -- we should check independent matching
                -- for directory and file
                elseif type(rule) == "table" then

                    local dir_rule = rule[1]
                    local file_rule = rule[2]
                    local is_exclusive = false

                    if #rule > 2 then
                        is_exclusive = rule[3]
                    end

                    --
                    -- TODO:
                    -- use is_exclusive value
                    -- ...
                    --

                    if cur_dir:match(dir_rule) and file:match(file_rule) then
                        pack_found = add_to_affected_packs(pack.name, file)
                    end

                -- when pack rule is a function
                -- we should call it check for return value
                elseif type(rule) == "function" then

                    if rule(cur_dir, file) == true then
                        pack_found = add_to_affected_packs(pack.name, file)
                    end

                -- this should never happend
                else
                    assert(false, "this should never happend, type(pack_rule) = " .. type(pack_rule))
                end
            end

            if is_exclusive and pack_found then
                -- exclusive rule will break matching no other rules
                break
            end
        end

        if pack_found == false then
            add_to_affected_packs(dava.im.packlist_default, file)
        end
    end

    --
    -- now generate lists
    --
	for affected_pack, affected_files in pairs(affected_packs) do
		local pack_group = dava.project_dir .. "/<" .. affected_pack .. ">"
        local pack_listname = affected_pack .. "_" .. dava.current_dir:gsub("/", "_")

        -- divide list of file on goups of 300 files
        -- and add tup rule for each group
        local files_to_process = 300
        local files_count = #affected_files
        local i = 0
        while i < files_count do
            local index = i / files_to_process
            local part_output = dava.im.packlist_output_dir .. "/" .. pack_listname .. "_" .. index .. "." .. dava.im.packlist_ext
            local part_files = {}
            
            for j = 1, math.min(files_to_process, files_count - i) do
                part_files[j] = affected_files[i + j]
            end

            tup.rule(part_files, "^ Gen " .. affected_pack .. i .. "^ " .. dava.im.cmd_fwdep .. " echo -p \"" .. dava.current_dir .. "\" %\"f > %o", { part_output, pack_group })
            i = i + files_to_process
        end
	end
end

dava.create_packs = function()
    dava.init()

    local packs_merged_sql_input = {}
    local packs_merged_sql_output = dava.project_dir .. "/" .. dava.config.packlist_output_dir .. "/merged/merged.sql"
    local packs_db_output = dava.project_dir .. "/" .. dava.config.output_dir .. "/" .. dava.config.output_db

    for pai, pack in pairs(dava.im.packs) do
	    local pack_output = dava.im.output_dir .. "/" .. pack.name .. ".pack"
        local pack_output_hash = dava.im.output_dir .. "/" .. pack.name .. ".hash"
        local pack_dependencies = ""
        local pack_merged_list_output = dava.im.packlist_output_dir .. "/merged/" .. pack.name .. ".merged"
        local pack_sql_output = dava.im.packlist_output_dir .. "/" .. pack.name .. ".sql"

	    local pack_in_group = dava.project_dir .. "/<"  .. pack.name .. ">"
        local pack_in_mask = dava.im.packlist_output_dir .. "/" .. pack.name .. "_*"
        local empty_pack = dava.im.packlist_output_dir .. "/" .. pack.name .. "_empty." .. dava.im.packlist_ext

        if dava.platform == "win32" then
            pack_in_mask = pack_in_mask:gsub("/", "\\")
            empty_pack = empty_pack:gsub("/", "\\")
        end

        -- check if pack has dependencies
        if type(pack.rules.depends) == "table" then
            for i, dep in pairs(pack.rules.depends) do
                pack_dependencies = pack_dependencies .. " " .. dep
            end
        end

        -- generate emply lists for each pack
        -- this will allow cat/type command not fail
        -- if no lists were generated for pack 
        tup.rule("^ Touching " .. pack.name .. "^ " .. dava.im.cmd_fwdep .. " echo > %o", { empty_pack })

        -- merge final pack list 
	    tup.frule{
	        inputs = { pack_in_mask, pack_in_group, empty_pack },
	        command = "^ Gen big " .. pack.name .. " list^ " .. dava.im.cmd_cat .." " .. pack_in_mask .. " > %o",
	        outputs = { pack_merged_list_output }
	    }

	    if pack.name ~= dava.im.packlist_default then
            -- archivate
	        tup.rule(pack_merged_list_output, "^ Pack " .. pack.name .. "^ " .. dava.im.cmd_fwzip .. " a -bd -bso0 -- %o @%f", pack_output)

            -- generate pack crc
            tup.rule(pack_output, "^ Pack " .. pack.name .. " HASH^ " .. dava.im.cmd_fwdep .. " hash %f > %o", pack_output_hash)

            -- generate pack sql
            tup.rule({ pack_merged_list_output, pack_output_hash } , "^ SQL for " .. pack.name .. "^ " .. dava.im.cmd_fwdep .. " sql -l " .. pack_merged_list_output .. " -h " .. pack_output_hash .. " " .. pack.name .. " " .. pack_dependencies .. " > %o", pack_sql_output)

            -- add sql into commin database input list
            packs_merged_sql_input[#packs_merged_sql_input + 1] = pack_sql_output
	    end
	end

    -- merge all sql
    tup.rule(packs_merged_sql_input, "^ Gen big SQL^ " .. dava.im.cmd_cat .. " %\"f > %o", packs_merged_sql_output)

    -- generate final packs DB
    print(dava.im.output_db)
    tup.rule({ packs_merged_sql_output }, "^ Gen final packs DB^ " .. dava.im.cmd_fwsql .. " -cmd \".read " .. packs_merged_sql_output .. "\" -cmd \".save " .. packs_db_output .. "\" \"\" \"\"", dava.im.packs_db_output)
end
