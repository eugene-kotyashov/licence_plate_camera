param(
    [string]$BuildConfig = "Release",
    [string]$VcpkgRoot = "$env:VCPKG_ROOT",
    [switch]$Clean,
    [switch]$Rebuild
)
Write-Host "Using vcpkg from: $VcpkgRoot"
# Check if VCPKG_ROOT is set or try to find vcpkg in common locations
if ([string]::IsNullOrEmpty($VcpkgRoot)) {
    $commonPaths = @(
        "C:\vcpkg",
        "C:\dev\vcpkg",
        "D:\Projects\vcpkg",
        "$env:USERPROFILE\vcpkg",
        "$env:USERPROFILE\source\vcpkg"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            $VcpkgRoot = $path
            break
        }
    }
    
    if ([string]::IsNullOrEmpty($VcpkgRoot)) {
        Write-Error @"
VCPKG_ROOT environment variable is not set and vcpkg installation was not found in common locations.
Please either:
1. Set VCPKG_ROOT environment variable to your vcpkg installation directory
2. Install vcpkg in one of these locations: $($commonPaths -join ', ')
3. Specify vcpkg path directly: .\build.ps1 -VcpkgRoot "path\to\vcpkg"

To install vcpkg:
1. git clone https://github.com/Microsoft/vcpkg.git
2. cd vcpkg
3. .\bootstrap-vcpkg.bat
4. .\vcpkg integrate install
"@
        exit 1
    }
}

Write-Host "Using vcpkg from: $VcpkgRoot"

# Verify vcpkg toolchain file exists
$VcpkgToolchain = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
if (-not (Test-Path $VcpkgToolchain)) {
    Write-Error @"
Vcpkg toolchain file not found at: $VcpkgToolchain
Please ensure vcpkg is properly installed. To install:
1. git clone https://github.com/Microsoft/vcpkg.git
2. cd vcpkg
3. .\bootstrap-vcpkg.bat
4. .\vcpkg integrate install
"@
    exit 1
}

# Create build directory name based on build type
$BuildDir = "build"

# Clean build if requested
if ($Clean -or $Rebuild) {
    if (Test-Path $BuildDir) {
        Write-Host "Cleaning build directory..."
        Remove-Item -Path $BuildDir -Recurse -Force
    }
}

# Create build directory if it doesn't exist
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configure CMake
Write-Host "Configuring CMake..."
cmake -B $BuildDir -S . `
    -DCMAKE_BUILD_TYPE=$BuildType `
    -DCMAKE_TOOLCHAIN_FILE="$VcpkgToolchain"

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed"
    exit 1
}

# Build the project
Write-Host "Building project..."
cmake --build $BuildDir --config $BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

$outputDir = "$BuildDir\bin\$BuildType"

# Copy required DLLs
& (Join-Path $PSScriptRoot copy_files.ps1) -OutputDir $outputDir -BuildDir $buildDir

Write-Host "`nBuild completed successfully!" -ForegroundColor Green
Write-Host "The executable can be found in: $BuildDir\bin\$BuildType\" 