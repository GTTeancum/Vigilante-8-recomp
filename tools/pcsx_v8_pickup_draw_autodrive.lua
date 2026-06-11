local statusPath = [[C:\Programming\GitHub\Vigilante 8 recomp\analysis\pcsx_v8_pickup_draw_autodrive_status.txt]]

local function status(s)
    local f = io.open(statusPath, "a")
    if f then
        f:write(s, "\n")
        f:close()
    end
end

status("pickup draw autodrive launcher start")

local pad = PCSX.SIO0.slots[1].pads[1]
local B = PCSX.CONSTS.PAD.BUTTON
local pressed = {}
local frame = 0

_G.v8_pickup_draw_autodrive_listeners = _G.v8_pickup_draw_autodrive_listeners or {}
local listeners = _G.v8_pickup_draw_autodrive_listeners

local traceOk, traceErr = pcall(dofile, [[C:\Programming\GitHub\Vigilante 8 recomp\tools\pcsx_v8_pickup_draw_trace.lua]])
status("trace immediate install ok=" .. tostring(traceOk) .. " err=" .. tostring(traceErr))
if not traceOk then
    PCSX.quit(2)
    return
end

local function press(button)
    if not pressed[button] then
        pad.setOverride(button)
        pressed[button] = true
    end
end

local function release(button)
    if pressed[button] then
        pad.clearOverride(button)
        pressed[button] = nil
    end
end

local function releaseAll()
    for b, _ in pairs(pressed) do
        release(b)
    end
end

local function pulse(button, period, held)
    if (frame % period) < held then
        press(button)
    end
end

local function onVsync()
    frame = frame + 1
    releaseAll()

    if frame < 2100 then
        pulse(B.START, 36, 8)
        pulse(B.CROSS, 48, 10)
    elseif frame < 4200 then
        pulse(B.CROSS, 36, 10)
        pulse(B.START, 180, 10)
    elseif frame < 18000 then
        press(B.CROSS)
        pulse(B.CIRCLE, 90, 14)
        pulse(B.TRIANGLE, 150, 12)
        pulse(B.SQUARE, 210, 12)
        local phase = frame % 420
        if phase < 130 then
            press(B.RIGHT)
        elseif phase >= 210 and phase < 340 then
            press(B.LEFT)
        end
    else
        status("pickup draw autodrive complete frame=" .. tostring(frame))
        releaseAll()
        PCSX.quit(0)
        return
    end

    if (frame % 120) == 0 then
        status("pickup draw autodrive frame=" .. tostring(frame))
    end
end

listeners[#listeners + 1] = PCSX.Events.createEventListener("GPU::Vsync", onVsync)
status("pickup draw autodrive installed")

