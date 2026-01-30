/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Johan Kanflo (github.com/kanflo)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file cli.h
 * @brief Command Line Interface Parser
 *
 * This module provides a simple CLI parser for debugging and configuration
 * via serial terminal. It parses input lines and dispatches to handlers.
 *
 * ## Features
 *
 * - Command name matching
 * - Argument parsing (space-separated)
 * - Argument count validation (min/max)
 * - Help text and usage message support
 *
 * ## Usage Example
 *
 * ```c
 * static const cli_command_t commands[] = {
 *     { "help", cmd_help, 0, 0, "Show help", "help" },
 *     { "set",  cmd_set,  2, 2, "Set value", "set <name> <value>" },
 * };
 *
 * cli_run(commands, 2, input_line);
 * ```
 *
 * @see serialhandler.h for serial input handling
 * @see dbg_printf.h for debug output
 */

#ifndef __CLI_H__
#define __CLI_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Command definition structure
 *
 * Defines a CLI command including its name, handler, argument
 * requirements, and documentation strings.
 */
typedef struct {
    /** @brief Command name (what user types) */
    char const *cmd;
    /**
     * @brief Command handler function
     * @param argc Number of arguments including command name
     * @param argv Array of argument strings
     */
    void (*handler)(uint32_t argc, char *argv[]);
    /** @brief Minimum arguments required (excluding command name) */
    uint32_t min_arg;
    /** @brief Maximum arguments allowed (excluding command name) */
    uint32_t max_arg;
    /** @brief Short help description */
    char const *help;
    /** @brief Usage syntax string */
    char const *usage;
} cli_command_t;

/**
 * @brief Parse and execute a command line
 *
 * Parses the input line, matches against registered commands,
 * validates argument count, and calls the appropriate handler.
 *
 * @param[in]     cmds Array of command definitions
 * @param[in]     num  Number of commands in the array
 * @param[in,out] line Input line to parse (will be modified)
 *
 * @note The line buffer is modified during parsing
 * @note Command matching is case-sensitive
 */
void cli_run(const cli_command_t cmds[], const uint32_t num, char *line);

#endif // __CLI_H__
