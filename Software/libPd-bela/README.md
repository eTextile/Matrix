# E256 libpd patches
Patches made to work with the E256 eTextile matrix sensor connected via USB to a Bela-mini

## To make your own patches you can follow this instructions
  - Download and install Puredata on your PC
  - Add the following library with the board manager (menu->...)
    - comport
    - mrpeach
  - You will need to add the location of tos externals to the Pd path (FIle -> Preferences -> Path)
 
  - Connect the Bela to your computer (Bela usb-mini -> USB PC)
  - Wait for the boot
  - Open a terminal and write the following commands
    - ssh root@192.168.6.2
	- root@bela:~/Bela/scripts# (Very useful scripts to compile and setup all what you need)
    - root@bela:~/Bela/projects# (Place to store all your projects)

## Load your new patch on the Bela
  - Open a terminal on your computer and navigate to the place you have your project
  - scp -r ./your_project/ root@192.168.6.2:/root/Bela/projects/ (Copy your projet to the Bela)

## Build/Compile your project on the Bela
  - root@bela:~/Bela/scripts# ./build_project.sh --clean --force -c "-p128" ../projects/your_project/


## Auto start your project at boot
  - root@bela:~/Bela/scripts# ./set_startup.sh 'projectname_name' startup -l -c "-p128"
