# The Integrality Gap of the Traveling Salesman Problem is $\frac{4}{3}$ if the LP Solution Has at Most $n + 8$ Non-zero Components

For this project we need many things.

## Install PPL
Clone [this](https://github.com/BUGSENG/PPL/tree/devel).

Then, you have to install dependencies, according to the error that appear, these might be:
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
Some LaTeX errors will appear for sure, ignore them (`CTRL + Q`). Then, you have the `src` folder. Modify the
`.bashrc` file to add the path of the PPL library:
```bash
# Add PPL library to the path
export PPL_INCLUDE_DIR=/path/to/PPL/src/
```

## Install Gurobi

### 1. Download Gurobi Optimizer

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

**Note:** For ARM Linux, use `gurobi13.0.2_armlinux64.tar.gz` instead.


### 2. Set Up Environment Variables

Add the following lines to your `.bashrc` file (for bash shell) or `.zshrc` file (for zsh shell):

```bash
# Gurobi environment variables
export GUROBI_HOME="/opt/gurobi1302/linux64"
export PATH="${PATH}:${GUROBI_HOME}/bin"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${GUROBI_HOME}/lib"
```

**Important Notes:**
- Adjust the path `/opt/gurobi1302/linux64` according to where you extracted Gurobi
- For ARM Linux, replace `linux64` with `armlinux64`
- If `LD_LIBRARY_PATH` is not already set, use: `export LD_LIBRARY_PATH="${GUROBI_HOME}/lib"`

Apply the changes:
```bash
source ~/.bashrc  # or source ~/.zshrc for zsh users
```

### 3. Obtain an Academic License

Since you're at an academic institution (USI), you can get a free academic license:

1. **Register and log in** at the [Gurobi User Portal](https://portal.gurobi.com)
2. **Request a license** by navigating to User Portal → Licenses → Request New License
3. **Run grbgetkey** with the command provided (you'll see something like):
   ```bash
   grbgetkey ae36ac20-16e6-acd2-f242-4da6e765fa0a
   ```
4. **Store the license file** when prompted. Press Enter to accept the default location (`$HOME/gurobi.lic`), which is strongly recommended.

**Important:** You must be connected to your academic network (USI network) when running `grbgetkey`.


### 4. Test the Installation

Verify that Gurobi is correctly installed:
```bash
# Check if gurobi is in your path
which gurobi_cl

# Test with the interactive shell
gurobi_cl
```

## Troubleshooting

### Issues with Gurobi Installation on WSL2

If you are working on WSL2, you likely encounter issues with Gurobi license.
The (Named-User) Academic Licenses are tied to an HostID which depends on the MAC address of the machine,
and WSL2 changes the MAC address on each restart, causing the error
`HostID mismatch (licensed to xxxxxxxx, HostID is yyyyyyyy)`.

To solve this issue, you need to set a persistent MAC address for WSL2 and make Gurobi reference it.

1. **Modify `/etc/wsl.conf`** to run the script as the root user at WSL startup, adding the following lines.
   ```ini
   [boot]
   command = "/usr/local/bin/persistent-mac-eth1.sh"
   ```

2. **Create the script** `/usr/local/bin/persistent-mac-eth1.sh` with the following content.
   ```bash
   #!/bin/bash
   
   # Assign a persistent MAC address for adapter eth1
   mac="1a:2b:3c:4d:5e:6f"
   if ! ip link show | grep -q "$mac"; then
      if ! ip link show eth1 &> /dev/null; then
         if ! ip link show bond0 &> /dev/null; then
            sudo ip link add bond0 type bond
         fi
         sudo ip link set dev bond0 down
         sudo ip link set dev bond0 name eth1
      fi
      sudo ip link set dev eth1 down
      sudo ip link set dev eth1 address "$mac"
      sudo ip link set dev eth1 up
   fi
   ```
   Make the script executable.
   ```bash
   sudo chmod +x /usr/local/bin/persistent-mac-eth1.sh
   ```
   
3. **Restart WSL** to apply the changes, and check that the MAC address is persistent.
   ```bash
   ip link show eth1
   ```

4. **Save the `eth1` adapter information** needed by Gurobi into a file.
   ```bash
   grbprobe --adapter eth1 --output grbprobe.info
   ```

5. **Get a new Academic License and link it with the `--input` flag**
   to the persistent MAC address `eth1`.
   ```bash
   grbgetkey xxxxxx-xxxx-xxxx-xxxx-xxxxxxxx --input grbprobe.info
   ```



## TODO List
- [ ] Enhance it with LKH3
