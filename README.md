# Welcome to Ponify!

You love ponies? You have difficulties with inventing pony-like names for your roleplays or fanfiction? Ponify is here to rescue!

The program takes your word and tries to replace it's parts with *pony* words from file `ponywords.txt`, printing the original word, ponified word and heuristical distance between them. The algorithms is able to generate such original names as *Manehattan*, *Maripony*, *Stalliongrad* and more!

You are free to change and extend list of words in `ponywords.txt` to make your elf-stylized words. You are free to change `DIST` constants to make results more accurate.

Enjoy!

### Build
Building process is automated and requires [CMake](https://cmake.org):
```
mkdir build
cd build
cmake ..
cmake --build .
```

### Example
```
```