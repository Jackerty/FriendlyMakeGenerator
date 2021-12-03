# FRIENDLY MAKEFILE GENERATOR
## Introduction
Have you looked at CMake and thought this generates unreadable garbage with too many files?
Have you being frustrated by IDE which Makefile generator doesn't allow you to edit Makefile?
Despite this you still find editing Makefile yourself repetitive task with easy mistakes?

Don't worry Friendly Makefile Generator (fmakegen) is here to make build system for you!

## Usage

## Examples

## Feature list
- NO makefile.am or makefile.in in the project folder
- NO CMakeList or CMakeFiles
- Makefile only used to know state of the makefile.
- Configure script only used know state of configure script.

## Todo list
First Release:
- Git like command before options.
- Commands not case sensitive.
- User given variable creation.
- Can add source files to list.
- Can add folders to list.
- When adding to a list simple name check is addition already in the list.
- Understanding addprefix addpostfix for list addition.
- Flag to disable checks when additing to a list.
- Can create targets.
- When creating a target check that target doesn't already exist.
- Target creation can add depencies.
- Depencies can be added to target.
- Create .PHONY targets.
- Operating system handling creation.
- Configure script in bash generator.
- Configuration handling by OPTIONAL makefile.config makefile
- User can choose name of OPTIONAL makefile.config makefile
- User can select makefile name.
- User can choose folder where makefile is.
- Initialazation from templates.
- Init creates CC variable.
- Init creates CXX variable.
- Init creates CFLAGS variable.
- Init creates CXXFLAGS variable.
- Init creates LDFLAGS variable.
- Init creates default targets
	* all (compile everything including documentation. Default of make)
	* install (install software)
	* uninstall (uninstall software)
	* clean (clean the build enviroment)
	* distclean (clean ./configure creations)
	* check (test suite)
	* intallcheck (Check existance of instalation of the software)
	* dist (make a software download package)
- Init file creation group rights option.
- Coupling between install and uninstall.
- install and uninstall understand prefixes.
- Automatic clean target creation.
- Program understands --help and -h options even without command.
- Friend command which can edit according to a template.
- Friend command can check are install uninstall coupled and fixes the matter.
- Program configuration file for libconfig.
- Program configuration file can choose default names for makefile and configure file.
- Program configutation file can choose makefile.config makefile name.
- Program group permission rights option for created files.
- Program work even if configuration file doesn't exist or is erroneous.
- Prongram configuration file error warning to user.
- Have comment on makefiles and configure scripts which store some state about project folder.
	* template used to create the file.
	* version of fmakegen init was run on.
	* version of fmakegen last modification was run on.
	* Option to turn off
	* Have something libconfig can read through config_read_string

After first release:
- Dynamicly loading Curse GUI.
- Configure script in Python generator.
- KDevelop plugin?
- Python build system support?
- Java build system support?
- Hostile takeover of CMake projects.
- Progress bar.
- Adding git hook scripts.
- config.h support?
	* make does not create config.h

## Build
```
make
```
