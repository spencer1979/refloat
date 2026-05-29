param(
    [string]$VescToolPath = "",
    [string]$VescToolFolderUrl = "https://drive.google.com/drive/folders/1Zpwz7hCzXyrV0Om2FOl2SfCteNpyoLae?usp=sharing",
    [string]$ToolchainRoot = "$PSScriptRoot\..\toolchain\gcc-arm-none-eabi-7-2018-q2-update-win32",
    [string]$ToolchainZipUrl = "https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-win32.zip",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

function Resolve-BashPath {
    $candidates = @(
        "C:\Program Files\Git\bin\bash.exe",
        "C:\Program Files\Git\usr\bin\bash.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $cmd = Get-Command bash.exe -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }

    throw "bash.exe not found. Please install Git for Windows."
}

function Resolve-PythonPath {
    $cmd = Get-Command python -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }

    throw "python not found in PATH. Please install Python 3."
}

function Is-WindowsHost {
    return [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform(
        [System.Runtime.InteropServices.OSPlatform]::Windows
    )
}

function Find-FirstFileRecursive([string]$root, [string]$fileName) {
    if (-not (Test-Path $root)) {
        return $null
    }

    $match = Get-ChildItem -Path $root -Recurse -File -Filter $fileName -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($match) {
        return $match.FullName
    }

    return $null
}

function Find-VescExecutable([string]$root, [bool]$isWindows) {
    if (-not (Test-Path $root)) {
        return $null
    }

    if ($isWindows) {
        $exe = Get-ChildItem -Path $root -Recurse -File -Filter "vesc_tool.exe" -ErrorAction SilentlyContinue |
            Select-Object -First 1
        if ($exe) {
            return $exe.FullName
        }

        $exeGeneric = Get-ChildItem -Path $root -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "vesc_tool*.exe" } |
            Select-Object -First 1
        if ($exeGeneric) {
            return $exeGeneric.FullName
        }

        return $null
    }

    $bin = Get-ChildItem -Path $root -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object {
            ($_.Name -eq "vesc_tool" -or $_.Name -like "vesc_tool*") -and
            -not $_.Name.EndsWith(".exe")
        } |
        Select-Object -First 1
    if ($bin) {
        return $bin.FullName
    }

    return $null
}

function Ensure-Toolchain([string]$repoRoot, [string]$toolchainRoot, [string]$toolchainZipUrl) {
    $toolchainParent = Join-Path $repoRoot "toolchain"
    $toolchainZip = Join-Path $repoRoot "gcc-arm-none-eabi-7-2018-q2-update-win32.zip"

    if (Test-Path $toolchainRoot) {
        $gccPath = Join-Path $toolchainRoot "bin\arm-none-eabi-gcc.exe"
        if (Test-Path $gccPath) {
            return (Resolve-Path $toolchainRoot).Path
        }
    }

    $existingGcc = Find-FirstFileRecursive -root $toolchainParent -fileName "arm-none-eabi-gcc.exe"
    if ($existingGcc) {
        return (Split-Path (Split-Path $existingGcc -Parent) -Parent)
    }

    if (-not (Test-Path $toolchainZip)) {
        New-Item -ItemType Directory -Path $toolchainParent -Force | Out-Null
        Invoke-WebRequest -Uri $toolchainZipUrl -OutFile $toolchainZip -UseBasicParsing
    }

    New-Item -ItemType Directory -Path $toolchainParent -Force | Out-Null
    Expand-Archive -LiteralPath $toolchainZip -DestinationPath $toolchainParent -Force

    $downloadedGcc = Find-FirstFileRecursive -root $toolchainParent -fileName "arm-none-eabi-gcc.exe"
    if (-not $downloadedGcc) {
        throw "Toolchain extraction completed but arm-none-eabi-gcc.exe was not found under $toolchainParent"
    }

    return (Split-Path (Split-Path $downloadedGcc -Parent) -Parent)
}

function Resolve-VescToolPath([string]$repoRoot, [string]$vescToolPath, [string]$vescToolFolderUrl) {
    $isWindows = Is-WindowsHost

    if ($vescToolPath -and (Test-Path $vescToolPath)) {
        return (Resolve-Path $vescToolPath).Path
    }

    $pathCandidates = @("vesc_tool")
    if ($isWindows) {
        $pathCandidates = @("vesc_tool.exe", "vesc_tool")
    }

    foreach ($candidate in $pathCandidates) {
        $cmd = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($cmd) {
            return $cmd.Source
        }
    }

    $localCandidates = @()
    if ($isWindows) {
        $localCandidates = @(
            (Join-Path $repoRoot "vesc_tool\vesc_tool.exe"),
            (Join-Path $repoRoot "tools\vesc_tool\vesc_tool.exe"),
            (Join-Path $env:USERPROFILE "vesc_tool\vesc_tool.exe")
        )
    } else {
        $localCandidates = @(
            (Join-Path $repoRoot "vesc_tool\vesc_tool"),
            (Join-Path $repoRoot "tools\vesc_tool\vesc_tool")
        )
    }

    foreach ($candidate in $localCandidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    $pythonExe = Resolve-PythonPath
    $downloadRoot = Join-Path $repoRoot "tools\vesc_tool_download"
    New-Item -ItemType Directory -Path $downloadRoot -Force | Out-Null

    & $pythonExe -m pip install --disable-pip-version-check gdown | Out-Null

    $oldProgressPreference = $ProgressPreference
    try {
        $ProgressPreference = 'SilentlyContinue'
        & $pythonExe -m gdown --quiet --folder $vescToolFolderUrl -O $downloadRoot
    } finally {
        $ProgressPreference = $oldProgressPreference
    }

    $zipName = "vesc_tool_android.zip"
    if ($isWindows) {
        $zipName = "vesc_tool_free_windows.zip"
    }

    $selectedZip = Get-ChildItem -Path $downloadRoot -Recurse -File -Filter $zipName -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if (-not $selectedZip) {
        throw "Expected archive $zipName was not found in downloaded folder: $downloadRoot"
    }

    $extractRoot = Join-Path $repoRoot "tools\vesc_tool"
    New-Item -ItemType Directory -Path $extractRoot -Force | Out-Null
    Expand-Archive -LiteralPath $selectedZip.FullName -DestinationPath $extractRoot -Force

    # Keep downloaded zips and extracted folders for reuse; only locate executable here.
    $vescExe = Find-VescExecutable -root $extractRoot -isWindows $isWindows
    if (-not $vescExe) {
        $targetName = "vesc_tool"
        if ($isWindows) {
            $targetName = "vesc_tool.exe"
        }
        throw "$targetName not found after extracting $zipName under $extractRoot"
    }

    if ($isWindows) {
        $targetName = "vesc_tool.exe"
        $foundName = Split-Path $vescExe -Leaf
        if ($foundName -ne $targetName) {
            $targetPath = Join-Path (Split-Path $vescExe -Parent) $targetName
            if (Test-Path $targetPath) {
                Remove-Item -Force $targetPath
            }
            Move-Item -Path $vescExe -Destination $targetPath
            $vescExe = $targetPath
        }
    }

    return $vescExe
}

function To-MsysPath([string]$path) {
    $resolved = (Resolve-Path $path).Path
    $drive = $resolved.Substring(0, 1).ToLower()
    $rest = $resolved.Substring(2).Replace('\\', '/')
    return "/$drive$rest"
}

function Resolve-RepoWebUrl([string]$repoRoot) {
    $originUrl = (git -C $repoRoot config --get remote.origin.url 2>$null)
    if (-not $originUrl) {
        return ""
    }

    $originUrl = $originUrl.Trim()

    if ($originUrl -match '^git@github\.com:(.+?)(\.git)?$') {
        return "https://github.com/$($matches[1])"
    }

    if ($originUrl -match '^https?://') {
        return ($originUrl -replace '\.git$', '')
    }

    return $originUrl
}

function Add-BuildCommitSectionToReadme([string]$repoRoot) {
    $readmePath = Join-Path $repoRoot "package_README.md"
    if (-not (Test-Path $readmePath)) {
        throw "package_README.md not found at $readmePath"
    }

    $originalReadme = [System.IO.File]::ReadAllText($readmePath)

    $shortHash = (git -C $repoRoot rev-parse --short HEAD).Trim()
    $commitMessage = (git -C $repoRoot log -1 --pretty=%B)
    if (-not $commitMessage) {
        $commitMessage = "(no commit message)"
    }

    $upstreamReleasesUrl = "https://github.com/lukash/refloat/releases"
    $repoUrl = Resolve-RepoWebUrl -repoRoot $repoRoot
    if ([string]::IsNullOrWhiteSpace($repoUrl)) {
        $repoUrl = "(remote.origin.url not set)"
    }

    $section = @"
## Based on the original project with local modifications
- Original Project Releases: $upstreamReleasesUrl
- This Build Repository: $repoUrl
- Build Commit: $shortHash

### Local Modification Summary (Latest Commit Message)
$commitMessage
"@

    $updatedReadme = $originalReadme.TrimEnd("`r", "`n") + "`r`n`r`n" + $section.TrimStart("`r", "`n") + "`r`n"
    [System.IO.File]::WriteAllText($readmePath, $updatedReadme)

    return $originalReadme
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $repoRoot

$bashExe = Resolve-BashPath

$toolchainRootResolved = Ensure-Toolchain -repoRoot $repoRoot -toolchainRoot $ToolchainRoot -toolchainZipUrl $ToolchainZipUrl
$resolvedVescToolPath = Resolve-VescToolPath -repoRoot $repoRoot -vescToolPath $VescToolPath -vescToolFolderUrl $VescToolFolderUrl

if ($SkipBuild) {
    Write-Host "SkipBuild enabled. Resolved vesc_tool path: $resolvedVescToolPath"
    return
}

$gccExe = Join-Path $toolchainRootResolved "bin\arm-none-eabi-gcc.exe"
$objdumpExe = Join-Path $toolchainRootResolved "bin\arm-none-eabi-objdump.exe"
$objcopyExe = Join-Path $toolchainRootResolved "bin\arm-none-eabi-objcopy.exe"

foreach ($required in @($resolvedVescToolPath, $gccExe, $objdumpExe, $objcopyExe)) {
    if (-not (Test-Path $required)) {
        throw "Required file not found: $required"
    }
}

$repoUnix = To-MsysPath $repoRoot
$vescToolUnix = To-MsysPath $resolvedVescToolPath
$gccUnix = To-MsysPath $gccExe
$objdumpUnix = To-MsysPath $objdumpExe
$objcopyUnix = To-MsysPath $objcopyExe

$buildScript = @"
set -euo pipefail
cd '$repoUnix'

make -C src clean \
  VESC_TOOL='$vescToolUnix' \
  CC='$gccUnix' \
  LD='$gccUnix' \
  OBJDUMP='$objdumpUnix' \
  OBJCOPY='$objcopyUnix'

make \
  VESC_TOOL='$vescToolUnix' \
  CC='$gccUnix' \
  LD='$gccUnix' \
  OBJDUMP='$objdumpUnix' \
  OBJCOPY='$objcopyUnix'

# Remove generated intermediate files; keep final refloat.vescpkg.
make -C src clean
rm -f package_README-gen.md ui.qml
"@
$readmePath = Join-Path $repoRoot "package_README.md"
$originalReadme = $null

try {
    $originalReadme = Add-BuildCommitSectionToReadme -repoRoot $repoRoot
    & $bashExe -lc $buildScript
}
finally {
    if ($null -ne $originalReadme) {
        [System.IO.File]::WriteAllText($readmePath, $originalReadme)
    }
}

$pkgPath = Join-Path $repoRoot "refloat.vescpkg"
if (-not (Test-Path $pkgPath)) {
    throw "Build finished but refloat.vescpkg was not generated."
}

Write-Host "Done: $pkgPath"
