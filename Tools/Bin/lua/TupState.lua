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
    conf.outputDir = tup.getcwd() .. "/" .. (userConf.outputDir or "Assets")
    conf.superpackDir = tup.getcwd() .. "/" .. (userConf.superpackDir or "AssetsSuperpack")
    conf.intermediateDir = tup.getcwd() .. "/" .. (userConf.intermediateDir or ".tmp")

    conf.superpack = userConf.superpack or false

    conf.outputDbExt = userConf.outputDbExt or ".db"
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

    conf.unusedPackName = userConf.unusedPackName or "__unused__"
    conf.delimiter = userConf.delimiter or "#"
    conf.cmdMaxFilesCount = userConf.cmdMaxFilesCount or 255

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
    
    self.outputDir = self.conf.outputDir
    self.packlistDir = self.conf.intermediateDir .. "/lists"
    self.mergeDir = self.conf.intermediateDir .. "/merged"
    self.sqlDir = self.conf.intermediateDir .. "/sql"
    self.dbDir = self.conf.intermediateDir .. "/db"
    self.packsDir = self.conf.intermediateDir .. "/packs"
       
    local fwPath = self:FindFWLuaPath(self.projectDir)
    self.frameworkPath = fwPath
           
    -- setting up commands
    self.cmd = { }
    self.cmd.cp = "cp"
    self.cmd.cat = "cat"
    self.cmd.fwdep = fwPath .. "../dep"    
    self.cmd.fwzip = fwPath .. "../7za"    
    self.cmd.fwsql = fwPath .. "../sqlite3"
    self.cmd.fwResourceArchive = fwPath .. "../x64/ResourceArchiver"
    
    if self.platform == "win32" then
        self.cmd.cp = "copy"
        self.cmd.cat = "type"
        self.cmd.fwzip = fwPath .. "../7z.exe"
        self.cmd.fwsql = self.cmd.fwsql .. ".exe"
        self.cmd.fwdep = self.cmd.fwdep .. ".exe" 
        self.cmd.fwResourceArchive = self.cmd.fwResourceArchive .. ".exe"
    
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
    return packName .. packGPU
end

function TupState.GetSqlGroup(self, packGPU)
    return "sql_" .. packGPU
end

function TupState.GetOutputDir(self, isBasePack, defaultDir)
    if self.conf.superpack == true then
        if isBasePack == true then
            return self.outputDir
        else
            return defaultDir
        end
    else
        return self.outputDir
    end
end

function TupState.FindFWLuaPath(self, projectDir)
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
            if pack.is_gpu then
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
            print "File is matching more than one pack"
        end
    end
    
    for pi, pack in pairs(self.packs) do
    
        for gpu, files in pairs(pack.files) do
            local packGroup = self:GetPackGroup(pack.name, gpu)
            local packGroupPath = self.projectDir .. "/<" .. packGroup .. ">"

            for i, part in UtilIterateTable(files, self.conf.cmdMaxFilesCount) do
                local partCmd = self.cmd.fwdep .. " echo " .. " %\"f -o %o"
                if self.currentDir ~= "." then 
                    partCmd = partCmd .. " -ap \"" ..  self.currentDir .. "/\""
                end
                local partCmdText = "^ Gen " .. gpu .. " list " .. i .. " for " .. pack.name .. "^ "
                local partOutput = self.packlistDir .. "/" .. gpu .. "/" .. pack.name 
                    .. self.conf.delimiter .. self.currentDirString 
                    .. "-" .. i .. self.conf.packlistExt
                    
                partOutput = UtilConvertToPlatformPath(self.platform, partOutput) 
                tup.rule(part, partCmdText .. partCmd, { partOutput, packGroupPath })
            end
        end
    end    
end

function TupState.BuildPacks(self)
    local superPackGroup = "<superpack>"
    local superPackFiles = { }

    for pai, pack in pairs(self.packs) do
        -- by default gpu list contain only common folder
        local gpus = { }
        gpus[self.conf.commonGpu] = self.conf.commonGpu
        
        -- if pack is gpu-dependent we should use 
        -- real gpu folders from config
        if pack.is_gpu then
            gpus = self.conf.gpus
        end
    
        -- now execute commands for each gpu in list
        for gpu, gpuv in pairs(gpus) do
            local packGroup = self:GetPackGroup(pack.name, gpu)
            local packGroupPath = self.projectDir .. "/<" .. packGroup .. ">"
            local sqlGroup = self:GetSqlGroup(gpu)
            local sqlGroupPath = self.sqlDir .. "/<" .. sqlGroup .. ">"

            -- merge lists
            local mergePackCmd = self.cmd.fwdep .. " merge %<" .. packGroup .. "> -o %o"
            local mergePackCmdText = "^ Gen merged list for " .. packGroup .. "^ "
            local mergePackOutput = self.mergeDir .. "/" .. gpu .. "/" .. pack.name .. self.conf.mergedlistExt
            
            tup.rule({  packGroupPath }, mergePackCmdText .. mergePackCmd, { mergePackOutput })

            -- archivate
            local archiveCmd = self.cmd.fwResourceArchive .. " pack -compression " .. pack.compression .. " -listfile %f %o"
            local archiveCmdText = "^ Archive " .. pack.name .. gpu .. "^ "
            local archiveOutput = self:GetOutputDir(pack.is_base, self.packsDir) .. "/" .. gpu .. "/" .. pack.name .. ".dvpk"
            tup.rule(mergePackOutput, archiveCmdText .. archiveCmd, archiveOutput)

            if pack.is_base ~= true then
                superPackFiles[#superPackFiles + 1] = UtilConvertToPlatformPath(self.platform, archiveOutput)
            end

            -- generate pack sql
            local packDepends = table.concat(pack.depends, " ")
            local isGpu = tostring(gpu ~= "common")
            local sqlCmd = self.cmd.fwdep .. " sql -l " .. mergePackOutput .. " -g " .. isGpu .. " " .. archiveOutput .. " ".. pack.name .. " " .. packDepends .. " -o %o"
            local sqlCmdText = "^ SQL for " .. pack.name .. gpu .. "^ "
            local sqlOutput = self.sqlDir .. "/" .. gpu .. "/" .. pack.name .. ".sql"
            tup.rule({ mergePackOutput, archiveOutput }, sqlCmdText ..sqlCmd, { sqlOutput, sqlGroupPath })            
        end
    end
    
    -- create sqlite database for each gpu
    for gpu, gpuv in pairs(self.conf.gpus) do
        local sqlGroup = self:GetSqlGroup(gpu)
        local sqlGroupPath = self.sqlDir .. "/<" .. sqlGroup .. ">"
        local sqlCommonGroup = self:GetSqlGroup(self.conf.commonGpu)
        local sqlCommonGroupPath = self.sqlDir .. "/<" .. sqlCommonGroup .. ">"
    
        -- merge final sql
        local mergeSqlCmd = self.cmd.fwdep .. " merge %<" .. sqlGroup .. "> %<" .. sqlCommonGroup .. "> -o %o"
        local mergeSqlCmdText = "^ Gen merged sql " .. gpu .. "^ "
        local mergeSqlOutput = self.mergeDir .. "/" .. gpu .. "/final.sql" 

        mergeSqlCmd = UtilConvertToPlatformPath(self.platform, mergeSqlCmd)
            
        tup.rule({ sqlGroupPath , sqlCommonGroupPath },
            mergeSqlCmdText .. mergeSqlCmd, mergeSqlOutput)
            
        -- generate packs database
        local dbName = "db_" .. gpu .. self.conf.outputDbExt
        local dbOutput = self.dbDir .. "/" .. dbName
        local dbCmd = self.cmd.fwsql .. ' -cmd ".read ' .. mergeSqlOutput .. '" -cmd ".save ' .. dbOutput .. '" "" ""'
        local dbCmdText = "^ Gen final packs DB for " .. gpu .. "^ "
        tup.rule(mergeSqlOutput, dbCmdText .. dbCmd, dbOutput)

        -- zip generated db
        local dbZipOutput = self:GetOutputDir(true, self.packsDir) .. "/" .. dbName .. ".zip"
        local dbZipCmd = self.cmd.fwzip .. " a -bd -bso0 -tzip %o %f"
        local dbZipCmdText = "^ Zip final packs DB for " .. gpu .. "^ "
        tup.rule(dbOutput, dbZipCmdText .. dbZipCmd, dbZipOutput)

        -- zip generated and output into packs dir
        -- will be used when generating superpack
        if self.conf.superpack == true then
            local dbZipOutput2 = self.packsDir .. "/" ..dbName .. ".zip"
            local dbZipCmd2 = self.cmd.fwzip .. " a -bd -bso0 -tzip %o %f"
            local dbZipCmdText2 = "^ Zip final packs DB for superpack^ "
            tup.rule(dbOutput, dbZipCmdText2 .. dbZipCmd2, dbZipOutput2)

            superPackFiles[#superPackFiles + 1] = UtilConvertToPlatformPath(self.platform, dbZipOutput2)
        end
    end

    if self.conf.superpack == true then
        local packsDirPrefix = UtilConvertToPlatformPath(self.platform, self.packsDir .. "/") 

        -- create superpack lists
        for i, part in UtilIterateTable(superPackFiles, self.conf.cmdMaxFilesCount) do
            local superPartOutput = self.packlistDir .. "/super-" .. i .. self.conf.packlistExt
            local superPartCmd = self.cmd.fwdep .. " echo %f -rp " .. packsDirPrefix .." -o %o"
            local superPartCmdText = "^ Gen superpack list-" .. i .. "^ "
            tup.rule(part, superPartCmdText .. superPartCmd, { superPartOutput, superPackGroup })
        end

        -- merge superpack lists
        local mergedSuperMask = self.packlistDir .. "/super-*" .. self.conf.packlistExt
        local mergedSuperCmd = self.cmd.cat .. " " .. mergedSuperMask .. " > %o"
        local mergedSuperCmdText = "^ Gen merged superlist^ "
        local mergedSuperOutput = self.mergeDir .. "/super" ..  self.conf.mergedlistExt

        mergedSuperCmd = UtilConvertToPlatformPath(self.platform, mergedSuperCmd)

        tup.rule({ mergedSuperMask, superPackGroup }, mergedSuperCmdText .. mergedSuperCmd, mergedSuperOutput)

        -- create super pack
        local superpackOutput = self.conf.superpackDir .. "/superpack.dvpk"
        local superpackCmdText = "^ Archive superpack^ "
        local superpackCmd = self.cmd.fwResourceArchive .. " pack -compression none -basedir " .. self.packsDir .. "/ -listfile %f %o"
        tup.rule(mergedSuperOutput, superpackCmdText .. superpackCmd, superpackOutput)
    end
end
