SMALL_TIMEOUT = 3.0
BIG_TIMEOUT = 30.0 -- Big time out for waiting
TIMEOUT = 10.0 -- DEFAULT TIMEOUT
TIMECLICK = 0.5 -- time for simple action
DELAY = 0.5 -- time for simulation of human reaction

MULTIPLAYER_TIMEOUT_COUNT = 300 -- Multiplayer timeout

EPSILON = 1

MAX_LIST_COUNT = 100 -- Max item of list
lPrint = print

-- API setup
function SetPackagePath(path)
    package.path = package.path .. ";" .. path .. "Actions/?.lua;" .. path .. "Scripts/?.lua;" .. ";" .. path .. "Tests/?.lua;"
    require "coxpcall"
end

function assert(isTrue, errorMsg)
    if not isTrue then OnError(tostring(errorMsg)) end
end

function assertEq(arg1, arg2, errorMsg)
    if arg1 ~= arg2 then 
        Log(string.format("Assert failed: '%s' is not equals to '%s'", tostring(arg1), tostring(arg2)), "ERROR")
        OnError(tostring(errorMsg)) 
    end
	return true
end

local function toboolean(condition)
    return not not condition
end

function Compare(arg1, arg2, delta)
    local delta = delta or EPSILON
    if math.abs(arg1 - arg2) <= delta then
        return true
    end
    Log(string.format("Diff between (%s) and (%s) is: %s", arg1, arg2, arg1 - arg2), "DEBUG")
    return false 
end

local function __GetNewPosition(list, vertical, invert, notInCenter)
    local position, newPosition = Vector.Vector2(), Vector.Vector2()
    local rect = GetElementRectByName(list)
    position.x, position.y = (rect.x + rect.dx / 2), (rect.y + rect.dy / 2)
    if invert then
        newPosition.y = position.y + rect.dy / 1.5
        newPosition.x = position.x + rect.dx / 1.5
    else
        newPosition.y = position.y - rect.dy / 1.5
        newPosition.x = position.x - rect.dx / 1.5
    end
    if vertical == true then
        newPosition.x = position.x
    else
        newPosition.y = position.y
    end
    if notInCenter then
        newPosition.x = newPosition.x - rect.dx / 8
        position.x = position.x - rect.dx / 8
    end
    return position, newPosition
end

function print(...)
    local printResult = ""
    for i,v in ipairs(arg) do
        printResult = printResult .. tostring(v) .. "\t"
    end
    lPrint(printResult)
end

----------------------------------------------------------------------------------------------------
-- High-level test function
----------------------------------------------------------------------------------------------------
-- This function for simple test step without any assertion. Fail while error throwing
-- Parameters:
-- description - step description
-- func - link to step function
-- ... - input parameters for step function
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
        OnError(err)
    elseif not err then
        OnError("Assertion failed, expect true, but function return " .. tostring(err))
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
    elseif err then
        OnError("Assertion failed, expect false, but function return " .. tostring(err))
    end
end

function CheckEquals(arg1, arg2)
    if arg1 == arg2 then
        return true
    end
    Log(string.format("'%s' not equal to '%s'", tostring(arg1), tostring(arg2)), "DEBUG")
    return false
end

function Log(message, level)
    level = level or "DEBUG"
    autotestingSystem:Log(level, tostring(message))
    coroutine.yield()
end

function Yield()
    for i = 0, 3 do
        coroutine.yield()
    end
end

function ResumeTest()
    if coroutine.status(co) == "suspended" then
        coroutine.resume(co)
    else
        StopTest()
    end
end

function CreateTest()
    co = coroutine.create(function(func)
        local status, err = copcall(func)
        if not status then
            OnError(err)
        end
    end) -- create a coroutine with foo as the entry
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
end

function StartTest(name, test)
    CreateTest()
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
    DEVICE_NAME = autotestingSystem:GetDeviceName()
    PLATFORM = autotestingSystem:GetPlatform()
    IS_PHONE = autotestingSystem:IsPhoneScreen()
    autotestingSystem:OnTestStart(name)
    coroutine.resume(co, test)
end

function OnError(text)
    autotestingSystem:OnError(text)
    Yield()
end

function StopTest()
    autotestingSystem:OnTestFinished()
end


-- DB communication
function SaveKeyedArchiveToDevice(name, archive)
    Log(string.format("Save '%s' archive", name), "Debug")
    autotestingSystem:SaveKeyedArchiveToDevice(name, archive)
end

function GetParameter(name, default)
    local var = autotestingSystem:GetTestParameter(name)
    if var ~= "not_found" then
        return var
    end
    if testData and testData[name] then
        return testData[name]
    elseif default ~= nil then
        return default
    end
    OnError("Couldn't find value for variable " .. name)
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
    coroutine.yield()
    return name
end

function GetTimeElapsed()
    return autotestingSystem:GetTimeElapsed()
end

----------------------------------------------------------------------------------------------------
-- Multiplayer API
----------------------------------------------------------------------------------------------------
MP_STATE = {}
MP_STATE['READY'] = "ready"
MP_STATE['NO_DEVICE'] = "not_found"

-- mark current device as ready to work in DB
function ReadState(name)
    return autotestingSystem:ReadState(name, "State")
end

function ReadCommand(name)
    return autotestingSystem:ReadState(name, "Command")
end

function WriteState(name, state)
    autotestingSystem:WriteState(name, "State", state)
    coroutine.yield()
end

function WriteCommand(name, command)
    autotestingSystem:WriteState(name, "Command", command)
    coroutine.yield()
end

function InitializeDevice()
    DEVICE = GetDeviceName()
    Log("Mark " .. DEVICE .. " device as Ready")
    autotestingSystem:InitializeDevice()
    WriteState(DEVICE, MP_STATE['READY'])
    Yield()
end

function WaitForDevice(name)
    Log("Wait while " .. name .. " device become Ready")
    Yield()
	local currentStatus = ""
    for i = 1, MULTIPLAYER_TIMEOUT_COUNT do
		currentStatus = ReadState(name)
        if currentStatus == MP_STATE['READY'] then
			Log("Device " .. name .. " is ready")
            return true
        elseif currentStatus == MP_STATE['NO_DEVICE'] then
			OnError("Could not find device " .. name)
		end
        Wait(1)
    end
    OnError("Device " .. name .. " is not ready during timeout")
end

function SendJob(name, command)
    Log("Send to slave " .. name .. " command: " .. command)
    for i = 1, MULTIPLAYER_TIMEOUT_COUNT do
        local state = ReadState(name)
        if state == "ready" then
            WriteCommand(name, command)
            WriteState(name, "wait_execution")
            Log("Device " .. name .. " ready, command was sent")
            return
        elseif state == "error" then
            OnError("Failed to send job to " .. name .. " cause error on device: " .. command)
        end
        Wait(1)
    end
    OnError("Failed to send job to " .. name .. " cause timeout: " .. command)
end

function WaitJob(name)
    Log("Wait for job on slave " .. name)
	local state
    for i = 1, MULTIPLAYER_TIMEOUT_COUNT do
        state = ReadState(name)
        if state == "execution_completed" then
            WriteState(name, "ready")
            Log("Device " .. name .. " finish his job")
            return
        elseif state == "error" then
            OnError("Error on " .. name .. " device")
        else
            Wait(1)
        end
    end
    OnError("Wait for job on " .. name .. " device failed by timeout. Last state " .. state)
end

function SendJobAndWait(name, command)
    SendJob(name, command)
    WaitJob(name)
end

function noneStep()
    Yield()
    return true
end

function GetDeviceName()
    return autotestingSystem:GetDeviceName()
end

function GetPlatform()
    return autotestingSystem:GetPlatform()
end

function IsPhone()
    return autotestingSystem:IsPhoneScreen()
end

------------------------------------------------------------------------------------------------------------------------
-- Work with UI controls
------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------
-- Getters
------------------------------------------------------------------------------------------------------------------------
function GetControl(name)
    return autotestingSystem:FindControl(name) or autotestingSystem:FindControlOnPopUp(name)
end

function GetCenter(element)
    local control = GetSize(element)
    return { dx = control.dx, dy = control.dy, x = control.x + control.dx / 2, y = control.y + control.dy / 2 }
end

function GetFrame(element)
    local control = GetControl(element)
    if control then
        return control:GetFrame()
    end
    return nil
end

function GetScreen()
    local screen =  autotestingSystem:GetScreen()
    return GetElementRect(screen)
end

function GetSize(controlName)
    local control = GetControl(controlName)
    if not control then
        OnError("Couldn't find element: " .. controlName)
    end
    local rect = GetElementRect(control)
    local floor = math.floor
    return { x = floor(rect.x + 0.5), y = floor(rect.y + 0.5), dx = floor(rect.dx + 0.5), dy = floor(rect.dy + 0.5) }
end

function GetState(controlName)
    local control = GetControl(controlName)
    if control then
        return control:GetState()
    end
    return nil
end

function GetText(controlName)
    local control = GetControl(controlName)
    if control then
        return autotestingSystem:GetText(control)
    end
    return nil
end

function GetElementRect(control)
    local geomData = control:GetGeometricData()
    return geomData:GetUnrotatedRect()
end

function GetElementRectByName(name)
    local control = GetControl(name)
    if control then
        local geomData = control:GetGeometricData()
        return geomData:GetUnrotatedRect()
    end
    return nil
end

function GetPosition(element)
    local control = GetSize(element)
    return { x = control.x, y = control.y, dx = control.x + control.dx, dy = control.y + control.dy }
end

------------------------------------------------------------------------------------------------------------------------
-- Check states
------------------------------------------------------------------------------------------------------------------------
function IsVisible(controlName, background)
    local control = autotestingSystem:FindControl(controlName) or autotestingSystem:FindControlOnPopUp(controlName)
    return toboolean(control and control:GetVisible() and control:IsOnScreen() and IsOnScreen(controlName, background))
end

function IsDisabled(controlName)
    Yield()
    local control = GetControl(controlName)
    if control then
        return control:GetDisabled() 
    end
    return nil
end

function IsOnScreen(controlName, background)
    local screen = background and autotestingSystem:FindControl(background) or autotestingSystem:GetScreen()
    local rect = GetElementRectByName(controlName)
    local backRect = GetElementRect(screen)
    return toboolean((backRect.x - rect.x <= 1) and ((rect.x + rect.dx) - (backRect.x + backRect.dx) <= 1) and (backRect.y - rect.y <= 1)
            and ((rect.y + rect.dy) - (backRect.y + backRect.dy) <= 1))
end

function IsCenterOnScreen(controlName, background)
    local screen = background and autotestingSystem:FindControl(background) or autotestingSystem:GetScreen()
    local center = GetCenter(controlName)
    local backRect = GetElementRect(screen)
    return toboolean((center.x >= backRect.x) and (center.x <= backRect.x + backRect.dx) and (center.y >= backRect.y)
            and (center.y <= backRect.y + backRect.dy))
end

function IsReady(controlName, waitTime)
    if not WaitControl(controlName, waitTime) then
        Log("Control " .. controlName .. " not found.")
        return false
    end
    if not IsVisible(controlName) then
        Log("Control " .. controlName .. " is not visible.")
        return false
    end
    if not IsCenterOnScreen(controlName) then
        Log("Control " .. controlName .. " is not on the screen.")
        return false
    end
    return true
end

function IsSelected(name)
    local control = GetControl(name)
    if control then
        return autotestingSystem:IsSelected(control)
    end
    return nil
end

function CheckText(name, txt)
    Log("Check that text '" .. txt .. "' is present on control " .. name)
    local control = GetControl(name)
    return autotestingSystem:CheckText(control, txt)
end

function CheckMsgText(name, key)
    Log("Check that text with key [" .. key .. "] is present on control " .. name)
    local control = GetControl(name)
    return autotestingSystem:CheckMsgText(control, key)
end

-- Check if control intersect between each other - return false
function ControlIntersection(elementA, elementB)
    local rectA = GetPosition(elementA)
    local rectB= GetPosition(elementB)
    return ( rectA.dx <= rectB.x or rectA.x >= rectB.dx or rectA.dy <= rectB.y or rectA.y >= rectB.dy );
end

------------------------------------------------------------------------------------------------------------------------
-- Waits
------------------------------------------------------------------------------------------------------------------------
function Wait(waitTime)
    waitTime = waitTime or DELAY
    local count, elapsedTime = 0, 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        count = count + 1
    end
    return count
end

function WaitUntil(time, func, ...)
    local waitTime, err = time or TIMEOUT, nil
    local elapsedTime, status = 0.0, nil
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        status, err = copcall(func, ...)
        if status and err then
            return elapsedTime
        end
    end
    return nil
end

function WaitControl(name, time)
    local waitTime, aSys = time or TIMEOUT, autotestingSystem
    Log("WaitControl name=" .. name .. " time=" .. tostring(waitTime), "DEBUG")
    local find_control_lua = function(x) return aSys:FindControl(x) or aSys:FindControlOnPopUp(x) end
	local result = WaitUntil(waitTime, find_control_lua, name)
    if not result then
        Log("Control not found " .. name, "DEBUG")
    end
    return result
end

function WaitControlDisappeared(name, time)
    local waitTime, aSys = time or TIMEOUT, autotestingSystem
    Log("WaitControlDisappeared name=" .. name .. " time=" .. tostring(waitTime), "DEBUG")
    local not_find_control_lua = function(x) return not aSys:FindControl(x) and not aSys:FindControlOnPopUp(x) end
    local result = WaitUntil(waitTime, not_find_control_lua, name)
	if not result then
        Log("Control still on the screen: " .. name, "DEBUG")
    end
    return result
end

function WaitControlBecomeVisible(name, time)
    local waitTime = time or TIMEOUT
    Log("WaitControlBecomeVisible name=" .. name .. " time=" .. tostring(waitTime), "DEBUG")
    local result = WaitUntil(waitTime, IsVisible, name)
	if not result then
        Log("Control not found " .. name, "DEBUG")
    end
    return result
end

function WaitUntilControlBecomeEnabled(name, time)
    local waitTime = time or TIMEOUT
    Log("WaitUntilControlBecomeEnabled name=" .. name .. " time=" .. tostring(waitTime), "DEBUG")
    local is_enabled = function(x) return not IsDisabled(x) end
    local result = WaitUntil(waitTime, is_enabled, name)
	if not result then
        Log("Control is disabled " .. name, "DEBUG")
    end
    return result
end

------------------------------------------------------------------------------------------------------------------------
-- Setters
------------------------------------------------------------------------------------------------------------------------
function SetText(path, text, time)
    local waitTime = time or DELAY
    Log("SetText path=" .. path .. " text=" .. text)
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

function SelectItemInList(listName, item)
    Log(string.format("Select '%s' cell in '%s' list.", item, listName))
    assert(WaitControl(listName), "Couldn't find " .. listName)

    local function __click() if IsVisible(item, listName) and IsVisible(item) then ClickControl(item) return true end return false end

    if __click() then
        return true
    end
    local listControl = GetControl(listName)
    local startPoint, __scroll = listControl:GetPivotPoint(), nil
    local finalPoint = autotestingSystem:GetMaxListOffsetSize(listControl)
    if autotestingSystem:IsListHorisontal(listControl) then
        __scroll, startPoint = HorizontalScroll, startPoint.y
    else
        __scroll, startPoint = VerticalScroll, startPoint.x
    end
    
    local function __getPosition() return autotestingSystem:GetListScrollPosition(listControl) end

    -- move to start of list and check cell
    local isEnd = false
    if __getPosition() == finalPoint then isEnd = true end
    for _ = 0, MAX_LIST_COUNT do
        local position = __getPosition()
        if position <= startPoint then
            break
        end
        __scroll(listName, true)
        if __click() then
            return true
        end
    end

    -- move to end of list and check cell
    if not isEnd then
        for _ = 0, MAX_LIST_COUNT do
            local position = __getPosition()
            if position >= finalPoint then
                break
            end
            __scroll(listName)
            if __click() then
                return true
            end
        end
    end
    Log(string.format("Cell '%s' in '%s' list is not found", item, listName))
    return false
end

function SelectItemInContainer(containerName, item, notInCenter, __condition)
    Log(string.format("Select '%s' cell in '%s' container.", item, containerName))
    assert(WaitControl(containerName), "Couldn't find " .. containerName)

    if not __condition then
        __condition = function(x) return true end
    end

    local function __click()
        if IsVisible(item, containerName) and IsVisible(item) and __condition(item) then
            ClickControl(item)
            return true
        end
        return false
    end

    if __click() then
        return true
    end
    local containerCtrl, position, invert = GetControl(containerName)
    local startPoint = containerCtrl:GetPivotPoint()
    print(string.format('Start points X: %f; Y: %f', startPoint.x, startPoint.y))
    local finalPoint = autotestingSystem:GetMaxContainerOffsetSize(containerCtrl)
    print(string.format('Final point: X: %f; Y: %f', finalPoint.x, finalPoint.y))

    local function __getPosition() return autotestingSystem:GetContainerScrollPosition(containerCtrl) end

    -- move to start of list and check cell
    for _ = 0, MAX_LIST_COUNT do -- move to up side
        position = __getPosition()
        if position.y <= startPoint.y then
            break
        end
        VerticalScroll(containerName, true, notInCenter)
        if __click() then
            return true
        end
    end
    -- move to left side
    for _ = 0, MAX_LIST_COUNT do
        position = __getPosition()
        if position.x <= startPoint.x then
            break
        end
        HorizontalScroll(containerName, true)
        if __click() then
            return true
        end
    end
    -- move to rigth side and down side, then to left and down
    for _ = 0, MAX_LIST_COUNT do
        invert = _ % 2 ~= 0 -- if true - right side, else - left
        -- move to right/left side
        for __ = 0, MAX_LIST_COUNT do
            position = __getPosition()
            if invert then
                if position.x <= startPoint.x then
                    break
                end
            else
                if position.x >= finalPoint.x then
                    break
                end
            end
            HorizontalScroll(containerName, invert)
            if __click() then
                return true
            end
        end
        -- move to down side
        position = __getPosition()
        if position.y >= finalPoint.y then
            break
        end
        VerticalScroll(containerName, false, notInCenter)
        if __click() then
            return true
        end
    end
    Log(string.format("Item '%s' in '%s' container is not found", item, containerName))
    return false
end

function VerticalScroll(list, invert, notInCenter)
    local position, new_position = __GetNewPosition(list, true, invert, notInCenter)
    TouchMove(position, new_position)
end

function HorizontalScroll(list, invert, notInCenter)
    local position, new_position = __GetNewPosition(list, false, invert, notInCenter)
    TouchMove(position, new_position)
end

----------------------------------------------------------------------------------------------------
-- Touch and click actions
----------------------------------------------------------------------------------------------------

-- Touch down
function TouchDownPosition(pos, touchId, tapCount)
    local tapCount = tapCount or 1
    local touchId = touchId or 1
    local position = Vector.Vector2(pos.x, pos.y)
    autotestingSystem:TouchDown(position, touchId, tapCount)
    Yield()
end

function TouchDown(x, y, touchId)
    local position = Vector.Vector2(x, y)
    TouchDownPosition(position, touchId)
end

-- Touch up
function TouchUp(touchId)
    local touchId = touchId or 1
    autotestingSystem:TouchUp(touchId)
end

function ClickPosition(position, waitTime, touchId, tapCount)
    TouchDownPosition(position, touchId, tapCount)
    Wait(waitTime)
    TouchUp(touchId)
    Wait(waitTime)
end

function Click(x, y, waitTime, touchId)
    local waitTime = waitTime or TIMECLICK
    local touchId = touchId or 1
    local position = Vector.Vector2(x, y)
    ClickPosition(position, touchId, waitTime)
end

function KeyPress(key, control)
    if control then
        ClickControl(control)
    end
    autotestingSystem:KeyPress(key)
    Wait(TIMECLICK)
end

function ClickControl(name, waitTime, touchId)
    local waitTime = waitTime or TIMECLICK
    local touchId = touchId or 1
    Log("ClickControl name=" .. name .. " touchId=" .. touchId .. " waitTime=" .. waitTime)
    if IsReady(name) then
        local position = GetCenter(name)
        ClickPosition(position, waitTime, touchId)
        return true
    end
    return false
end

function DoubleClick(name, waitTime, touchId)
    local waitTime = waitTime or TIMECLICK
    local touchId = touchId or 1
    Log("DoubleClick name=" .. name .. " touchId=" .. touchId .. " waitTime=" .. waitTime)
    if IsReady(name) then
        local position = GetCenter(name)
        ClickPosition(position, waitTime, touchId, 2)
        return true
    end
    return false
end

function ShiftClickControl(name, x, y, touchId)
    Log(string.format("ShiftClickControl name=%s with shift on [%d, %d] touchId=%d", name, x, y, (touchId or 1)))
    if not WaitControl(name, TIMEOUT) then
        Log("Control " .. name .. " not found.")
        return false
    end
    if IsVisible(name) and IsCenterOnScreen(name) then
        local position = GetCenter(name)
        position.x = position.x + x
        position.y = position.y + y
        ClickPosition(position, TIMECLICK, touchId)
        return true
    end
    Log("Control " .. name .. " is not visible.")
    return false
end

-- Move touch actions
function TouchMovePosition(pos, touchId)
    local position = Vector.Vector2(pos.x, pos.y)
    autotestingSystem:TouchMove(position, touchId or 1)
    Yield()
end

function TouchMove(position, new_position, time, touchId)
    local waitTime = time or TIMECLICK
    TouchDownPosition(position, touchId)
    Wait(waitTime)
    TouchMovePosition(new_position, touchId)
    Wait(waitTime)
    Wait(waitTime)
    TouchUp(touchId)
    Wait(waitTime)
end