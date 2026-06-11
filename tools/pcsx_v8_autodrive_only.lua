local statusPath = [[C:\Programming\GitHub\Vigilante 8 recomp\analysis\pcsx_v8_autodrive_only_status.txt]]

local function status(s)
    local f = io.open(statusPath, "a")
    if f then
        f:write(s, "\n")
        f:close()
    end
end

local pad = PCSX.SIO0.slots[1].pads[1]
local B = PCSX.CONSTS.PAD.BUTTON
local pressed = {}
local frame = 0

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
    if (frame % 20) < 8 then press(B.START) end
    if (frame % 36) >= 8 and (frame % 36) < 18 then press(B.CROSS) end
    if frame > 3600 then
        press(B.CROSS)
        if (frame % 90) < 20 then press(B.RIGHT) end
        if (frame % 120) >= 60 and (frame % 120) < 80 then press(B.CIRCLE) end
    end
    if frame % 120 == 0 then status("frame=" .. tostring(frame)) end
    PCSX.resumeEmulator()
end

status("autodrive only installed")
PCSX.Events.createEventListener("GPU::Vsync", onVsync)
PCSX.resumeEmulator()
