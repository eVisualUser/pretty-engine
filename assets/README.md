# Assets

## How to work

All assets will be moved when building the game.
So you must follow the rules imposed by the special folders.

## List of special folders

### ENGINE_BUILTIN

All files in this folder are loaded in a header, and saved in a static const char*;

### ENGINE_PUBLIC

All files in this folder are moved to a folder named "public" next to the executable.

### ENGINE_TRASH

All the files in this folder are moved next to the executable without any modification.
