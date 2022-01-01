#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<stdint.h>
#include<stdbool.h>
#include<libconfig.h>
#include"PrintTools.h"
#include"OpHand.h"

/**********************************************
* TODO: Add macro checks for this!            *
**********************************************/
#define PROG_LOC "/etc/fmakegen/"
/**********************************************
* Simple macro to find miminum of two values. *
**********************************************/
#define MIN(_x_,_y_) (_x_ < _y_ ? _x_ : _y_ )
/**********************************************
* Standardize error formating for this        *
* program's main.                             *
**********************************************/
#define printErr(err) printconst(STDERR_FILENO,"fmakegen:ERROR: " #err "\n")
/**********************************************
* Standardize warming formating for this      *
* program's main.                             *
**********************************************/
#define printWarm(warm) printconst(STDERR_FILENO,"fmakegen:WARM: " #warm "\n")

/**********************************************
* Print out libconfig parse error.            *
**********************************************/
void printParseErr(const config_t *config){
	const char *errorfile=config_error_file(config);
	const char *errortext=config_error_text(config);
	int errorint=config_error_line(config);
	IoVec vec[]={
		{.iov_base="        ",.iov_len=8},
		{.iov_base=(char*)errorfile,.iov_len=strlen(errorfile)},
		{.iov_base=":\n        ",.iov_len=10},
		{.iov_base=(char*)errortext,.iov_len=strlen(errortext)},
		{.iov_base=" at line ",.iov_len=9},
		{.iov_base=0,.format_type=IO_VEC_INT_TO_DEC},
		{.iov_base=".\n",.iov_len=2}
	};
	writevf(STDERR_FILENO,vec,sizeof(vec)/sizeof(IoVec),errorint);
}
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
static OpeningReturn makePath(char **pathforopen,char *path,char *defaultpath){

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
						// It is a folder but did the path include slash as a ending or not?
						// Has to be added if not.
						size_t templen;
						if(path[pathlen-1]=='/') templen=pathlen+defaultpathlen+1;
						else templen=pathlen+1+defaultpathlen+1;
						*pathforopen=malloc(templen);
						memcpy((char*)*pathforopen,path,pathlen);
						if(path[pathlen-1]=='/'){
							memcpy((char*)*pathforopen+pathlen,defaultpath,defaultpathlen+1);
						}
						else{
							(*pathforopen)[pathlen]='/';
							memcpy((char*)*pathforopen+pathlen+1,defaultpath,defaultpathlen+1);
						}
					}
					break;
				case S_IFREG:
					// Handle regular file.
					*pathforopen=path;
					break;
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
static OpeningReturn createFile(int *fd,char *path,char *defaultpath){

	char *pathforopen;
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
		*fd=open(pathforopen,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		// Before continuing free allocated memory.
		if(pathforopen!=path && pathforopen!=defaultpath){
			free((char*)pathforopen);
		}
		if(*fd==-1){
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
* Makefile init flags? TODO*
**********************************************/
/**********************************************
* Initialize makefile.                        *
**********************************************/
static void initMakefile(int makefilefd,char *templatename){

	// To copy what features init command should have at this time.
	// - Initialazation from templates.
	// - Init creates CC variable.
	// - Init creates CXX variable.
	// - Init creates CFLAGS variable.
	// - Init creates CXXFLAGS variable.
	// - Init creates LDFLAGS variable.
	// - Init creates default targets
	//   - all (compile everything including documentation. Default of make)
	//   - install (install software)
	//   - uninstall (uninstall software)
	//   - clean (clean the build enviroment)
	//   - distclean (clean ./configure creations)
	//   - check (test suite)
	//   - intallcheck (Check existance of instalation of the software)
	//   - dist (make a software download package)
	//   - help
	// - Init file creation group rights option.
	//
	// Last one is just option but how to add templates and certain targets and variables?
	// - Make so that by default default template from init file is used.
	// - This can be overwritten by telling targets and variables manually.
	// - Other template can be pointed at? @include in the format enough?
	//
	// What is template file format? What format did autotools use?
	// - INI file with makefile with snprintf_s formatting and variable answers?
	// - Makefile.in uses @variable@ it seems. Although @recipe is interperted by make as echo silencer so other symbol should be used.
	// - Init could ask variables it does not know.
	// Best option is INI config file with makefile and configure
	// which uses ‰variables‰. ‰ is used since it is one byte in UTF-8 and makefile does not use it (@ is used for command silencer).
	// Variables has to be decleared in INI to define behavior (like default value or
	// can be skipped and behavior for the skip).
	// How to read with one swoop?
	//   Loop config_read_string untill buffer hits makefile group?
	//     - Is everything before parsing error written in to the config file? Could abuse.
	//   Make push request to library to add parsing callback?
	//     - Code is genereated via yyparse so would have to mess with that.
	//   Read untill you find makefile group?
	//     - Double reading the file ...
	//    Make makefile template proper string variable in the configs format...
	//     - Makefile template is double read but if statement possible makes
	//       it so that buffer copy has to be done no matter what.
	// Lets go with proper string option as it is the best for the system.

	// Open template file.
	if(templatename && templatename[0]!='\0'){

		// Makefile template search locations.
		// TODO: Template path macro for makefile/configuration editing
		#if DEBUG
			char *templatesearchloc[]={"Etc/templates/"};
			int32_t templatesearchloclen=1;
			int32_t longesttemplatesearchloc=strlen(templatesearchloc[0]);
		#else
			//TODO: Add template directory to home directory's .config?
			char *templatesearchloc[]={PROG_LOC"templates/"};
			int32_t templatesearchloclen=1;
			int32_t longesttemplatesearchloc=0;
			// Get the max. Hopefully optimized out.
			for(int32_t i=templatesearchloclen-1;i>=0;i--){
				int32_t temp=strlen(templatesearchloc[i]);
				if(longesttemplatesearchloc<temp) templatesearchloclen=temp;
			}
		#endif

		// Search the template name.
		// Allocate path for the template. Only one allocation needed as
		// longesttemplatesearchloc is quarenteed to be longest string in
		// templatesearchloc.
		char *templatepath=malloc(longesttemplatesearchloc+strlen(templatename)+1);
		for(int32_t i=templatesearchloclen-1;i>=0;i--){
			// Concatenate two string to make the potential path.
			memcpy(templatepath,templatesearchloc[i],strlen(templatesearchloc[i]));
			memcpy(templatepath+strlen(templatesearchloc[i]),templatename,strlen(templatename));
			*(templatepath+strlen(templatesearchloc[i])+strlen(templatename))='\0';

			// Check access.
			if(access(templatepath,R_OK)==0) goto jmp_FILE_FOUND;
		}
		// Abort no file was found.
		printErr("Makefile template not found!");
		free(templatepath);
		return;

		jmp_FILE_FOUND:
		#if DEBUG
			printStrCat(STDOUT_FILENO,templatepath,"\n",strlen(templatepath),1);
		#endif

		// Create config strcutre for template file.
		config_t makefiletemplate;
		config_init(&makefiletemplate);

		// Read the configuration file.
		if(config_read_file(&makefiletemplate,templatepath)==CONFIG_TRUE){

			// Read the makefile variable.
			const char *template;
			config_lookup_string(&makefiletemplate,"makefile",&template);

			#if DEBUG
				print(STDOUT_FILENO,template);
			#endif

		}
		else{
			switch(config_error_type(&makefiletemplate)){
				case CONFIG_ERR_FILE_IO:
					printErr("libconfig could not IO with template file. ABORT!");
					break;
				case CONFIG_ERR_PARSE:;
					printErr("libconfig could not parse template file. ABORT!");
					//TODO: Error line printing.
					//      Make function for this since parse error can happen else where.
					printParseErr(&makefiletemplate);
					break;
				default:
					// This should not happen.
					printWarm("libconfig whut with template file? ABORT!");
			}
		}

		// No need to keep this structure a round after this function.
		config_destroy(&makefiletemplate);
		free(templatepath);
	}
}
/**********************************************
* Main handles CLI mainly.                    *
**********************************************/
#define MAIN_RETURN_SUCCESS 0
#define MAIN_RETURN_UNKNOWN_OPTION 1
#define MAIN_RETURN_OPTION_WITHOUT_ARGUMENT 2
#define MAIN_RETURN_ARGUMENT_ON_NON_ARGUMENT_OPTION 3
#define MAIN_RETURN_UNKNOWN_COMMAND 4
int main(int argc, char **argv){
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
	"  -c, --configuration\tFor init make configuration script (configuration script name can be given in as second parameter).\n"
	"  -t, --template\tWhat template should be used during init command.\n"
	"  --no-config\tDo not use template during init command.\n";

	if(argc>1){

		// Program flags for later.
		// Union is done so that flags can be feeded to ophand.
		union{
			struct{
				// Is configuration file included.
				bool configurationincluded:1;
				union{
					// Is template path mallocated.
					bool freetemplatepath:1;
				};
			};
			int32_t shadow;
		}progflags;
		progflags.shadow=0;
		// Variable parameter given by user.
		// Different for every command.
		union{
			struct{
				char *templatepath;
			};
		}uservars={.templatepath=NULL};

		// INI configuration location for the program.
		// Default value depends on DEBUG mode so development is seperate.
		// TODO: config file name macro for makefile/configuration editing?
		char *cfgpath;
		#if DEBUG
			cfgpath="Etc/config.cfg";
		#else

			cfgpath=PROG_LOC "/config.cfg";
		#endif


		// Call opHand to handle options. NOTE: argc-1 and argv+1 jumps over program name.
		const Option globaloptions[]={
			{"help",.variable.printstr=usage,.value.v32=sizeof(usage),'h',{false,true,OPHAND_CMD_PRINT}},
			{"version",.variable.p32=&progflags.shadow,.value.v32=1,'v',{false,true,OPHAND_CMD_OR}},
			{"config",.variable.str=&cfgpath,.value.str=NULL,'\0',{true,false,OPHAND_CMD_POINTER_VALUE}},
			{"no-config",.variable.str=&cfgpath,.value.str=NULL,'\0',{false,false,OPHAND_CMD_POINTER_VALUE}},
		};
		const Option initoptions[]={
			{"configuration",.variable.p32=&progflags.shadow,.value.v32=0,'c',{false,false,OPHAND_CMD_VALUE}},
			{"template",.variable.str=&uservars.templatepath,.value.str=NULL,'t',{true,false,OPHAND_CMD_POINTER_VALUE}},
			{"no-template",.variable.str=&uservars.templatepath,.value.str='\0','\0',{false,false,OPHAND_CMD_POINTER_VALUE}}
		};
		const OpHandCommand commands[]={
			{"init",initoptions,sizeof(initoptions)/sizeof(Option)}
		};

		// Id command that was called and pointer to start of non-option arguments.
		int32_t cmdid;
		char **nonparameters;

		switch(opHandCommand(argc-1,argv+1,globaloptions,sizeof(globaloptions)/sizeof(Option),commands,sizeof(commands)/sizeof(OpHandCommand),&cmdid,&nonparameters)){
			case OPHAND_PROCESSING_STOPPED: return MAIN_RETURN_SUCCESS;
			case OPHAND_UNKNOW_OPTION: printErr("Unknown option!"); return MAIN_RETURN_UNKNOWN_OPTION;
			case OPHAND_NO_ARGUMENT: printErr("Option without a argument!"); return MAIN_RETURN_OPTION_WITHOUT_ARGUMENT;
			case OPHAND_ARGUMENT_ON_NONE_ARGUMENT: printErr("Argument on non-argument option!"); return MAIN_RETURN_ARGUMENT_ON_NON_ARGUMENT_OPTION;
			case OPHAND_UNKNOWN_ARG_CMD: printErr("Unknown command!"); return MAIN_RETURN_UNKNOWN_COMMAND;
			case OPHAND_NO_ARG_CMD: goto jmp_PRINT_USAGE;
			case OPHAND_PROCESSING_DONE:;
		}

		// Load configuration file.
		//
		// How to implement? Options:
		//  1) Grap the info needed and close the configuration?
		//  2) Leave config structure and query when needed?
		//
		// Libconfig does reads everything at ones which means it
		// could be used as storage medium. Problem is that
		// some command line arguments can overwritten these.
		// Hence it is best to offload to variables now based upon if
		// commandline didn't do anyhing.
		if(cfgpath){
			config_t globalconfig;
			config_init(&globalconfig);
			if(config_read_file(&globalconfig,cfgpath)==CONFIG_FALSE){
				// Program must work even if configuration does not exist
				// fails for bizarre reasons.
				switch(config_error_type(&globalconfig)){
					case CONFIG_ERR_FILE_IO:
						printWarm("libconfig coudn't read fmakegen config!");
						break;
					case CONFIG_ERR_PARSE:
						printWarm("Libconfig parse error!");
						printParseErr(&globalconfig);
						break;
					default:
						// This should not happen.
						printWarm("libconfig whut?");
				}
			}

			// Initialize commands parameters.
			switch(cmdid)
				case 0:
					if(!uservars.templatepath){
						// Strings variables of configuration
						// are manages by the library so copy has to be done
						// before config_destroy.
						// NOTE: malloc is freed at below during the actual command running.
						const char *temp;
						if(config_lookup_string(&globalconfig,"default_makefile_template",&temp)==CONFIG_TRUE){
							uservars.templatepath=malloc((strlen(temp)+1)*sizeof(char));
							strcpy(uservars.templatepath,temp);
							progflags.freetemplatepath=true;
						}
			}

			// Free the global config structure as is no longer
			// needed.
			config_destroy(&globalconfig);
		}

		// Run the actual command.
		switch(cmdid){
			case 0:
				// Init creates makefile from scratch.
				// Makefile isn't created if file exists and has content.

				int makefilefd;
				OpeningReturn makefilereturn=createFile(&makefilefd,nonparameters[0],"Makefile");
				if(makefilereturn==OPEN_SUCCESS){
					// Is file empty?
					// Two stat system calls needed since
					// path is not always to target file.
					struct stat statbuff;
					if(fstat(makefilereturn,&statbuff)==0){
						if(statbuff.st_size==0){
							// Init makefile.
							initMakefile(makefilefd,uservars.templatepath);
						}
						else printErr("Init was not run on makefile as it was not an empty file!");
					}
					else printErr("fstat gave an error!");
					close(makefilefd);
				}
				else if(makefilereturn==OPEN_CREATED){
					// Init makefile.
					initMakefile(makefilefd,uservars.templatepath);
					close(makefilefd);
				}
				else printErr("Error to create the makefile!");

				// Free things allocated configuration handling for init.
				// NOTE: some of these may have not malloced in first
				//       place hence check is made.
				if(progflags.freetemplatepath) free(uservars.templatepath);
				break;
		}
	}
	else{
		jmp_PRINT_USAGE:
		printconst(STDOUT_FILENO,usage);
	}

	return MAIN_RETURN_SUCCESS;
}
