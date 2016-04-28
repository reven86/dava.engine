require("TupUtil")

TupPack = {}
TupPack.__index = TupPack

function TupPack.New(params, gpuVar, gpus)
    local self = setmetatable({}, TupPack)
    self.rules = { } 

    TupPack.CheckInitialParams(params)
        
    -- create default TupPack table
    self.name = params.name
    self.exclusive = params.exclusive or false
    self.depends = params.depends or { }
    
    -- parse pack rules
    self:SetRules(params.rules or { }, gpuVar, gpus)
    
    return self
end

function TupPack.CheckInitialParams(params)
    -- check params is a table
    if type(params) ~= "table" then
        error "Pack should be defined as table { name = name, rules = { rules table}, depends = { dependencies }}"
    end

    -- check that params have name entity
	if type(params.name) ~= "string" then
        error "Pack name should be a specified as string"
    end
    
    return true
end

function TupPack.SetRules(self, rules, gpuVar, gpus)
    -- check that rules are defined in table
	if type(rules) ~= "table" then
        error "Pack rules should be a table"
    end

    -- check that rules are well formated
	for k, v in pairs(rules) do
        -- key type is number
        if type(k) == "number" then
            if type(v) == "table" then
                if #v ~= 2 or type(v[1]) ~= "string" or type(v[2]) ~= "string" then
                    print("pack = " .. self.name .. ", rule #" .. k)
                    error "Pack rule # table should be defined as { 'dir pattern', 'file pattern' }"
                end
            elseif type(v) ~= "function" then
                print("pack = " .. self.name .. ", rule #" .. k)
                error "Pack rule can be either string, table or function"
            end
        -- key type is string
        elseif type(k) == "string" then
            if k ~= "depends" and k ~= "exclusive" then
                print("pack = " .. self.name .. ", k = " .. tostring(k))
                error "Pack dependencies should be declared with 'depends' or 'exclusive' key."
            end
        -- unknow key type
        else
            print(v)
            error "Unknown pack rule"
        end
	end
    
    parsedRules = { }
    
    -- check for gpus
    if #rules > 0 and type(gpus) == "table" then
        for k, rule in pairs(rules) do
            if type(k) == "number" then
                local hasGpuPattern = false
            
                if type(rule) == "string" then
                    if rule:find(gpuVar) then
                        hasGpuPattern = true                
                        for gpu, gpuPattern in pairs(gpus) do
                            parsedRules[#parsedRules + 1] = rule:gsub(gpuVar, gpuPattern)
                        end
                    end
                elseif type(rule) == "table" then
                    local dirRule = rule[1]
                    local fileRule = rule[2]
                    
                    if dirRule:find(gpuVar) or fileRule:find(gpuVar) then
                        hasGpuPattern = true
                        for gpu, gpuPattern in pairs(gpus) do
                            local d = dirRule:gsub(gpuVar, gpuPattern)
                            local f = fileRule:gsub(gpuVar, gpuPattern)
                            parsedRules[#parsedRules + 1] = { d, f }
                        end
                    end
                end
                
                if hasGpuPattern ~= true then
                    parsedRules[#parsedRules + 1] = rule
                end
            end
        end
    else
        parsedRules = rules
    end
    
    -- assign rules
    self.rules = parsedRules
end

function TupPack.Match(self, dir, file)
    local full_path = dir .. "/" .. file
    
    -- each pack has multiple rules
    for ri, rule in pairs(self.rules) do

        -- dependency specification
        if ri == "depends" then

            -- nothing to do here

        -- when pack rule is a simple string
        -- we should just compare it to match
        elseif type(rule) == "string" then

            if rule == full_path then
                return true
            end

        -- when pack rule is a table
        -- we should check independent matching
        -- for directory and file
        elseif type(rule) == "table" then

            local dir_rule = rule[1]
            local file_rule = rule[2]

            if dir:match(dir_rule) and file:match(file_rule) then
                return true
            end

        -- when pack rule is a function
        -- we should call it check for return value
        elseif type(rule) == "function" then

            if rule(dir, file) == true then
                return true
            end

        -- this should never happend
        else
            error "Unknown rule type"
        end
    end
    
    return false    
end
