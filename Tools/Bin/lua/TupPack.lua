require("TupUtil")

TupPack = {}
TupPack.__index = TupPack

function TupPack.New(params)
    local self = setmetatable({}, TupPack)
    self.rules = { } 

    TupPack.IsOkPackParams(params)
    gpu = gpu or ""   
        
    -- create default TupPack table
    self.name = params.name
    self.is_gpu = params.is_gpu or false
    self.is_base = params.is_base or false
    self.is_lowercase = params.is_lowercase or false
    self.depends = params.depends or { }
    self.compression = params.compression or "lz4hc"
    self.exclusive = params.exclusive or false
    self.files = { }
    
    -- parse pack rules
    self:SetRules(params.rules or { })
    
    return self
end

function TupPack.IsOkPackParams(params)
    -- check params is a table
    if type(params) ~= "table" then
        error "Pack should be defined as table { name = name, rules = { rules table}, depends = { dependencies }}"
    end

    -- check that params have name entity
	if type(params.name) ~= "string" then
        error "Pack name should be a specified as string"
    end
    
    if params.is_gpu ~= nil and type(params.is_gpu) ~= "boolean" then
        error "Pack gpu mark should be specified as boolean true or false"
    end
    
    return true
end

function TupPack.SetRules(self, rules)
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
            elseif type(v) ~= "function" and type(v) ~= "string" then
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
    
    -- assign rules
    self.rules = rules
end



function TupPack.Match(self, dir, file, gpuName, gpuVar, gpuPattern)
    local ret = false
    local full_path = file

    if dir ~= "." then
        full_path = dir .. "/" .. file
    else
        dir = ""
    end

    if self.is_lowercase then
        full_path = full_path:lower()
    end

    -- check for correct input arguments: gpuName    
    if gpuName == nil or type(gpuName) ~= "string" then
        error "No GPU name specified"
    end
    
    -- check for correct input arguments: gpuVar and gpuPattern    
    if self.is_gpu then
        if gpuVar == nil then
            error "No gpuVar specified"
        end

        if gpuPattern == nil then
            error "No gpuPattern specified"
        end
        
        -- escape all % symbols in pattern 
        gpuPattern = gpuPattern:gsub("%%", "%%%%")
    end
    
    -- each pack has multiple rules
    for ri, rule in pairs(self.rules) do

        -- dependency specification
        if ri == "depends" then

            -- nothing to do here

        -- when pack rule is a simple string
        -- we should just compare it to match
        elseif type(rule) == "string" then
        
            if self.is_gpu then
                rule = rule:gsub(gpuVar, gpuPattern)
            end

            if self.is_lowercase then
                rule = rule:lower()
            end            

            if full_path:match(rule) then
                ret = true
                break
            end

        -- when pack rule is a table
        -- we should check independent matching
        -- for directory and file
        elseif type(rule) == "table" then

            local dir_rule = rule[1]
            local file_rule = rule[2]
            
            if self.is_gpu then
                dir_rule = dir_rule:gsub(gpuVar, gpuPattern)
                file_rule = file_rule:gsub(gpuVar, gpuPattern)
            end

            if self.is_lowercase then
                dir_rule = dir_rule:lower()
                file_rule = file_rule:lower()
            end

            if nil ~= dir:match(dir_rule) and nil ~= file:match(file_rule) then
                ret = true
                --print("!" .. full_path .. " is mathing [" .. dir_rule .. ", " .. file_rule .. "]")
                break
            else
                --print(full_path .. " is not mathing [" .. dir_rule .. ", " .. file_rule .. "]")
            end

        -- when pack rule is a function
        -- we should call it check for return value
        elseif type(rule) == "function" then

            if rule(dir, file, gpuPattern) == true then
                ret = true
                break
            end

        -- this should never happend
        else
            error "Unknown rule type"
        end
    end

    -- if matched populate files table
    if ret then
        self.files[gpuName] = self.files[gpuName] or { }
        self.files[gpuName][#self.files[gpuName] + 1] = file
    end
    
    return ret    
end
