/********************************************************************
* This module is general function for option handling.              *
********************************************************************/
#ifndef _OP_HAND_H_
#define _OP_HAND_H_

#include<stdint.h>
#include<stdbool.h>

/*********************************************************
* Macro for checking does given option flag ask for      *
* argument.                                              *
*********************************************************/
#define HAS_ARGUMENT(A) A.argument

/*********************************************************
* Return value of the ophand.                            *
* Designed so that zero or one is returned on SUCCESS.   *
* OPHAND_PROCESSING_STOPPED differs from                 *
* OPHAND_PROCESSING_DONE by indicating stop bit was      *
* used to stop the processing.                           *
*********************************************************/
typedef enum OpHandReturn{
	OPHAND_PROCESSING_DONE=0,
	OPHAND_PROCESSING_STOPPED,
	OPHAND_UNKNOW_OPTION,
	OPHAND_NO_ARGUMENT,
	OPHAND_ARGUMENT_ON_NONE_ARGUMENT,
	OPHAND_NO_ARG_CMD,
	OPHAND_UNKNOWN_ARG_CMD,
}OpHandReturn;
/*********************************************************
* Macros for the option flags.                           *
*                                                        *
* Members:                                               *
*   argument bit tells does option have argument.        *
*   stop     bit tells should parsing to be stoped after *
*            this option.                                *
*   type     bits tell what operation is performed.      *
*            - OPHAND_VALUE set 32 bit integer to        *
*              constant given or to argument.            *
*            - OPHAND_POINTER_VALUE sets a pointer to    *
*              constant given or points to argument as a *
*              string.                                   *
*            - OPHAND_FUNCTION calls given function to   *
*              handle the option.                        *
*            - OPHAND_OR performs OR operation to 32 bit *
*              variable.                                 *
*            - OPHAND_AND performs AND operation to 32   *
*              bit variable.                             *
*            - OPHAND_PRINT prints constant message.     *
*********************************************************/
typedef struct OptionFlag{
	bool argument : 1;
	bool stop : 1;
	enum{
		OPHAND_CMD_PRINT,
		OPHAND_CMD_VALUE,
		OPHAND_CMD_POINTER_VALUE,
		OPHAND_CMD_FUNCTION,
		OPHAND_CMD_OR,
		OPHAND_CMD_AND,
	}type : 6;
}OptionFlag;
/*********************************************************
* Type for the function call if argument is hit.         *
* Programmer should send true if OptFunction doesn't     *
* cause error and false if error happened so that opHand *
* can stop.                                              *
*********************************************************/
typedef bool (*OptFunction)(char option,void *restrict coderdata,const char *restrict arg);
/*********************************************************
* Structure declaring option for ophand function.        *
*********************************************************/
typedef struct Option{
	char *longoption;
	union{
		int32_t *p32;
		char **str;
		OptFunction func;
		const char *printstr;
	}variable;
	union{
		int32_t *p32;
		void *coderdata;
		int32_t v32;
		char *str;
	}value;
	char option;
	OptionFlag flags;
}Option;
/*********************************************************
* Ophand command for parsing CLI with commands. Command  *
* is first non-option argument which decides what        *
* operation is done. This works somewhat like git.       *
* Commands change what options are valid.                *
*********************************************************/
typedef struct OpHandCommand{
	char *cmdstr;
	const Option *options;
	uint32_t optionslen;
}OpHandCommand;

/*********************************************************
* Function performs the option handling. Returns 1 if    *
* everything went fine. Zero if error occured.           *
* Non-options arguments are put to args (will override!) *
* and null ending tells the end. If "--" is encountered  *
* then opHands execution returns to caller.              *
*                                                        *
* NOTE I: optionslen isn't' sanity checked so better     *
* put right size in.                                     *
* NOTE II: args or options aren't null check so segment  *
* faults are on you!                                     *
* NOTE III: if args element is null pointer it is just   *
* ignored.                                               *
*********************************************************/
OpHandReturn opHand(int argn,char *restrict *restrict args,const Option *restrict options,uint32_t optionslen);
/*********************************************************
* Function performs option handling where first          *
* non-option is a command with it's own sub options.     *
*                                                        *
* Return value is same as opHand but OPHAND_NO_ARG_CMD,  *
* and OPHAND_UNKNOWN_ARG_CMD are added for cases where   *
* non-option command is not found or is unknown, and     *
* id of non-option command and other non-parameter are   *
* retuned by cmdid and nonoptionargs.                    *
*********************************************************/
OpHandReturn opHandCommand(int argn,char *restrict *args,const Option *restrict globaloptions,uint32_t globaloptionslen,const OpHandCommand *restrict cmds,const int32_t cmdssize,int32_t *cmdid,char ***nonoptionargs);

#endif /* _OP_HAND_H_ */
