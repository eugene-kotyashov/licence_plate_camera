param( 
    [Parameter(Mandatory=$true)]
    [string]$outputDir,
    [Parameter(Mandatory=$true)]
    [string]$buildDir
)

Write-Host "`nCopying required DLLs..."
# Define required DLLs
$requiredDlls = @(
    
    "bin\zlib1.dll",
    "bin\jpeg62.dll",
    "bin\libpng16.dll",
    "bin\libcurl.dll"
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
    "plate.png",
    "vehicle.jpg",
    "plate.jpg",
    "black_white_list_upload.xls"
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