#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
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
* program's main.                             *
**********************************************/
#define printErr(err) printconst(STDERR_FILENO,"fmakegen:ERROR: " #err "\n")

/**********************************************
* Makefile/configure script opening return    *
* values.                                     *
**********************************************/
typedef enum OpeningReturn{
	OPEN_SUCCESS=0,
	OPEN_CREATED=1,
	OPEN_DOES_NOT_EXIST,
	OPEN_ACCESS_ERROR,
	OPEN_NOT_SUPPORTED_DEVICE,
	OPEN_OTHER_ERROR
}OpeningReturn;
/**********************************************
* Make the path for the make file or          *
* configure. If path is null defaultpath is   *
* used. If path is to folder then defaultpath *
* is added to path. Space for this string is  *
* malloced so use path and defaultpath        *
* address values to detect if coder has to    *
* free space.                                 *
*                                             *
* Parameters:                                 *
*   pathforopen is final made path to file.   *
*   path is user input for file to be opened. *
*   defaultpath is default file to be opened. *
* Returns:                                    *
*   OPEN_SUCCESS if function no problems to   *
*   make a path. Reports error if problems    *
*   arise. Note OPEN_DOES_NOT_EXIST error     *
*   means that path does not exist.           *
**********************************************/
static OpeningReturn makePath(const char **pathforopen,const char *path,const char *defaultpath){

	if(path==NULL){
		*pathforopen=defaultpath;
	}
	else{
		// Check now where path points to.
		// If it is folder add default name to
		// path.
		struct stat info;
		if(stat(path,&info)>-1){
			switch(info.st_mode & S_IFMT){
				case S_IFDIR:
					{
						// Hadle case where path points to directory.
						// Allocate temporary memory to hold new path.
						// New path strings length is path's length plus
						// defaultpath's length plus one for null symbol.
						const size_t pathlen=strlen(path);
						const size_t defaultpathlen=strlen(defaultpath);
						const size_t templen=strlen(path)+strlen(defaultpath)+1;
						*pathforopen=malloc(templen);
						memcpy((char*)*pathforopen,path,pathlen);
						memcpy((char*)*pathforopen+pathlen,defaultpath,defaultpathlen);
					}
				case S_IFREG:
					// Handle regular file.
					*pathforopen=path;
				default:
					return OPEN_NOT_SUPPORTED_DEVICE;
			}
		}
		else{
			if(errno==EACCES){
				return OPEN_ACCESS_ERROR;
			}
			else if(errno==ENOENT){
				return OPEN_DOES_NOT_EXIST;
			}
		}
	}
	return OPEN_SUCCESS;
}
/**********************************************
* Open makefile or configure script pointed   *
* by the path. If path is null defaultpath is *
* used. If path is to folder then defaultpath *
* is added to path.                           *
*                                             *
* Parameters:                                 *
*   fd is the pointer file description.       *
*   path is user input for file to be opened. *
*   defaultpath is default file to be opened. *
* Returns:                                    *
*   OPEN_SUCCESS if function open a file and  *
*   writes file description to fd. If file    *
*   wasn't found OPEN_NOT_FOUND is given.     *
*   For other errors look at OpeningReturn    *
*   enumerator.                               *
**********************************************/
static OpeningReturn createFile(int *fd,const char *path,const char *defaultpath){

	const char *pathforopen;
	OpeningReturn returnvalue=makePath(&pathforopen,path,defaultpath);

	// Open the file.
	{
		if((*fd=open(pathforopen,O_RDWR))==-1){
			// What error happened.
			if(errno==EACCES){
				returnvalue=OPEN_ACCESS_ERROR;
			}
			else if(errno==EISDIR){
				returnvalue=OPEN_NOT_SUPPORTED_DEVICE;
			}
			else if(errno==ENOENT){
				// Didn't find the
				goto jmp_CREATE_THE_FILE;
			}
			else returnvalue=OPEN_OTHER_ERROR;
		}

		// Before returning free allocated memory.
		if(pathforopen!=path && pathforopen!=defaultpath){
			free((char*)pathforopen);
		}

		return returnvalue;
	}

	jmp_CREATE_THE_FILE:
	{
		*fd=open(path,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		// Before continuing free allocated memory.
		if(pathforopen!=path && pathforopen!=defaultpath){
			free((char*)pathforopen);
		}
		if(*fd){
			// What error happened.
			if(errno==EACCES){
				return OPEN_ACCESS_ERROR;
			}
			else if(errno==EISDIR){
				return OPEN_NOT_SUPPORTED_DEVICE;
			}
			else return OPEN_OTHER_ERROR;
		}
	}
	return OPEN_CREATED;
}
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

			int makefilefd;
			OpeningReturn makefilereturn=createFile(&makefilefd,argv[2],"Makefile");
			if(makefilereturn==OPEN_SUCCESS){
				// Is file empty?
				// Two stat system calls needed since
				// path is not always to target file.
				struct stat statbuff;
				if(fstat(makefilereturn,&statbuff)==0){
					if(statbuff.st_size==0){
						//TODO: INIT makefile
						;
					}
					else printErr("Init was not run on makefile as it was not empty file!");
				}
				else printErr("fstat gave a error!");
				close(makefilefd);
			}
			else if(makefilereturn==OPEN_CREATED){
				//TODO: INIT makefile
				;
			}
		}
		else{
			printErr("Provided a command doesn't exist!");
			return 0;
		}

	}
	else printconst(STDOUT_FILENO,usage);

	return 0;
}
