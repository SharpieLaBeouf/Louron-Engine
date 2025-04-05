New-Item -ItemType File -Path "$PSScriptRoot\AlwaysRun.flag" -Force | Out-Null
$sourceRoot = Resolve-Path "source"
$targetRoot = "$PSScriptRoot\..\Louron Editor\Resources\Script Core API"

# Clean target folder
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $targetRoot
New-Item -ItemType Directory -Path $targetRoot | Out-Null

# Copy only .h and .cpp files, preserving folder structure
Get-ChildItem $sourceRoot -Include *.h, *.cpp -Recurse -File | Where-Object {
    $_.FullName -notmatch '\\\.vs\\|\\.git\\'
} | ForEach-Object {
    $relativePath = $_.FullName.Substring($sourceRoot.Path.Length).TrimStart('\')
    $destination = Join-Path $targetRoot $relativePath
    New-Item -ItemType Directory -Path (Split-Path $destination) -Force | Out-Null
    Copy-Item $_.FullName -Destination $destination -Force
}
