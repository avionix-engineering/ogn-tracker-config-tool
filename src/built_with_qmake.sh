#!/bin/bash
sudo apt install libqt5serialport5-dev
sudo apt install qmake
qmake ogn-config-tool.pro -spec linux-g++
make
