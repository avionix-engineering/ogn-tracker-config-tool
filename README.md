# ogn-tracker-config-tool
Qt based config tool for OGN trackers

Communicates to OGN tracker over USB serial port and reads current configuration and allows changing parameters.

## Building
The tool is based on Qt. On Windows use QT Creator to compile by double-clicking on the file ogn-config-tool.pro. On Linux call the built_with_qmake.sh script to install dependencies and trigger compilation.

## Execution
The application scans all existing serial ports for the one with the proper name and sends 0x03 byte to make the tracker dump the current configuration. The values are loaded into the GUI and can then be edited. In Normal mode only a few important parameters are shown with dropdown boxes for selection. In Export Mode all paramters can be changed but raw values need to be used. The "Apply" button sends back the configuration to the device. 

![Alt text](pictures/normal_mode.png?raw=true "Normal Mode")

![Alt text](pictures/expert-mode.png?raw=true "Export Mode")

Details about the paramters can be found here: https://github.com/pjalocha/cubecell-ogn-tracker
