## About

This drag-n-drop tool extracts graphics from NES roms into modern PNG tilesheets for modern usage via database entries. A completed database will effectively document every game's graphics locations, bitplane type, pattern type, compression type, and palettes. It has powerful features like a colorizer and tile deduplicator. Future map and sprite definition tools using these modernized sheets are planned.

## Supported compilers
* [TCC](https://github.com/TinyCC/tinycc)
* [GCC](https://www.mingw-w64.org/)
* [Clang](https://clang.llvm.org/)
* [Visual Studio](https://visualstudio.microsoft.com/)
* [Make](https://www.gnu.org/software/make)

## Building and usage

The easiest way to build this tool on Windows is by using TCC (TinyCC), a super small compiler with no installer. A prebuilt version can be found [here](https://github.com/FitzRoyX/tinycc/releases/tag/tcc_20251005). Simply extract it into the nesrip folder so that nesrip/tcc/tcc.exe exists, then run "compile_with_tcc_local.bat". This will create nesrip.exe, onto which roms can be dragged to create PNG output files. A game must be in the database for the tool to work. The database is a text file called nes_gfxdb.txt.

## Graphics database commands
```
//comments and inline comments are supported
hash {ROM SHA-256 hash}
s {} {} (section start and end addresses)
p {1/2/4/8/16} {h/v} (pattern size and direction)
i {} (any combo of bwld grayscale palette order: [b]lack, [w]hite, [l]ight gray, [d]ark gray)
c {} (compression type)
b {1/2} (bitplane type: [1]bpp, [2]bpp)
r {true/false} (tile redundancy checker)
k (clear the tile redundancy checker cache)
end
```

## Command-Line usage
```
nesrip.exe file [arguments]                                                                                      
Arguments:
 -s {} {} (section start and end addresses)
 -o {} (output filename when using -s)
 -d {} (graphics database filename)
 -p {1/2/4/8/16} {h/v} (pattern size and direction)
 -i {} (any combo of bwld grayscale palette order: [b]lack, [w]hite, [l]ight gray, [d]ark gray)
 -c {} (compression type)
 -b {1/2} (bitplane type: [1]bpp, [2]bpp)
 -r {true/false} (tile redundancy checker)
```

## Compression types

* `raw`: Uncompressed graphics

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

## Credits

Matys Guéroult - Lead programmer<br>
FitzRoyX - Designer/commissioner of tool, database maintainer<br>
Invertego - Colorizer programmer<br>
Richard Wheeler - Programmer<br>

## License

MIT - © 2024 [Matys Guéroult](https://github.com/GeekJoystick)
