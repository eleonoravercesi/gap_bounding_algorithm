# The Integrality Gap of the Traveling Salesman Problem is $\frac{4}{3}$ if the LP Solution Has at Most $n + 8$ Non-zero Components

For this project we need many things.

## Install PPL
Clone [this](https://github.com/BUGSENG/PPL/tree/devel).

Then, you have to install dependencies, acording to the error that appear, these might be:
```bash
# Install doxygen for documentation
sudo apt-get install doxygen

# Install tex, for documentation
sudo apt-get install texlive-science
```
There was a trick to make the `configure` script appear. **TODO**

Then, you can install PPL with:
```bash
# Install PPL
cd PPL
./configure
make
sudo make install
```
Some LaTeX errors will appear for sure, ignore them (`CTRL + Q`. Then, you have the `src` folder. Modify the
`.bashrc` file to add the path of the PPL library:
```bash
# Add PPL library to the path
export PPL_INCLUDE_DIR=PPL_INCLUDE_DIR:/path/to/PPL/src/
```

## Install SCIP
------------------------------
First, download the SCIPOpt Optimization Suite: https://scipopt.org/index.php#download and install it.
The executable `scip` will be in `/usr/bin/`, but we will need headers and libraries. 
Libraries will be in `usr/lib/` and headers in `/usr/include/`. You might want to add these paths to the environment variables:

Then, the C++ wrapper for SCIP. Download this repository: [SCIP](https://github.com/scipopt/SCIPpp)

Install [Conan](https://github.com/conan-io/conan) to make your life easier. The installation is 
with Python; you might want to do keep it in mind when choosing th right environment. 

Then follow the instructions on the README file in the section "With Conan v2 and CMake v3.19 or later".
You might need some `sudo`s here and there.

# TODO link libs in CMakeLists.txt
