# cliPro App Overview
cliPro is a desktop application that allows you to copy & paste text from & to the clipboard.  
You can set-up multiple profiles, each having their own 'clips'.  
Currently only text-clips are supported, with a maximum of 100.000 characters per clip (roughly ~100KB).  
A "cliProApp" folder will be created within the directory where the program has been started.  
It contains saved program settings as well as all profiles and clips (in JSON form).  
  
# cliPro "Installation" Guide
Using this app does not require a system wide installation.  
Instead, it only needs to be downloaded (or compiled from source) and can then simply be started via terminal (linux) or double-click (windows / newer linux distros).  
  
## Linux
### Download pre-compiled binary here (7z file, @ Google Drive):  
https://drive.google.com/file/d/11Dcmp8fIdT-_sHr7bTz4De-KuYHoHPVU  
No pre-requisites are required, as all dependencies have already been compiled into the binary.  
  
Give the app permission to be executed:  
```
chmod +x ./cliPro
```
Start the program:  
```
./cliPro
```
  
### How-To Compile from source
There are two ways to compile from source:  
**(1)** Build via qmake, qmake6 to be precise  
**(2)** Compile from within qt creator  
The **(1)** option is recommended, if you quickly want to compile the software without installing the whole suite (which is a few GB in size).  
  
*Both steps require the repository to be copied/cloned:*  
```
git clone https://github.com/getzin/cliPro.git
```
  
#### (1) Compile via qmake6
**1.** Install make (already installed on most linux distributions, check if `make -v` or `make -h` work if unsure)
```
sudo apt install make
```
**2.** Install qmake6
```
sudo apt install qt6-base-dev
```
**3.** Go into the repository
```
cd <path-to-cloned-repository>
```
e.g.
```
cd cliPro/
```
**4.** Compile .pro file using qmake6 [this will produce a makefile]
```
qmake6 cliPro.pro
```
**5.** Compile makefile with regular make
```
make
```
  
#### (2) Compile via Qt Creator
**1.** Register @ qt  
Head over to https://www.qt.io/ and create an account. The qt creator software cannot be used without an account.  
  
**2.** Install qt creator  
Download the installer from here: https://www.qt.io/download-qt-installer-oss  
Make sure to select **install for desktop development** during the installation process. ("installation folder" step)  
  
**3.** Open the project by opening the cliPro.pro file from `File->Open File or Project`.  
  
**4.** `Build->Compile` to compile. Select debug/release as you like.  
  
  
## Windows
### Download pre-compiled binary here (7z file, hosted @ Google Drive):  
https://drive.google.com/file/d/1ziDfA3ta-sf93YM_3uVsUqEBFAt600CL  
  
Extract the 7z file. Then start `cliPro.exe` file within the extracted folder.
  
### How-To Compile from source
See the linux steps **(2)** (compile via qt creator), the process for windows is the same.  
I have not tried this myself, but it is possible to compile qt software via Microsoft Visual Studio Code.  
There are several tutorials available online if that's the route you want to take.  
  
After compiling, the .exe can be found in /build/Desktop.../<release or debug>/ (exact path differs depending on environment/installation).  
You need to add the dependencies/libraries (dlls) or else the program won't run (dll errors) when it is started outside of qt.  
To fix:  
1) While not necessary, it is a good idea to put the binary into a new folder, e.g. win64.  
2) Then., the easiest way to add dependencies/libraries is using the official `windeplayqt.exe` tool,  
which can be found under `C:\Qt\6.8.1\mingw_64\bin\windeployqt.exe` or similar.  
  
The dependencies found within the 7z file linked above have been gathered using the following command:  
```
C:\Qt\6.8.1\mingw_64\bin\windeployqt.exe C:\Users\<user>\cliPro\win64\cliPro.exe --no-translations --no-opengl-sw --release
```
(replace <user> accordingly, as well as the path to `windeployqt.exe`)  
This command assumes that the repository has been compiled for release (from within qt creator).  
The .exe can now be started regardless of qt being installed or not (only when inside the folder containing all the dlls).  