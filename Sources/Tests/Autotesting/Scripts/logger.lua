-- module setup
local modname = ...
local M = {}
_G[modname] = M
package.loaded[modname] = M

setmetatable(M, {__index = _G})

-- Import Section:
-- declare everything module needs from outside
local cprint = print

-- no  more external access after this point
--setfenv(1, M)


-- Privat functions
local function get_date_time()
	local date = os.date("*t",os.time())

	return string.format("%4.0f-%02.0f-%02.0f %02.0f:%02.0f:%02.0f  ", date.year, date.month, date.day, date.hour, date.min, date.sec)
end

-- Functions
function print(txt)
	local output
	if txt == nil then
		output = ""	
	else
		output = get_date_time() .. tostring(txt)	
	end
	
	local log = io.open("test_log.txt", "a")
	
	local function foo()
		log:write(output)
	end
	
	local status, err = pcall(foo)
	if not status then
		log:write("Couldn't write to file:\n" .. err)
	end
	
	log:write("\n")
	log:flush()
	log:close()
end

