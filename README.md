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
