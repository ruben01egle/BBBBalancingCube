# C++ Application Template for BeagleBone & Desktop

This project template provides a structured starting point for building and running C++ applications on both **your local machine** and a **remote BeagleBone Black (BBB)**.

It supports development in **Visual Studio Code** as well as from the **terminal**, with an automated build and task system using `makefile` and `manage_cube.sh`.

---

## 📁 Project Structure

```
BBBBalancingCube/
├── include/        # Header files for application logic
├── src/            # Source files (must match include declarations)
├── lib/            # Local libraries (grouped by domain)
├── .vscode/        # VS Code tasks, debugger configs, settings
├── makefile        # Central build file (local or cross)
├── manage_cube.sh  # Helper script for building/deploying from terminal
├── connect_cube.sh # Helper script to establish and manage connection to remote BBB
├── tools/          # directory for additional tools (calibration, data post processing etc.)
```

You **must follow this structure** for the build system and tasks to work properly. Read the file `lib/README.md` for information on the provided framework.

---

## Getting Started

If your machine comes with any of these pre-installed skip the installation (the emsys VM can skip 2.)
Get this repo by downloading the zip or cloning it:
```bash
git clone git@github.com:ruben01egle/BBBCube.git
```

### 1. Install VS Code

- Set up apt-repository following: https://code.visualstudio.com/docs/setup/linux or download from: https://code.visualstudio.com/download (do not use snap)

### 2. Install Required Tools

Install Make:

```bash
sudo apt update
sudo apt install make
```
Compiler for native/local development:
```bash
sudo apt update
sudo apt install build-essential gdb
```
Cross Compiler for BBB developement:
```bash
sudo apt update
sudo apt install g++-arm-linux-gnueabihf
```
Install autossh to connect to remote server:
```bash
sudo apt update
sudo apt install autossh
```
Install OpenCV for python for the live video stream:
```bash
sudo apt update
sudo apt install python3-opencv
```
Install python QT packages for live data visualization:
```bash
sudo apt update
sudo apt install python3-pyqt5
sudo apt install python3-pyqtgraph
```

### 3. Install Required Extensions

Search and install the following extensions in VS Code:

- `C/C++` (by Microsoft) – for IntelliSense and debugging  
- `Makefile Tools` – for make-based builds  
- `Task Explorer` by `Scott Meesseman` – to execute tasks from the primary sidebar with one click

### 4. Open the Project Folder

Open the repository with VS Code.

### 5. Connect to the cube from remote network

The script `connect_cube.sh` opens and manages a SSh tunnels to the remote MSys-Server for your group. It establishes port forwarding to all required devices for each group - including the BBB, the servo controlling the cube, webcam and power socket. Start the connection:
```bash
./connect_cube.sh start groupX
```
Then follow the promts in the terminal and keep the connection open.
Monitor the connection:
```bash
./connect_cube.sh status groupX
```
Close the connection:
```bash
./connect_cube.sh stop groupX
```

Open a browser and acces the page [localhost:42000](http://localhost:42000/) to turn on the power for the BBB.

Now you can reach the BBB with SSH:
```bash
ssh root@localhost -p48000
```
To see the live camera feed and change the position of the cube run the python gui `cube_control.py`.

### 5. Use Predefined Tasks

- Open the **Task Explorer** in the VSCode Explorer sidebar and open the vscode tasks.
- Click to run:
    - `build native`
    - `run native`
    - `build for BBB`
    - `run on BBB`
    - `gui control`
    - `gui viewer`
- Debugger are setup:
    - `Debug Local`
    - `Debug on BBB`

All configurations (compilers, debuggers, paths) are preconfigured in `.vscode/tasks.json` and `.vscode/launch.json`.

### 6. Set up SSH Key Authentication for Root Login on BeagleBone Black (BBB)

If you start out fresh, it’s a good idea to configure SSH key authentication to avoid having to enter the root password every time you connect to the BeagleBone Black (BBB).

To do this, follow these steps:

1. **Ensure you have access to the BBB**: Make sure you can still log in to your BeagleBone Black using the root password. If you don't have access, make sure the BBB is properly connected and accessible over the network.

2. **Run the SSH key setup script**: Use the task `copy ssh-key to BBB` and enter the root password to copy your PC's ssh-Key to the BeagleBone Black,  allowing for password-less login in the future.
The `manage_cube.sh` script also contains a helper function. To copy your SSH key to the BBB, run the following command:

```bash
./manage_cube.sh --copy-sshkey
```

### 7. Run code on BBB

Use the pre defined tasks in VSCode or the script `manage_cube.sh` to launch tasks on the BBB. To visualize and record the live data sent form the BBB run the python gui `cube_data_viewer.py` directly in the terminal, with the pre defined VSCode task or with the script `manage_cube.sh`. The data gets stored in the folder `ExperimentsData/` with a timestamp in a csv-file. You can influence the data visualized and recorded by changing modes in the file `gui/cube_data_viewer.py`:
```python
plotter = PlotManager(mode="state")
...
recorder = DataRecorder(file_path, mode="imu+state")
```
---

## 💻 Terminal-Only Usage (no VS Code)

(warning: debugger is setup in VS Code)
You can also use the project entirely from the terminal and any text editor:

Check `./manage_cube -h` for more information