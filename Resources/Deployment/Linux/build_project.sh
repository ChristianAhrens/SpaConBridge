# we are in Resources/Deployment/Linux/ -> change directory to project root
cd ../../../

Cores=8
if ! [ -z "$1" ]
then
  Cores="$1"
fi
echo Using "$Cores" cores to build

# set convenience variables
JUCEDir=submodules/JUCE
ProjucerMakefilePath="$JUCEDir"/extras/Projucer/Builds/LinuxMakefile
ProjucerBinPath="$ProjucerMakefilePath"/build/Projucer
JucerProjectPath=SpaConBridge.jucer
ProjectMakefilePath=Builds/LinuxMakefile

# build projucer
echo Build Projucer
cd "$ProjucerMakefilePath"
make -j "$Cores" "LDFLAGS=-latomic" "CONFIG=Release"
cd ../../../../../..

# export projucer project
echo Export Projucer Project
"$ProjucerBinPath" --resave "$JucerProjectPath"

# start building the project
echo Build the project
cd "$ProjectMakefilePath"
make -j "$Cores" "LDFLAGS=-latomic" "CONFIG=Release"