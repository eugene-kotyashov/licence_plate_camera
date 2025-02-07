param(
    [switch]$Rebuild,
    [string]$BuildConfig = "Release"
)



# Find MSBuild.exe
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    Write-Error "Could not find vswhere.exe. Please ensure Visual Studio is installed."
    exit 1
}

$msbuildPath = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
if (-not $msbuildPath) {
    Write-Error "Could not find MSBuild. Please ensure Visual Studio is installed with MSBuild components."
    exit 1
}

# Set paths
$buildDir = "build_release"
$solutionFile = Join-Path $buildDir "fltk_app.sln"
$outputDir = Join-Path $buildDir  "bin\$BuildConfig"
$exePath = Join-Path $outputDir "fltk_app.exe"

# Check if solution exists
if (-not (Test-Path $solutionFile)) {
    Write-Error @"
Solution file not found at: $solutionFile
Please run .\build.ps1 first to generate the solution.
"@
    exit 1
}

# Build the project
Write-Host "Building project..."

# Determine build target
$buildTarget = if ($Rebuild) { "Rebuild" } else { "Build" }

# Build arguments as separate strings
$buildArgs = @(
    "$solutionFile"
    "/p:Configuration=$BuildConfig"
    "/p:Platform=x64"
    "/t:$buildTarget"

)

& $msbuildPath $buildArgs

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

# Check if executable exists
if (-not (Test-Path $exePath)) {
    Write-Error "Executable not found at: $exePath"
    exit 1
}

# Copy required DLLs
Write-Host "`nCopying required DLLs..."

# Define required DLLs
$requiredDlls = @(
    
    "bin\zlib1.dll",
    "bin\jpeg62.dll",
    "bin\libpng16.dll"
)


# Copy SDK DLLs if they exist
$sdkDlls = @(
    "libssl-1_1-x64.dll",
    "libcrypto-1_1-x64.dll",
    "HCCore.dll",
    "HCNetSDK.dll",
    "PlayCtrl.dll",
    "SuperRender.dll",
    "AudioRender.dll",
    "HCNetSDKCom\HCAlarm.dll",
    "HCNetSDKCom\HCGeneralCfgMgr.dll",
    "HCNetSDKCom\HCPreview.dll"
)

$images = @(
    "vehicle.png",
    "plate.png"
)

# Create HCNetSDKCom directory if needed
$sdkComDir = Join-Path $outputDir "HCNetSDKCom"

if (-not (Test-Path $sdkComDir)) {
    New-Item -ItemType Directory -Path $sdkComDir | Out-Null
}
foreach ($image in $images) {
    $sourcePath = "images\$image"
    if (Test-Path $sourcePath) {
        Copy-Item -Path $sourcePath -Destination $outputDir -Force
        Write-Host "Copied $image"
    }
}

# Copy vcpkg DLLs
foreach ($dll in $requiredDlls) {
    $sourcePath = Join-Path $buildDir "vcpkg_installed\x64-windows\$dll"
    if (Test-Path $sourcePath) {
        Copy-Item -Path $sourcePath -Destination $outputDir -Force
        Write-Host "Copied $dll"

    } else {
        Write-Warning "Could not find $dll at $sourcePath"
    }
}

# Copy SDK DLLs
foreach ($dll in $sdkDlls) {
    $sourcePath = "lib\$dll"
    $destPath = Join-Path $outputDir $dll
    if (Test-Path $sourcePath) {
        if ($dll -like "HCNetSDKCom\*") {
            # For DLLs in HCNetSDKCom subdirectory
            Copy-Item -Path $sourcePath -Destination $sdkComDir -Force
            Write-Host "Copied $dll to HCNetSDKCom"
        } else {
            Copy-Item -Path $sourcePath -Destination $outputDir -Force
            Write-Host "Copied $dll"
        }
    } else {
        Write-Warning "Could not find SDK DLL: $dll"
    }
}

# Run the application
# Save the script path for reference
$scriptPath = $PSScriptRoot
Write-Host "Script running from: $scriptPath"

Write-Host "`nLaunching application..."
Set-Location $outputDir
Start-Process -FilePath fltk_app.exe -Wait 
Set-Location $scriptPath
