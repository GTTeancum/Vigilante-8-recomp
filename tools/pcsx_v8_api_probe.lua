local outPath = [[C:\Programming\GitHub\Vigilante 8 recomp\analysis\pcsx_v8_api_probe.txt]]
local out = io.open(outPath, "w")

local function log(s)
    if out then
        out:write(tostring(s), "\n")
        out:flush()
    end
end

local function dump(name, value, depth)
    depth = depth or 0
    if depth > 2 then return end
    log(string.rep(" ", depth * 2) .. name .. " = " .. type(value) .. " " .. tostring(value))
    if type(value) == "table" then
        for k, v in pairs(value) do
            dump(tostring(k), v, depth + 1)
        end
    end
end

log("api probe start")
dump("PCSX", PCSX, 0)
dump("PCSX.Events", PCSX and PCSX.Events, 0)
dump("PCSX.CONSTS", PCSX and PCSX.CONSTS, 0)
dump("PCSX.SIO0", PCSX and PCSX.SIO0, 0)

local names = {
    "GPU::Vsync",
    "GPU::VSync",
    "GPU::vsync",
    "GPU::VBlank",
    "GPU::vblank",
    "Emulator::VSync",
    "Emulator::Frame",
    "GUI::Frame",
}

for _, name in ipairs(names) do
    local ok, err = pcall(function()
        PCSX.Events.createEventListener(name, function()
            log("event fired " .. name)
        end)
    end)
    log("listener " .. name .. " ok=" .. tostring(ok) .. " err=" .. tostring(err))
end

PCSX.resumeEmulator()
