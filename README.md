# ogn-tracker-config-tool
Qt based config tool for OGN trackers

Communicates to OGN tracker over USB serial port and reads current configuration and allows changing parameters.

## Building
The tool is based on Qt. On Windows use QT Creator to compile. On Linux call the built_with_qmake.sh script to install dependencies and trigger compilation.

## Execution
The scans all existing serial ports for the one with the proper name and sends 0x03 byte to make the tracker send the current configuration. The paramters can then be changed. In Normal mode only a few important parameters are shown with dropdown boxes for selection. In Export Mode all paramters can be changed but raw values need to be used.

![Alt text](pictures/normal-mode.png?raw=true "Title")

![Alt text](pictures/export-mode.png?raw=true "Title")
