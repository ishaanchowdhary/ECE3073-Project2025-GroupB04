# ECE3073 Team B04 Github Repository

Official Github Repository for Team B04

## General Information

### Team Members: 
- Ishaan Chowdhary (icho0011@student.monash.edu): 33115303
- Pranav Khatore (pkha0008@student.monash.edu): 33867860
- Xiang Ji (xjii0020@student.monash.edu): 3311316
- Zach Nabulsi (znab0001@student.monash.edu): 33893012
- Saam Wadyalkar (swad0015@student.monash.edu): 33878749

### Hyperlinks
- Google Drive: https://drive.google.com/drive/folders/1Io3ZJ5lEii86HpRdqcxurHwWRgZr2M0c
- Project Outline: https://docs.google.com/document/d/1Q_rvMKkylZkh_7S5NG182kRnvroPgW-FkgPO-uKmHZA/edit?tab=t.0

## Project Milestone 1:
- Document: https://docs.google.com/document/d/1dCBVDn1swcVkEB9k41yAv5fwvRmAM_QDuVHKTNi9dGI/edit?tab=t.0

### Steps to Run the Milestone 1 Project:

1. Clone the main branch of the repository.
2. Open the PROJECT_M1_FINAL.qsf (might have to regenerate qsys hdl to get a qip file) and then compile the Verilog, setting the PROJECT_M1_FINAL.v file as the top-level module.
3. Open the NIOS II Eclipse (Tools > NIOS II Software Build Tools for Eclipse).
4. In Eclipse, open an existing project (File > Import > General  > Existing Projects into Workspace) and open the software folder as the project.
5. Now open Qartus Prime Programmer in NIOS and add the .sof file, running the code on the FPGA board. 
6. Build the Eclipse project (Ctrl + B) and then run the configuration (run > run configurations). 

Or if using the zip folder:

1. Unzip the folder
2. Run the PROJECT_M1.QAR
3. Follow step 2 from above

## Project Milestone 2:
- Document: https://docs.google.com/document/d/17p9ONadSBzzAWHCJbZrsCfOODOhzqoRT8UTNIHmoJIc/edit?tab=t.0

### Steps to Run the Milestone 2 Project:

1. Follow the same instructions to set up from the Milestone 1 Project above.
2. To switch between single and quadrant image modes, press KEY1.
3. In single display mode, tapping the glass panel twice will cycle through the display modes (normal frame, flipped, blurred and edge detection), changing the display.
5. In quadrant mode, tilting the FPGA board rearranges the order of the sub-displays (normal frame, flipped, blurred and edge detection) shown. 
