## About

This tool extracts 8x8 tile graphics from NES roms into modern PNG tilesheets. It has drag-n-drop and command-line functionality. The database, when completed, will effectively document every game's graphics locations, bitplane type, pattern type, compression type, and palettes. It has powerful features like a colorizer and tile deduplicator. Future map and sprite definition tools using these modernized sheets are planned.

## Supported compilers
* [TCC](https://github.com/TinyCC/tinycc)
* [GCC](https://www.mingw-w64.org/)
* [Clang](https://clang.llvm.org/)
* [Visual Studio](https://visualstudio.microsoft.com/)
* [Make](https://www.gnu.org/software/make)

## Building and usage

The easiest way to build this tool on Windows is by using TCC (TinyCC), a super small compiler with no installer. A prebuilt version can be found [here](https://github.com/FitzRoyX/tinycc/releases/tag/tcc_20251005). Simply extract it into the nesrip folder so that nesrip/tcc/tcc.exe exists, then run "compile_with_tcc_local.bat". This will create nesrip.exe, onto which roms can be dragged to create PNG output files. A game must be in the database for the tool to work. The database is a text file called nes_gfx_db.txt.

## Graphics database commands
```
//trailing comments are supported
hash {}
c {} (compression type, default = raw)
b {1/2/3/4/5/6/7/8} (bitplane type, default = 2)
p {1/2/4/8/16} {h/v} (pattern size and direction, default = 1 h)
r {true/false} (tile deduplicator, default = true)
k (clear tile deduplicator cache)
f {} {} (compressed gfx finder start and end)
s {} {} (section start and end)
end
```

## Command-Line usage
```
nesrip.exe filename
```

## Compression types

* `raw`: Uncompressed
* `rle_konami`: RLE_Konami
* `lzss`: LZSS
* `lz2`: LZ2

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
