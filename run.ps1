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
$buildDir = "build"
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
& (Join-Path $PSScriptRoot copy_files.ps1) -OutputDir $outputDir -BuildDir $buildDir

# Run the application
# Save the script path for reference
$scriptPath = $PSScriptRoot
Write-Host "Script running from: $scriptPath"

Write-Host "`nLaunching application from $outputDir..."
Set-Location $outputDir
Start-Process -FilePath fltk_app.exe -Wait 
Set-Location $scriptPath
