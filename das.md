# DENG asset file format (.das) specifications  
DAS is a file format implementation for storing 3D models by DENG project. 
This file format uses .das extension and can be read header by header. 
DENG project includes a tool for translating various 3D model formats into 
DENG asset format, but if you want to create your own implementation of the 
format translator then you can read about the file structure below.  

### Magic number and FILE\_HDR
FILE\_HDR is a special header that contains only the file signature and padding of 12 bytes.
DENG asset file signature is DAS or in uint32_t format 0x44415300, assuming that little endian 
is used.


### INFO\_HDR  
The layout for the INFO\_HDR is following:  
* 8 bytes: Magic number (in little endian: 0x5453544553535144)
* 8 bytes: Header name  
* 4 bytes: Header size 
* 32 bytes: Asset UUID
* 8 bytes: Timestamp  
* 1 byte: Asset type
* 1 byte Compression level (default 0, reservered for future)

List of valid asset types are following:  
* 0 - Texture mapped normalised asset
* 1 - Unmapped normalised asset
* 2 - Texture mapped unormalised asset
* 3 - Unmapped unnormalised asset  

Currently compression algorithm specifying byte has to be 0 and is reserved for future use.


### META\_HDR
Meta headers can be used to store some additional information
or they can be optionally ignored as well.  
The layout for META\_HDR is following:  
* 8 bytes: Header beginning (META\_HDR)
* 4 bytes: Header size
* 4 bytes: Data size
* n bytes: Data


### VERT\_HDR  
The layout for the VERT\_HDR is following:  
* 8 bytes: Header name  
* 4 bytes: Header size  

#### VPOS\_HDR
VPOS\_HDR is a subheader of VERT\_HDR, which contains information about vertex positions. This header is necessary at 
all cases otherwise the asset is considered as corrupted.  
The layout of VPOS\_HDR is following:  
* 8 bytes: Header name
* 4 bytes: Header size
* 4 bytes: Vertices count  
* 1 byte Element count (3)
* n * 24 bytes: Vertex coordinates (x, y, z: type f32 + pointer offset)

#### VTEX\_HDR
VTEX\_HDR is a subheader of VERT\_HDR, which contains information about texture positions. This header is not necessary if
the asset mode is any of the following: `DAS\_ASSET\_MODE\_UNMAPPED`, `\_\_DAS\_ASSET\_MODE\_UNMAPPED\_UNOR`  
The layout of VTEX\_HDR is following:  
* 8 bytes: Header name  
* 4 bytes: Header size  
* 4 bytes: Vertices count  
* 1 byte Element count (2)
* n * 8 bytes: Texture coordinates (x, y: type f32)  

#### VNOR\_HDR
VNOR\_HDR is a subheader of VERT\_HDR, which contains information about vertex normals. This header is not necessary if
the asset mode is any of the following: `\_\_DAS\_ASSET\_MODE\_UNMAPPED\_UNOR`, `\_\_DAS\_ASSET\_MODE\_TEXTURE\_MAPPED\_UNOR`  
The layout of VNOR\_HDR is following:  
* 8 bytes: Header name  
* 4 bytes: Header size  
* 4 bytes: Vertices count  
* 1 byte Attribute count (3)
* n * 12 bytes: Vertex normal coordinates (x, y, z: type f32)

### INDX\_HDR  
The layout for the INDX\_HDR is following:  
* 8 bytes: Header name  
* 4 bytes: Header size (bytes)  
* 4 bytes: Indices count  
* m * n * 4 bytes: Indices data (type ui32)  

Where m represents the vertex elements count which can be determined from the asset mode as follows:
`\_\_DAS\_ASSET\_MODE\_3D\_UNMAPPED\_UNOR` - 1
`DAS\_ASSET\_MODE\_3D\_UNMAPPED` - 2
`\_\_DAS\_ASSET\_MODE\_3D\_TEXTURE\_MAPPED\_UNOR` - 2
`DAS\_ASSET\_MODE\_3D\_TEXTURE\_MAPPED` - 3
