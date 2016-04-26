TupState = {}
TupState.__index = TupState

function TupState.New(config)
    -- check params is a table
    if type(params) ~= "table" then
        error "Pack should be defined as table { name = name, rules = { rules table}, depends = { dependencies }}"
    end

    -- check that params have name entity
	if type(params.name) ~= "string" then
        error "Pack name should be a specified as string"
    end
    
    -- create default TupPack table
    local self = setmetatable({}, TupPack)
    self.name = params.name
    self.rules = { } 
    self.exclusive = params.exclusive or false
    self.depends = params.depends or { }
    
    -- parse pack rules
    self.setRule(params.rules or { })
end

function TupState.AddPack(name, rules)
end

function TupState.AddPacks()