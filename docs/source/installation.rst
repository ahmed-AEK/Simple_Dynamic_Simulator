Installation
############

Download binaries from github
-----------------------------

Binaries for both window and linux are available on the `github page <https://github.com/ahmed-AEK/Simple_Dynamic_Simulator>`_ in the `releases section <https://github.com/ahmed-AEK/Simple_Dynamic_Simulator/releases>`_.

* Windows binaries have ``win`` in their name and end with ``.zip`` suffix.
* Linux banries have ``linux`` in their name and end with ``.tar.xz`` suffix.

Use Online version
------------------

An `online version <https://ahmed-aek.github.io/DynamicSimulator-Online/>`_ is available that uses emscripten, the main difference is that for now it doesn't allow you to save to disk or load from disk. and doesn't have blocks that save to disk or load from disk.

https://ahmed-aek.github.io/DynamicSimulator-Online/


Building from source on windows
-------------------------------
You need to have **visual studio** with C++ compilation and **python** installed.

#. Clone the repo ``git clone https://github.com/ahmed-AEK/Simple_Dynamic_Simulator``
#. Open the ``x64 Native Tools Command Prompt for VS 2022`` from the stard menu and ``cd`` to the project root
#. Generate the resrouce files with python using ``python AssetsPacker/packer.py``
#. Generate cmake configuration using ``cmake . --preset=vcpkg-win-rel``
#. Build the project using ``camke --build build/Release_win``, the output binary should be in ``build/Release_win/bin`` subdirectory named ``DynamicSimulator.exe``

Building from source on linux (Ubuntu 24)
-----------------------------------------
The process is similar to windows, but you need at least gcc 12 for C++20 features, and you'll need to install vcpkg yourself, along with its prequisites.

For example ubuntu ships a recent version of python. python3.7 is the least version that can build this application, so on other distros you'll need to download python3.7 (or newer) using your package manager then run the build steps inside a venv. on an old centos i had to install ``autoconf`` and perl ``perl-IPC-Cmd`` and ``perl-open.noarch``, generally follow the errors in the log file as they pop up, no errors should happen on ubuntu 24

#. Install `vcpkg <https://github.com/microsoft/vcpkg>`_ using their cmake instructions, you should have ``VCPKG_ROOT`` pointing to where vcpkg is installed and it should also be on your ``PATH`` so it can be called through the terminal.
#. Install `SDL3 linux dependencies <https://wiki.libsdl.org/SDL3/README/linux>`_, without them some things won't work like opengl or wayland support.
#. Clone the repo ``git clone https://github.com/ahmed-AEK/Simple_Dynamic_Simulator``
#. Generate the resrouce files with python using ``python AssetsPacker/packer.py``
#. Make sure jinja2 is installed with `sudo apt install python3-jinja2`
#. Generate cmake configuration using ``cmake . --preset=vcpkg-linux-rel``, on this step many errors could occur on other distros by unavailable dependencies, follow the error logs for now.
#. Build the project using ``camke --build build/Release_linux``, the output binary should be in ``build/Release_linux/bin`` subdirectory named ``DynamicSimulator``
