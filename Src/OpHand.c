/********************************************************************
* This module is general function for option handling.              *
********************************************************************/
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<ctype.h>
#include"OpHand.h"

/******************************
* Special macro handle a      *
* situation where two dashes  *
* are given.                  *
******************************/
#define OPHAND_INTERNAL_DONE 5

	/*********************************
	* Handles non-argument options.  *
	*********************************/
	static bool switchNonArgument(const Option *option){
		switch(option->flags.type){
			case OPHAND_CMD_VALUE:
				*option->variable.p32=option->value.v32;
				break;
			case OPHAND_CMD_OR:
				*option->variable.p32|=option->value.v32;
				break;
			case OPHAND_CMD_AND:
				*option->variable.p32&=option->value.v32;
				break;
			case OPHAND_CMD_POINTER_VALUE:
				*option->variable.str=option->value.str;
				break;
			case OPHAND_CMD_PRINT:
				(void)write(STDOUT_FILENO,(char*)option->variable.str,option->value.v32);
				break;
			case OPHAND_CMD_FUNCTION:
				// Return whatever callback returns
				return option->variable.func(option->option,option->value.coderdata,0);
		}
		return !option->flags.stop;
	}
	/*********************************
	* Handles argument options.      *
	*********************************/
	static bool switchArgument(char *arg,const Option *option){
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wswitch"
		switch(option->flags.type){
			case OPHAND_CMD_VALUE:
				*option->variable.p32=atoi(arg);
				break;
			case OPHAND_CMD_POINTER_VALUE:
				*option->variable.str=arg;
				break;
			case OPHAND_CMD_FUNCTION:
				// Return whatever callback returns
				return option->variable.func(option->option,option->value.coderdata,arg);
		}
		#pragma GCC diagnostic pop
		return !option->flags.stop;
	}
	/*********************************
	* Subruotine to handle option    *
	* argument. DOES NOT CHECK WHAT  *
	* IS GIVEN IS A OPTION!          *
	*********************************/
	static OpHandReturn opHandOptionInteral(int argn,char *restrict *args,const Option *restrict options,uint32_t optionslen,int arg){
		// This stores what option is hit after long or short option
		// is hit.
		uint32_t foundoption;
		if(args[arg][1]=='-'){
			if(args[arg][2]!='\0'){
				// GNU standard allows '=' sign to be used to indicate that after
				// '=' is the argument. As this effects string comparasion
				// every long option has to be checked for '='.
				char *equalsignargument=NULL;
				for(char *c=args[arg]+3;*c!='\0';c++){
					if(*c=='='){
						*c='\0';
						equalsignargument=c+1;
						break;
					}
				}
				// Try to find the option
				for(foundoption=0;foundoption<optionslen;foundoption++){
					if(options[foundoption].longoption && strcmp(options[foundoption].longoption,args[arg]+2)==0){

						// If argument is needed we know that it is
						// next string in args array.
						if(HAS_ARGUMENT(options[foundoption].flags)){
							// Check that next argument exist.
							if(equalsignargument){
								bool result=switchArgument(equalsignargument,options+foundoption);
								if(result) return OPHAND_PROCESSING_DONE;
								else return OPHAND_PROCESSING_STOPPED;
							}
							else if(++arg<argn){
								bool result=switchArgument(args[arg],options+foundoption);
								if(result) return OPHAND_PROCESSING_DONE;
								else return OPHAND_PROCESSING_STOPPED;
							}
							return OPHAND_NO_ARGUMENT;
						}
						else{
							// If equal sign argument was given then error should be reported on
							// if option did not have an argument requirement.
							if(equalsignargument) return OPHAND_ARGUMENT_ON_NONE_ARGUMENT;
							// Call the handling of the non-argument option.
							bool result=switchNonArgument(options+foundoption);
							if(result) return OPHAND_PROCESSING_DONE;
							else return OPHAND_PROCESSING_STOPPED;
						}
					}
				}
				return OPHAND_UNKNOW_OPTION;
			}
			else{
				// Since two lines where given execution ends here.
				return OPHAND_INTERNAL_DONE;
			}
		}
		else{
			if(args[arg][1]!='\0'){
				for(uint32_t foundoption=0;foundoption<optionslen;foundoption++){
					if(args[arg][1]==options[foundoption].option){
						// If argument is needed we have to check
						// is argument start second offset or is it next argument.

						if(HAS_ARGUMENT(options[foundoption].flags)){
							if(args[arg][2]!='\0'){
									bool result=switchArgument(args[arg]+2,options+foundoption);
									if(result) return OPHAND_PROCESSING_DONE;
									else return OPHAND_PROCESSING_STOPPED;
							}
							else{
								// Check that next argument exist.
								if(++arg<argn){
									bool result=switchArgument(args[arg],options+foundoption);
									if(result) return OPHAND_PROCESSING_DONE;
									else return OPHAND_PROCESSING_STOPPED;
								}
							}
							return OPHAND_NO_ARGUMENT;
						}
						else{
							if(args[arg][2]=='\0'){
								bool result=switchNonArgument(options+foundoption);
								if(result) return OPHAND_PROCESSING_DONE;
								else return OPHAND_PROCESSING_STOPPED;
							}
							else return OPHAND_UNKNOW_OPTION;
						}
					}
				}
			}
			return OPHAND_UNKNOW_OPTION;
		}
	}
	/***************
	* See OpHand.h *
	***************/
	OpHandReturn opHand(int argn,char *restrict *args,const Option *restrict options,uint32_t optionslen){

		// Points to next location where loop would
		// put non-option arguments. Used after all
		// arguments are processed to mark end of
		// non-option arguments.
		uint32_t nonoptpoint=0;

		// Simple start going through arguments and compare to options.
		// Remember that first argument is the location of the execution.
		for(int arg=0;arg<argn;arg++){

			// Check that argument wasn't null pointer since POSIX standard does
			// not define check for it at exec* function family.
			// Behavior for this is continue processing.
			if(args[arg]==0) continue;

			// Actual processing
			if(args[arg][0]=='-'){
				OpHandReturn returnval=opHandOptionInteral(argn,args,options,optionslen,arg);
				if(returnval!=OPHAND_PROCESSING_DONE) return returnval;
			}
			else args[nonoptpoint++]=args[arg];
		}

		// Mark the ending as description wanted!
		*(args+nonoptpoint)=0;

		return OPHAND_PROCESSING_DONE;
	}
	/***************
	* See OpHand.h *
	***************/
	OpHandReturn opHandCommand(int argn,char *restrict *args,const Option *restrict globaloptions,uint32_t globaloptionslen,const OpHandCommand *restrict cmds,const int32_t cmdssize,int32_t *cmdid,char *** nonoptionargs){
		// Global options must be before the command.
		int arg=0;
		while(arg<argn){
			// Handle the argument
			if(args[arg][0]=='-'){
				OpHandReturn returnval=opHandOptionInteral(argn,args,globaloptions,globaloptionslen,arg);
				if(returnval!=OPHAND_PROCESSING_DONE) return returnval;
			}
			else break;
		}
		// Command non-option or end of command line arguments is been hit!
		// Handle case where command was not given.
		if(arg<argn){
			// lowercase the command.
			// Handle command given.
			// First turn it lower case.
			for(char *c=args[arg];*c;c++) *c=tolower(*c);

			// What command was called.
			int32_t i;
			for(i=cmdssize-1;i>=0;i--){
				// Compare the string.
				// NOTE: that strncmp is not needed here since it is
				//       guarenteed that args are null ending and
				//       cmdstr is null ending.
				if(strcmp(cmds[i].cmdstr,args[arg])==0){
					*cmdid=i;

					// Call regular ophand for commands options.
					OpHandReturn returnval=opHand(argn-arg-1,args+arg+1,cmds[i].options,cmds[i].optionslen);
					// Return non-option arguments by parameter since sorting args is waste of time for this function.
					// Disable discarded-qualifiers because GCC doesn't understand restrict isn't ignored here.
					#pragma GCC diagnostic push
					#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
						*nonoptionargs=args+arg+1;
					#pragma GCC diagnostic pop
					return returnval;
				}
			}
		}
		return OPHAND_NO_ARG_CMD;
	}
