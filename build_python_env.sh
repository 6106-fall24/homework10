#!/bin/bash
set -e

if [[ $EUID -ne 0 ]]; then
#    if ! command -v pip3 &> /dev/null; then
#       echo "pip3 not found. Installing requirements..."
#       sudo apt install python3-pip python3-venv
#    fi
   sudo -E $0
else
   echo "Installing matplotlib and pandas"
   python3 -m venv homework10-venv
   source homework10-venv/bin/activate
   pip3 install matplotlib
   pip3 install pandas
   echo "Done!"
fi