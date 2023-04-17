# we are in Resources/Deployment/Linux/ -> change directory to project root
cd ../../../

# set convenience variables
JUCEDir=submodules/JUCE
ProjucerMakefilePath="$JUCEDir"/extras/Projucer/Builds/LinuxMakefile
ProjucerBinPath="$ProjucerMakefilePath"/build/Projucer
JucerProjectPath=SpaConBridge.jucer
ProjectMakefilePath=Builds/LinuxMakefile

# build projucer
cd "$ProjucerMakefilePath"
make -j 8
cd ../../../../../..

# export projucer project
"$ProjucerBinPath" --resave "$JucerProjectPath"

# start building the project
cd "$ProjectMakefilePath"
make -j 8