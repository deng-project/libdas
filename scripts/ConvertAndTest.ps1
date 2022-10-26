# Source paths
$AsciiSTLPath = "..\third_party\3DModelSamples\stla\*.stl"
$BinarySTLPath = "..\third_party\3DModelSamples\stlb\*.stl"
$WavefrontOBJPath = "..\third_party\3DModelSamples\wobj\*.obj"
$GLTFBlobbedPath = "..\third_party\glTF-Sample-Models\2.0\*\glTF\*.gltf"
$GLTFEmbeddedPath = "..\third_party\glTF-Sample-Models\2.0\*\glTF-Embedded\*.gltf"
$dastool_Binary = "..\build\Debug\dastool.exe"

# Destination paths
$AsciiSTLDestination = ".\Models\stla"
$BinarySTLDestination = ".\Models\stlb"
$WavefrontOBJDestination = ".\Models\obj"
$GLTFBlobbedDestination = ".\Models\glTF\Blobbed"
$GLTFEmbeddedDestination = ".\Models\glTF\Embedded"

function Make-Destinations {
	if(-Not (Test-Path -Path $AsciiSTLDestination)) {
		New-Item $AsciiSTLDestination -ItemType Directory
	}
	
	if(-Not (Test-Path -Path $BinarySTLDestination)) {
		New-Item $BinarySTLDestination -ItemType Directory
	}
	
	if(-Not (Test-Path -Path $WavefrontOBJDestination)) {
		New-Item $WavefrontOBJDestination -ItemType Directory
	}
	
	if(-Not (Test-Path -Path $GLTFBlobbedDestination)) {
		New-Item $GLTFBlobbedDestination -ItemType Directory
	}
	
	if(-Not (Test-Path -Path $GLTFEmbeddedDestination)) {
		New-Item $GLTFEmbeddedDestination -ItemType Directory
	}
}

function Convert-ToDas ([String]$SourcePath, [String]$DestinationPath){
	Get-ChildItem -Path $SourcePath | ForEach-Object {
		$destination = $DestinationPath + "\" + ($_.FullName.Split("\")[-1])
		echo "Converting $_ to $destination.das ..."
		$AllArgs = @("convert", "$_", "-o", "$destination")
		& $dastool_Binary $AllArgs
	}
}


Make-Destinations
Convert-ToDas $AsciiSTLPath $AsciiSTLDestination
Convert-ToDas $BinarySTLPath $BinarySTLDestination
Convert-ToDas $WavefrontOBJPath $WavefrontOBJDestination
Convert-ToDas $GLTFBlobbedPath $GLTFBlobbedDestination
Convert-ToDas $GLTFEmbeddedPath $GLTFEmbeddedDestination
