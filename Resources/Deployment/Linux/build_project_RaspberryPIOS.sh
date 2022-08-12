# we are in Resources/Deployment/Linux/ -> change directory to project root
cd ../../../

# setup git submodules first
git submodule update --init --recursive

# set convenience variables
JUCEDir=submodules/JUCE
ProjucerMakefilePath="$JUCEDir"/extras/Projucer/Builds/LinuxMakefile
ProjucerBinPath="$ProjucerMakefilePath"/build/Projucer
JucerProjectPath=SpaConBridge.jucer
ProjectMakefilePath=Builds/LinuxMakefile

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

# build projucer
cd "$ProjucerMakefilePath"
make LDFLAGS=-latomic
cd ../../../../../..

# export projucer project
"$ProjucerBinPath" --resave "$JucerProjectPath"

# start building the project
cd "$ProjectMakefilePath"
make LDFLAGS=-latomic