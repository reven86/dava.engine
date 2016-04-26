local dbg = require("debugger")

require("TupPack")

TupState = {}
TupState.__index = TupState

function TupState.New(userConf)    
    local userConf = userConf or { }
    local self = setmetatable({ }, TupState)
    
    -- can be defined by user
    local conf = { }
    conf.outputDir = userConf.outputDir or "Output"
    conf.outputDbName = userConf.outputDbName or "packs.db"
    conf.intermediateDir = userConf.intermediateDir or conf.outputDir .. "/.tmp"
    conf.unusedPackName = userConf.unusedPackName or "__unused__"
    conf.delimiter = userConf.delimiter or "#"

    -- setting up configuration
    self.conf = conf
            
    -- can't be defined by user
    self.platform = tup.getconfig("TUP_PLATFORM");
    self.projectDir = tup.getcwd()
    self.currentDir = tup.getrelativedir(self.projectDir)

    local fwPath = self:FindFWPath(self.projectDir)
    self.frameworkPath = fwPath
           
    -- setting up commands
    self.cmd = { }
    self.cmd.cp = "cp"
    self.cmd.cat = "cat"
    self.cmd.fwdep = fwPath .. "../Tools/Bin/dep"    
    self.cmd.fwzip = fwPath .. "../Tools/Bin/7za"    
    self.cmd.fwsql = fwPath .. "../Tools/Bin/sqlite3"
    
    if self.platform == "win32" then
        self.cmd.cat = "type 2> nul"
        self.cmd.fwzip = fwPath .. "../Tools/Bin/7z.exe"
        self.cmd.fwsql = self.cmd.fwsql .. ".exe"
        self.cmd.fwdep = self.cmd.fwdep .. ".exe" 
    
        self.ConvertToPlatformPath(self.cmd)
    end
    
    self.packs = { }
    self.packNames = { }    
    
    if userConf.packs ~= nil then
        self:AddPacks(userConf.packs)
    end
    
    return self
end

function TupState.FindFWPath(self, projectDir)
    -- get lua debug info
    local info = debug.getinfo(1, 'S')
    local sourcePath = info.source:sub(2)
    
    -- crop string from the end to the first
    -- separator presence
    local cropDir = function(path, sep)
        sep = sep or "/"
        return path:match("(.*" .. sep .. ")")
    end
    
    -- get dir path from file path
    local fwPath = cropDir(sourcePath, "/") or cropDir(sourcePath, "\\")

    -- if got dir isn't absolute we should concat
    -- it with project dir
    if fwPath:sub(1, 1) ~= "/" and fwPath:match(":\\") == nil then
        fwPath = projectDir .. "/" .. fwPath
    end
    
    return fwPath
end

function TupState.ConvertToPlatformPath(p)
    -- convert only unix pathes into win pathes
    if platform ~= "win32" then
        return
    end
    
    if type(p) == "string" then
        -- if p is string just convert it 
        p = v:gsub("/", "\\")        
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

function TupState.AddPacks(self, packs)

    -- packs should be defined as table
    if type(packs) ~= "table" then
        error "Packs should be specified as array"
    end
    
    dbg()
        
    -- go throught packs and create
    -- corresponding TupPack tables
    for k,v in pairs(packs) do
        
        if type(k) ~= "number" then
            error "Packs should be defined in indexed array" 
        end
    
        local pack = TupPack.New(v)
        local packName = pack.name
        
        -- check if pack with such name 
        -- already exists        
        if self.packNames[packName] ~= nil then
            print("PackName = " .. packName)
            error "Such pack name already defined"
        end
        
        -- remember created pack and its name
        self.packs[#self.packs + 1] = pack
        self.packNames[packName] = true
    end
end

function TupState.BuildLists(self)
    
    local files = tup.glob("*")
    
    -- go throught files
    for k, file in pairs(files) do 

        local matchedPacks = { }
        
        -- find all packs that match current file
        for pi, pack in pairs(self.packs) do
            if pack:Match(self.currentDir, file) then
                matchedPacks[#matchedPacks + 1] = pack.name
                if pack.exclusive == true then
                    break
                end
            end
        end
        
        -- check that file match only one pack
        -- if not - that should be thread as error
        if #matchedPacks > 1 then
            print("Packs: " .. matchedPacks[1] .. " and " .. matchedPacks[2])
            error "File is matching more than one pack"
        end
    end
    
end

function TupState.DbgDumpTable(self, table, indent)
    indent = indent or 0 

    for key, val in pairs(table) do
        formatting = string.rep("  ", indent) .. key .. ": "
        if type(val) == "table" then
            print(formatting)
            self:DbgDumpTable(val, indent + 1)
        elseif type(val) == "boolean" then
            print(formatting .. tostring(val))
        else
            print(formatting .. tostring(val))
        end
    end
end 
