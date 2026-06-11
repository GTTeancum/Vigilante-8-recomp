local statusPath = [[C:\Programming\GitHub\Vigilante 8 recomp\analysis\pcsx_v8_attached_weapon_autodrive_status.txt]]

local function status(s)
    local f = io.open(statusPath, "a")
    if f then
        f:write(s, "\n")
        f:close()
    end
end

status("attached weapon autodrive launcher start")

local pad = PCSX.SIO0.slots[1].pads[1]
local B = PCSX.CONSTS.PAD.BUTTON
local pressed = {}
local frame = 0
_G.v8_attached_weapon_autodrive_listeners = _G.v8_attached_weapon_autodrive_listeners or {}
local listeners = _G.v8_attached_weapon_autodrive_listeners

local traceOk, traceErr = pcall(dofile, [[C:\Programming\GitHub\Vigilante 8 recomp\tools\pcsx_v8_attached_weapon_trace.lua]])
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

local function onVsync()
    frame = frame + 1
    releaseAll()

    if frame < 1800 then
        if (frame % 24) < 6 then press(B.START) end
        if (frame % 40) >= 12 and (frame % 40) < 20 then press(B.CROSS) end
    elseif frame < 3600 then
        if (frame % 30) < 8 then press(B.CROSS) end
        if (frame % 180) < 8 then press(B.START) end
    elseif frame < 14400 then
        press(B.CROSS)
        if (frame % 90) < 15 then press(B.CIRCLE) end
        if (frame % 150) < 12 then press(B.TRIANGLE) end
        if (frame % 210) < 12 then press(B.SQUARE) end
        local phase = frame % 360
        if phase < 110 then
            press(B.RIGHT)
        elseif phase >= 180 and phase < 290 then
            press(B.LEFT)
        end
    else
        status("attached weapon autodrive complete frame=" .. tostring(frame))
        releaseAll()
        PCSX.quit(0)
        return
    end

    if (frame % 120) == 0 then
        status("attached weapon autodrive frame=" .. tostring(frame))
    end
    PCSX.resumeEmulator()
end

listeners[#listeners + 1] = PCSX.Events.createEventListener("GPU::Vsync", onVsync)
status("attached weapon autodrive installed")
PCSX.resumeEmulator()
