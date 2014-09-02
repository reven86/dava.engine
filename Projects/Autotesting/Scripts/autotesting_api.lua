TIMEOUT = 30.0 -- Big time out for waiting
TIMECLICK = 0.5 -- time for simple action
DELAY = 0.5 -- time for simulation of human reaction

MULTIPLAYER_TIMEOUT = 300 -- Multiplayer timeout



-- API setup
function SetPackagePath(path)
    package.path = package.path .. ";" .. path .. "Actions/?.lua;" .. path .. "Scripts/?.lua;"
    require "coxpcall"
end

function assert(isTrue, errorMsg)
    if not isTrue then OnError(tostring(errorMsg)) end
end

EPSILON = 1
----------------------------------------------------------------------------------------------------
-- High-level test function
----------------------------------------------------------------------------------------------------
-- This function for simple test step without any assertion. Fail while error throwing
-- Parameters:
--        description - step description
--        func - link to step function
--        ... - input parameters for step function
function Step(description, func, ...)
    autotestingSystem:OnStepStart(description)
    Yield()
    local status, err = copcall(func, ...)
    
    Yield()
    if not status then OnError(err) end
end

-- This function for test step with assertion. Fail when step is returned NIL or FALSE
-- Parameters:
--        description - step description
--        func - link to step function
--        ... - input parameters for step function
function Assert(description, func, ...)
    autotestingSystem:OnStepStart(description)
    Yield()
    local status, err = copcall(func, ...)

    Yield()
    if not status then
        -- Some error during test step
        OnError(err)
    elseif err ~= true then
        OnError("Assertion failed, expect true, but function return "..tostring(err))
    end
end

function AssertNot(description, func, ...)
    autotestingSystem:OnStepStart(description)
    Yield()
    local status, err = copcall(func, ...)

    Yield()
    if not status then
        -- Some error during test step
        OnError(err)
    elseif not ((err == false) or (err == nil)) then
        OnError("Assertion failed, expect false, but function return "..tostring(err))
    end
end

function CheckEquals(arg1, arg2)
    if arg1 == arg2 then
        return true
    else
        Log(string.format("'%s' not equal to '%s'", tostring(arg1), tostring(arg2)), "DEBUG")
        return false
    end
end

function Log(message, level)
    level = level or "DEBUG"
    --if level ~= "DEBUG" then
    autotestingSystem:Log(level, tostring(message))
    --end
    coroutine.yield()
end 

function Yield()
    for i=0, 3 do
        coroutine.yield()
    end
end

function ResumeTest()
    --print("ResumeTest")
    if coroutine.status(co) == "suspended" then
        coroutine.resume(co)
        --print("ResumeTest done")
    else
        --print("ResumeTest failed. status:", coroutine.status(co))
        StopTest()
    end
end

function CreateTest()
    --print("CreateTest")
    co = coroutine.create(function (func)
            local status, err = copcall(func)
            --print(status, err)
            if not status then
                OnError(err)
            end
        end) -- create a coroutine with foo as the entry
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
    
    --print(autotestingSystem:GetTimeElapsed())    
end

function StartTest(name, test)      
    CreateTest()
    --print('StartTest')
    --Yield()
    autotestingSystem:OnTestStart(name)
    coroutine.resume(co, test)
end

function OnError(text)
    autotestingSystem:OnError(text)
    Yield()
end

function StopTest()
--    print("StopTest")
    autotestingSystem:OnTestFinished()
end

-- DB communication
function SaveArchiveToDB(name, archive, document)
    Log(string.format("Save '%s' archive to '%s' document", name, tostring(document)), "Debug")
    
    autotestingSystem:SaveKeyedArchiveToDB(name, archive, document)
end

function GetParameter(name, default)
    local var = autotestingSystem:GetTestParameter(name)
    if var == "not_found" then
        var = testData[name]
        if not var then
            if default then
                return default
            else
                OnError("Couldn't find value for variable "..name)
            end
        end
    end
    
    return var
end

function ReadString(name)
    return autotestingSystem:ReadString(name)
end

function WriteString(name, text)
    autotestingSystem:WriteString(name, text)
    coroutine.yield()
end

function MakeScreenshot()
    local name = autotestingSystem:MakeScreenshot()
    Yield()
    return name
end

function GetTimeElapsed()
    return autotestingSystem:GetTimeElapsed()
end

----------------------------------------------------------------------------------------------------
-- Multiplayer API
----------------------------------------------------------------------------------------------------
-- mark current device as ready to work in DB
function ReadState(name)
    return autotestingSystem:ReadState(name)
end

function ReadCommand(name)
    return autotestingSystem:ReadCommand(name)
end

function WriteState(name, state)
    autotestingSystem:WriteState(name, state)

    --[[local afterWrite = autotestingSystem:ReadState(name)
    if state ~= afterWrite then
        OnError("After writing: expected state '"..state.."' on device '"..name.."', actual:"..afterWrite)
    end]]
    coroutine.yield()
end

function WriteCommand(name, command)
    autotestingSystem:WriteCommand(name, command)

    --[[local afterWrite = autotestingSystem:ReadCommand(name)
    if command ~= afterWrite then
        OnError("After writing: expected command '"..command.."' on device '"..name.."', actual:"..afterWrite)
    end]]
    coroutine.yield()
end

function InitializeDevice(name)
    DEVICE = name
    Log("Mark "..name.." device as Ready")
    Yield()
    autotestingSystem:InitializeDevice(DEVICE)
    Yield()
    WriteState(DEVICE, "ready")
end

function WaitForDevice(name)
    Log("Wait while "..name.." device become Ready")
    Yield()
    for i=1,MULTIPLAYER_TIMEOUT do
        if ReadState(name) == "ready" then
            return
        else
            Wait(1)
        end
    end
    OnError("Device "..name.." is not ready during timeout")
end

function SendJob(name, command)
    Log("Send to slave "..name.." command: "..command)
    --Yield()
    
    for i=1,MULTIPLAYER_TIMEOUT do
        local state = ReadState(name)
        if state == "ready" then
            WriteCommand(name, command)
            --Yield()
            WriteState(name, "wait_execution")
            --Yield()
            Log("Device "..name.." ready, command was sent")
            return
        elseif state == "error" then 
            OnError("Failed to send job to "..name.." cause error on device: "..command)
        end
        Wait(1)
    end
    OnError("Failed to send job to "..name.." cause timeout: "..command)
end

function WaitJob(name)
    Log("Wait for job on slave "..name)
    --Yield()
    local state
    
    for i=1,MULTIPLAYER_TIMEOUT do
        state = ReadState(name)
        --Yield()
        if state == "execution_completed" then
            WriteState(name, "ready")
            Log("Device "..name.." finish his job")
            return
        elseif state == "error" then
            OnError("Error on "..name.." device")
        else
            Wait(1)
        end
    end
    
    OnError("Wait for job on "..name.." device failed by timeout. Last state "..state)
end

function SendJobAndWait(name, command)
    SendJob(name, command)
    WaitJob(name)
end

function noneStep()
    Yield()
    return true
end

------------------------------------------------------------------------------------------------------------------------
-- Work with UI controls
------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------
--  Getters
------------------------------------------------------------------------------------------------------------------------
function GetControl(name)
    local control
    if (type(name) == "string") then
        control = autotestingSystem:FindControl(name)
    else
        control = name
    end
    Yield()
    if control then
        return control
    end
    OnError("Couldn't find control " .. tostring(name))
end

function GetCenter(element)
    local control = GetControl(element)
    if not control then
        OnError("Couldn't find element: "..element)
    end
    local position = Vector.Vector2()
    local geomData = control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
    position.x = rect.x + rect.dx/2
    position.y = rect.y + rect.dy/2
    return position
end

function GetFrame(element)
    local control = GetControl(element)
    return control:GetFrame()
end

function GetText(element)
    local control = GetControl(element)
    return autotestingSystem:GetText(control)
end

------------------------------------------------------------------------------------------------------------------------
-- Check states
------------------------------------------------------------------------------------------------------------------------
function IsVisible(element, background)
    Yield()
    local control = autotestingSystem:FindControl(element)
    return not not (control and control:GetVisible() and control:IsOnScreen() and IsOnScreen(control, background))
end

function IsDisabled(element)
    Yield()
    local control = GetControl(element)
    return control:GetDisabled()
end

function IsOnScreen(control, background)
    local screen = background and autotestingSystem:FindControl(background) or autotestingSystem:GetScreen()
    local geomData = control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
    geomData = screen:GetGeometricData()
    local backRect = geomData:GetUnrotatedRect()
    return not not ((rect.x >= backRect.x) and (rect.x + rect.dx <= backRect.x + backRect.dx) and (rect.y >= backRect.y)
                        and (rect.y + rect.dy <= backRect.y + backRect.dy))
end

function IsCenterOnScreen(control, background)
    local screen = background and autotestingSystem:FindControl(background) or autotestingSystem:GetScreen()
    local center = GetCenter(control)
    local geomData = screen:GetGeometricData()
    local backRect = geomData:GetUnrotatedRect()
    return not not ((center.x >= backRect.x) and (center.x <= backRect.x + backRect.dx) and (center.y >= backRect.y)
                        and (center.y <= backRect.y + backRect.dy))
end

function CheckText(name, txt)
    Log("Check that text '" .. txt .. "' is present on control " .. name)
    local control = GetControl(name)
    Wait(waitTime)
    return autotestingSystem:CheckText(control, txt)
end

function CheckMsgText(name, key)
    Log("Check that text with key [" .. key .. "] is present on control " .. name)
    local control = GetControl(name)
    Wait(waitTime)
    return autotestingSystem:CheckMsgText(control, key)
end

------------------------------------------------------------------------------------------------------------------------
--  Waits
------------------------------------------------------------------------------------------------------------------------
function Wait(waitTime)
    waitTime =  waitTime or DELAY
    local count, elapsedTime = 0, 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        count = count + 1
    end
    return count
end

function WaitControl(name, time)
    local waitTime = time or TIMEOUT
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        if autotestingSystem:FindControl(name) then
            return true
        end
    end
    Log("WaitControl not found " .. name, "DEBUG")
    return false
end

function WaitControlDisappeared(name, time)
    local waitTime = time or TIMEOUT
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        if not autotestingSystem:FindControl(name) then
            return true
        end
    end
    Log("WaitControl still on the screen: " .. name, "DEBUG")
    return false
end

function WaitControlBecomeVisible(name, time)
    local waitTime = time or TIMEOUT
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        local control = autotestingSystem:FindControl(name)
        if IsVisible(name) then
            return true
        end
    end
    Log("WaitControl not found "..name, "DEBUG")
    return false
end

------------------------------------------------------------------------------------------------------------------------
--  Setters
------------------------------------------------------------------------------------------------------------------------
function SetText(path, text, time)
    local waitTime = time or DELAY
    Log("SetText path="..path.." text="..text)
    local res = autotestingSystem:SetText(path, text)
    Yield()
    Wait(waitTime)
    return res
end

function ClearField(field)
    SetText(field, "")
    ClickControl(field)
    KeyPress(2)
end

-- TODO: Update!
function SelectHorizontal(list, item)
    Log("Select " .. item .. " item in horizontal list " .. list)
    local cell = list.."/".. tostring(item)
    assert(WaitControl(list), "Couldn't select " .. cell)
    if IsVisible(cell, list) then
        ClickControl(cell)
        return true
    end
    local last_visible, previous_last, index = 0, 0, 0
    -- find first visible element
    for i = 0, 100 do --to avoid hanging up in empty list
        if IsVisible(list .. "/" .. i) then
            previous_last, last_visible = i, i
            break
        end
    end
    -- find last visible
    index = previous_last + 1
    while true do
        if not IsVisible(list .. "/" .. index) then
            last_visible = index - 1
            break
        end
        index = index + 1
    end
    repeat
        if IsVisible(cell, list) then
            break
        end
        previous_last = last_visible
        HorizontalScroll(list)
        index = last_visible + 1
        while true do
            if not IsVisible(list .. "/" .. index, list) then
                last_visible = index - 1
                break
            end
            index = index + 1
        end
    until previous_last == last_visible
    if IsVisible(cell, list) then
        ClickControl(cell)
        return true
    end
    Log("Item " .. item .. " in " .. list .. " not found")
    return false
end

function SelectFirstHorizontal(list)
    Log("Select first item in horizontal list " .. list)
    -- find first visible element
    for i = 0, 100 do -- to avoid hanging up in empty list
        if IsVisible(list.."/0", list) then
            return true
        end
        HorizontalScroll(list, true)
    end
    Log("First item in "..list.." not found")
    return false
end

-- TODO: Update!
function SelectVertical(list, item)
    Log("Select " .. tostring(item) .. " item in vertical list " .. list)
    local cell = list .. "/" .. tostring(item)
    assert(WaitControl(list), "Couldn't select "..cell)
    if IsVisible(cell, list) then
        ClickControl(cell)
        return true
    end
    local last_visible, previous_last, index = 0, 0, 0
    -- find first visible element
    for i = 0, 100 do --to avoid hanging up in empty list
        if IsVisible(list.."/"..tostring(i)) then
            previous_last, last_visible = i, i
            break
        end
    end
    -- find last wisible
    index = previous_last + 1
    while true do
        if not IsVisible(list .. "/" .. index) then
            last_visible = index - 1
            break
        end
        index = index + 1
    end
    repeat
        if IsVisible(cell, list) then
            break
        else
            previous_last = last_visible
            VerticalScroll(list)
            index = last_visible + 1
            while true do
                if not IsVisible(list .. "/" .. index, list) then
                    last_visible = index - 1
                    break
                end
                index = index + 1
            end
        end
    until previous_last == last_visible
    if IsVisible(cell, list) then
        ClickControl(cell)
        return true
    end
    Log("Item "..item.." in "..list.." not found")
    return false
end

function SelectFirstVertical(list)
    Log("Select first item in vertical list " .. list)
    -- find first visible element
    for i = 0, 100 do --to avoid hanging up in empty list
        if IsVisible(list .. "/0", list) then
            return true
        end
        VerticalScroll(list, true)
    end
    Log("First item in " .. list .. " not found")
    return false
end


function GetNewPosition(list, vertical, invert)
    local control, position = GetControl(list), Vector.Vector2()
    local new_position, geomData = Vector.Vector2(), control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
    position.x, position.y = (rect.x + rect.dx / 2), (rect.y + rect.dy / 2)
    if invert then
        new_position.y = position.y + rect.dy / 2
        new_position.x = position.x + rect.dx / 2
    else
        new_position.y = position.y - rect.dy / 2
        new_position.x = position.x - rect.dx / 2
    end
    if vertical == true then
        new_position.x = position.x
    else
        new_position.y = position.y
    end
    return position, new_position
end

function VerticalScroll(list, invert)
    local position, new_position = GetNewPosition(list, true, invert)
    TouchMove(position, new_position)
end

function HorizontalScroll(list, invert)
    local position, new_position = GetNewPosition(list, false, invert)
    TouchMove(position, new_position)
end


----------------------------------------------------------------------------------------------------
-- Touch and click actions
----------------------------------------------------------------------------------------------------
-- Touch down
function TouchDownPosition(position, touchId)
    local touchId = touchId or 1
    autotestingSystem:TouchDown(position, touchId)
    Yield()
end

function TouchDown(x, y, touchId)
    local touchId = touchId or 1
    local position = Vector.Vector2(x, y)
    autotestingSystem:TouchDown(position, touchId)
    Yield()
end

-- Touch up
function TouchUp(touchId)
    local touchId = touchId or 1
    autotestingSystem:TouchUp(touchId)
end

function ClickPosition(position, time, touchId)
    local touchId = touchId or 1
    local waitTime = time or TIMECLICK
    TouchDownPosition(position, touchId)
    Wait(waitTime)
    TouchUp(touchId)
    Wait(waitTime)
end

function Click(x, y, time, touchId)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    local position = Vector.Vector2(x, y)
    ClickPosition(position, touchId, waitTime)
end

function KeyPress(key, control)
    if control then
        ClickControl(control)
    end
    autotestingSystem:KeyPress(key)
end

function ClickControl(name, time, touchId)
    local waitTime = time or TIMEOUT
    local touchId = touchId or 1
    Log("ClickControl name=" .. name .. " touchId=" .. touchId .. " waitTime=" .. waitTime)
    if not WaitControl(name, waitTime) then
        Log("ClickControl not found "..name)
        return false
    end
    local control = autotestingSystem:FindControl(name)
    if IsVisible(name) then
        local position = GetCenter(name)
        ClickPosition(position, TIMECLICK, touchId)
        return true
    elseif not IsCenterOnScreen(control) then
        Log(name .. " is not on the Screen")
    else
        Log(name .. " is not visible")
    end
    return false
end

-- Move touch actions
function TouchMovePosition(position, time, touchId)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    autotestingSystem:TouchMove(position, touchId)
    Yield()
end

function TouchMove(position, new_position, time, touchId)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    TouchDownPosition(position, touchId)
    Wait(waitTime)
    TouchMovePosition(new_position, touchId)
    Wait(waitTime)
    Wait(waitTime)
    TouchUp(touchId)
    Wait(waitTime)
end

