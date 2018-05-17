# Conduino 1.2

For any questions or comments please contact 
[Marco Carminati](http://www.deib.polimi.it/eng/people/details/126878) or [Paolo Luzzatto-Fegiz](https://feslab.me.ucsb.edu/people/paolo-luzzatto-fegiz) 

If you use a Conduino in your work, please include the citation:
Marco Carminati, Paolo Luzzatto-Fegiz, Conduino: Affordable and high-resolution multichannel water conductivity sensor using micro USB connectors, In Sensors and Actuators B: Chemical, Volume 251, 2017, Pages 1034-1041, ISSN 0925-4005, https://doi.org/10.1016/j.snb.2017.05.184.


The repository contains the following files:

Hardware:

- Conduino_Gerber:  Gerber-format design files to prepare a PCB;
- AssemblyFile.png: Assembly overview;
- Cond1.png, Cond2.png: 3D (approximate) drawings of completed Conduino 1.2;
- Schematic Conduino 1.2.pdf: Schematics for the electronic design;
- BOM_PartType-Conduino.xlsx: Bill of materials;
- License.md: CC BY 4.0 license description.


Software:

- Matlab: Files for operation through a Matlab Script;
    - Firmware_Conduino_Matlab: Arduino Sketch;
    - ConduinoMultichannel.m: Marlab script, suitable for both Mac or PC;
- Terminal: Files for operation through a serial port terminal;
    - Firmware_Conduino_Terminal: Arduino Sketch;
    - Termite.exe: RS232 serial port terminal for PC, requires no installation see https://www.compuphase.com/software_termite.htm;
    - termite.cat: place in the same folder as Termite.exe;
    - Settings Termite.pdf: Settings for Termite, or similar serial port terminals.
- _Software_for_older_Conduinos_1p0-1p1: versions for older Conduinos;


Conduino 1.2 differs from the Conduino 1.0 described in the 2017 paper:

- Parts that were needed exclusively for the 4-electrode circuit have been removed. This revision substantially simplifies probe design, and reduces complexity. In our tests so far there has been no need to switch from 2-electrode to 4-electrode measurement;
- The overall layout has been revised for a more compact design, and to eliminate a slight kinematic conflict between Ch.4 and the Arduino Uno's USB port.


Areas of ongoing work on Conduino 1.2:

- There is currently no enclosure design. If you design an enclosure, we would be happy to add the CAD file to this repository, of course with attribution;
- If using more than one Conduino (that is, in applications where more than four sensors are needed) in the same tank, interference may develop. We are working to address this, most likely with a software update. There are no interference issues between the four sensors operated by a single Conduino.


If you are looking for an assembled system for purchase, please contact conduino@gmail.com. 

