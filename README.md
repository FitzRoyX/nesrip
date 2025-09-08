# nesrip

This tool takes in a ROM and a graphics database file *(described further down)*, and automatically extracts graphics from the ROM.

## Installation

### Pre-built binaries

Pre-built binaries can be downloaded from the [GitHub Releases section](https://github.com/GeekJoystick/nesrip/releases/latest)

### Compilation

#### Dependencies

* Windows 10/11

## Supported compiler tools
* [TCC](https://github.com/TinyCC/tinycc)
* [GCC](https://www.mingw-w64.org/)
* [Clang](https://clang.llvm.org/)
* [Visual Studio C++ or it's Clang](https://visualstudio.microsoft.com/)
* [Make](https://www.gnu.org/software/make)

### Building

* Clone or download the repository
## TCC
* Run `compile` in the root folder of the project
## GCC
* Run `make` in the root folder of the project
## Visual Studio C++ or Clang
* Open `nesrip.sln` with Visual Studio, Clang can be chosen via the `nesrip` project properties in the Platform Toolset option
## Clang
* Update the `compile` and change `tcc` to `clang`

### Executing program

#### Usage

```
nesrip.exe file [arguments]                                                                                      
Arguments:
 -S {start address} {end address}                 Directly rip graphics from specified memory in ROM.
 -o {filename}                                    Output filename (without file extension) when using -S.
 -d {filename}                                    Graphics database filename.
 -c {compression type, raw}                       Graphics decompression algorithm.
 -p {pattern size, 1/2/4/8/16} {direction, h/v}   Set or override tile block size and direction.
 -i {4 letter combination of b/o/t/w}             Set or override palette order for rendering.
 -b {bitplane type, 1/2}                          Set or override bitplane type
 -r {redundancy check enable, true/false}         Set or override enabling redundancy checks
```

### Graphics database file

The Graphics database file is used by **nesrip** to match the given ROM to a set of commands written for extracting its graphics.
The file contains a number of **description blocks**, each of them containing a SHA-256 hash of a ROM, and a list of commands that determine palette indices, pattern sizes, decompression algorithms and finally the ROM addresses of the graphics data.

#### Note

> By default the tool will look for a graphics database file called `nes_gfxdb.txt`.
> You can change this by using the `-d` argument when running **nesrip**

> You also must provide the database file yourself.

### Graphics data base commands

```
hash {ROM SHA-256 hash}
end
s {Start address} {End address}                (Rip graphics from rom between start address and end address)
p {Pattern size, 1/2/4/8/16} {Direction, h/v}  (Set extraction pattern size and direction)
i {4 letter combination of b/o/t/w}            (Set palette used for rendering extracted graphics: [b]lack, [o]range, [t]eal, [w]hite)
c {compression type, raw}                      (Set graphics compression algorithm)
b {1/2}                                        (Set bitplane type: [1]bpp, [2]bpp)
r {true/false}                                 (Enable or disable tile redundancy checks)
k                                              (Clear the tile redundancy check cache)
```

### Compression types

* `raw`: Uncompressed graphics

#### Note

> No compression algorithm is supported at the moment

### Example database file

```
//Spacegulls
Hash B69BD1809E26400336AF288BC04403C00D77030B931BC31875594C9A0AE92F67
Pattern 1 h
Palette botw
Section bg 00000 FFFFF
EndHash

//Micro Mages
Hash A4B5B736A84B260314C18783381FE2DCA7B803F7C29E78FB403A0F9087A7E570
Pattern 1 v
Palette btow
Section spr 8000 8FFF
Section bg 9000 9FFF
EndHash

...
```

#### Note

> Text written outside of a **description block** is ignored by the tool, though the latter will throw errors if you put text other than commands in a **description block**.

## Version History

* 0.3
	* Added bitplane types
	* Added 1bpp bitplane
	* Added tile redundancy checks
* 0.2
	* Made ROM headers be ignored from any operation.
	* Hashes are now case insensitive.
	* Made output files go into a folder named after the input ROM.
	* Added batch files for processing multiple ROMs.
	* Added pattern directionnality.
	* Sheets will now extend vertically instead of splitting in multiple sheets.
	* Added a pattern size of 16 to allow for fully vertical 128x128 sections.
* 0.1
	* Initial Release.

## License

MIT - © 2024 [Matys Guéroult](https://github.com/GeekJoystick)