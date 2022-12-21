# OdysseyDecomp
![Badge Decompiled]

[Badge Decompiled]: https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/shibbo/OdysseyDecomp/main/data/percent.json&style=flat

A decompilation of Super Mario Odyssey v1.0.0 for the Nintendo Switch

**This project will not generate a complete binary of the game, and is only meant as a place for research. Game assets are not needed, nor included in this repository.**

*Discord server can be found [here](https://discord.gg/WHbGwKyH8m)*
*In progress functions can be found [here](https://docs.google.com/spreadsheets/d/198vrkkDqktrRDLInSAkK2HsG5hy1Fl8cmCNRMND3nCY/edit?usp=sharing)*

### Setup
#### Requirements
* Super Mario Odyssey 1.0.0 and the `main` file from its exefs. Place this in the project root with the name `odyssey.nso`.
* Linux or WSL
* Python 3
* pip
* Depending on your setup, you may need these apt packages:
* - `libncurses5-dev g++-multilib libc6 libc6-dev`

***Windows users: cloning inside of a WSL directory will greatly improve build performance***
Then run this command to clone the repository along with the submodules under `lib`. These include `sead`, `nnheaders`, and `agl` from [open-ead](https://github.com/open-ead/).
`git clone --recurse-submodules https://github.com/shibbo/OdysseyDecomp.git`


*The following can be downloaded with the `setup.py` in this repository*
* Python modules `capstone`, `elftools`, `lz4`, `ninja`, and `colorama`
* [Clang 3.9.1 debian build](https://releases.llvm.org/3.9.1/clang+llvm-3.9.1-x86_64-linux-gnu-debian8.tar.xz)

### Building
Building has only been tested on WSL2 running Ubuntu 20.04.4.

* `setup.py` - download and install required Python modules and Clang 3.9.1
* `build.py [-non-matching]` - build the repository and generate `.o` files under the `build` directory
* - `-non-matching` - also builds non-matching functions enclosed in the `NON_MATCHING` macro

* `check.py [-no-diff] <mangled symbol>` - checks for differences in the specified compiled function
* - `-no-diff` - disables printing differences
* `progress.py` - prints and updates the progress of the repository

### Using this repository
#### Visual Studio Code
If you're using Windows, please use a WSL window for this project. This will ensure the best compatibility with extensions.

The [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) extension is recommended for code formatting, code completion, and errors/warnings.
* To use this extension properly, please let the extension download Clang and follow these steps:
* * Create a `compile_commands.json` in your project root. Paste the following into it and adjust your repository path to match the repository location.
```json
[
    {
      "directory": "/home/user/path/to/OdysseyDecomp",
      "command": "clang++ -x c++ -O3 -std=gnu++1z --target=aarch64-linux-elf -mcpu=cortex-a57+fp+simd+crypto+crc -fno-exceptions -mno-implicit-float -fno-strict-aliasing -fno-short-enums -fdata-sections -fPIC -g -Wall -I include -I lib/agl/include -I lib/nnheaders/include -I lib/sead/include -I tools/clang/include/c++/v1 -D NNSDK -c",
      "file": "required/line.cpp"
    }
  ]
```
* * Do `CTRL + SHIFT + P` and search for `Restart Language Server`. This will restart `clangd` and it should find the json now.

### Contributing
* TODO