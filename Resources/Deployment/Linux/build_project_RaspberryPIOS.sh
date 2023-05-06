# get required dependencies on Raspberry PI OS
yes | sudo apt-get update
yes | sudo apt-get upgrade
yes | sudo apt install	clang \
						g++ \
						libasound2-dev \
						libjack-jackd2-dev \
						ladspa-sdk \
						libcurl4-openssl-dev \
						libfreetype6-dev \
						libx11-dev \
						libxcomposite-dev \
						libxcursor-dev \
						libxext-dev \
						libxinerama-dev \
						libxrandr-dev \
						libxrender-dev \
						libwebkit2gtk-4.0-dev \
						libglu1-mesa-dev \
						mesa-common-dev

# we are in Resources/Deployment/Linux/ -> change directory to project root
cd ../../../

# setup git submodules first
git submodule update --init --recursive

# we are in project root, change to buildscript directory
cd Resources/Deployment/Linux

# start the buildscript (make -j 2 instead of default 8 to not overload the poor raspi)
chmod +x build_project.sh
./build_project.sh 2

# -> after a successful build, one could e.g. use the following as contents for .xsession to have the app start in kind of a kiosk mode. VNC of raspbian will still work in this scenario btw
##!/bin/sh
#exec Documents/Development/GitHub/SpaConBridge/Builds/LinuxMakefile/build/SpaConBridge