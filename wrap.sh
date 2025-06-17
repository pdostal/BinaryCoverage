#!/bin/bash

# A script to wrap a binary, save it, and run it with Intel Pin for instrumentation.
# Also includes an --undo switch to clean up saved binaries and logs.

# --- Configuration ---
# Root directory for Pin. Make sure this is set in your environment.
# If not set, the script will default to a placeholder.
PIN_ROOT=${PIN_ROOT:-/path/to/your/pin/root}

# The Pin tool to be used for instrumentation.
PIN_TOOL="$PIN_ROOT/source/tools/ManualExamples/obj-intel64/FuncTracer.so"

# --- Directories ---
# Directory to store the copies of the original binaries.
PROGRAM_SAVE_DIR="/var/coverage/bin"
# Directory to store the Pin tool's output logs.
LOG_DIR="/var/coverage/data"
# Main working directory
WORK_DIR="/var/coverage"

# --- Script Logic ---

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Undo Function ---
# Handles the cleanup of saved binaries and logs for a given program.
handle_undo() {
  if [ -z "$1" ]; then
    echo "Error: --undo requires a binary path to identify which program to clean up."
    echo "Usage: $0 --undo <path_to_binary>"
    exit 1
  fi

  local binary_path="$1"
  local binary_name
  binary_name=$(basename "$binary_path")

  echo "--> Performing cleanup for '$binary_name'..."
  echo "    This will remove saved program copies and their corresponding logs."

  # Remove saved binary copies
  echo "--> Deleting saved binary copies from '$PROGRAM_SAVE_DIR':"
  # Use find to safely print and delete files. It won't error if no files are found.
  find "$PROGRAM_SAVE_DIR" -name "$binary_name.*" -print -delete
  echo "    Done."

  # Remove log files
  echo "--> Deleting log files from '$LOG_DIR':"
  find "$LOG_DIR" -name "${binary_name}_*.log" -print -delete
  echo "    Done."

  echo "--> Cleanup for '$binary_name' complete."
}


# --- Main Execution ---

# 1. Check for command line switch
if [ "$#" -gt 0 ]; then
    case "$1" in
        -u|--undo)
            handle_undo "$2"
            exit 0
            ;;
        -h|--help)
            echo "Usage: $0 <path_to_binary> [args...]"
            echo "       $0 -u|--undo <path_to_binary>"
            echo "       $0 -h|--help"
            echo ""
            echo "Modes:"
            echo "  <default>      Wrap and run the binary with Intel Pin."
            echo "  -u, --undo     Clean up saved binaries and logs for the specified program."
            echo "  -h, --help     Show this help message."
            exit 0
            ;;
    esac
fi

# 2. Check for correct number of arguments for wrapping
if [ "$#" -lt 1 ]; then
    echo "Error: Missing binary path."
    echo "Usage: $0 <path_to_binary> [args...]"
    echo "Use --help for more options."
    exit 1
fi

# The binary to be executed is the first argument.
BINARY_PATH="$1"
# All other arguments are passed to the binary.
shift

# 3. Create working directories if they don't exist
# Use -p to create parent directories as needed.
echo "--> Creating directories..."
mkdir -p "$WORK_DIR"
mkdir -p "$PROGRAM_SAVE_DIR"
mkdir -p "$LOG_DIR"
echo "    Done."

# 4. Check if the binary exists and is executable
if [ ! -x "$BINARY_PATH" ]; then
    echo "Error: Binary '$BINARY_PATH' does not exist or is not executable."
    exit 1
fi

# Get the base name of the binary (e.g., "gzip" from "/usr/bin/gzip")
BINARY_NAME=$(basename "$BINARY_PATH")

# 5. Save the binary to a "safe" place to avoid race conditions
echo "--> Saving binary..."
# Create a temporary file in the destination directory.
TMP_BINARY_PATH=$(mktemp "$PROGRAM_SAVE_DIR/$BINARY_NAME.XXXXXX")
# Copy the binary to the temporary file.
cp "$BINARY_PATH" "$TMP_BINARY_PATH"
# The saved binary path is now the temporary file path. This is our secure copy.
SAVED_BINARY_PATH="$TMP_BINARY_PATH"
echo "    Binary saved to: $SAVED_BINARY_PATH"

# 6. Prepare for Pin execution and logging
echo "--> Preparing for instrumentation..."

# Generate a unique filename for the log file.
# Format: <binary_name>_<YYYYMMDD-HHMMSS>_<nanoseconds>.log
TIMESTAMP=$(date "+%Y%m%d-%H%M%S")
NANO_SECONDS=$(date "+%N")
LOG_FILE="$LOG_DIR/${BINARY_NAME}_${TIMESTAMP}_${NANO_SECONDS}.log"

echo "    Pin tool: $PIN_TOOL"
echo "    Log file will be: $LOG_FILE"

# Check if Pin and the Pin tool exist
if [ ! -f "$PIN_ROOT/pin" ]; then
    echo "Error: Pin executable not found at '$PIN_ROOT/pin'."
    echo "Please ensure the PIN_ROOT environment variable is set correctly."
    exit 1
fi

if [ ! -f "$PIN_TOOL" ]; then
    echo "Error: Pin tool not found at '$PIN_TOOL'."
    exit 1
fi

# 7. Run the original binary with Pin instrumentation
echo "--> Running binary with Pin..."
# The -- separates the arguments for Pin from the arguments for the target binary.
# "$@" passes all the remaining arguments to the wrapped binary.
"$PIN_ROOT/pin" -t "$PIN_TOOL" -o "$LOG_FILE" -- "$SAVED_BINARY_PATH" "$@"

echo "--> Execution finished."
echo "    Log saved to: $LOG_FILE"

exit 0