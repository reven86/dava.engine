require("TupUtil")
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
    conf.unusedPackName = userConf.unusedPackName or "__unused__"
    conf.delimiter = userConf.delimiter or "#"
    conf.cmdMaxFilesCount = userConf.cmdMaxFilesCount or 255
    conf.intermediateDir = userConf.intermediateDir or ".tmp"
    conf.intermediateListsDir = userConf.intermediateListsDir or "lists"
    conf.intermediateMergedListsDir = userConf.intermediateMergedListsDir or "mergedlists"
    conf.packlistExt = userConf.packlistExt or ".list"
    conf.mergedlistExt = userConf.mergedlistExt or ".mergedlist"
    conf.gpus = userConf.gpus or { ".pvr", ".mali", ".tegra" }

    -- setting up configuration
    self.conf = conf
            
    -- can't be defined by user
    self.platform = tup.getconfig("TUP_PLATFORM");
    self.projectDir = tup.getcwd()
    self.currentDir = tup.getrelativedir(self.projectDir)
    self.currentDirString = self.currentDir:gsub("/", "_") 
    
    self.outputDir = self.projectDir
        .. "/" .. self.conf.outputDir

    self.packlistDir = self.projectDir 
        .. "/" .. self.conf.intermediateDir 
        .. "/" .. self.conf.intermediateListsDir
        
    self.mergedlistDir = self.projectDir 
        .. "/" .. self.conf.intermediateDir 
        .. "/" .. self.conf.intermediateMergedListsDir
        
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
    
        UtilConvertToPlatformPath(self.platform, self.cmd)
    end
    
    self.packs = { }
    self.packNames = { }    
    
    if userConf.packs ~= nil then
        self:AddPacks(userConf.packs)
    end
    
    return self
end

function TupState.GetPackGroup(self, packName)
    return self.projectDir .. "/<" .. packName .. ">"
end

function TupState.GetPackListOutput(self, packName, index)
    return self.packlistDir .. "/" .. packName .. self.conf.delimiter 
        .. self.currentDirString .. "-" .. index .. self.conf.packlistExt
end

function TupState.GetMergedPackListOutput(self, packName)
    return self.mergedlistDir .. "/" .. packName .. self.conf.mergedlistExt
end

function TupState.GetEmptyPackListOutput(self, packName)
    return self.packlistDir .. "/" .. packName .. self.conf.delimiter 
        .. "_empty" .. self.conf.packlistExt
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

function TupState.AddPack(self, t)
    -- TODO:
    -- check if pack has ${variables}
    -- ... 

    local pack = TupPack.New(t)
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

function TupState.AddPacks(self, t)

    -- packs should be defined as table
    if type(t) ~= "table" then
        error "Packs should be specified as array"
    end
    
    -- go throught packs and create
    -- corresponding TupPack tables
    for k,v in pairs(t) do
        
        if type(k) ~= "number" then
            error "Packs should be defined in indexed array" 
        end
        
        self:AddPack(v)
    end
end

function TupState.BuildLists(self)
    
    local tupFiles = tup.glob("*")
    local matchedFiles = { }
    
    -- go throught files and build
    -- table { pack_name, { files... } } 
    for k, file in pairs(tupFiles) do 

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
        
        -- if pack doesn't match any defined
        -- we will assign it with default "unused" pack 
        if #matchedPacks == 0 then
            matchedPacks[#matchedPacks + 1] = self.conf.unusedPackName
        end
        
        -- check if db table exsists
        -- if not - create one
        local p = matchedPacks[1]
        if matchedFiles[p] == nil then
            matchedFiles[p] = { }
        end
        
        -- add file to matched pack
        local sz = #matchedFiles[p]
        matchedFiles[p][sz + 1] = file
    end
    
    for pack, files in pairs(matchedFiles) do
        local packGroup = self:GetPackGroup(pack)
            
        for i, part in UtilIterateTable(files, self.conf.cmdMaxFilesCount) do
            local partOutput = self:GetPackListOutput(pack, i)
            local cmd = self.cmd.fwdep .. " echo -p \"" .. self.currentDir .. "\" %\"f > %o"
            local cmdText = "^ Gen list " .. i .. " for " .. pack .. "^ "
            
            tup.rule(part, cmdText .. cmd, { partOutput, packGroup })
        end
    end
end

function TupState.BuildPacks(self)
    for pai, pack in pairs(self.packs) do
        local packGroup = self:GetPackGroup(pack.name)

        -- generate emply lists for each pack
        -- this will allow cat/type command not fail
        -- if no lists were generated for pack
        local emptyPackCmd = self.cmd.fwdep .. " echo > %o"
        local emptyPackCmdText = "^ Get empty list for " .. pack.name .. "^ "
        local emptyPackOutput = self:GetEmptyPackListOutput(pack.name) 
        
        tup.rule(emptyPackCmdText .. emptyPackCmd, { emptyPackOutput })
        
        -- merge final pack list
        local mergePackMask = self.packlistDir .. "/" .. pack.name .. self.conf.delimiter .. "*" .. self.conf.packlistExt
        local mergePackCmd = self.cmd.cat .. " " .. mergePackMask .. " > %o"
        local mergePackCmdText = "^ Gen merged list for " .. pack.name .. "^ "
        local mergePackOutput = self:GetMergedPackListOutput(pack.name)
        
        mergePackCmd = UtilConvertToPlatformPath(self.platform, mergePackCmd)
         
	    tup.rule({ mergedPackMask, packGroup, emptyPackOutput }, 
            mergePackCmdText .. mergePackCmd, mergePackOutput)
            
        -- archivate
        local archiveCmd = self.cmd.fwzip .. " a -bd -bso0 -- %o @%f"
        local archiveCmdText = "^ Archive " .. pack.name .. "^ "
        local archiveOutput = self.outputDir .. "/" .. pack.name .. ".pack"
        tup.rule(mergePackOutput, archiveCmdText .. archiveCmd, archiveOutput)

        -- generate pack hash
        local hashCmd = self.cmd.fwdep .. " hash %f > %o"
        local hashCmdText = "^ Hash for " .. pack.name .. "^ "
        local hashOutput = self.outputDir .. "/" .. pack.name .. ".hash"
        tup.rule(archiveOutput, hashCmdText .. hashCmd, hashOutput)
    end    
end
