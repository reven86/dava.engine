require("TupUtil")
require("TupPack")

TupState = {}
TupState.__index = TupState

function TupState.New(userConf)    
    local userConf = userConf or { }
    local self = setmetatable({ }, TupState)
    
    -- check for gpus correctness
    if userConf.gpus ~= nil then
        if type(userConf.gpus) ~= "table" then
            error "gpus are defined incorrent. Should be { gpu1 = \"patten1\"}, ...  gpuN = \"pattenN\"} }"
        end
    end
    
    -- can be defined by user
    local conf = { }
    conf.outputDir = userConf.outputDir or "Output"
    conf.outputDbName = userConf.outputDbName or "packs"
    conf.outputDbExt = userConf.outputDbExt or ".db"
    conf.unusedPackName = userConf.unusedPackName or "__unused__"
    conf.delimiter = userConf.delimiter or "#"
    conf.cmdMaxFilesCount = userConf.cmdMaxFilesCount or 255
    conf.intermediateDir = userConf.intermediateDir or ".tmp"
    conf.intermediateListsDir = userConf.intermediateListsDir or "lists"
    conf.intermediateMergedDir = userConf.intermediateMergedDir or "merged"
    conf.intermediateSqlDir = userConf.intermediateSqlDir or "sql"
    conf.packlistExt = userConf.packlistExt or ".list"
    conf.mergedlistExt = userConf.mergedlistExt or ".mergedlist"
    conf.commonGpu = userConf.commonGpu or "common" 
    conf.gpuVar = userConf.gpuVar or "{gpu}" 
    conf.gpus = userConf.gpus or { 
        pvr_ios = "PowerVR_iOS%.",
        pvr_android = "PowerVR_Android%.", 
        tegra = "tegra%.",
        mali = "mali%.",
        adreno = "adreno%.",
        dx11 = "dx11%."
    }
    -- check for gpu names intersection
    if conf.gpus[conf.commonGpu] ~= nil then
        print("GPU name: " .. conf.commonGpu)
        error "Common gpu name is same as real gpu"
    end

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
        
    self.mergeDir = self.projectDir 
        .. "/" .. self.conf.intermediateDir 
        .. "/" .. self.conf.intermediateMergedDir
    
    self.sqlDir = self.projectDir
        .. "/" .. self.conf.intermediateDir 
        .. "/" .. self.conf.intermediateSqlDir
        
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

function TupState.GetPackGroup(self, packName, packGPU)
    return self.projectDir .. "/<" .. packName .. packGPU .. ">"
end

function TupState.GetSqlGroup(self, packGPU)
    return self.sqlDir .. "/<sql_" .. packGPU .. ">"
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
    if TupPack.IsOkPackParams(t) then
        -- create pack
        local pack = TupPack.New(t)

        -- check if pack with such name 
        -- already exists        
        if self.packNames[pack.name] ~= nil then
            print("PackName = " .. pack.name)
            error "Such pack name already defined"
        end
        
        -- remember created pack and its name
        self.packs[#self.packs + 1] = pack
        self.packNames[pack.name] = true
    end
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

    for k, file in pairs(tupFiles) do 

        local matchedPacks = { }

        -- find all packs that match current file
        for pi, pack in pairs(self.packs) do

            local isMatched = false 

            -- if pack is gpu dependent
            -- try to match file for each gpu
            if pack.isgpu then
                for gpu, gpuPattern in pairs(self.conf.gpus) do
                    local m = pack:Match(self.currentDir, file, gpu, self.conf.gpuVar, gpuPattern)
                    isMatched = isMatched or m
                end
            else
            -- try to match pack for common gpu
                local m = pack:Match(self.currentDir, file, self.conf.commonGpu)
                isMatched = isMatched or m
            end

            if isMatched then
                matchedPacks[pack.name] = true
                if pack.exclusive == true then
                    break
                end
            end
        end
        
        local firstMatch = next(matchedPacks)
        local secondMatch = next(matchedPacks, firstMatch)

        -- check that file match only one pack
        -- if not - that should be thread as error
        if secondMatch ~= nil then
            print("Packs: " .. firstMatch .. " and " .. secondMatch)
            error "File is matching more than one pack"
        end
    end
    
    for pi, pack in pairs(self.packs) do
    
        for gpu, files in pairs(pack.files) do
            local packGroup = self:GetPackGroup(pack.name, gpu)

            for i, part in UtilIterateTable(files, self.conf.cmdMaxFilesCount) do
                local partCmd = self.cmd.fwdep .. " echo -p \"" .. self.currentDir .. "\" %\"f -o %o"
                local partCmdText = "^ Gen " .. gpu .. " list " .. i .. " for " .. pack.name .. "^ "
                local partOutput = self.packlistDir .. "/" .. gpu .. "/" .. pack.name 
                    .. self.conf.delimiter .. self.currentDirString 
                    .. "-" .. i .. self.conf.packlistExt
                    
                tup.rule(part, partCmdText .. partCmd, { partOutput, packGroup })
            end
        end
    end    
end

function TupState.BuildPacks(self)
    for pai, pack in pairs(self.packs) do
        -- by default gpu list contain only common folder
        local gpus = { }
        gpus[self.conf.commonGpu] = self.conf.commonGpu
        
        -- if pack is gpu-dependent we should use 
        -- real gpu folders from config
        if pack.isgpu then
            gpus = self.conf.gpus
        end
    
        -- now execute commands for each gpu in list
        for gpu, gpuv in pairs(gpus) do
            local packGroup = self:GetPackGroup(pack.name, gpu)
            local sqlGroup = self:GetSqlGroup(gpu)
        
            -- generate emply lists for each pack
            -- this will allow cat/type command not fail
            -- if no lists were generated for pack
            local emptyPackCmd = self.cmd.fwdep .. " echo -o %o"
            local emptyPackCmdText = "^ Get empty list for " .. pack.name .. gpu .. "^ "
            local emptyPackOutput = self.packlistDir .. "/" .. gpu .. "/" .. pack.name .. self.conf.delimiter .. "_empty" .. self.conf.packlistExt 
            
            tup.rule(emptyPackCmdText .. emptyPackCmd, { emptyPackOutput })
            
            -- merge final pack list
            local mergePackMask = self.packlistDir .. "/" .. gpu .. "/" .. pack.name .. self.conf.delimiter .. "*" .. self.conf.packlistExt
            local mergePackCmd = self.cmd.cat .. " " .. mergePackMask .. " > %o"
            local mergePackCmdText = "^ Gen merged list for " .. pack.name .. gpu .. "^ "
            local mergePackOutput = self.mergeDir .. "/" .. gpu .. "/" .. pack.name .. self.conf.mergedlistExt
            
            mergePackCmd = UtilConvertToPlatformPath(self.platform, mergePackCmd)
            
            tup.rule({ mergePackMask, packGroup, emptyPackOutput }, 
                mergePackCmdText .. mergePackCmd, mergePackOutput)

            -- archivate
            local archiveCmd = self.cmd.fwzip .. " a -bd -bso0 -- %o @%f"
            local archiveCmdText = "^ Archive " .. pack.name .. gpu .. "^ "
            local archiveOutput = self.outputDir .. "/" .. gpu .. "/" .. pack.name .. ".pack"
            tup.rule(mergePackOutput, archiveCmdText .. archiveCmd, archiveOutput)

            -- generate pack hash
            local hashCmd = self.cmd.fwdep .. " hash %f -o %o"
            local hashCmdText = "^ Hash for " .. pack.name .. gpu .. "^ "
            local hashOutput = self.outputDir .. "/" .. gpu .. "/" .. pack.name .. ".hash"
            tup.rule(archiveOutput, hashCmdText .. hashCmd, hashOutput)
            
            -- generate pack sql
            local packDepends = table.concat(pack.depends, " ")
            local isGpu = tostring(gpu ~= "common")
            local sqlCmd = self.cmd.fwdep .. " sql -l " .. mergePackOutput .. " -h " .. hashOutput .. " -g " .. isGpu .. " " .. pack.name .. " " .. packDepends .. " -o %o"
            local sqlCmdText = "^ SQL for " .. pack.name .. gpu .. "^ "
            local sqlOutput = self.sqlDir .. "/" .. gpu .. "/" .. pack.name .. ".sql"
            tup.rule({ mergePackOutput, hashOutput }, sqlCmdText ..sqlCmd, { sqlOutput, sqlGroup })            
        end
    end
    
    -- create sqlite database for each gpu
    for gpu, gpuv in pairs(self.conf.gpus) do
        local sqlGroup = self:GetSqlGroup(gpu)
        local sqlCommonGroup = self:GetSqlGroup(self.conf.commonGpu)
    
        -- merge final sql
        local mergeSqlMaskCommon = self.sqlDir .. "/" .. self.conf.commonGpu .. "/*.sql"
        local mergeSqlMaskGpu = self.sqlDir .. "/" .. gpu .. "/*.sql"  
        local mergeSqlCmd = self.cmd.cat .. " " .. mergeSqlMaskCommon .. " " .. mergeSqlMaskGpu .. " > %o"
        local mergeSqlCmdText = "^ Gen merged sql " .. gpu .. "^ "
        local mergeSqlOutput = self.mergeDir .. "/" .. gpu .. "/final.sql" 

        mergeSqlCmd = UtilConvertToPlatformPath(self.platform, mergeSqlCmd)
            
        tup.rule({ mergeSqlMask, sqlCommonGroup, sqlGroup }, mergeSqlCmdText .. mergeSqlCmd, mergeSqlOutput)
            
        -- generate packs database
        local dbOutput = self.outputDir .. "/" .. self.conf.outputDbName .. "_" .. gpu .. self.conf.outputDbExt
        local dbCmd = self.cmd.fwsql .. ' -cmd ".read ' .. mergeSqlOutput .. '" -cmd ".save ' .. dbOutput .. '" "" ""'
        local dbCmdText = "^ Gen final packs DB for " .. gpu .. "^ "
        tup.rule(mergeSqlOutput, dbCmdText .. dbCmd, dbOutput)
    end
end
