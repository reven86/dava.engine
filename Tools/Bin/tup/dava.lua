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
			dava.dump_table(val, indent+1)
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
	ret = dava.default_pack
	full_path = dir .. "/" .. file
	for pack_name, pack_rules in pairs(dava.pack_rules) do

		for k, rule in pairs(pack_rules) do

			-- simple string pattern 
			if(type(rule) == "string") then
				if full_path == rule then
					--print(full_path .. " is matching to [" .. pattern)
					dava.add_pack_file(pack_name, file)
					ret = pack_name
					break
				end
			-- dir pattern + file pattern
			elseif (type(rule) == "table") then
				rule_dir = rule[1]
				rule_file = rule[2]

				if dir:match(rule_dir) and file:match(rule_file) then
					--print(dir .. " and " .. "is matching to [" .. dir_pattern .. " and " .. file_pattern .. "]")
					dava.add_pack_file(pack_name, file)
					ret = pack_name
					break
				end
			end
		end
	end
	return ret
end


dava.run_tup = function()
	files = tup.glob("*")

	affected_packs = {}

	for k, file in pairs(files) do
		pack_name = dava.match_pack(dava.tup_dir, file)

		if affected_packs[pack_name] == nil then
			affected_packs[pack_name] = {}
		end

		sz = #affected_packs[pack_name]
		affected_packs[pack_name][sz + 1] = file
	end

	for affected_pack, affected_files in pairs(affected_packs) do
		pack_group = dava.root_dir .. "/<" .. affected_pack .. ">"
		pack_list_output = dava.output_dir .. dava.packlist_dir .. affected_pack .. "-" .. dava.tup_dir:gsub("/", "_") .. "." .. dava.packlist_ext

		tup.rule(affected_files, "^ Gen " .. affected_pack .. "^" .. dava.dep_tool .. " " .. dava.tup_dir .. " %\"f > %o", { pack_list_output, pack_group })
	end
end

