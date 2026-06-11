param(
    [Parameter(Mandatory=$true)]
    [string]$LogPath,

    [switch]$RequirePickup,
    [switch]$RequireAttach,
    [switch]$RequireMountedDispatch,
    [switch]$RequireStateAnimation,
    [switch]$RequireFireCreation,
    [switch]$RequireVisualProbeFire,
    [switch]$RequireProjectileDraw,
    [switch]$RequireSeekerTracking,
    [switch]$RequireVisualDraw,
    [switch]$RequireCompleteGeometry,
    [switch]$RequireIdentityChain,
    [switch]$RequireStrictLifecycleChain,
    [switch]$RequireNoPacketRejects
)

if (-not (Test-Path -LiteralPath $LogPath)) {
    Write-Error "missing log: $LogPath"
    exit 2
}

$text = Get-Content -LiteralPath $LogPath -Raw
$wtypeMatches = [regex]::Matches($text, 'weapon_visual_probe wtype=(\d+)')
$visualProbeWtypes = New-Object System.Collections.Generic.HashSet[int]
foreach ($m in $wtypeMatches) {
    [void]$visualProbeWtypes.Add([int]$m.Groups[1].Value)
}

function Count-Pattern([string]$Pattern) {
    return ([regex]::Matches($text, $Pattern)).Count
}

function Get-FieldSet([string]$Pattern) {
    $set = New-Object System.Collections.Generic.HashSet[string]
    foreach ($m in [regex]::Matches($text, $Pattern)) {
        [void]$set.Add($m.Groups[1].Value.ToUpperInvariant())
    }
    return $set
}

function Test-SetContains([System.Collections.Generic.HashSet[string]]$Set, [string]$Value) {
    if ($null -eq $Set) {
        return $false
    }
    if ([string]::IsNullOrEmpty($Value)) {
        return $false
    }
    return $Set.Contains($Value.ToUpperInvariant())
}

$identity_attach = Get-FieldSet 'weapon_lifecycle attach_stored[^\r\n]* child=([0-9A-Fa-f]+)'
$identity_register = Get-FieldSet 'weapon_lifecycle attached_visual_register[^\r\n]* child=([0-9A-Fa-f]+)'
$identity_dispatch = Get-FieldSet 'weapon_lifecycle mounted_dispatch_begin[^\r\n]* node=([0-9A-Fa-f]+)'
$identity_draw = Get-FieldSet 'attached_draw tag=(?:player|ai)[^\r\n]* obj=([0-9A-Fa-f]+)'
$identity_fire = Get-FieldSet 'weapon_lifecycle (?:spawn_bullet|spawn_shard|spawn_missile|spawn_guided|spawn_generic)[^\r\n]* weapon=([0-9A-Fa-f]+)'
$identity_anim = Get-FieldSet 'weapon_lifecycle (?:wtype2_mounted_dispatch|wtype6_tracking|state_weapon_aim|state_weapon_fire_anim|state_weapon_fire_projectile|state_weapon_fire_effect|state_weapon_fire_blob|state_weapon_fire_cluster|state_weapon_fire_burst)[^\r\n]*(?:obj|weapon)=([0-9A-Fa-f]+)'
$identity_fire_direct = Get-FieldSet 'weapon_lifecycle (?:wtype2_mounted_dispatch|wtype6_event11_projectile|wtype6_combo_242_projectile|wtype6_combo_244_first_projectile|wtype6_combo_244_extra_projectile)[^\r\n]* other=([0-9A-Fa-f]+)'
$created_projectiles = Get-FieldSet 'weapon_lifecycle (?:spawn_bullet|spawn_shard|spawn_missile|spawn_guided|spawn_generic|state_weapon_fire_projectile|state_weapon_fire_blob|state_weapon_fire_cluster|state_weapon_fire_burst)[^\r\n]* projectile=([0-9A-Fa-f]+)'
$created_effects = Get-FieldSet 'weapon_lifecycle (?:state_weapon_fire_effect|state_weapon_fire_projectile|state_weapon_fire_blob|state_weapon_fire_cluster|state_weapon_fire_burst)[^\r\n]* effect=([0-9A-Fa-f]+)'
$drawn_projectiles = Get-FieldSet 'weapon_lifecycle renderer_projectile_draw[^\r\n]* obj=([0-9A-Fa-f]+)'
$drawn_effects = Get-FieldSet 'weapon_lifecycle renderer_effect_draw[^\r\n]* obj=([0-9A-Fa-f]+)'
$identity_full = New-Object System.Collections.Generic.List[string]
$identity_strict = New-Object System.Collections.Generic.List[string]
foreach ($child in $identity_attach) {
    if ((Test-SetContains $identity_register $child) -and
        (Test-SetContains $identity_dispatch $child) -and
        (Test-SetContains $identity_draw $child) -and
        ((Test-SetContains $identity_fire $child) -or
         (Test-SetContains $identity_anim $child))) {
        $identity_full.Add($child)
    }
    if ((Test-SetContains $identity_register $child) -and
        (Test-SetContains $identity_dispatch $child) -and
        (Test-SetContains $identity_draw $child) -and
        (Test-SetContains $identity_anim $child) -and
        ((Test-SetContains $identity_fire $child) -or
         (Test-SetContains $identity_fire_direct $child))) {
        $identity_strict.Add($child)
    }
}

$linked_projectile_draws = 0
foreach ($projectile in $created_projectiles) {
    if ($projectile -ne '0000000000000000' -and (Test-SetContains $drawn_projectiles $projectile)) {
        $linked_projectile_draws++
    }
}

$linked_effect_draws = 0
foreach ($effect in $created_effects) {
    if ($effect -ne '0000000000000000' -and (Test-SetContains $drawn_effects $effect)) {
        $linked_effect_draws++
    }
}

$geometry_bad = 0
$geometry_rows = 0
foreach ($m in [regex]::Matches($text, 'attached_draw tag=(player|ai|loose-pickup|static-pickup)[^\r\n]* prim=(\d+) tri=(\d+) quad=(\d+) line=(\d+)')) {
    $tag = $m.Groups[1].Value
    $prim = [int]$m.Groups[2].Value
    $tri = [int]$m.Groups[3].Value
    $quad = [int]$m.Groups[4].Value
    $line = [int]$m.Groups[5].Value
    $emitted = $tri + $quad + $line

    $geometry_rows++
    if ($prim -le 0 -or $emitted -le 0 -or $emitted -lt $prim) {
        $geometry_bad++
    }
}

$counts = [ordered]@{
    pickup_child_materialized = Count-Pattern 'weapon_lifecycle pickup_child_materialized'
    pickup_attach_begin      = Count-Pattern 'weapon_lifecycle pickup_attach_begin'
    pickup_attach_complete   = Count-Pattern 'weapon_lifecycle pickup_attach_complete'
    attach_appended          = Count-Pattern 'weapon_lifecycle attach_appended|weapon slot attach vehicle='
    attach_stored            = Count-Pattern 'weapon_lifecycle attach_stored|weapon slot attach stored'
    attached_visual_register = Count-Pattern 'weapon_lifecycle attached_visual_register'
    attached_visual_clear    = Count-Pattern 'weapon_lifecycle attached_visual_clear'
    slot_drop                = Count-Pattern 'weapon_lifecycle slot_drop_start|weapon_lifecycle attach_eject_slot'
    slot_spawn               = Count-Pattern 'weapon_lifecycle slot_spawn_alloc|weapon_lifecycle slot_spawn_ready'
    mounted_dispatch_begin   = Count-Pattern 'weapon_lifecycle mounted_dispatch_begin'
    mounted_dispatch_ret     = Count-Pattern 'weapon_lifecycle mounted_dispatch_ret'
    state_aim                = Count-Pattern 'weapon_lifecycle state_weapon_aim'
    state_fire_anim          = Count-Pattern 'weapon_lifecycle state_weapon_fire_anim'
    state_fire_projectile    = Count-Pattern 'weapon_lifecycle state_weapon_fire_projectile'
    state_fire_effect        = Count-Pattern 'weapon_lifecycle state_weapon_fire_effect'
    state_fire_blob          = Count-Pattern 'weapon_lifecycle state_weapon_fire_blob'
    state_fire_cluster       = Count-Pattern 'weapon_lifecycle state_weapon_fire_cluster'
    state_fire_burst         = Count-Pattern 'weapon_lifecycle state_weapon_fire_burst'
    spawn_bullet             = Count-Pattern 'weapon_lifecycle spawn_bullet'
    spawn_shard              = Count-Pattern 'weapon_lifecycle spawn_shard'
    spawn_missile            = Count-Pattern 'weapon_lifecycle spawn_missile'
    spawn_guided             = Count-Pattern 'weapon_lifecycle spawn_guided'
    spawn_generic            = Count-Pattern 'weapon_lifecycle spawn_generic'
    event_projectile         = Count-Pattern 'weapon_lifecycle .*projectile'
    event_effect             = Count-Pattern 'weapon_lifecycle .*effect'
    wtype6_tracking          = Count-Pattern 'weapon_lifecycle wtype6_tracking'
    wtype6_fire              = Count-Pattern 'weapon_lifecycle wtype6_event11_projectile|weapon_lifecycle wtype6_combo_242_projectile|weapon_lifecycle wtype6_combo_244'
    wtype2_lifecycle         = Count-Pattern 'weapon_lifecycle wtype2_mounted_dispatch|weapon_lifecycle wtype2_dispatch|weapon_lifecycle wtype2_impact_|weapon_lifecycle wtype2_lifetime_'
    wtype4_lifecycle         = Count-Pattern 'weapon_lifecycle wtype4_dispatch|weapon_lifecycle wtype4_impact_|weapon_lifecycle wtype4_lifetime_|weapon_lifecycle wtype4_die_'
    wtype8_fire              = Count-Pattern 'weapon_visual_probe .*wtype=8 .*source_fire=1|weapon_lifecycle .*LAB_8003565c|weapon_lifecycle .*seeker'
    wtype9_fire              = Count-Pattern 'weapon_visual_probe .*wtype=9 .*source_fire=1|weapon_lifecycle .*wtype9|weapon_lifecycle .*spray'
    visual_probe_fire        = Count-Pattern 'weapon_visual_probe .*source_fire=1'
    attached_draw_player     = Count-Pattern 'attached_draw tag=player'
    attached_draw_pickup     = Count-Pattern 'attached_draw tag=loose-pickup|attached_draw tag=static-pickup'
    attached_geometry_rows   = $geometry_rows
    attached_geometry_bad    = $geometry_bad
    renderer_projectile_draw = Count-Pattern 'weapon_lifecycle renderer_projectile_draw'
    renderer_effect_draw     = Count-Pattern 'weapon_lifecycle renderer_effect_draw'
    identity_full_chain      = $identity_full.Count
    identity_strict_chain    = $identity_strict.Count
    fire_created_projectiles = $created_projectiles.Count
    fire_created_effects     = $created_effects.Count
    linked_projectile_draws  = $linked_projectile_draws
    linked_effect_draws      = $linked_effect_draws
    unresolved_mounted_route = Count-Pattern 'mounted_(?:update|fire)_route_not_decoded'
    runtime_packet_skip      = Count-Pattern 'runtime_packet_skip'
    runtime_packet_invalid   = Count-Pattern 'runtime_packet_invalid'
    fatal                    = Count-Pattern 'FATAL'
    fail                     = Count-Pattern '(?m)\bFAIL\b'
}

$failures = New-Object System.Collections.Generic.List[string]

function Require-Positive([string]$Name) {
    if ($counts[$Name] -le 0) {
        $failures.Add("$Name=0")
    }
}

if ($RequirePickup) {
    Require-Positive 'pickup_child_materialized'
}
if ($RequireAttach) {
    Require-Positive 'pickup_attach_begin'
    Require-Positive 'attach_stored'
    Require-Positive 'attached_visual_register'
    Require-Positive 'pickup_attach_complete'
}
if ($RequireMountedDispatch) {
    Require-Positive 'mounted_dispatch_begin'
    Require-Positive 'mounted_dispatch_ret'
}
if ($RequireStateAnimation) {
    if (($counts.state_aim + $counts.state_fire_anim + $counts.wtype6_tracking +
         $counts.wtype2_lifecycle + $counts.wtype4_lifecycle) -le 0) {
        $failures.Add('state animation rows missing')
    }
}
if ($RequireFireCreation) {
    if (($counts.state_fire_projectile + $counts.state_fire_effect +
         $counts.state_fire_blob + $counts.state_fire_cluster +
         $counts.state_fire_burst +
         $counts.spawn_bullet + $counts.spawn_shard + $counts.spawn_missile +
         $counts.spawn_guided + $counts.spawn_generic +
         $counts.wtype6_fire + $counts.event_projectile + $counts.event_effect) -le 0) {
        $failures.Add('fire/projectile/effect creation rows missing')
    }
}
if ($RequireVisualProbeFire) {
    $requiresSourceFire = $true
    if ($visualProbeWtypes.Count -gt 0) {
        $requiresSourceFire = $false
        foreach ($w in $visualProbeWtypes) {
            if ($w -ne 4) {
                $requiresSourceFire = $true
            }
        }
    }
    if ($requiresSourceFire) {
        Require-Positive 'visual_probe_fire'
    }
}
if ($RequireProjectileDraw) {
    Require-Positive 'renderer_projectile_draw'
}
if ($RequireSeekerTracking) {
    Require-Positive 'wtype6_tracking'
}
if ($RequireVisualDraw) {
    if (($counts.attached_draw_player + $counts.attached_draw_pickup) -le 0) {
        $failures.Add('visual attached/pickup draw rows missing')
    }
}
if ($RequireCompleteGeometry) {
    Require-Positive 'attached_geometry_rows'
    if ($counts.attached_geometry_bad -ne 0) {
        $failures.Add("attached_geometry_bad=$($counts.attached_geometry_bad)")
    }
}
if ($RequireIdentityChain) {
    Require-Positive 'identity_full_chain'
}
if ($RequireStrictLifecycleChain) {
    Require-Positive 'identity_strict_chain'
    if (($counts.linked_projectile_draws + $counts.linked_effect_draws) -le 0) {
        $failures.Add('linked projectile/effect draw rows missing')
    }
    Require-Positive 'visual_probe_fire'
    if ($counts.unresolved_mounted_route -ne 0) {
        $failures.Add("unresolved_mounted_route=$($counts.unresolved_mounted_route)")
    }
}
if ($RequireNoPacketRejects) {
    if ($counts.runtime_packet_skip -ne 0) {
        $failures.Add("runtime_packet_skip=$($counts.runtime_packet_skip)")
    }
    if ($counts.runtime_packet_invalid -ne 0) {
        $failures.Add("runtime_packet_invalid=$($counts.runtime_packet_invalid)")
    }
}
if ($counts.fatal -ne 0) {
    $failures.Add("FATAL=$($counts.fatal)")
}
if ($counts.fail -ne 0) {
    $failures.Add("FAIL=$($counts.fail)")
}

$counts.GetEnumerator() | ForEach-Object {
    '{0}={1}' -f $_.Key, $_.Value
}

if ($failures.Count -gt 0) {
    'RESULT=FAIL'
    $failures | ForEach-Object { "missing_or_bad=$_" }
    exit 1
}

'RESULT=PASS'
exit 0
