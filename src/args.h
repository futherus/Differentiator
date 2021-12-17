#ifndef ARGS_H
#define ARGS_H

#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>

enum args_msg
{
    ARGS_NOMSG           = 0,  /// no message
    ARGS_HLP             = 1,  /// reference
    ARGS_HLP_NOTE        = 2,  /// reference with note
    ARGS_NO_OPT          = 3,  /// error: no options
    ARGS_OPT_OVRWRT      = 4,  /// options overwrite
    ARGS_NO_IFL_NAME     = 5,  /// no input file name
    ARGS_NO_DIFF_VAR     = 7,  /// no diff variable
    ARGS_FLNAME_OVRFLW   = 8,  /// filename overflow
    ARGS_BAD_CMD         = 9,  /// bad input for command
    ARGS_UNEXPCTD_ERR    = 10, ///     
};

const char HELP[]              = "-h, --help             reference\n"
                                 "--src <filename>       input file\n"
                                 "--diff <variable>      diff variable\n";

const char NOTE[]              = "(program ignores other options if -h entered)\n";
const char NO_OPTIONS[]        = "Error: enter options (-h to open reference)\n";
const char OPTION_OVERWRITE[]  = "Error: option overwrite\n";
const char NO_IFILE_NAME[]     = "Error: enter source file name\n";
const char FILENAME_OVERFLOW[] = "Error: filename is too long\n";
const char BAD_COMMAND[]       = "Error: bad command\n";
const char NO_DIFF_VAR[]       = "Error: diff variable is not specified\n";
const char UNEXPECTED_ERROR[]  = "UNEXPECTED ERROR\n";

/** \brief Prints message corresponding to param

    \param [out] ostream FILE* pointer to output file
    \param [in]  param   args_msg parameter, sets message
*/
void response_args(args_msg param, FILE* const ostream = stderr);

/** \brief Processes command line arguments and sets according parameters

    \param [in]  argc         Number of arguments
    \param [in]  argv         Array of arguments
    \param [out] infile_name  Name of input file
    \param [out] outfile_name Name of output file

    \return ARGS_NOMSG, args_msg with error otherwise
*/
args_msg process_args(int argc, char* argv[], char infile_name[] = nullptr, char diff_variable[] = nullptr);

#endif // ARGS_H
