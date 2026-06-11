-- PCSX-Redux trace harness for original Vigilante 8 attached weapons.
--
-- Boot-safe version: breakpoints are installed immediately, but callbacks only
-- dereference objects once their callback fields look like real game pointers.
-- This avoids the title/load freeze caused by reading uninitialized boot data.

local bit = bit

local outPath = [[C:\Programming\GitHub\Vigilante 8 recomp\analysis\pcsx_v8_attached_weapon_trace.log]]
local statusPath = [[C:\Programming\GitHub\Vigilante 8 recomp\analysis\pcsx_v8_attached_weapon_trace_status.txt]]
local out = nil
local counts = {}
local statusCounts = {}
_G.v8_attached_weapon_trace_listeners = _G.v8_attached_weapon_trace_listeners or {}
_G.v8_attached_weapon_trace_breakpoints = _G.v8_attached_weapon_trace_breakpoints or {}
local listeners = _G.v8_attached_weapon_trace_listeners
local breakpoints = _G.v8_attached_weapon_trace_breakpoints
local frame = 0

local function openLog()
    if out ~= nil then return out end
    local ok, f = pcall(io.open, outPath, "w")
    if ok and f then
        out = f
        return out
    end
    error("could not open trace log: " .. outPath)
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
    return tonumber(ramPtr(addr)[0])
end

local function ru16(addr)
    local p = ramPtr(addr)
    return tonumber(p[0]) + bit.lshift(tonumber(p[1]), 8)
end

local function ru32(addr)
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
    return addr >= 0x80010000 and addr < 0x80100000
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
    return counts[name] <= max
end

local function markHit(name)
    statusCounts[name] = (statusCounts[name] or 0) + 1
    if statusCounts[name] == 1 or (statusCounts[name] % 100) == 0 then
        status(string.format("hit %s count=%d", name, statusCounts[name]))
    end
end

local function objSummary(addr)
    addr = realObj(addr)
    if not addr then
        return "obj=invalid"
    end
    return string.format(
        "obj=%08x flags=%08x type=%02x id=%d state08=%02x kind0a=%d hp=%d maxhp=%d cb=%08x bank=%08x child=%08x next=%08x pos24=(%d,%d,%d) pos48=(%d,%d,%d)",
        addr, ru32(addr), ru8(addr + 4), sx16(ru16(addr + 6)), ru8(addr + 8),
        sx16(ru16(addr + 0x0a)), sx16(ru16(addr + 0x0c)), sx16(ru16(addr + 0x0e)),
        ru32(addr + 0x64), ru32(addr + 0x58), ru32(addr + 0x38), ru32(addr + 0x34),
        sx32(ru32(addr + 0x24)), sx32(ru32(addr + 0x28)), sx32(ru32(addr + 0x2c)),
        sx32(ru32(addr + 0x48)), sx32(ru32(addr + 0x4c)), sx32(ru32(addr + 0x50)))
end

local function vehicleSummary(addr)
    addr = realObj(addr)
    if not addr then
        return "veh=invalid"
    end
    return string.format(
        "veh=%08x flags=%08x id=%d state08=%02x meshd0=%d target=%08x activeWeapon=%02x slot43=%08x slots=(%08x,%08x,%08x) pos=(%d,%d,%d) speed8c=%d",
        addr, ru32(addr), sx16(ru16(addr + 6)), ru8(addr + 8), ru8(addr + 0xd0),
        ru32(addr + 0xe4), ru8(addr + 0xb3), ru32(addr + 0x10c),
        ru32(addr + 0x110), ru32(addr + 0x114), ru32(addr + 0x118),
        sx32(ru32(addr + 0x48)), sx32(ru32(addr + 0x4c)), sx32(ru32(addr + 0x50)),
        sx32(ru32(addr + 0x8c)))
end

local function shouldLogObj(addr)
    return realObj(addr) ~= nil
end

local function shouldLogVeh(addr)
    return realObj(addr) ~= nil
end

local function liveCounts()
    local weaponLive = ru32(0x80065a10)
    local weaponMax = ru32(0x80065a14)
    local utilityLive = ru32(0x80065aac)
    local utilityMax = ru32(0x80065ab0)
    return weaponLive, weaponMax, utilityLive, utilityMax
end

local function attachSlotKinds(veh)
    veh = realObj(veh)
    if not veh then
        return 8, 8, 8
    end
    local s0 = ru32(veh + 0x110)
    local s1 = ru32(veh + 0x114)
    local s2 = ru32(veh + 0x118)
    local k0 = realObj(s0) and ru8(s0 + 8) or 8
    local k1 = realObj(s1) and ru8(s1 + 8) or 8
    local k2 = realObj(s2) and ru8(s2 + 8) or 8
    return k0, k1, k2
end

local function attachState(veh)
    veh = realObj(veh)
    if not veh then
        return "vehicle=invalid"
    end
    local k0, k1, k2 = attachSlotKinds(veh)
    return string.format(
        "vehicle=%08x active=%d cooldown=%d comboCtr=%d specialState=%d timers(shield=%d upgrade=%d jammer=%d) slots=[%08x,%08x,%08x] slotKinds=[%d,%d,%d] target=%08x chassis=%08x",
        veh, ru8(veh + 0xb3), sx16(ru16(veh + 0xb4)), sx16(ru16(veh + 0xb6)), ru8(veh + 0xb8),
        sx16(ru16(veh + 0xba)), sx16(ru16(veh + 0xbc)), sx16(ru16(veh + 0xbe)),
        ru32(veh + 0x110), ru32(veh + 0x114), ru32(veh + 0x118),
        k0, k1, k2, ru32(veh + 0xe4), ru32(veh + 0x10c))
end

local function rawRegs()
    local r = regs()
    return string.format("ra=%08x a0=%08x a1=%08x a2=%08x a3=%08x t0=%08x t1=%08x t2=%08x v0=%08x v1=%08x",
        u32(r.ra), u32(r.a0), u32(r.a1), u32(r.a2), u32(r.a3),
        u32(r.t0), u32(r.t1), u32(r.t2), u32(r.v0), u32(r.v1))
end

local function addExec(addr, label, maxRows, fn)
    local bp = PCSX.addBreakpoint(addr, "Exec", 4, label, function(address, width, cause)
        local ok, err = pcall(function()
            markHit(label)
            if maxRows == nil or limited(label, maxRows) then
                fn(address, width, cause)
            end
        end)
        if not ok then
            logf("ERROR cyc=%d label=%s pc=%08x err=%s\n",
                cycle(), label, pc(), tostring(err))
        end
        return true
    end, label)
    if bp and bp.enable then
        bp:enable()
    end
    breakpoints[#breakpoints + 1] = bp
end

logf("v8 attached weapon trace start\n")
logf("source-grounded breakpoints from previous full-match traces installed; object summaries are best-effort\n")
status("attached weapon trace installed source-grounded")

local function addWeaponCallback(addr, label, maxRows)
    addExec(addr, label, maxRows, function()
        local r = regs()
        logf("WEAPON frame=%d cyc=%d cb=%s pc=%08x %s event=%d param3=%08x self{%s} owner{%s}\n",
            frame, cycle(), label, pc(), rawRegs(), sx16(r.a1), u32(r.a2),
            objSummary(r.a0), attachState(r.a2))
    end)
end

local function addPickupCallback(addr, label, maxRows)
    addExec(addr, label, maxRows, function()
        local r = regs()
        local weaponLive, weaponMax, utilityLive, utilityMax = liveCounts()
        logf("PICKUP frame=%d cyc=%d %s pc=%08x %s event=%d param3=%08x weaponLive=%d/%d utilityLive=%d/%d obj{%s}\n",
            frame, cycle(), label, pc(), rawRegs(), sx16(r.a1), u32(r.a2),
            weaponLive, weaponMax, utilityLive, utilityMax, objSummary(r.a0))
end)
end

listeners[#listeners + 1] = PCSX.Events.createEventListener("GPU::Vsync", function()
    frame = frame + 1
    if frame <= 10 or (frame % 120) == 0 then
        local weaponLive, weaponMax, utilityLive, utilityMax = liveCounts()
        logf("VSYNC frame=%d cyc=%d pc=%08x weaponLive=%d/%d utilityLive=%d/%d\n",
            frame, cycle(), pc(), weaponLive, weaponMax, utilityLive, utilityMax)
    end
    PCSX.resumeEmulator()
end)

addExec(0x8002d494, "Vehicle_WeaponDispatch", 5000, function()
    local r = regs()
    logf("WEAPON frame=%d cyc=%d vehicleDispatch pc=%08x %s eventNode=%08x owner{%s}\n",
        frame, cycle(), pc(), rawRegs(), u32(r.a0), attachState(r.a1))
end)

addExec(0x8002c99c, "Attach_PrepareA", 2000, function()
    local r = regs()
    logf("ATTACH frame=%d cyc=%d prepareA pc=%08x %s owner{%s} child{%s}\n",
        frame, cycle(), pc(), rawRegs(), attachState(r.a0), objSummary(r.a1))
end)

addExec(0x8002ca94, "Attach_PrepareB", 2000, function()
    local r = regs()
    logf("ATTACH frame=%d cyc=%d prepareB pc=%08x %s owner{%s} child{%s}\n",
        frame, cycle(), pc(), rawRegs(), attachState(r.a0), objSummary(r.a1))
end)

addWeaponCallback(0x80031864, "chassisGun_80031864", 5000)
addWeaponCallback(0x800336fc, "attachType_800336fc", 3000)
addWeaponCallback(0x80034920, "attachType_80034920", 5000)
addWeaponCallback(0x8003b3c8, "attachType_8003b3c8", 3000)
addWeaponCallback(0x8003ff28, "attachType_8003ff28", 1000)
addWeaponCallback(0x8003d1e8, "specialCallback_8003d1e8", 2000)

addPickupCallback(0x80023d00, "worldManager", 6000)
addPickupCallback(0x800239c0, "populateGeneral", 6000)
addPickupCallback(0x80023a60, "populateUtility", 6000)
addPickupCallback(0x8003d0d0, "populateSpecial", 6000)
addPickupCallback(0x8001edb4, "collide", 6000)
addPickupCallback(0x8001ffd4, "listFind", 3000)
addPickupCallback(0x80020120, "liveCount", 3000)
addPickupCallback(0x80020190, "selectSlot", 6000)
addPickupCallback(0x800202f4, "liveRegA", 6000)
addPickupCallback(0x8002036c, "liveRegB", 6000)
addPickupCallback(0x800205f8, "remove", 6000)
addPickupCallback(0x80020890, "scheduleRespawn", 6000)
addPickupCallback(0x80021394, "queueCallback", 6000)
addPickupCallback(0x8003ce24, "surpriseRandom", 6000)

addExec(0x80021b80, "Object_AllocFromCallback", 4000, function()
    local r = regs()
    logf("ALLOC_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x a2=%08x a3=%08x cbText=%d\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), u32(r.a2), u32(r.a3), textPtr(r.a0) and 1 or 0)
end)

addExec(0x80021c6c, "Spawn_ClonePlaceholder", 4000, function()
    local r = regs()
    logf("CLONE_RAW cyc=%d pc=%08x ra=%08x a0=%08x %s\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), objSummary(r.a0))
end)

addExec(0x8002cbe8, "Vehicle_LinkAttachedObject", 4000, function()
    local r = regs()
    logf("LINK_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x %s child{%s}\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), vehicleSummary(r.a0), objSummary(r.a1))
end)

addExec(0x8002cce8, "Vehicle_InitJoints", 2000, function()
    local r = regs()
    logf("JOINT_INIT_RAW cyc=%d pc=%08x ra=%08x a0=%08x mask=%08x %s\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), vehicleSummary(r.a0))
end)

addExec(0x8003d188, "Vehicle_QueryAttachedJoint", 4000, function()
    local r = regs()
    logf("JOINT_QUERY_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x owner{%s} sub{%s}\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), vehicleSummary(r.a0), objSummary(r.a1))
end)

addExec(0x8002ea94, "Vehicle_Retarget", 3000, function()
    local r = regs()
    logf("RETARGET_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x flag=%d %s\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), sx16(r.a1), vehicleSummary(r.a0))
end)

addExec(0x8002ce68, "Vehicle_AttachedDispatch", 9000, function()
    local r = regs()
    logf("ATTACH_DISPATCH_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x lock=%d %s\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), sx16(r.a1), vehicleSummary(r.a0))
end)

addExec(0x8003c61c, "Pickup_Callback", 9000, function()
    local r = regs()
    logf("PICKUP_CB_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x a2=%08x event=%d param=%08x %s\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), u32(r.a2), sx16(r.a1), u32(r.a2), objSummary(r.a0))
end)

addExec(0x80031fa0, "AttachState_Callback_1", 9000, function()
    local r = regs()
    logf("STATE_31FA0_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x a2=%08x event=%d owner=%08x sub{%s} owner{%s}\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), u32(r.a2), sx16(r.a1), u32(r.a2),
        objSummary(r.a0), vehicleSummary(r.a2))
end)

addExec(0x8003302c, "AttachState_Callback_2", 9000, function()
    local r = regs()
    logf("STATE_3302C_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x a2=%08x event=%d owner=%08x sub{%s} owner{%s}\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), u32(r.a2), sx16(r.a1), u32(r.a2),
        objSummary(r.a0), vehicleSummary(r.a2))
end)

addExec(0x8003565c, "AttachState_Seeker", 12000, function()
    local r = regs()
    local pivot = realObj(r.a0) and ru32(u32(r.a0) + 0x38) or 0
    logf("SEEKER_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x a2=%08x event=%d sub{%s} owner{%s} pivot{%s}\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), u32(r.a2), sx16(r.a1), objSummary(r.a0),
        vehicleSummary(r.a2), objSummary(pivot))
end)

addExec(0x80031300, "Projectile_SpawnFromAttached", 9000, function()
    local r = regs()
    logf("PROJECTILE_RAW cyc=%d pc=%08x ra=%08x a0=%08x a1=%08x a2=%08x a3=%08x t0=%08x parent{%s} carrier{%s} kind=%d size=%08x joint=%08x\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), u32(r.a1), u32(r.a2), u32(r.a3), u32(r.t0), vehicleSummary(r.a0), objSummary(r.a1),
        sx16(r.a2), u32(r.a3), u32(r.t0))
end)

addExec(0x8002305c, "Vehicle_PlayerWeaponTick", 6000, function()
    local r = regs()
    logf("PLAYER_WEAPON_TICK_RAW cyc=%d pc=%08x ra=%08x a0=%08x %s\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), vehicleSummary(r.a0))
end)

addExec(0x8002346c, "Vehicle_AIWeaponTick", 6000, function()
    local r = regs()
    logf("AI_WEAPON_TICK_RAW cyc=%d pc=%08x ra=%08x a0=%08x %s\n",
        cycle(), pc(), u32(r.ra), u32(r.a0), vehicleSummary(r.a0))
end)

logf("breakpoints installed\n")
PCSX.resumeEmulator()
