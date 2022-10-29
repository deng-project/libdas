# Paths to read from
$AsciiSTLDestination = ".\Models\stla\*.das"
$BinarySTLDestination = ".\Models\stlb\*.das"
$WavefrontOBJDestination = ".\Models\obj\*.das"
$GLTFBlobbedDestination = ".\Models\glTF\Blobbed\*.das"
$GLTFEmbeddedDestination = ".\Models\glTF\Embedded\*.das"
$dastool_Binary = "..\build\Debug\dastool.exe"


function Validate-Das ([String]$Wildcard) {
	Get-ChildItem -Path $Wildcard | ForEach-Object {
		echo "Validating $_.FullName"
		& $dastool_Binary validate $_
	}
}

Validate-Das $AsciiSTLDestination
Validate-Das $BinarySTLDestination
Validate-Das $WavefrontOBJDestination
Validate-Das $GLTFBlobbedDestination
Validate-Das $GLTFEmbeddedDestination