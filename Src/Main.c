#include<stdlib.h>
#include<string.h>
#include<unistd.h>
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

int main(int argc, char **argv) {
	//** HELLO LINE **//
	printconst(STDOUT_FILENO,"Friendly Make Generator!\nCommit:"COMMIT"\nNumber of commit:"NUMCOMMITS"\nAuthor: Jackerty\n\n");

	//** CLI HANDLING. **//
	// Basic variables that are effected by CLI.
	// Counts are unsigned so that zero comparasion (faster) can be used at the loops.

	// Usage message printed if --help is used.
	const char usage[]="Usage: fmakegen <command> [options] <makefile>\n"
	"\n"
	"List of commands (not case sensitive):\n"
	"  init\tCreate Makefile if does not exits. \n"
	"\nList of general options:\n"
	"  -h, --help\tPrint usage information and stop execution.\n";

	if(argc>1){

		const Option options[]={
			{"help",.variable.printstr=usage,.value.v32=sizeof(usage),'h',{0,1,OPHAND_PRINT}},
		};
		// Call opHand to handle options. NOTE: argc-1 and argv+1 jumps over program name.
		switch(opHand(argc-1,argv+1,options,sizeof(options)/sizeof(Option))){
			case OPHAND_PROCESSING_STOPPED: return 0;
			case OPHAND_UNKNOW_OPTION: printconst(STDERR_FILENO,"ERROR! Unknown option!\n"); return 1;
			case OPHAND_NO_ARGUMENT: printconst(STDERR_FILENO,"ERROR! Option without a argument!\n"); return 2;
			case OPHAND_PROCESSING_DONE:;
		}

		// Handle command given.
		if(constStrEq(argv[1],"init")){

			;
		}
		else{
			printconst(STDOUT_FILENO,"Provided a command doesn't exist!\n");
			return 0;
		}

	}
	else printconst(STDOUT_FILENO,usage);

	return 0;
}
