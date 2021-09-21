#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>
#include<stdint.h>
#include<stdbool.h>
#include"PrintTools.h"
#include"OpHand.h"

/**********************************************
* Simple macro to find miminum of two values. *
**********************************************/
#define MIN(_x_,_y_) (_x_ < _y_ ? _x_ : _y_ )
/**********************************************
* Macro for constant string equality check    *
* against user input.                         *
**********************************************/
#define constStrEq(U,C) strncmp(U,C,MIN(strlen(U),strlen(C)))==0
/**********************************************
* Standardize error formating for this        *
* program* calling.                           *
**********************************************/
#define printErr(err) printconst(STDERR_FILENO,"fmakegen:ERROR: " #err "\n")

/**********************************************
* Main handles CLI mainly.                    *
**********************************************/
int main(int argc, char **argv) {
	//** HELLO LINE **//
	// TODO: Add bug report line after author leaving newline
	printconst(STDOUT_FILENO,"Friendly Make Generator!\nCommit: "COMMIT"\nNumber of commits: "NUMCOMMITS"\nAuthor: Jackerty\n\n");

	//** CLI HANDLING. **//
	// Basic variables that are effected by CLI.
	// Counts are unsigned so that zero comparasion (faster) can be used at the loops.

	// Usage message printed if --help is used.
	const char usage[]="Usage: fmakegen <command> [options] [makefile | configure | folder]\n"
	"\n"
	"Generate or add to makefile and configuration script. Makefile/configure is ether given manually or folder of the makefile/configure is given. Without name \"Makefile\" and \"configure\" are assumed file names."
	"\n"
	"List of commands (not case sensitive):\n"
	"  init\tCreate Makefile if does not exits. \n"
	"\nList of general options:\n"
	"  -h, --help\tPrint usage information and stop execution.\n"
	"  -v, --version\tPrint only version information.\n"
	"  --config <fmakegen-ini-file>\tSelect INI configuration file for this fmakegen run.\n"
	"\nList of options which work with commands init:\n"
	"  -c, --configuration\tFor init make configuration script.\n";

	if(argc>1){

		// Program flags for later.
		// Union is done so that flags can be feeded to ophand.
		union{
			struct{
				bool configurationincluded:1;
			};
			int32_t shadow;
		}progflags;
		progflags.shadow=0;

		// INI configuration location for the program.
		// Default value depends on DEBUG mode so development is seperate.
		// TODO: config file name macro?
		char *inipath;
		#if DEBUG
			inipath="etc/config.ini";
		#else
			//TODO: Add macro checks for this!
			#define PROG_LOC "/etc/fmakegen/"
			inipath=PROG_LOC "/config.ini";
		#endif

		// Call opHand to handle options. NOTE: argc-1 and argv+1 jumps over program name.
		const Option options[]={
			{"help",.variable.printstr=usage,.value.v32=sizeof(usage),'h',{false,true,OPHAND_CMD_PRINT}},
			{"version",.variable.p32=&progflags.shadow,.value.v32=1,'v',{false,true,OPHAND_CMD_OR}},
			{"config",.variable.str=&inipath,.value.str=NULL,'\0',{true,false,OPHAND_CMD_POINTER_VALUE}},
			{"configuration",.variable.p32=&progflags.shadow,.value.v32=0,'c',{false,false,OPHAND_CMD_VALUE}},
		};
		switch(opHand(argc-1,argv+1,options,sizeof(options)/sizeof(Option))){
			case OPHAND_PROCESSING_STOPPED: return 0;
			case OPHAND_UNKNOW_OPTION: printErr("Unknown option!"); return 1;
			case OPHAND_NO_ARGUMENT: printErr("Option without a argument!"); return 2;
			case OPHAND_PROCESSING_DONE:;
		}


		// Handle command given.
		// First turn it lower case.
		for(char *c=argv[1];*c;c++) *c=tolower(*c);
		// Second test what it is.
		if(constStrEq(argv[1],"init")){
			// Init creates makefile from scratch.
			// Makefile isn't created if file exists and has content.
			uint8_t iobuffer[4096];

		}
		else{
			printErr("Provided a command doesn't exist!");
			return 0;
		}

	}
	else printconst(STDOUT_FILENO,usage);

	return 0;
}
