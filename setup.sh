#!/bin/bash

# Ensure the script is being run as root
if [[ "$EUID" -ne 0 ]]; then
    echo "This script must be run as root. Please use sudo." >&2
    exit 1
fi

# Help function
function show_help {
    echo "Usage: sudo ./install_script.sh [options]"
    echo "Options:"
    echo "  -h            Display this help message"
}

# Parse arguments
while getopts "h" option; do
    case $option in
        h)
            show_help
            exit 0
            ;;
        *)
            echo "Invalid option. Use -h for help." >&2
            exit 1
            ;;
    esac
done

# Set non-interactive mode and timezone
export DEBIAN_FRONTEND=noninteractive
export TZ=Etc/UTC

# Update and install necessary packages
sudo apt-get update && sudo apt-get install -y --no-install-recommends \
    wget \
    bzip2 \
    build-essential \
    git \
    ca-certificates \
    && sudo update-ca-certificates

# Check if conda is installed
if ! command -v conda &> /dev/null; then
    echo "Conda is not installed. Installing Miniconda..."
    wget https://repo.anaconda.com/miniconda/Miniconda3-py312_24.11.1-0-Linux-x86_64.sh -O /tmp/miniconda.sh \
        && bash /tmp/miniconda.sh -b -p /opt/miniconda \
        && rm /tmp/miniconda.sh

    # Add conda to PATH for the current session
    export PATH="/opt/miniconda/bin:$PATH"
fi

# Create a new conda environment
conda create -n ray1 python=3.11.8 -y && conda activate ray1

# Clear pip cache
pip cache purge

# Install the Python package with optional extras
pip install -e /home/uerl/uerl_python[torch]

# Script complete
echo "Installation complete. Please activate the conda environment with 'conda activate ray1'."
