#!/bin/bash

HOSTNAME="localhost"
PORT="48000"
USERNAME="root"
HOMEDIR="/home/"
PROJECTNAME="BBBBalancingCube"
APPNAME="${PROJECTNAME}_app"
BUILD_DIR_DEBUG="debug"
BUILD_DIR_RELEASE="release"
BUILD_DIR_NATIVE="build_native"
BUILD_DIR_BBB="build_BBB"

RELEASE_PATH_NATIVE="$BUILD_DIR_NATIVE/$BUILD_DIR_RELEASE/$APPNAME"
DEBUG_PATH_NATIVE="$BUILD_DIR_NATIVE/$BUILD_DIR_DEBUG/$APPNAME"
RELEASE_PATH_BBB="$BUILD_DIR_BBB/$BUILD_DIR_RELEASE/$APPNAME"
DEBUG_PATH_BBB="$BUILD_DIR_BBB/$BUILD_DIR_DEBUG/$APPNAME"

help() {
    echo "Usage: $0 [OPTION]"
    echo
    echo "Options:"
    echo "  -bdn, --build-debug-native      Build the application in Debug mode for local (native) platform"
    echo "  -bdbbb, --build-debug-bbb       Build the application in Debug mode for BeagleBone Black (BBB)"
    echo "  -brn, --build-release-native    Build the application in Release mode for local (native) platform"
    echo "  -brbbb, --build-release-bbb     Build the application in Release mode for BeagleBone Black (BBB)"
    echo "  -rdn, --run-debug-native        Run the Debug version of the application locally"
    echo "  -rdbbb, --run-debug-bbb         Run the Debug version of the application on BeagleBone Black (BBB)"
    echo "  -rrn, --run-release-native      Run the Release version of the application locally"
    echo "  -rrbbb, --run-release-bbb       Run the Release version of the application on BeagleBone Black (BBB)"
    echo "  -gv,  --gui-viewer              Start the GUI to visualize and log data on the host machine"
    echo "  -gc,  --gui-control             Start the GUI to control cube on the host machine"
    echo "  -c, --clean                     Clean the project (remove compiled files)"
    echo "  -sdn, --start-debugger-native   Start the debugger for the local (native) application"
    echo "  -sdbbb, --start-debugger-bbb    Start the debugger for the BeagleBone Black (BBB) application"
    echo "  -sshkey, --copy-sshkey          Copy SSH key to the BeagleBone Black for password-less SSH"
    echo
    echo "Example usage:"
    echo "  $0 --build-debug-native        # Build locally in Debug mode"
    echo "  $0 --run-release-bbb           # Run the Release version on BeagleBone Black"
    echo "  $0 --start-debugger-native     # Start the debugger for local (native) platform"
    echo "  $0 --copy-sshkey               # Copy SSH key to BBB for password-less login"
    echo
}

gui_viewer() {
    python3 gui/cube_data_viewer.py
}

gui_control() {
    python3 gui/cube_control.py
}

file2BBB() {
    if [ $# -ne 1 ]; then
        echo "copy file to BBB needs 1 argument"
        exit 1
    fi
    local file=$1
    echo "Copying file $file to BBB"
    scp -P "${PORT}" "$file" "$USERNAME"@"$HOSTNAME":"$HOMEDIR"
    return $?
}

build_debug_native() {
    echo "Building debug native"
    make BUILD=Debug TARGET_PLATFORM=native
    return $?
}

build_debug_bbb() {
    echo "Building debug bbb"
    make BUILD=Debug TARGET_PLATFORM=bbb
    return $?
}

build_release_native() {
    echo "Building release native"
    make BUILD=Release TARGET_PLATFORM=native
    return $?
}

build_release_bbb() {
    echo "Building release bbb"
    make BUILD=Release TARGET_PLATFORM=bbb
    return $?
}

run_debug_native() {
    echo "Running debug native"
    if ! build_debug_native; then
        echo "Failed to build debug native"
        exit 1
    fi
    ./"$DEBUG_PATH_NATIVE"
}

run_debug_bbb() {
    echo "Running debug on bbb"
    if ! build_debug_bbb; then
        echo "Failed to build debug bbb"
        exit 1
    fi
    if ! file2BBB "$DEBUG_PATH_BBB"; then
        echo "Failed to copy file to BBB"
        exit 1
    fi
    # shellcheck disable=SC2029
    ssh -p "${PORT}" "$USERNAME"@"$HOSTNAME" "sudo -n ${HOMEDIR}${APPNAME}"
}

run_release_native() {
    echo "Running release native"
    if ! build_release_native ; then
        echo "Failed to build release native"
        exit 1
    fi
    ./"$RELEASE_PATH_NATIVE"
}

run_release_bbb() {
    echo "Running release on bbb"
    if ! build_release_bbb ; then
        echo "Failed to build release bbb"
        exit 1
    fi
    if ! file2BBB "$RELEASE_PATH_BBB"; then
        echo "Failed to copy file to BBB"
        exit 1
    fi
    # shellcheck disable=SC2029
    ssh -p "${PORT}" "$USERNAME"@"$HOSTNAME" "sudo -n ${HOMEDIR}${APPNAME}"
}

clean() {
    echo "Cleaning project"
    make clean
}

start_debugger_native() {
    echo "Starting native debugger"
    if ! build_debug_native; then
        echo "Failed to build debug native"
        exit 1
    fi
}

start_debugger_bbb() {
    echo "Starting gdb"
    if ! build_debug_bbb; then
        echo "Failed to build debug bbb"
        exit 1
    fi
    if ! file2BBB "$DEBUG_PATH_BBB"; then
        echo "Failed to copy file to BBB"
        exit 1
    fi
    # shellcheck disable=SC2029
    ssh -p "${PORT}" "$USERNAME"@"$HOSTNAME" "gdbserver :2345 ${HOMEDIR}${APPNAME}"
}

sshkey2BBB() {
    echo "Copy ssh-Key to BBB"
    ssh-copy-id -p "$PORT" "$USERNAME"@"$HOSTNAME"
}


if [ $# -eq 0 ]; then
    echo "No arguments found. Please chose an action."
    help
    exit 1
fi
while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            help
            ;;
        -bdn|--build-debug-native)
            build_debug_native
            ;;
        -bdbbb|--build-debug-bbb)
            build_debug_bbb
            ;;
        -brn|--build-release-native)
            build_release_native
            ;;
        -brbbb|--build-release-bbb)
            build_release_bbb
            ;;
        -rdn|--run-debug-native)
            run_debug_native
            ;;
        -rdbbb|--run-debug-bbb)
            run_debug_bbb
            ;;
        -rrn|--run-release-native)
            run_release_native
            ;;
        -rrbbb|--run-release-bbb)
            run_release_bbb
            ;;
        -gv|--gui-viewer)
            gui_viewer
            ;;
        -gc|--gui-control)
            gui_control
            ;;
        -c|--clean)
            clean
            ;;
        -sdn|--start-debugger-native)
            start_debugger_native
            ;;
        -sdbbb|--start-debugger-bbb)
            start_debugger_bbb
            ;;
        -sshkey|--copy-sshkey)
            sshkey2BBB
            ;;
        -*)
            echo "Unknown option: $1"
            exit 1
            ;;
        *) 
            echo "Uknown argument: $1"
            ;;
    esac
    shift
done

echo "done"