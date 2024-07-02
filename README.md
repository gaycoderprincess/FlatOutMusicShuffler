# FlatOut Music Shuffler

Simple plugin to play multiple songs per race in FlatOut

## Installation

- Make sure you have v1.1 of the game, as this is the only version this plugin is compatible with. (exe size of 2822144 bytes)
- Plop the files into your game folder.
- Enjoy, nya~ :3

## Known problems

- Changing songs currently results in a lag spike, the game doesn't asynchronously stream songs so I'm currently not sure how to solve this without a ton of extra work >.<

## Building

Building is done on an Arch Linux system with CLion and vcpkg being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common) to a folder next to this one, so it can be found.

Required packages: `mingw-w64-gcc`

You should be able to build the project now in CLion.
