-- Focused PCSX-Redux source trace for Vigilante 8 pickup / attached weapon
-- visual materialization.  This intentionally traces the original SLUS draw
-- path instead of inferring host packet layout from symptoms.

local bit = bit

local outPath = [[C:\Programming\GitHub\Vigilante 8 recomp\analysis\pcsx_v8_pickup_draw_trace.log]]
local statusPath = [[C:\Programming\GitHub\Vigilante 8 recomp\analysis\pcsx_v8_pickup_draw_trace_status.txt]]
local out = nil
local frame = 0
local counts = {}
local statusCounts = {}
local drawStack = {}
local watched = {}

_G.v8_pickup_draw_trace_listeners = _G.v8_pickup_draw_trace_listeners or {}
_G.v8_pickup_draw_trace_breakpoints = _G.v8_pickup_draw_trace_breakpoints or {}
local listeners = _G.v8_pickup_draw_trace_listeners
local breakpoints = _G.v8_pickup_draw_trace_breakpoints

local function openLog()
    if out ~= nil then return out end
    local ok, f = pcall(io.open, outPath, "w")
    if ok and f then
        out = f
        return out
    end
    error("could not open pickup draw trace log: " .. outPath)
end

local function logf(fmt, ...)
    local f = openLog()
    f:write(string.format(fmt, ...))
    f:flush()
end

local function status(s)
    local f = io.open(statusPath, "a")
    if f then
        f:write(s, "\n")
        f:close()
    end
end

local function u32(x)
    return tonumber(x or 0) % 0x100000000
end

local function sx16(x)
    x = bit.band(u32(x), 0xffff)
    if x >= 0x8000 then return x - 0x10000 end
    return x
end

local function sx32(x)
    x = u32(x)
    if x >= 0x80000000 then return x - 0x100000000 end
    return x
end

local function ramPtr(addr)
    return PCSX.getMemPtr() + bit.band(u32(addr), 0x1fffff)
end

local function ru8(addr)
    if not addr or not (u32(addr) >= 0x80000000 and u32(addr) < 0x80200000) then return 0 end
    return tonumber(ramPtr(addr)[0])
end

local function ru16(addr)
    if not addr or not (u32(addr) >= 0x80000000 and u32(addr) < 0x80200000) then return 0 end
    local p = ramPtr(addr)
    return tonumber(p[0]) + bit.lshift(tonumber(p[1]), 8)
end

local function ru32(addr)
    if not addr or not (u32(addr) >= 0x80000000 and u32(addr) < 0x80200000) then return 0 end
    local p = ramPtr(addr)
    return u32(tonumber(p[0]) +
        bit.lshift(tonumber(p[1]), 8) +
        bit.lshift(tonumber(p[2]), 16) +
        bit.lshift(tonumber(p[3]), 24))
end

local function regs()
    return PCSX.getRegisters().GPR.n
end

local function pc()
    return u32(PCSX.getRegisters().pc)
end

local function cycle()
    return tonumber(PCSX.getCPUCycles())
end

local function textPtr(addr)
    addr = u32(addr)
    return addr >= 0x80010000 and addr < 0x80120000
end

local function ramObj(addr)
    addr = u32(addr)
    return addr >= 0x80010000 and addr < 0x80200000
end

local function cbLooksReal(addr)
    if not ramObj(addr) then return false end
    local cb = ru32(addr + 0x64)
    return cb == 0 or textPtr(cb)
end

local function realObj(addr)
    if not ramObj(addr) then return nil end
    if not cbLooksReal(addr) then return nil end
    return u32(addr)
end

local function limited(name, max)
    counts[name] = (counts[name] or 0) + 1
    return max == nil or counts[name] <= max
end

local function markHit(name)
    statusCounts[name] = (statusCounts[name] or 0) + 1
    if statusCounts[name] == 1 or (statusCounts[name] % 100) == 0 then
        status(string.format("hit %s count=%d", name, statusCounts[name]))
    end
end

local function rawRegs()
    local r = regs()
    return string.format("ra=%08x a0=%08x a1=%08x a2=%08x a3=%08x t0=%08x t1=%08x t2=%08x v0=%08x v1=%08x",
        u32(r.ra), u32(r.a0), u32(r.a1), u32(r.a2), u32(r.a3),
        u32(r.t0), u32(r.t1), u32(r.t2), u32(r.v0), u32(r.v1))
end

local function liveCounts()
    return ru32(0x80065a10), ru32(0x80065a14), ru32(0x80065aac), ru32(0x80065ab0)
end

local function objSummary(addr)
    addr = realObj(addr)
    if not addr then return "obj=invalid" end
    return string.format(
        "obj=%08x flags=%08x type=%02x id=%d state08=%02x kind0a=%d hp=%d maxhp=%d bank=%08x raw30=%08x next34=%08x child38=%08x cb=%08x sec68=%08x thresh6c=%d pos48=(%d,%d,%d)",
        addr, ru32(addr), ru8(addr + 4), sx16(ru16(addr + 6)), ru8(addr + 8),
        sx16(ru16(addr + 0x0a)), sx16(ru16(addr + 0x0c)), sx16(ru16(addr + 0x0e)),
        ru32(addr + 0x58), ru32(addr + 0x30), ru32(addr + 0x34), ru32(addr + 0x38),
        ru32(addr + 0x64), ru32(addr + 0x68), sx32(ru32(addr + 0x6c)),
        sx32(ru32(addr + 0x48)), sx32(ru32(addr + 0x4c)), sx32(ru32(addr + 0x50)))
end

local function shouldTraceObj(addr)
    addr = realObj(addr)
    if not addr then return false end
    local typ = ru8(addr + 4)
    local kind = sx16(ru16(addr + 0x0a))
    local cb = ru32(addr + 0x64)
    if typ == 3 or typ == 7 or cb == 0x8003c61c then return true end
    if kind == 13 or kind == 14 or kind == 6 or kind == 9 then return true end
    if watched[addr] then return true end
    return false
end

local function cacheSummary(cached)
    cached = u32(cached)
    if cached == 0 or not ramObj(cached) then return "cache=0" end
    return string.format(
        "cache=%08x flags=%04x genBytes=%d verts=%08x primCount=%d src=%08x gen=%08x scale=%d texBase=%04x",
        cached, ru16(cached + 0x00), ru16(cached + 0x02), ru32(cached + 0x08),
        ru16(cached + 0x14), ru32(cached + 0x18), ru32(cached + 0x1c),
        ru8(cached + 0x26), ru16(cached + 0x2a))
end

local function dumpBytes(label, base, size, maxRows)
    base = u32(base)
    if base == 0 or not ramObj(base) then return end
    local rows = math.min(maxRows or 8, math.floor((size or 0x10) / 0x10))
    for row = 0, rows - 1 do
        local a = base + row * 0x10
        logf("    %s[%02d] @%08x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
            label, row, a,
            ru8(a + 0), ru8(a + 1), ru8(a + 2), ru8(a + 3),
            ru8(a + 4), ru8(a + 5), ru8(a + 6), ru8(a + 7),
            ru8(a + 8), ru8(a + 9), ru8(a + 10), ru8(a + 11),
            ru8(a + 12), ru8(a + 13), ru8(a + 14), ru8(a + 15))
    end
end

local function dumpCache(cached, prefix)
    cached = u32(cached)
    if cached == 0 or not ramObj(cached) then return end
    local primCount = ru16(cached + 0x14)
    local src = ru32(cached + 0x18)
    local gen = ru32(cached + 0x1c)
    local genBytes = ru16(cached + 0x02)
    logf("  %s %s\n", prefix, cacheSummary(cached))
    dumpBytes(prefix .. ".src", src, math.min(primCount * 0x10, 0x100), 10)
    dumpBytes(prefix .. ".gen", gen, math.min(genBytes, 0x100), 10)
end

local function addExec(addr, label, maxRows, fn)
    local bp = PCSX.addBreakpoint(addr, "Exec", 4, label, function(address, width, cause)
        local ok, err = pcall(function()
            markHit(label)
            if limited(label, maxRows) then
                fn(address, width, cause)
            end
        end)
        if not ok then
            logf("ERROR frame=%d cyc=%d label=%s pc=%08x err=%s\n",
                frame, cycle(), label, pc(), tostring(err))
        end
        return true
    end, label)
    if bp and bp.enable then bp:enable() end
    breakpoints[#breakpoints + 1] = bp
end

logf("v8 pickup draw executable trace start\n")
logf("targets: clone/callback/materialization/draw/cache packet consumption; no host inference\n")
status("pickup draw trace installed")

listeners[#listeners + 1] = PCSX.Events.createEventListener("GPU::Vsync", function()
    frame = frame + 1
    if frame <= 10 or (frame % 120) == 0 then
        local wl, wm, ul, um = liveCounts()
        logf("VSYNC frame=%d cyc=%d pc=%08x live weapon=%d/%d utility=%d/%d\n",
            frame, cycle(), pc(), wl, wm, ul, um)
    end
end)

addExec(0x80021c6c, "Spawn_ClonePlaceholder", 2500, function()
    local r = regs()
    local src = realObj(r.a0)
    if src then watched[src] = true end
    logf("CLONE frame=%d cyc=%d %s src{%s}\n",
        frame, cycle(), rawRegs(), objSummary(r.a0))
end)

addExec(0x8001ac44, "Object_BuildFromTemplate", 3000, function()
    local r = regs()
    local flags = u32(r.a3)
    local slot = bit.band(flags, 0x7ff)
    if slot == 13 or slot == 14 or slot == 6 or slot == 9 or bit.band(flags, 0xc000) ~= 0 then
        logf("BUILDOBJ frame=%d cyc=%d %s bank=%08x templ=%08x flags=%08x slot=%d\n",
            frame, cycle(), rawRegs(), u32(r.a0), u32(r.a1), flags, slot)
    end
end)

addExec(0x8001b49c, "Cache_BuildGroup", 5000, function()
    local r = regs()
    local key = bit.band(u32(r.a1), 0x7ff)
    if key == 13 or key == 14 or key == 6 or key == 9 or limited("Cache_BuildGroup.extra", 100) then
        logf("CACHE_BUILD frame=%d cyc=%d %s bank=%08x key=%d table=%08x desc=%08x\n",
            frame, cycle(), rawRegs(), u32(r.a0), key, ru32(u32(r.a0) + 0x10), ru32(ru32(u32(r.a0) + 0x10) + key * 4))
    end
end)

addExec(0x8003c61c, "Pickup_Callback", 10000, function()
    local r = regs()
    local obj = realObj(r.a0)
    if obj then watched[obj] = true end
    local wl, wm, ul, um = liveCounts()
    logf("PICKUP_CB frame=%d cyc=%d %s event=%d param3=%08x live=%d/%d,%d/%d obj{%s}\n",
        frame, cycle(), rawRegs(), sx16(r.a1), u32(r.a2), wl, wm, ul, um, objSummary(r.a0))
end)

addExec(0x8001de08, "Object_DrawTree", 14000, function()
    local r = regs()
    local obj = realObj(r.a0)
    if shouldTraceObj(r.a0) then
        drawStack[#drawStack + 1] = obj or 0
        logf("DRAWOBJ frame=%d cyc=%d %s obj{%s}\n",
            frame, cycle(), rawRegs(), objSummary(r.a0))
        if obj then
            dumpCache(ru32(obj + 0x30), "obj30")
            dumpCache(ru32(obj + 0x68), "obj68")
        end
    else
        drawStack[#drawStack + 1] = 0
    end
end)

addExec(0x8001be5c, "Cache_Draw", 14000, function()
    local r = regs()
    local cur = drawStack[#drawStack] or 0
    local cached = u32(r.a0)
    local rel = "unknown"
    if cur ~= 0 and cached == ru32(cur + 0x30) then rel = "obj30" end
    if cur ~= 0 and cached == ru32(cur + 0x68) then rel = "obj68" end
    if cur ~= 0 or limited("Cache_Draw.extra", 200) then
        logf("DRAWCACHE frame=%d cyc=%d %s owner=%08x rel=%s %s\n",
            frame, cycle(), rawRegs(), cur, rel, cacheSummary(cached))
        dumpCache(cached, "draw")
    end
end)

addExec(0x80031300, "Projectile_SpawnFromAttached", 2500, function()
    local r = regs()
    logf("PROJ_SPAWN frame=%d cyc=%d %s shooter{%s} weapon{%s}\n",
        frame, cycle(), rawRegs(), objSummary(r.a0), objSummary(r.a1))
end)

addExec(0x8003565c, "AttachState_Seeker", 5000, function()
    local r = regs()
    local self = realObj(r.a0)
    if self then watched[self] = true end
    logf("SEEKER frame=%d cyc=%d %s event=%d self{%s} owner{%s}\n",
        frame, cycle(), rawRegs(), sx16(r.a1), objSummary(r.a0), objSummary(r.a2))
end)

