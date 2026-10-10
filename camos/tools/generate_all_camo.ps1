# Batch-generates masks, .skin files, and camo_weapons.shader stanzas for every
# firearm using the mp44/thompson maskMap pipeline. Data-driven: one entry per
# weapon describing its "main" surfaces (get camo'd) and "accessory" surfaces
# (stay at their stock default shader, same idea as hqhand/hqarm).
# Pass -OnlyIds to regenerate a subset (e.g. just-added weapons) without re-touching everyone else's already-correct output.

param(
    [string[]]$OnlyIds
)

$CamosRoot = "G:\Steam\steamapps\common\RealRTCW\camos"
$Compositor = Join-Path $PSScriptRoot "composite_camo.ps1"
$NumCamos = 8

$HandDefaults = @(
    "hqhand,models/weapons/hands/glove_default.jpg",
    "hqarm,models/weapons/hands/sleeve_default.jpg"
)

$Weapons = @(
    @{ id="bar";            dir="assault_rifles/bar";       hands="v_bar_hand";        accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".15"; tcMod="1.4 1.4"}) },

    @{ id="fg42";           dir="assault_rifles/fg42";      hands="v_fg42_hand";       accessories=$HandDefaults + @("wpn_optic,models/weapons/assault_rifles/fg42/wpn_optic.jpg");
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".10"; tcMod="1.2 1.2"}) },

    @{ id="g43";            dir="auto_rifles/g43";          hands="v_g43_hand";        accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".13"; tcMod="1.2 1.2"}) },

    @{ id="m1_garand";      dir="auto_rifles/m1_garand";    hands="v_m1_hand";         accessories=$HandDefaults + @(
           "wpn_bolt,models/weapons/auto_rifles/m1_garand/wpn_bolt.jpg",
           "wpn_gl,models/weapons/auto_rifles/m1_garand/wpn_gl2.jpg",
           "wpn_gren,models/weapons/auto_rifles/m1_garand/wpn_gl.jpg" );
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".13"; tcMod="1.2 1.2"}) },

    @{ id="browning";       dir="heavy/browning";           hands="v_brown30cal_hand"; accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".15"; tcMod="1.2 1.2"}) },

    @{ id="mg42";           dir="heavy/mg42";               hands="v_mg42_hand";       accessories=$HandDefaults + @("wpn_mag,models/weapons/heavy/mg42/wpn_mag.jpg");
       mains=@(
           @{surf="wpn";  shaderName="wpn_base";  tex="wpn_base.jpg";  alphaGen=".15"; tcMod="1.2 1.2"},
           @{surf="wpn2"; shaderName="wpn_base2"; tex="wpn_base2.jpg"; alphaGen=".15"; tcMod=$null}
       ) },

    @{ id="colt";           dir="pistols/colt";             hands="v_colt_hand";       accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".15"; tcMod="1.2 1.2"}) },

    @{ id="hdm";            dir="pistols/hdm";               hands="v_hdm_hand";       accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".15"; tcMod="1.2 1.2"}) },

    @{ id="luger";          dir="pistols/luger";             hands="v_luger_hand";     accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".10"; tcMod="1.2 1.2"}) },

    @{ id="luger_silenced"; dir="pistols/luger_silenced";    hands="v_lugers_hand";    accessories=$HandDefaults + @("wpn_silencer,models/weapons/pistols/luger_silenced/wpn_silencer.jpg");
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".10"; tcMod="1.2 1.2"}) },

    @{ id="revolver";       dir="pistols/revolver";          hands="v_revolver_hand";  accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".10"; tcMod="1.2 1.2"}) },

    @{ id="tt33";           dir="pistols/tt33";              hands="v_tt33_hand";      accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".1"; tcMod="1.2 1.2"}) },

    @{ id="mauser";         dir="rifles/mauser";             hands="v_mauser_hand";    accessories=$HandDefaults + @(
           "wpn_optic,models/weapons/rifles/mauser/wpn_optic.jpg",
           "wpn_clip,models/weapons/rifles/mauser/wpn_clip.jpg",
           "wpn_scope,models/weapons/rifles/mauser/wpn_scope.jpg" );
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".1"; tcMod=$null}) },

    @{ id="mosin";          dir="rifles/mosin";              hands="v_mosin_hand";     accessories=$HandDefaults + @("wpn_clip,models/weapons/rifles/mosin/wpn_clip.jpg");
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".14"; tcMod=$null}) },

    @{ id="snooper";        dir="rifles/snooper";            hands="v_snooper_hand";   accessories=$HandDefaults + @(
           "wpn_mag,models/weapons/rifles/snooper/wpn_mag.jpg",
           "wpn_scope,models/weapons/rifles/snooper/wpn_scope.jpg",
           "wpn_silencer,models/weapons/rifles/snooper/wpn_silencer.jpg",
           "wpn_optic,models/weapons/rifles/snooper/wpn_optic.jpg" );
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.png"; alphaGen=".1"; tcMod="1.6 1.6"}) },

    @{ id="ithaca";         dir="shotguns/ithaca";           hands="v_ithaca_hand";    accessories=$HandDefaults + @("wpn_shell,models/weapons/shotguns/ithaca/wpn_shell.jpg");
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".1"; tcMod="1.6 1.6"}) },

    @{ id="mp34";           dir="smgs/mp34";                 hands="v_mp34_hand";      accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".1"; tcMod="1.6 1.6"}) },

    @{ id="mp40";           dir="smgs/mp40";                 hands="v_mp40_hand";      accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".10"; tcMod="1.2 1.2"}) },

    @{ id="ppsh";           dir="smgs/ppsh";                 hands="v_ppsh_hand";      accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".12"; tcMod=$null}) },

    @{ id="sten";           dir="smgs/sten";                 hands="v_sten_hand";      accessories=$HandDefaults;
       mains=@(
           @{surf="wpn";  shaderName="wpn_base";  tex="wpn_base.jpg";  alphaGen=".1"; tcMod="1.3 1.3"},
           @{surf="wpn2"; shaderName="wpn_base2"; tex="wpn_base2.jpg"; alphaGen=".1"; tcMod="1.3 1.3"}
       ) },

    @{ id="mp44";           dir="assault_rifles/mp44";      hands="v_mp44_hand";       accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".14"; tcMod="1.2 1.2"}) },

    @{ id="thompson";       dir="smgs/thompson";             hands="v_thompson_hand";  accessories=$HandDefaults;
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".10"; tcMod="1.2 1.2"}) },

    # DLC1 weapons (z_zrealrtcw_dlc1.pk3)
    @{ id="m1941";          dir="auto_rifles/m1941";        hands="v_m1941_hand";      accessories=$HandDefaults + @(
           "wpn_scope,models/weapons/auto_rifles/m1941/wpn_scope.jpg",
           "wpn_clip,models/weapons/rifles/mosin/wpn_clip.jpg" );
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".14"; tcMod=$null}) },

    @{ id="delisle";        dir="rifles/delisle";           hands="v_delisle_hand";    accessories=$HandDefaults + @("scope,models/weapons/rifles/delisle/wpn_scope.jpg");
       mains=@(@{surf="wpn"; shaderName="wpn_base"; tex="wpn_base.jpg"; alphaGen=".14"; tcMod=$null}) },

    # wpn_trigger shares wpn_base's own texture (same atlas, separate smartskin surface) - camo'd via the aliases
    # mechanism below instead of its own mains entry, so it doesn't get a redundant mask/shader of its own.
    # wpn_barrel is a genuinely separate texture (confirmed low-saturation/metal via pixel sampling) so it's a
    # second main surface, like mg42/sten's wpn2. wpn_stock is left as an accessory (confirmed high-saturation/
    # wood via pixel sampling, same "don't repaint the wood" policy as every other weapon's grip/stock).
    @{ id="auto5";          dir="shotguns/auto5";            hands="v_auto5_hand";     accessories=$HandDefaults + @("wpn_stock,models/weapons/shotguns/auto5/wpn_stock.jpg");
       mains=@(
           @{surf="wpn";        shaderName="wpn_base";   tex="wpn_base.jpg";   alphaGen=".1"; tcMod="1.6 1.6"},
           @{surf="wpn_barrel"; shaderName="wpn_barrel"; tex="wpn_barrel.jpg"; alphaGen=".1"; tcMod="1.6 1.6"}
       );
       aliases=@(@{surf="wpn_trigger"; shaderName="wpn_base"}) }
)

# alias weapons: identical texture/geometry to another weapon, so they just reuse that
# weapon's already-generated camo shaders directly - no new mask/shader needed
$Aliases = @(
    @{ id="colt2";  dir="pistols/colt2";   hands="v_colt2_hand";  reuseDir="pistols/colt"; surf="wpn"; shaderName="wpn_base"; accessories=$HandDefaults },
    @{ id="tt33_2"; dir="pistols/tt33_2";  hands="v_tt33_2_hand"; reuseDir="pistols/tt33"; surf="wpn"; shaderName="wpn_base"; accessories=$HandDefaults }
)

if ($OnlyIds) {
    $Weapons = $Weapons | Where-Object { $OnlyIds -contains $_.id }
    $Aliases = $Aliases | Where-Object { $OnlyIds -contains $_.id }
    Write-Output "Scoped to: $($OnlyIds -join ', ')"
}

Write-Output "=== Building masks ==="
foreach ($w in $Weapons) {
    foreach ($m in $w.mains) {
        $baseTex = Join-Path "$CamosRoot\weapons\$($w.dir)" $m.tex
        $outDir = "$CamosRoot\weapons\$($w.dir)"
        Write-Output "-- $($w.id) / $($m.surf) : $baseTex --"
        & $Compositor -BaseTexture $baseTex -OutDir $outDir -OutPrefix $m.shaderName
    }
}

Write-Output "=== Writing skin files ==="
foreach ($w in $Weapons) {
    $outDir = "$CamosRoot\weapons\$($w.dir)"
    for ($i = 1; $i -le $NumCamos; $i++) {
        $lines = New-Object System.Collections.Generic.List[string]
        foreach ($m in $w.mains) {
            $lines.Add("$($m.surf),models/weapons/$($w.dir)/$($m.shaderName)_camo$i")
        }
        if ($w.aliases) {
            foreach ($al in $w.aliases) {
                $lines.Add("$($al.surf),models/weapons/$($w.dir)/$($al.shaderName)_camo$i")
            }
        }
        foreach ($a in $w.accessories) { $lines.Add($a) }
        $skinPath = Join-Path $outDir "$($w.hands)_camo$i.skin"
        [System.IO.File]::WriteAllText($skinPath, ($lines -join "`n") + "`n")
    }
    Write-Output "skins: $($w.id)"
}

foreach ($a in $Aliases) {
    $outDir = "$CamosRoot\weapons\$($a.dir)"
    if (!(Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir -Force | Out-Null }
    for ($i = 1; $i -le $NumCamos; $i++) {
        $lines = New-Object System.Collections.Generic.List[string]
        $lines.Add("$($a.surf),models/weapons/$($a.reuseDir)/$($a.shaderName)_camo$i")
        foreach ($acc in $a.accessories) { $lines.Add($acc) }
        $skinPath = Join-Path $outDir "$($a.hands)_camo$i.skin"
        [System.IO.File]::WriteAllText($skinPath, ($lines -join "`n") + "`n")
    }
    Write-Output "skins (alias): $($a.id) -> $($a.reuseDir)"
}

Write-Output "=== Generating shader stanzas (maskMap-based, not baked) ==="
$shaderOut = New-Object System.Text.StringBuilder
foreach ($w in $Weapons) {
    foreach ($m in $w.mains) {
        for ($i = 1; $i -le $NumCamos; $i++) {
            $shaderName = "models/weapons/$($w.dir)/$($m.shaderName)_camo$i"
            [void]$shaderOut.AppendLine($shaderName)
            [void]$shaderOut.AppendLine("{")
            [void]$shaderOut.AppendLine("`t{")
            [void]$shaderOut.AppendLine("`t`tmap models/weapons/$($w.dir)/$($m.tex)")
            [void]$shaderOut.AppendLine("`t`trgbGen lightingDiffuse")
            [void]$shaderOut.AppendLine("`t}")
            [void]$shaderOut.AppendLine("`t{")
            [void]$shaderOut.AppendLine("`t`tmap textures/effects/envmap_slateH.tga")
            [void]$shaderOut.AppendLine("`t`tblendFunc GL_SRC_ALPHA GL_ONE")
            [void]$shaderOut.AppendLine("`t`talphaGen const $($m.alphaGen)")
            if ($m.tcMod) {
                [void]$shaderOut.AppendLine("`t`ttcMod scale $($m.tcMod)")
            }
            [void]$shaderOut.AppendLine("`t`ttcGen environment")
            [void]$shaderOut.AppendLine("`t`trgbGen lightingDiffuse")
            [void]$shaderOut.AppendLine("`t}")
            [void]$shaderOut.AppendLine("`t{")
            [void]$shaderOut.AppendLine(("`t`tmap camos/camouflage_{0:D2}.png" -f $i))
            [void]$shaderOut.AppendLine("`t`tmaskMap models/weapons/$($w.dir)/$($m.shaderName)_mask.png")
            [void]$shaderOut.AppendLine("`t`tblendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA")
            [void]$shaderOut.AppendLine("`t`trgbGen lightingDiffuse")
            [void]$shaderOut.AppendLine("`t}")
            [void]$shaderOut.AppendLine("}")
            [void]$shaderOut.AppendLine("")
        }
    }
}
$shaderOutPath = Join-Path $PSScriptRoot "generated_camo_stanzas.shader"
[System.IO.File]::WriteAllText($shaderOutPath, $shaderOut.ToString())
Write-Output "Wrote shader stanzas to $shaderOutPath"

Write-Output "Done."
