// Copyright (2018), Delta
//
// @author: Bowen Tan, email: notebowentan@gmail.com
//
// Created on May. 3, 2018
// Lastest revised on May. 3, 2018
//
// This file is the implementation of program option library.

// Self include
#include "program_options/program_options.h"

// C libraries
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// Project's other libraries
#include "program_options/log.h"

prog_t* init_prog(char const* name, char const* desc) {
    prog_t* prog = malloc(sizeof(prog_t));
    prog->name = name;
    prog->desc = desc;
    prog->options = NULL;
    prog->subprog_num = 0;
    prog->subprogs = NULL;
    prog->parent_prog = NULL;
    prog->main_func = NULL;
    return prog;
}

void add_options(prog_t* prog, int option_num, ...) {
    va_list valist;
    va_start(valist, option_num);
    
    prog->option_num = option_num;
    prog->options = malloc(option_num * sizeof(option_t*));
    
    for (int i = 0; i < option_num; i++) {
        prog->options[i] = malloc(sizeof(option_t));
        prog->options[i]->short_name = (char)va_arg(valist, int);
        prog->options[i]->long_name = va_arg(valist, char const*);
        prog->options[i]->desc = va_arg(valist, char const*);
        prog->options[i]->along_type = va_arg(valist, int);
        prog->options[i]->opt_type = va_arg(valist, int);
        prog->options[i]->value_type = va_arg(valist, int);
        prog->options[i]->depend_options = va_arg(valist, char const*);
        prog->options[i]->var_length_holder = va_arg(valist, int*);
        prog->options[i]->var_holder = va_arg(valist, void*);
        prog->options[i]->given = 0;
    }
}

void add_subprogs(prog_t* prog, int subprog_num, ...) {
    va_list valist;
    va_start(valist, subprog_num);

    prog->subprogs = malloc(prog->subprog_num * sizeof(prog_t*));
    for (int i = 0; i < subprog_num; i++) {
        char const* subprog_name = va_arg(valist, char const*);
        char const* subprog_desc = va_arg(valist, char const*);
        prog_t* subprog = init_prog(subprog_name, subprog_desc);
        prog->subprog_num++;
        prog->subprogs = realloc(prog->subprogs, prog->subprog_num * sizeof(prog_t*));
        prog->subprogs[prog->subprog_num - 1] = subprog;

        subprog->parent_prog = prog;
        subprog->main_func = va_arg(valist, void (*)(prog_t*, int, char const**));
    }
}

static void _init_opt_parse(void) {
    _arg_idx = 1;
    _opt = '-';
    _clear_arg();
    _current_opt = NULL;
}

static void _clear_arg(void) {
    for (int i = 0; i < _arg_num; i++) {
        free(_arg[i]);
        _arg[i] = NULL;
    }
    free(_arg);
    _arg_num = 0;
    _arg = NULL;
}

static void _check_opt_type(void) {
    if ((_current_opt->opt_type & OPTION_HAS_ARG) != 0) {
        if ((_current_opt->opt_type & OPTION_REQUIRE_ARG) != 0) {
            if (_arg_num == 0) {
                // Require argument but not given
                fprintf(stderr, "%sOption '-%c', '%s' requires argument, not given.\n",
                        LOG_OPTION_ERROR, _current_opt->short_name, _current_opt->long_name);
                exit(1);
            }
        }
        if ((_current_opt->opt_type & OPTION_MULTIPLE_ARG) == 0) {
            if (_arg_num > 1) {
                fprintf(stderr, "%sOption '-%c', '%s' accepts at most one argument, multiple given.\n",
                        LOG_OPTION_ERROR, _current_opt->short_name, _current_opt->long_name);
                exit(1);
            }
        }
    } else {
        if (_arg_num > 0) {
            fprintf(stderr, "%sOption '-%c', '%s' accepts no argument but given.\n",
                    LOG_OPTION_ERROR, _current_opt->short_name, _current_opt->long_name);
            exit(1);
        }
    }
}

static void _check_value_type(void) {
    if (_current_opt->value_type != VALUE_STRING) {
        for (int i = 0; i < _arg_num; i++) {
            int dot_count = 0;
            for (int j = 0; j < strlen(_arg[i]); j++) {
                if (_arg[i][j] == '.') {
                    dot_count++;
                } else {
                    if ((_arg[i][j] < '0' || _arg[i][j] > '9')) {
                        fprintf(stderr, "%sArgument of option '-%c', '%s' contains invalid number" 
                                        "characters.\n",
                                LOG_OPTION_ERROR, _current_opt->short_name, _current_opt->long_name);
                        exit(1);
                    }
                }
            }
            if (dot_count > 1) {
                fprintf(stderr, "%sInvalid floating number argument: %s.\n",
                        LOG_OPTION_ERROR, _arg[i]);
                exit(1);
            }
        }
    }
}

static void _assign_arg(void) {
    _current_opt->given = 1;
    switch (_current_opt->value_type) {
        case VALUE_INT:
            if (_arg_num > 0) {
                if ((_current_opt->opt_type & OPTION_MULTIPLE_ARG) == 0) {
                    *((int*)(_current_opt->var_holder)) = atoi(_arg[0]);
                } else {
                    *(int**)(_current_opt->var_holder) = malloc(_arg_num * sizeof(int));
                    for (int i = 0; i < _arg_num; i++) {
                        (*(int**)(_current_opt->var_holder))[i] = atoi(_arg[i]);
                    }
                    *(_current_opt->var_length_holder) = _arg_num;
                }
            }
            break;
        case VALUE_FLOAT:
            if (_arg_num > 0) {
                if ((_current_opt->opt_type & OPTION_MULTIPLE_ARG) == 0) {
                    *((double*)(_current_opt->var_holder)) = atof(_arg[0]);
                } else {
                    *(double**)(_current_opt->var_holder) = malloc(_arg_num * sizeof(double));
                    for (int i = 0; i < _arg_num; i++) {
                        (*(double**)(_current_opt->var_holder))[i] = atof(_arg[i]);
                    }
                    *(_current_opt->var_length_holder) = _arg_num;
                }
            }
            break;
        case VALUE_STRING:
            if (_arg_num > 0) {
                if ((_current_opt->opt_type & OPTION_MULTIPLE_ARG) == 0) {
                    *((char**)(_current_opt->var_holder)) = malloc((strlen(_arg[0]) + 1) * sizeof(char));
                    strcpy(*((char**)(_current_opt->var_holder)), _arg[0]);
                } else {
                    *(char***)(_current_opt->var_holder) = malloc(_arg_num * sizeof(char*));
                    for (int i = 0; i < _arg_num; i++) {
                        (*(char***)(_current_opt->var_holder))[i] = malloc((strlen(_arg[i]) + 1) * sizeof(char));
                        strcpy((*(char***)(_current_opt->var_holder))[i], _arg[i]);
                    }
                    *(_current_opt->var_length_holder) = _arg_num;
                }
            }
            break;
        default:
            fprintf(stderr, "Invalid\n");
            exit(1);
    }
}

static int _find_option(prog_t* prog) {
    for (int i = 0; i < prog->option_num; i++) {
        if (prog->options[i]->short_name == _opt) {
            return i;
        }
    }
    return -1;  // Not found
}

static int _find_subprog(prog_t* prog, char const* subprog_name) {
    for (int i = 0; i < prog->subprog_num; i++) {
        if (strcmp(subprog_name, prog->subprogs[i]->name) == 0) {
            return i;
        }
    }
    return -1;
}

static int _get_option(prog_t* prog, int argc, char const** argv) {
    if (_arg_idx >= argc) {
        return -1;  // No argument left
    }
    
    _opt = argv[_arg_idx][1];  // Short name of current option
    int opt_idx = _find_option(prog);
    if (opt_idx < 0) {
        fprintf(stderr, "%sInvalid option '-%c'.\n", LOG_OPTION_ERROR, _opt);
        exit(1);
    } else {
        _clear_arg();
        _arg = malloc(_arg_num * sizeof(char*));
        for (int i = _arg_idx + 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                break;
            }
            _arg_num++;
            _arg = realloc(_arg, _arg_num * sizeof(char*));
            _arg[_arg_num - 1] = malloc((strlen(argv[i]) + 1) * sizeof(char));
            strcpy(_arg[_arg_num - 1], argv[i]);
        }
        _current_opt = prog->options[opt_idx];
        _arg_idx += _arg_num + 1;
    }
    return 0;
}

static int _is_along_option_given(prog_t* prog) {
    for (int i = 0; i < prog->option_num; i++) {
        if (prog->options[i]->along_type == OPTION_ALONG) {
            if (prog->options[i]->given == 1) {
                return 1;
            }
        }
    }
    return 0;
}

static void _check_option_necessity(prog_t* prog) {
    int not_given_count = 0;
    for (int i = 0; i < prog->option_num; i++) {
        if ((prog->options[i]->opt_type & OPTION_NECESSARY) != 0) {
            if (prog->options[i]->given == 0) {
                not_given_count++;
                fprintf(stderr, "%sOption '-%c', '%s' (and its arguments) must be given.\n",
                        LOG_OPTION_ERROR, prog->options[i]->short_name, prog->options[i]->long_name);
            }
        }
    }
    if (not_given_count > 0) {
        exit(1);
    }
}

static void _check_option_dependency(prog_t* prog) {
    int dependency_incomplete_count = 0;
    for (int i = 0; i < prog->option_num; i++) {
        _current_opt = prog->options[i];
        if (_current_opt->depend_options == NULL) continue;
        int not_given_count = 0;
        int* not_given_idx = malloc(0 * sizeof(int));
        for (int j = 0; j < strlen(prog->options[i]->depend_options); j += 2) {
            _opt = _current_opt->depend_options[j];
            int opt_idx = _find_option(prog);
            if (prog->options[opt_idx]->given == 0) {
                not_given_count++;
                not_given_idx = realloc(not_given_idx, not_given_count * sizeof(int));
                not_given_idx[not_given_count - 1] = opt_idx;
            }
        }
        if (not_given_count > 0) {
            dependency_incomplete_count++;
            fprintf(stderr, "%sOption '-%c', '%s' needs option(s) ", 
                    LOG_OPTION_ERROR, _current_opt->short_name, _current_opt->long_name);
            for (int j = 0; j < not_given_count; j++) {
                fprintf(stderr, "'-%c', '%s', ", prog->options[not_given_idx[j]]->short_name,
                        prog->options[not_given_idx[j]]->long_name);
            }
            fprintf(stderr, "\b\b.\n");
        }
        free(not_given_idx);
    }
    if (dependency_incomplete_count > 0) {
        exit(1);
    }
}

void parse_args(prog_t* prog, int argc, char const** argv) {
    if (argc < 2) {
        _print_help(prog);
        exit(1);
    }

    if (argv[1][0] != '-') {  // Check if subprogram
        int idx = _find_subprog(prog, argv[1]);
        if (idx < 0) {
            fprintf(stderr, "%sInvalid subprogram '%s'.\n", LOG_SUBPROG_ERROR, argv[1]);
            exit(1);
        } else {
            prog->subprogs[idx]->main_func(prog->subprogs[idx], argc - 1, argv + 1);
        }
    } else {
        _init_opt_parse();
        while (_get_option(prog, argc, argv) >= 0) {
            _check_opt_type();
            _check_value_type();
            _assign_arg();
        }
        if (!_is_along_option_given(prog)) {
            _check_option_necessity(prog);
            _check_option_dependency(prog);
        }
    }
}

static void _print_help(prog_t* prog) {
    fprintf(stderr, "Usage: ");
    if (prog->parent_prog == NULL) {
        fprintf(stderr, "%s [subprog] [option]\n\n", prog->name);
    } else {
        int parent_num = 0;
        char** parent_progs = malloc(parent_num * sizeof(char*));
        prog_t* current = prog;
        while (current->parent_prog != NULL) {
            parent_num++;
            parent_progs = realloc(parent_progs, parent_num * sizeof(char*));
            parent_progs[parent_num - 1] = malloc((strlen(current->parent_prog->name) + 1) * sizeof(char));
            strcpy(parent_progs[parent_num - 1], current->parent_prog->name);
            current = current->parent_prog;
        }
        for (int i = parent_num - 1; i >= 0; i--) {
            fprintf(stderr, "%s ", parent_progs[i]);
            free(parent_progs[i]);
            parent_progs[i] = NULL;
        }
        fprintf(stderr, "%s ", prog->name);
        fprintf(stderr, "[option]\n\n");
        free(parent_progs);
    }
    if (prog->subprog_num > 0) {
        fprintf(stderr, "Available subprograms:\n");
        for (int i = 0; i < prog->subprog_num; i++) {
            fprintf(stderr, "\t%s\t%s\n", prog->subprogs[i]->name, prog->subprogs[i]->desc);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "Available options:\n");
    for (int i = 0; i < prog->option_num; i++) {
        fprintf(stderr, "\t-%c, %s\t%s\n",
                prog->options[i]->short_name, prog->options[i]->long_name, prog->options[i]->desc);
    }
}
