# What is this project?

This project allows you to receive waveform data from Ethernet enabled scope (Created for RIGOL DS1054, though should work with others too) and save it in Matlab MAT file format - this allows for quite fast capture and load cycle in Matlab. This project was created out of need, but hopefully someone else will benefit form it too.

Currently we support Windows and Linux/Unix (maybe Mac too, I don't have one, so I can't check). Adding other platforms should be fairly easy (just need to port TCP code).

# How to build?

You will need C++17 enabled compiler (eg. GCC 8 or higher) and cmake.

Under linux this looks like this:

Clone the repository with submodules: `git clone https://github.com/enbyted/scope_receiver.git --recursive`
Create build directory:
```bash
cd scope_receiver
mkdir build
cd build
```
Generate build files: `cmake .. -G "Unix Makefiles"`
Build: `make -j$(nproc)`
Run: `./scope_receiver`

# How to use?

TODO...
For now please run `scope_receiver -h`

# Others

Mat file format documentation: https://pub.ist.ac.at/~schloegl/matlab/matfile_format.pdf
