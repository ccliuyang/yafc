/* local.c -- local commands
 * 
 * This file is part of Yafc, an ftp client.
 * This program is Copyright (C) 1998, 1999, 2000 Martin Hedenfalk
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "syshdr.h"
#include "input.h"
#include "commands.h"
#include "gvars.h"
#include "strq.h"

/* print local working directory */
void cmd_lpwd(int argc, char **argv)
{
	char tmp[PATH_MAX+1];

	if(argv) {
		OPT_HELP("Print local working directory.  Usage:\n"
				 "  lpwd [options]\n"
				 "Options:\n"
				 "  -h, --help    show this help\n");

		maxargs(optind - 1);
	}

	if(getcwd(tmp, PATH_MAX) == 0) {
		fprintf(stderr, _("Couldn't get local working directory...\n"));
		return;
	}
	printf(_("local working directory is '%s'\n"), tmp);
}

/* local change directory */
void cmd_lcd(int argc, char **argv)
{
	char *e = 0, tmp[PATH_MAX+1];
	char *te;

	OPT_HELP("Change local working directory.  Usage:\n"
			 "  lcd [options] [directory]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n"
			 "if [directory] is '-', lcd changes to the previous working directory\n"
			 "if omitted, changes to home directory\n");

	maxargs(optind);

	if(argc < optind + 1)
		e = gvLocalHomeDir;
	else {
		e = argv[optind];
		if(strcmp(e, "-") == 0)
			e = gvLocalPrevDir;
	}
	if(!e)
		return;
	getcwd(tmp, PATH_MAX);
	te = tilde_expand_home(e, gvLocalHomeDir);
	if(chdir(te) == -1)
		perror(te);
	else {
		xfree(gvLocalPrevDir);
		gvLocalPrevDir = xstrdup(tmp);
	}
	xfree(te);
	cmd_lpwd(0, 0);
}

void invoke_shell(char *cmdline)
{
	char *shell;
	pid_t pid;

	ftp_set_signal(SIGINT, SIG_IGN);
	shell = getenv("SHELL");
	if(!shell)
		shell = STD_SHELL;
	pid = fork();
	if(pid == 0) { /* child thread */
		if(cmdline)
			execl(shell, shell, "-c", cmdline, 0);
		else {
			printf(_("Executing '%s', use 'exit' to exit from shell...\n"), shell);
			execl(shell, shell, 0);
		}
		perror(shell);
		exit(1);
	}
	if(pid == -1) {
		perror("fork()");
		return;
	}
	waitpid(pid, 0, 0);  /* wait for child to finish execution */
}

/* FIXME: consider the following:
 *  shell wget -r -q %f
 * which expands to
 *  shell wget -r -q ftp://user:pass@site:port/dir
 * could be useful!?
 * 
 * %f - full path ("ftp://user:pass@site:port/dir")
 * %h - hostname ("ftp.foo.bar")
 * %p - port number
 * %u - username
 * %d - current directory
 */

void cmd_shell(int argc, char **argv)
{
	char *e = 0;

	if(argc > 1)
		e = args_cat(argc, argv, 1);
	invoke_shell(e);
	xfree(e);
}
