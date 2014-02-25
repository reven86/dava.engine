TIMEOUT = 30.0 -- Big time out for waiting
TIMECLICK = 0.2 -- time for simple action
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
-- High-level test function
----------------------------------------------------------------------------------------------------
-- This function for simple test step without any assertion. Fail while error throwing
-- Parameters:
--		description - step description
--		func - link to step function
--		... - input parameters for step function
function Step(description, func, ...)
    autotestingSystem:OnStepStart(description)
	Yield()
	local status, err = copcall(func, ...)
	
	Yield()
	if not status then OnError(err) end
end

-- This function for test step with assertion. Fail when step is returned NIL or FALSE
-- Parameters:
--		description - step description
--		func - link to step function
--		... - input parameters for step function
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

-- Work with UI controls
function GetControl(name)
	local control
	if (type(name) == "string") then
		control = autotestingSystem:FindControl(name)
		--Log(tostring(control))
	else
		control = name
	end
	
	Yield()
	
	if control then
		return control
	else
		OnError("Couldn't find control " .. tostring(name))
	end
end

function GetCenter(element)
	local control = GetControl(element)
	
	--Log(tostring(control))
	if control then
		local position = Vector.Vector2()
		--Log("control:GetGeometricData")
		local geomData = control:GetGeometricData()
		--Log("geomData:GetUnrotatedRect")
		local rect = geomData:GetUnrotatedRect()
	            
		position.x = rect.x + rect.dx/2
		position.y = rect.y +rect.dy/2
		--Log("Return position")
		--Log(string.format("Return position of element center [%d, %d]", position.x, position.y))
		return position
	else
		OnError("Couldn't find element: "..element)
	end
end

function GetText(element)
	local control = GetControl(element)

	return autotestingSystem:GetText(control)
end

function IsVisible(element, background)
	Yield()
	local control = autotestingSystem:FindControl(element)
	if control and control:GetVisible() then
		Yield()
		if background then
			local back = autotestingSystem:FindControl(background)
			assert(back, background.." background not found")
			local geomData = control:GetGeometricData()
            local rect = geomData:GetUnrotatedRect()
			local geomData = back:GetGeometricData()
			local backRect = geomData:GetUnrotatedRect()
            --Log("Control "..tostring(rect.x)..","..tostring(rect.y).." ["..tostring(rect.dx)..", "..tostring(rect.dy).."]")
            --Log("Background "..tostring(backRect.x)..","..tostring(backRect.y).." ["..tostring(backRect.dx)..", "..tostring(backRect.dy).."]")
			
			if (rect.x >= backRect.x) and (rect.x + rect.dx <= backRect.x + backRect.dx) and (rect.y >= backRect.y) and (rect.y + rect.dy <= backRect.y + backRect.dy) then
				return true
			else
				return false
			end
		else
			return true
		end
	else
		return false
	end
end

function IsDisabled(element)
	Yield()
	local control = GetControl(element)
	if control:GetDisabled() then
		return true
	else
		return false
	end 
end

function IsOnScreen(control)
	local screen = autotestingSystem:GetScreen()
	local geomData = control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
	local geomData = screen:GetGeometricData()
	local backRect = geomData:GetUnrotatedRect()
    --Log("Control "..tostring(rect.x)..","..tostring(rect.y).." ["..tostring(rect.dx)..", "..tostring(rect.dy).."]")
    --Log("Background "..tostring(backRect.x)..","..tostring(backRect.y).." ["..tostring(backRect.dx)..", "..tostring(backRect.dy).."]")
			
	if (rect.x >= backRect.x) and (rect.x + rect.dx <= backRect.x + backRect.dx) and (rect.y >= backRect.y) and (rect.y + rect.dy <= backRect.y + backRect.dy) then
		return true
	else
		return false
	end
end

function IsCenterOnScreen(control)
	local screen = autotestingSystem:GetScreen()
    local center = GetCenter(control)
	local geomData = screen:GetGeometricData()
	local backRect = geomData:GetUnrotatedRect()
			
	if (center.x >= backRect.x) and (center.x <= backRect.x + backRect.dx) and (center.y >= backRect.y) and (center.y <= backRect.y + backRect.dy) then
		return true
	else
		return false
	end
end

function Wait(waitTime)
    waitTime =  waitTime or DELAY
    local count = 0
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        count = count + 1
    end
    
    return count
end

function GetTimeElapsed()
	return autotestingSystem:GetTimeElapsed()

end

function WaitControl(name, time)
    local waitTime = time or TIMEOUT
    --Log("WaitControl name="..name.." waitTime="..waitTime,"DEBUG")
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
		coroutine.yield()
        
        if autotestingSystem:FindControl(name) then
            --Log("WaitControl found "..name, "DEBUG")
            return true
        end
    end
    
    Log("WaitControl not found "..name, "DEBUG")
    return false
end

function WaitControlDisappeared(name, time)
    local waitTime = time or TIMEOUT
    --Log("WaitControl name="..name.." waitTime="..waitTime,"DEBUG")
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
		coroutine.yield()
        
        if not autotestingSystem:FindControl(name) then
            --Log("WaitControl found "..name, "DEBUG")
            return true
        end
    end
    
    Log("WaitControl still on the screen: "..name, "DEBUG")
    return false
end

function TouchDownPosition(position, touchId)
    local touchId = touchId or 1
    --print("TouchDownPosition position="..position.x..","..position.y.." touchId="..touchId)
    autotestingSystem:TouchDown(position, touchId)
     Yield()
end

function TouchDown(x, y, touchId)
    local touchId = touchId or 1
    --print("TouchDown x="..x.." y="..y.." touchId="..touchId)
    local position = Vector.Vector2(x, y)
    autotestingSystem:TouchDown(position, touchId)
    Yield()
end

function TouchMovePosition(position, touchId, waitTime)
	waitTime =  waitTime or TIMECLICK
    local touchId = touchId or 1
    Log("TouchMovePosition position="..position.x..","..position.y.." touchId="..touchId)
    autotestingSystem:TouchMove(position, touchId)
    Yield()
end

--[[ old and deprecated
function TouchMove(x, y, touchId, waitTime)
	waitTime =  waitTime or TIMECLICK
    local touchId = touchId or 1
    --Log("TouchMove x="..x.." y="..y.." touchId="..touchId)
    local position = Vector.Vector2(x, y)
    autotestingSystem:TouchMove(position, touchId)
    Wait(waitTime)
end
]]

function TouchMove(position, new_position, waitTime, touchId)
	waitTime =  waitTime or TIMECLICK
    local touchId = touchId or 1
    --Log("TouchMove x="..x.." y="..y.." touchId="..touchId)
    autotestingSystem:TouchDown(position, touchId)
    Wait(waitTime)
	autotestingSystem:TouchMove(new_position, touchId)
	Wait(waitTime)
	autotestingSystem:TouchUp(touchId)
	Wait(waitTime)
end

function TouchUp(touchId)
	local touchId = touchId or 1
    --Log("TouchUp "..touchId)
    autotestingSystem:TouchUp(touchId)
end

function ClickPosition(position, touchId)
    local touchId = touchId or 1
    --Log("ClickPosition position="..position.x..","..position.y.." touchId="..touchId)
    
    TouchDownPosition(position, touchId)
	Yield()
    TouchUp(touchId)
    Yield()
end

function Click(x, y, touchId)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    --print("Click x="..x.." y="..y.." touchId="..touchId)
    
    local position = Vector.Vector2(x, y)
    ClickPosition(position, touchId, waitTime)
end

function ClickControl(name, time, touchId)
    local waitTime = time or TIMEOUT
    local touchId = touchId or 1
	
    Log("ClickControl name="..name.." touchId="..touchId.." waitTime="..waitTime)
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
--        print("Searching "..elapsedTime)
        
        local control = autotestingSystem:FindControl(name)
        local screen = autotestingSystem:GetScreen()
		
        if control and IsVisible(name) and IsCenterOnScreen(control) then     
            -- local position = control:GetPosition(true)
            local position = GetCenter(name)
--            print(position)
--            print("position="..position.x..","..position.y)

            ClickPosition(position, touchId)
            
            return true
        else
			coroutine.yield()
        end
    end
    
	local control = autotestingSystem:FindControl(name)
	if  control then
		if not IsVisible(name) then
			Log(name .. " is not visible")
		end
		if not IsCenterOnScreen(control) then
			Log(name .. " is not on the Screen")
		end
	else
		Log("ClickControl not found "..name)
	end
    return false
end

-- Work with Text field and labels 

function SetText(path, text, time)
	local waitTime = time or DELAY
    Log("SetText path="..path.." text="..text)
    local res = autotestingSystem:SetText(path, text)
    Yield()
    Wait(waitTime)
    return res
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

function KeyPress(key, control)
	--Log("Emulate key press key="..key)
	if control then
		ClickControl(control)
	end
	autotestingSystem:KeyPress(key)
end

function ClearField(field)
	--Log("Emulate key press key="..key)
	SetText(field, "")
	ClickControl(field)
	KeyPress(2)
end

-- Work with List
function SelectHorizontal(list, item)
	Log("Select "..tostring(item).." item in horizontal list "..list)
	
	local cell = list.."/".. tostring(item)
	assert(WaitControl(list), "Couldn't select "..cell)
	
	if IsVisible(cell, list) then
		ClickControl(cell)
		return true
	end
	
	local last_visible = 0
	local previous_last = 0
	local index = 0
	
	-- find first visible element
	for i = 0, 100 do --to avoid hanging up in empty list
		if IsVisible(list.."/"..tostring(i)) then
			previous_last = i
			last_visible = i
			--Log( "previous_last = "..tostring(previous_last)..",last_visible = "..tostring(last_visible) )
			break
		end
	end
    
	-- find last visible
	index = previous_last + 1
	while true do
		if not IsVisible(list.."/"..tostring(index)) then
			last_visible = index - 1
			--Log( "last_visible = "..tostring(last_visible) )
			break
		end
		index = index + 1
	end
	
	repeat
		if IsVisible(cell, list) then
			break
		else
			previous_last = last_visible
			ScrollLeft(list)
			
			index = last_visible + 1
			while true do
				if not IsVisible(list.."/"..tostring(index)) then
					last_visible = index - 1
					--Log( "previous_last = "..tostring(previous_last) )
					break
				end
				index = index + 1
			end
		end
	until previous_last == last_visible
    
	if IsVisible(cell, list) then
		ClickControl(cell)
		return true
	else
		Log("Item "..item.." in "..list.." not found")
		return false
	end
end

function SelectFirstHorizontal(list)
	Log("Select first item in horizontal list "..list)
	
	-- find first visible element
	for i = 0, 100 do --to avoid hanging up in empty list
		if IsVisible(list.."/0", list) then
			return true
		else
			ScrollLeft(list, true)
		end
	end
    
	Log("First item in "..list.." not found")
	return false
end

function SelectVertical(list, item)
	Log("Select "..tostring(item).." item in vertical list "..list)
	local cell = list.."/".. tostring(item)
	assert(WaitControl(list), "Couldn't select "..cell)
	
	if IsVisible(cell, list) then
		ClickControl(cell)
		return true
	end
	
	local last_visible = 0
	local previous_last = 0
	local index = 0
	
	-- find first visible element
	for i = 0, 100 do --to avoid hanging up in empty list
		if IsVisible(list.."/"..tostring(i)) then
			previous_last = i
			last_visible = i
			--Log( "previous_last = "..tostring(previous_last)..",last_visible = "..tostring(last_visible) )
			break
		end
	end
    
	-- find last wisible
	index = previous_last + 1
	while true do
		if not IsVisible(list.."/"..tostring(index)) then
			last_visible = index - 1
			--Log( "last_visible = "..tostring(last_visible) )
			break
		end
		index = index + 1
	end
	
	repeat
		if IsVisible(cell, list) then
			break
		else
			previous_last = last_visible
			ScrollDown(list)

			index = last_visible + 1
			while true do
				if not IsVisible(list.."/"..tostring(index)) then
					last_visible = index - 1
					--Log( "previous_last = "..tostring(previous_last) )
					break
				end
				index = index + 1
			end
		end
	until previous_last == last_visible
	
	if IsVisible(cell, list) then
		ClickControl(cell)
		return true
	else
		Log("Item "..item.." in "..list.." not found")
		return false
	end
end

function SelectFirstVertical(list)
	Log("Select first item in vertical list "..list)
	
	-- find first visible element
	for i = 0, 100 do --to avoid hanging up in empty list
		if IsVisible(list.."/0", list) then
			return true
		else
			ScrollDown(list, true)
		end
	end
    
	Log("First item in "..list.." not found")
	return false
end

function ScrollDown(list, invert)
	local control = GetControl(list)

	local position = Vector.Vector2()
            
    local geomData = control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
       
    position.x = rect.x + rect.dx/2
    position.y = rect.y + rect.dy/2
		
	TouchDownPosition(position)
	Wait(0.5)
		
	if invert then
		position.y = position.y + rect.dy/3
	else
       	position.y = position.y - rect.dy/3
    end
        
	TouchMovePosition(position)
	TouchUp()
	Wait(0.5)
end

function ScrollLeft(list, invert)
	local control = GetControl(list)	
    
    local position = Vector.Vector2()
            
    local geomData = control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
       
    position.x = rect.x + rect.dx/2
    position.y = rect.y + rect.dy/2
		
	TouchDownPosition(position)
	Wait(0.5)
    if invert then
		position.x = position.x + rect.dx/3
	else
      	position.x = position.x - rect.dx/3
    end
	TouchMovePosition(position)
	TouchUp()
	Wait(0.5)
end