local info = debug.getinfo(1,'S');

dava = {}
dava.initialized = false;
dava.config = { }
dava.im = { }

require("dava_debug")

-- 
-- this can be used in user rules
--
dava.platform = tup.getconfig("TUP_PLATFORM");
dava.framework_dir = info.short_src
dava.project_dir = tup.getcwd()
dava.current_dir = tup.getrelativedir(dava.project_dir)

-- 
-- this can be configuren in user rules
--
dava.config.output_dir = "Output"
dava.config.packlist_output_dir = "Output/packlist"
dava.config.packlist_ext = "list"
dava.config.packlist_default = "__everything_else__"

-- 
-- this function will be automatically called
--
dava.init = function()
    if dava.initialized ~= true then
        dava.im.output_dir = dava.project_dir .. "/" .. dava.config.output_dir
        dava.im.packlist_output_dir = dava.project_dir .. "/" .. dava.config.packlist_output_dir
        dava.im.packlist_ext = dava.config.packlist_ext
        dava.im.packlist_default = dava.config.packlist_default
        dava.im.packs = { __everything_else__ = { } }

        dava.im.cmd_cp = "cp"
        dava.im.cmd_cat = "cat"
        dava.im.cmd_dep = dava.project_dir .. "/" .. dava_get_dir(dava.framework_dir) .. "dep"
        dava.im.cmd_zip = dava.project_dir .. "/" .. dava_get_dir(dava.framework_dir) .. "../7za"

        if dava.platform == "win32" then
            dava.im.cmd_cat = "type"

            for k,v in pairs(dava.im) do
                if type(v) == "string" then
                    dava.im[k] = v:gsub("/", "\\")
                end
            end
        end

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
        if type(v) ~= "string" then
            if type(v) == "table" then
                assert(#v >= 2 and type(v[1]) == "string" and type(v[2]) == "string",
                    "Pack rule #" .. k .. " table should be defined as { 'dir pattern', 'file pattern', exclusive = false }")
            elseif type(v) ~= "function" then
                assert(false, "Pack rule #" .. k .. " can be either string, table or function")
            end
        end
	end

	dava.im.packs[pack_name] = pack_rules
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
        for pack_name, pack_rules in pairs(dava.im.packs) do

            -- each pack has multiple rules
            for kk, rule in pairs(pack_rules) do

                -- when pack rule is a simple string
                -- we should just compare it to match
                if type(rule) == "string" then

                    if pack_rules == file_full_path then
                        pack_found = add_to_affected_packs(pack_name, file)
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
                        pack_found = add_to_affected_packs(pack_name, file)
                    end

                -- when pack rule is a function
                -- we should call it check for return value
                elseif type(rule) == "function" then

                    if rule(cur_dir, file) == true then
                        pack_found = add_to_affected_packs(pack_name, file)
                    end

                -- this should never happend
                else
                    assert(false, "this should never happend, type(pack_rule) = " .. type(pack_rule))
                end
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
        local pack_listname = affected_pack .. "-" .. dava.current_dir:gsub("/", "_") .. "." .. dava.im.packlist_ext
        local pack_listoutput = dava.im.packlist_output_dir .. "/" .. pack_listname

        print(pack_listoutput)
		tup.rule(affected_files, "^ Gen " .. affected_pack .. "^ " .. dava.im.cmd_dep .. " \"" .. dava.current_dir .. "\" %\"f > %o", { pack_listoutput, pack_group })
	end
end

dava.create_packs = function()
    dava.init()

    for pack_name, pack_rules in pairs(dava.im.packs) do
	    local pack_output = dava.im.output_dir .. "/" .. pack_name .. ".pack.zip"
        local pack_merged_list_output = dava.im.output_dir .. "/" .. pack_name .. "." .. dava.im.packlist_ext

	    local pack_in_group = dava.project_dir .. "/<"  .. pack_name .. ">"
        local pack_in_mask = dava.im.packlist_output_dir .. "/" .. pack_name .. "-*"

        if dava.platform == "win32" then
            pack_in_mask = pack_in_mask:gsub("/", "\\")
        end

	    tup.frule{
	        inputs = { pack_in_group },
	        command = dava.im.cmd_cat .." " .. pack_in_mask .. " > %o",
	        outputs = { pack_merged_list_output }
	    }

	    if pack_name ~= dava.im.packlist_default then
	        tup.rule(pack_merged_list_output, "^ Pack " .. pack_name .. "^ " ..dava.im.cmd_zip .. " a -bd -bso0 -- %o @%f", pack_output)
	    end
	end
end

