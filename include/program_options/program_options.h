// Copyright (2018), Delta
//
// @author: Bowen Tan, email: notebowentan@gmail.com
//
// Created on May. 3, 2018
// Lastest revised on May. 3, 2018
//
// This file is the header file of program option library.

#ifndef PROGRAM_OPTIONS_PROGRAM_OPTIONS_H_
#define PROGRAM_OPTIONS_PROGRAM_OPTIONS_H_

// Argument value type
#define VALUE_INT 1
#define VALUE_FLOAT 2
#define VALUE_STRING 3
#define VALUE_BOOL 4

// Option/Argument type
#define OPTION_UNNECESSARY 0
#define OPTION_NECESSARY (1 << 4)  // Option MUST be given
#define OPTION_HAS_ARG (1 << 3)  // Option CAN have argument but NOT necessary
#define OPTION_REQUIRE_ARG (1 << 2)  //Option MUST have argument
#define OPTION_MULTIPLE_ARG (1 << 1)  // Opiont CAN have multiple arguments
#define OPTION_HAS_DEFAULT 1

#define OPTION_ALONG 0  // Program CAN execute, given only option
#define OPTION_NOT_ALONG 1  // Program CANNOT execute, given only option

typedef struct Option {
    char short_name;
    char const* long_name;
    char const* desc;

    int along_type;
    int opt_type;
    int value_type;
    
    char const* depend_options;

    void* var_holder;
    int* var_length_holder;

    int given;
} option_t;

typedef struct Program {
    char const* name;
    char const* desc;

    int option_num;
    option_t** options;  // Array of options

    int subprog_num;
    struct Program** subprogs;
    struct Program* parent_prog;
    // char const** subprog_names;

    void (*main_func)(struct Program* prog, int argc, char const** argv);
} prog_t;

// Option and program operations
void destroy_option(option_t* option);
void destroy_prog(prog_t* prog);

// Create program with/without subprograms
prog_t* init_prog(char const* name, char const* desc);
void add_subprogs(prog_t* prog, int subprog_num, ...);
// Add options to prog
// Param:
//   prog       : program holding options
//   option_num : total option number
//   ...        : option specifications
//
// For option specifications, each option requires the following arguments
// short_name        : char
// full_name         : string
// description       : string
// along_type        : int, OPTION_ALONG or OPTION_NOT_ALONG
// option_type       : int, combinations of option types above, with '+' sign
// depend_options    : string, ':' separated short names of other options
// var_length_holder : int*, the length of argument values, for array of arguments
// var_holder        : void*, pointer to the variable
// has_default       : int
void add_options(prog_t* prog, int option_num, ...);

/* Parsing */
static int _arg_idx;
static char _opt;
static int _arg_num;
static char** _arg;
static option_t* _current_opt;
static void _init_opt_parse(void);
static void _clear_arg(void);
static void _check_opt_type(void);
static void _check_value_type(void);
static void _assign_arg(void);
static int _find_option(prog_t* prog);
static int _find_subprog(prog_t* prog, char const* subprog_names);
static int _get_option(prog_t* prog, int argc, char const** argv);
static int _is_along_option_given(prog_t* prog);
static void _check_option_necessity(prog_t* prog);
static void _check_option_dependency(prog_t* prog);
void parse_args(prog_t* prog, int argc, char const** argv);

void print_help(prog_t* prog);

#endif  // PROGRAM_OPTIONS_PROGRAM_OPTIONS_H_
