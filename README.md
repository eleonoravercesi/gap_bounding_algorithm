# The Integrality Gap of the Traveling Salesman Problem is $\frac{4}{3}$ if the LP Solution Has at Most $n+8$ Non-zero Components

This repository contains the code used to reproduce the computational experiments presented in Section 6.3 of the paper.

## Installation

The experiments rely on the following software:
- [Parma Polyhedra Library (PPL)](https://github.com/BUGSENG/PPL/tree/devel);
- [Gurobi](https://www.gurobi.com/);
- [nauty](https://pallini.di.uniroma1.it/).


### Install PPL

First, clone the PPL repository:

```bash
git clone https://github.com/BUGSENG/PPL.git
cd PPL
````

Depending on your system, you may need to install additional dependencies. For example:

```bash
# Install Doxygen (required for generating the documentation)
sudo apt-get install doxygen

# Install TeX packages (required for generating the documentation)
sudo apt-get install texlive-science
```

You can then configure and install PPL with:

```bash
./configure
make
sudo make install
```

During the installation, you may encounter LaTeX-related warnings or errors while generating the documentation. These do not affect the library itself and can be safely ignored.

After the installation, the PPL source directory contains the required header files and libraries. To make PPL accessible to the code, add the following environment variable to your `~/.bashrc`:

```bash
export PPL_INCLUDE_DIR=/path/to/PPL/src/
```

Replace `/path/to/PPL/src/` with the absolute path to the `src` directory of your PPL installation.

After modifying `~/.bashrc`, reload it with:

```bash
source ~/.bashrc
```

### Install Gurobi
This has been automatically generated with [Gurobot](https://www.gurobi.com/product/gurobot).

#### 1. Download Gurobi Optimizer

Download the latest version of Gurobi Optimizer (v13.0.2) for your platform from the [Gurobi Downloads page](https://www.gurobi.com/downloads/gurobi-software/).

For Linux x64:
```bash
# Download the installer
wget https://packages.gurobi.com/13.0/gurobi13.0.2_linux64.tar.gz

# Extract the archive
tar xvfz gurobi13.0.2_linux64.tar.gz

# Move to installation directory (optional but recommended)
sudo mv gurobi1302 /opt/gurobi1302
```

Note: For ARM Linux, use `gurobi13.0.2_armlinux64.tar.gz` instead.

#### 2. Set Up Environment Variables

Add the following lines to your `.bashrc` file (for bash shell) or `.zshrc` file (for zsh shell):

```bash
# Gurobi environment variables
export GUROBI_HOME="/opt/gurobi1302/linux64"
export PATH="${PATH}:${GUROBI_HOME}/bin"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${GUROBI_HOME}/lib"
```

Important Notes:
- Adjust the path `/opt/gurobi1302/linux64` according to where you extracted Gurobi
- For ARM Linux, replace `linux64` with `armlinux64`
- If `LD_LIBRARY_PATH` is not already set, use: `export LD_LIBRARY_PATH="${GUROBI_HOME}/lib"`

Apply the changes:
```bash
source ~/.bashrc  # or source ~/.zshrc for zsh users
```

#### 3. Obtain an Academic License

Since you're at an academic institution (USI), you can get a free academic license:

1. Register and log in at the [Gurobi User Portal](https://portal.gurobi.com)
2. Request a license by navigating to User Portal → Licenses → Request New License
3. Run grbgetkey with the command provided (you'll see something like):
   ```bash
   grbgetkey ae36ac20-16e6-acd2-f242-4da6e765fa0a
   ```
4. Store the license file when prompted. Press Enter to accept the default location (`$HOME/gurobi.lic`), which is strongly recommended.

Important: You must be connected to your academic netwo
rk (USI network) when running `grbgetkey`.

#### 4. Test the Installation

Verify that Gurobi is correctly installed:
```bash
# Check if gurobi is in your path
which gurobi_cl

# Test with the interactive shell
gurobi_cl
```

### Install Nauty
The graph-generation step requires [nauty](https://pallini.di.uniroma1.it/), in particular the `geng` and `showg` executables.

Download and extract the latest version of nauty from the official website, then compile it:

```bash
tar -xzf nauty*.tar.gz
cd nauty*
./configure
make
````
Keep track of the path to your nauty installation, as you will need it in a later step.


## Git LFS

This repository uses [Git LFS](https://git-lfs.com/) to manage large files.

Before cloning the repository, install Git LFS:

```bash
git lfs install
````

Then clone the repository normally:

```bash
git clone https://github.com/eleonoravercesi/gap_bounding_algorithm.git
```


## Building and Running the Code

### 1. Build the solver

From the root directory of the repository, create a `build` directory and move into it:

```bash
mkdir build
cd build
````

Configure the project using CMake:

```bash
cmake ..
```

Then compile the code:

```bash
make
```

If the compilation is successful, the `GB` executable will be created in the `build` directory.

### 2. Run the default experiment: $k=3$

The default setting uses $k=3$. From the root directory of the repository, run:

```bash
./build/GB
```

This runs the Gap-Bounding (GB) algorithm for $k=3$ using the default configuration.

### 3. Run the experiments for other values of $k$

For $k>3$, the support graphs used by the GB algorithm must first be generated.

Go to the `graph` directory and run:

```bash
./generate.sh k /path/to/your/nauty/folder
```

Here, `/path/to/your/nauty/folder` must contain the `geng` and `showg` executables from the [nauty](https://pallini.di.uniroma1.it/) package.

For example:

```bash
cd graph
./generate.sh 8 /path/to/nauty
```

This generates all candidate support graphs required for the computation for the specified value of $k$.

After the graphs have been generated, return to the root directory and run:

```bash
./build/GB k
```

For example, for $k=4$:

```bash
./build/GB 4
```

The generated support graphs are reused by the GB solver when running the computation.


## Tested Systems

Our code has been successfully tested on:

- Linux Ubuntu 24.04.4 LTS

If you successfully run the code on another system, please let us know so that we can add it to the list of tested systems.