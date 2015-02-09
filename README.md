# Pathfinding demo #
2D demonstration of the pathfinding abilities of the A* and Dijkstra's algorithms.
Built using xsngine (See: Razish/xsngine)
Written with native execution on 32/64 bit Windows and Linux in mind. May work on Mac, but no official support as of yet.

# Compiling #

## General ##
There is a scons project capable of producing Release, Debug and Optimised-Debug builds. You can also force 32 bit builds.
There are some project files provided for VS2013, CodeLite and Code::Blocks which just invoke scons. This is for ease of debugging.
Steps:
* Install Python 2.7
* Install SCons (preferrably latest, at-least 2.3.0)
* Run `scons` from the `src` directory


## Windows ##

Make sure `C:\Python27` and `C:\Python27\Scripts` are in your `%PATH%` environment variable.
Install Visual Studio 2013 or later for C++11 features.
You can try to use VS2010 but it is unsupported, and you may have to patch it with `msinttypes` (first google result)

## Linux ##

You will require libfreetype6-dev and libpng16-dev (which is not available in the main repositories as of 14.10)
Beware a dependency issue for libfreetype6-dev, which requires libpng12, you must install libpng16-dev from [dimensio/libpng16-deb](https://github.com/dimensio/libpng16-deb)

```bash
$ git clone https://github.com/dimensio/libpng16-deb.git && cd libpng16-deb
$ sudo apt-get install bundler curl
$ bundle install
$ make
$ dpkg -i ./*.deb
```

## Mac ##

There is currently no support for Mac, though xsngine aims to be platform-agnostic as can be.
The scons project may work. The code will likely work.

# Debugging #
All the provided project files assume there is an executable file named `pathfinding` in the current working directory.
Because different platforms will have different binary names/extensions, and these tools are used on all platforms, it is recommended to create a symlink from `pathfinding` to e.g. `pathfinding.x86.exe` - On Windows you will require the Win32 Symlink Extensions.
On Windows, you may also want to symlink SDL2.dll to the correct one for your architecture, e.g. src/SDL2.dll -> bin/SDL2x86.dll

Run the binary with `+set com_path ../bin` to load assets correctly.
