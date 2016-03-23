local info = debug.getinfo(1,'S');

dava = {}
dava.debug = {} 

dava.debug.get_dir = function(path, sep)
    sep = sep or'/'
    return path:match("(.*"..sep..")")
end

dava.debug.dump_table = function(tbl, indent)
	if not indent then 
		indent = 0 
	end
	for key, val in pairs(tbl) do
		formatting = string.rep("  ", indent) .. key .. ": "
		if type(val) == "table" then
			print(formatting)
			dava.debug.dump_table(val, indent+1)
		elseif type(val) == 'boolean' then
			print(formatting .. tostring(val))      
		else
			print(formatting .. tostring(val))
		end
	end
end

dava.dava_dir = info.short_src
dava.root_dir = tup.getcwd()
dava.tup_dir = tup.getrelativedir(dava.root_dir)
dava.output_dir = dava.root_dir .. "/Output/"
dava.packs = {}
dava.pack_rules = { __everything_else__ = { } }
dava.default_pack = "__everything_else__"
dava.packlist_ext = "list"
dava.packlist_dir = "packlists/"
dava.dep_tool = dava.root_dir .. "/" .. dava.debug.get_dir(dava.dava_dir) .. "dep"

dava.add_pack_rule = function(pack_name, pack_rule)
	assert(type(pack_name) == "string", "Pack name should be a string")
	assert(type(pack_rule) == "table", "Pack rule should be a table")
	assert(dava.pack_rules[pack_name] == nil, "Pack '" .. pack_name .. "' is already defined")

	for k, v in pairs(pack_rule) do
		if(type(v) ~= "string") then
			assert(type(v) == "table", "Pack description row can be either string or table")
			assert(#v == 2, "Pack description should be defined as { 'dir pattern', 'file pattern' }")
		end	
	end

	dava.pack_rules[pack_name] = pack_rule
end

dava.add_pack_file = function(pack_name, file)
	if dava.packs[pack_name] == nil then
		dava.packs[pack_name] = {}
	end

	sz = #dava.packs[pack_name];
	dava.packs[pack_name][sz + 1] = file
end

dava.match_pack = function(dir, file)
	local ret = {}
	local full_path = dir .. "/" .. file

	for pack_name, pack_rules in pairs(dava.pack_rules) do
		local found = false
		for k, rule in pairs(pack_rules) do
			-- simple string pattern 
			if(type(rule) == "string") then
				if full_path == rule then
					--print(full_path .. " is matching to [" .. pattern)
					found = true
					break
				end
			-- dir pattern + file pattern
			elseif (type(rule) == "table") then
				rule_dir = rule[1]
				rule_file = rule[2]

				if dir:match(rule_dir) and file:match(rule_file) then
					--print(full_path .. " is matching to [" .. rule_dir .. " and " .. rule_file .. "]")
					found = true
					break
				else
					--print("FAIL: ".. full_path .. " is NOT matching to [" .. rule_dir .. " and " .. rule_file .. "]")
				end
			end
		end

		if found == true then
			if #ret ~= 0 then
				print("Warning: file " .. file .." match more than one pack: " .. ret[1] .. " and " .. pack_name)
			end
			dava.add_pack_file(pack_name, file)
			ret[#ret + 1] = pack_name
		end
	end

	if #ret == 0 then
		ret[#ret + 1] = dava.default_pack
	end

	return ret
end


dava.create_lists = function()
	files = tup.glob("*")

	affected_packs = {}

	for k, file in pairs(files) do
		pack_names = dava.match_pack(dava.tup_dir, file)

		for n, pack_name in pairs(pack_names) do
			if affected_packs[pack_name] == nil then
				affected_packs[pack_name] = {}
			end

			sz = #affected_packs[pack_name]
			affected_packs[pack_name][sz + 1] = file
		end
	end

	for affected_pack, affected_files in pairs(affected_packs) do
		pack_group = dava.root_dir .. "/<" .. affected_pack .. ">"
		pack_list_output = dava.output_dir .. dava.packlist_dir .. affected_pack .. "-" .. dava.tup_dir:gsub("/", "_") .. "." .. dava.packlist_ext

		tup.rule(affected_files, "^ Gen " .. affected_pack .. "^" .. dava.dep_tool .. " " .. dava.tup_dir .. " %\"f > %o", { pack_list_output, pack_group })
	end
end

dava.create_packs = function()
	for pack_name, pack_rule in pairs(dava.pack_rules) do
	    pack_output = dava.output_dir .. pack_name .. ".pack.zip"
	    pack_group = dava.root_dir .. "/<"  .. pack_name .. ">"
	    pack_merged_list = dava.output_dir .. pack_name .. "." .. dava.packlist_ext

	    tup.frule{
	        inputs = { pack_group },
	        command = "cat %<" .. pack_name .. "> > %o",
	        outputs = { pack_merged_list }
	    }

	    if pack_name ~= dava.default_pack then
	        tup.rule(pack_merged_list, "cat %f | zip %o -@", pack_output)
	    end
	end
end

