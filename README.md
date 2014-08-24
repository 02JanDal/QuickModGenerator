# Deprecated, see [here](https://github.com/02JanDal/QMTools) for the successor

## QuickModGenerator

A command line or GUI tool for handling QuickMod files

## Building

### What you need

* [Qt](https://qt-project.org/)
* CMake
* Make
* A C++11 compiler (tested with GCC)
* The source (`git clone https://github.com/02JanDal/QuickModGenerator.git`

### Instructions

These are for linux, should be somewhat similar on other operating systems.

In the directory of the source:

```
mkdir build
cd build
cmake ..
make
```

## Usage

### Command line

`QuickModGenerator <command> <options> <files>`

Generally, if you don't specify any files, all QuickMod files in the current directory will be used.

Commands can be one of:

* create-index - (Re)creates the `index.json` file using the QuickMod files given
    * `--base <URL>` Can be used to override the default baseUrl of `http://localhost/quickmod/`
* dump - Takes one QuickMod file and gives you it's contents in a human-readable format
* fixup - This will go through all QuickMod files specified and ask you about missing fields. Fields can be left blank.
    * `--no-checksums` Don't ask for checksums. Usable if you have a huge amount of versions in some QuickMod files and don't want to be asked for all checksums
    * `--browser` If this flag is given a browser will be opened with the website URL of the current QuickMod.
* format - This will format all QuickMod files given
* verify - Verifies all QuickMod files given against the spec
* graph - Generates a graph of QuickMod dependencies. Use [dot](http://graphviz.org) to get an image, `QuickModGenerator graph | dot -Tpng -o graph.png`
* setup - `QuickModGenerator setup <options> <name>`, creates a new QuickMod file using the given name.
    * `--nem <NEMNAME>` The name of the QuickMod as used by [NEM](http://bot.notenoughmods.com/)
    * `--curse <CURSEID>` The id as of the QuickMod as used by Curse. `http://www.curse.com/mc-mods/minecraft/<CURSEID>`
    * `--server <SERVER>` The URL that will be prepended to the update URL.
* update - This command will check for new versions of QuickMods, fetch metadata etc. It will also download mod files (opening a browser if needed) to generate checksums and look for modids.
    * `--no-network` Don't check for new updates

### GUI

`QuickModGenerator gui` to start

Basically the same functionallity, but wrapped into a GUI, some differences:

* The files selected to the left are the files given to the command
* The graph button will take care of rendering aswell, but you'll need to have dot installed and in your path
* The fixup command will bring up a table of all QuickMods. You can change stuff here and then click save.
