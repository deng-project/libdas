$gltf_path = "C:\git\glTF-Sample-Models\2.0"
(Get-ChildItem $gltf_path -Recurse | Select-String -pattern .gltf | % {$_.Path} > match.txt)
$FILESARRAY = Get-Content match.txt
foreach ($file in $FILESARRAY) {
	Get-Content $file
}