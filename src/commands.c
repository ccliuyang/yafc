/* commands.c -- 
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
#include "commands.h"
#include "cmd.h"
#include "alias.h"
#include "ftp.h"
#include "strq.h"
#include "gvars.h"
#include "bashline.h"

/* in bookmark.c */
void auto_create_bookmark(void);

#undef __  /* gettext no-op */
#define __(text) (text)

#undef CMD
#define CMD(NAME, NC, NL, AU, CPL, HINT) \
 {#NAME, NC, NL, AU, CPL, HINT, cmd_ ## NAME}

cmd_t cmds[] = {
	CMD(flush, 1,1,1, cpNone, __("flushes replies")),
	{"!", 0,0,0, cpLocalFile, __("same as 'shell'"), cmd_shell},
	CMD(alias, 0,0,1, cpAlias, 0),
	CMD(bookmark, 0,0,1, cpBookmark, 0),
	CMD(cache, 1,1,1, cpNone, 0),
	CMD(cat, 1,1,1, cpRemoteFile, __("view a remote file\n usage: cat <remote file>")),
	CMD(cd, 1,1,1, cpRemoteDir, __("change remote working directory\n usage: cd [directory]\n cd without parameters changes to home directory\n Use 'cd -' for previous directory")),
	CMD(cdup, 1,1,1, cpNone, __("change remote working directory to parent directory\n usage: cdup")),
	CMD(chmod, 1,1,0, cpRemoteFile, __("change permissions on remote file(s)")),
	CMD(close, 0,0,1, cpNone, 0),
	CMD(copyright, 0,0,1, cpNone, __("show copyright notice\n usage: copyright")),
	CMD(filesize, 1,1,1, cpRemoteFile, __("show remote file's size in bytes")),
	CMD(filetime, 1,1,1, cpRemoteFile, __("show remote file's time")),
	CMD(fxp, 0,0,0, cpRemoteFile, 0),
	CMD(get, 0,0,0, cpRemoteFile, 0),
	CMD(help, 0,0,1, cpCommand, __("DON'T PANIC\n usage: help [command(s)]")),
	CMD(lcd, 0,0,1, cpLocalFile, 0),
	CMD(list, 1,1,1, cpRemoteFile, __("send a LIST command\n usage: list [options]")),
	CMD(lpwd, 0,0,1, cpNone, 0),
	CMD(ls, 0,0,0, cpRemoteFile, 0),
	CMD(ltag, 0,0,1, cpLocalFile, 0),
	CMD(luntag, 0,0,0, cpLocalTaglist, 0),
	CMD(mkdir, 1,1,1, cpNone, __("create remote directory")),
	CMD(nlist, 1,1,1, cpRemoteFile, __("send a NLST command")),
	CMD(nop, 1,0,1, cpNone, __("send a NOOP command to server (does nothing)")),
	CMD(idle, 1,1,1, cpNone, __("get or set idle timeout")),
	CMD(open, 0,0,1, cpHostname, 0),
	CMD(put, 0,0,1, cpLocalFile, 0),
	CMD(pwd, 1,1,1, cpNone, __("print current (remote) working directory")),
	CMD(quit, 0,0,1, cpNone, 0),
	CMD(quote, 1,0,1, cpNone, __("send arbitrary ftp command")),
	CMD(mv, 1,1,1, cpRemoteFile, __("rename a remote file or directory\n usage: mv <from> <to>")),
	CMD(rhelp, 1,0,1, cpNone, __("get remote help\n usage: rhelp [command]")),
	CMD(rm, 0,0,1, cpRemoteFile, 0),
	CMD(rmdir, 1,1,1, cpRemoteDir, __("remove remote directory\n usage: rmdir <remote directory>")),
	CMD(rstatus, 1,0,1, cpRemoteFile, __("show remote status")),
	CMD(set, 0,0,1, cpVariable, __("set program variables\n usage: set [variable] [value]")),
	CMD(shell, 0,0,0, cpLocalFile, __("execute shell command or invoke shell\n usage: shell [shell command]")),
	CMD(site, 1,1,1, cpNone, __("send site specific command (try 'site help' or 'rhelp site')")),
	CMD(source, 0,0,1, cpLocalFile, 0),
	CMD(status, 0,0,1, cpNone, 0),
	CMD(switch, 0,0,0, cpFtpList, 0),
	CMD(system, 1,1,0, cpNone, __("show type of remote system")),
	CMD(tag, 0,0,0, cpRemoteFile, 0),
	CMD(unalias, 0,0,1, cpAlias, 0),
	CMD(untag, 1,1,1, cpTaglist, __("untag file(s) in the tag list")),
	CMD(url, 1,1,1, cpNone, __("show URL")),
	CMD(user, 1,0,1, cpNone, __("send new user information\n usage: user [username]")),
	CMD(version, 0,0,1, cpNone, __("show version")),
	CMD(warranty, 0,0,1, cpNone, __("show warranty (or lack of)")),
	CMD(zcat, 1,1,1, cpRemoteFile, __("view a compressed remote file")),
#if defined(KRB4) || defined(KRB5)
	CMD(prot, 1,1,1, cpNone, __("set protection level")),
#endif
#ifdef KRB4
	CMD(afslog, 1,1,1, cpNone, __("obtain remote AFS tokens")),
	CMD(klist, 1,1,1, cpNone, __("show remote tickets")),
	CMD(kauth, 1,1,1, cpNone, __("get remote tokens")),
	CMD(kdestroy, 1,1,1, cpNone, __("destroy remote tickets")),
	CMD(krbtkfile, 1,1,1, cpNone, __("set filename of remote tickets")),
#endif

    /* this _must_ be the last entry, unless you like segfaults ;) */
    {0, 0,0,1, 0, 0}
};

cmd_t *find_cmd(const char *cmd)
{
	int i;
	cmd_t *r = 0;

	for(i=0;cmds[i].cmd;i++) {
		/* compare only strlen(cmd) chars, allowing commands
		 * to be shortened, as long as they're not ambiguous
		 */
		if(strncmp(cmds[i].cmd, cmd, strlen(cmd)) == 0) {
			/* is this an exact match? */
			if(strlen(cmd) == strlen(cmds[i].cmd))
				return &cmds[i];
			if(r)
				r = CMD_AMBIGUOUS;
			else
				r = &cmds[i];
		}
	}
	return r;
}

int rearrange_args(args_t *args, const char *rchars)
{
	int r = -1;
	int i;

	for(i=0; i<args->argc; i++) {
		char *e = args->argv[i];
		while(*e) {
			if(strchr(rchars, *e) != 0 && !char_is_quoted(args->argv[i], (int)(e - args->argv[i]))) {
				r = i;
				break;
			}
			e++;
		}
		if(*e && e != args->argv[i]) {
			char *a;
			size_t num;
#if 0
			if(e[-1] == '\\')
				/* skip backslash-quoted chars */
				continue;
#endif
			num = (size_t)(e - args->argv[i]);
			a = xstrndup(args->argv[i], num);
			args_insert_string(args, i, a);
			xfree(a);
			strpull(args->argv[i+1], num);
			break;
		}
	}
	args_remove_empty(args);
	return r;
}

/* rearranges redirections characters (|, >, <) so
 * they appear in separate argsuments
 */
int rearrange_redirections(args_t *args)
{
	return rearrange_args(args, "|><");
}

/* given an alias expanded args_t, ARGS, which may include
 * %-parameters (%*, %1, %2, ...), expand those parameters
 * using the arguments in alias_args
 * 
 * redirection stuff is not included in the %-parameters
 * 
 * alias_args is modified (possibly cleared)
 * 
 * modifies ARGS
 */

void expand_alias_parameters(args_t **args, args_t *alias_args)
{
	int i;
	args_t *new_args;
	args_t *redir_args = 0;
	args_t *deleted;
	
/*	if(alias_args->argc == 0)
		return;*/

	rearrange_args(*args, ";");
	
	/* do not include redirections in %-parameters */
	i = rearrange_redirections(alias_args);
	if(i != -1) {
		redir_args = args_create();
		args_add_args2(redir_args, alias_args, i);
		args_del(alias_args, i, alias_args->argc - i);
	}

	deleted = args_create();
	args_add_args(deleted, alias_args);
	
	new_args = args_create();
	args_add_args(new_args, *args);

	for(i=0; i<new_args->argc; i++)
	{
		char *e;

		/* %* can not be used embedded in an argument */
		if(strcmp(new_args->argv[i], "%*") == 0) {
			args_del(new_args, i, 1);
			args_insert_args(new_args, i, deleted, 0, deleted->argc);
			args_clear(alias_args);
			continue;
		}

		e = strqchr(new_args->argv[i], '%');
		if(e) {
			char *ep;
			char *ins;
			char *tmp;
			int n;

			*e = 0;
			n = strtoul(++e, &ep, 0);

			if(ep != e && n < alias_args->argc && n >= 0) {
				ins = deleted->argv[n];
				xfree(alias_args->argv[n]);
				alias_args->argv[n] = 0;
			} else
				ins = 0;

			/* insert the parameter in this argument */
			asprintf(&tmp, "%s%s%s", new_args->argv[i], ins ? ins : "", ep);
			xfree(new_args->argv[i]);
			new_args->argv[i] = tmp;
		}
		
	}

	args_destroy(deleted);
	args_remove_empty(alias_args);
	args_add_args(new_args, alias_args);

	if(redir_args) {
		args_add_args(new_args, redir_args);
		args_destroy(redir_args);
	}

	args_remove_empty(new_args);

	args_destroy(*args);
	*args = new_args;
}

cmd_t *find_func(const char *cmd, bool print_error)
{
	cmd_t *c;
	alias *a;

	/* finds (abbreviated) command */
	c = find_cmd(cmd);

	/* finds (abbreviated) alias */
	a = alias_search(cmd);

	if(!c && !a) {
		if(print_error)
			fprintf(stderr, _("No such command '%s', try 'help'\n"), cmd);
		return 0;
	}

	if(a == ALIAS_AMBIGUOUS) {
		/* skip alias if exact command */
		if(c && c != CMD_AMBIGUOUS && strlen(c->cmd) == strlen(cmd))
			return c;
		else {
			if(print_error)
				fprintf(stderr, _("ambiguous alias '%s'\n"), cmd);
			return 0;
		}
	}

	if(c == CMD_AMBIGUOUS) {
		if(!a) {
			if(print_error)
				fprintf(stderr, _("ambiguous command '%s'\n"), cmd);
			return 0;
		}
		/* skip ambiguous command, run exact alias instead */
		c = 0;
	}

	if(c && !a)
		return c;

	/* found both alias and command */
	if(a && c) {
		if(strlen(a->name) == strlen(cmd))
			/* skip exact command, run exact alias */
			c = 0;
		else if(strlen(c->cmd) == strlen(cmd))
			/* skip alias, run exact command */
			return c;
		else {
			/* both alias and command are abbreviated
			 * Q: which wins? A: no one!?
			 */
			if(print_error)
				fprintf(stderr, _("ambiguous alias/command '%s'\n"), cmd);
			return 0;
		}
	}
	/* now we should have the alias only */

	c = find_cmd(a->value->argv[0]);
	if(!c) {
		if(print_error)
			fprintf(stderr, _("No such command '%s' (expanded alias '%s')\n"),
				   a->value->argv[0], a->name);
		return 0;
	}
	if(c == CMD_AMBIGUOUS) {
		if(print_error)
			fprintf(stderr, _("ambiguous command '%s' (expanded alias '%s')\n"),
				   a->value->argv[0], a->name);
		return 0;
	}

/*	expand_alias_parameters(args, a->value);*/
	
	return c;
}

void cmd_list(int argc, char **argv)
{
	OPT_HELP("List files.  Usage:\n"
			 "  list [options] [file]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	if(argc == optind + 1)
		ftp_list("LIST", 0, stdout);
	else {
		char *args;

		args = args_cat(argc, argv, optind);
		ftp_list("LIST", args, stdout);
		xfree(args);
	}
}

void cmd_nlist(int argc, char **argv)
{
	OPT_HELP("Simple file list.  Usage:\n"
			 "  nlist [options] [file]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	if(argc == optind + 1)
		ftp_list("NLST", 0, stdout);
	else {
		char *args;

		args = args_cat(argc, argv, optind);
		ftp_list("NLST", args, stdout);
		xfree(args);
	}
}

/* FIXME: option --type={binary | ascii} to cmd_cat
 */
void cmd_cat(int argc, char **argv)
{
	int i;

	OPT_HELP("Print file(s) on standard output.  Usage:\n"
			 "  cat [options] <file>...\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	minargs(optind);

	for(i=optind;i<argc;i++) {
		listitem *gli;
		list *gl = rglob_create();
		stripslash(argv[i]);
		if(rglob_glob(gl, argv[i], true, false, 0) == -1)
			fprintf(stderr, _("%s: no matches found\n"), argv[i]);
		for(gli = gl->first; gli; gli=gli->next) {
			rfile *rf = (rfile *)gli->data;
			const char *fn = base_name_ptr(rf->path);
			if(strcmp(fn, ".")!=0 && strcmp(fn, "..")!=0)
				ftp_receive(rf->path, stdout, tmBinary, 0);
		}
		rglob_destroy(gl);
	}
}

/* FIXME: is zcat needed? why not use 'cat | gunzip -c' or something
 */
void cmd_zcat(int argc, char **argv)
{
	int i;

	minargs(1);

	for(i=1; i<argc; i++) {
		char *cmd = "zcat";
		char *e = strrchr(argv[i], '.');
		if(e && strcmp(e+1, "bz2") == 0)
			cmd = "bunzip2";

		ftp_getfile(argv[i], cmd, getPipe, tmBinary, 0);
	}
}

void cmd_cd(int argc, char **argv)
{
	char *e;
#if 0
	OPT_HELP("Change working directory.  Usage:\n"
			 "  cd [options] [directory]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n"
			 "if [directory] is '-', cd changes to the previous working directory\n"
			 "if omitted, changes to home directory\n");

	maxargs(optind);
#endif

	maxargs(1);

	if(argc <= 1)
		e = ftp->homedir;
	else if(strcmp(argv[1], "-") == 0) {
		e = ftp->prevdir;
		if(e == 0) /* no previous directory */
			return;
	} else
		e = argv[1];
	e = tilde_expand_home(e, ftp->homedir);
	ftp_chdir(e);
	xfree(e);
}

void cmd_cdup(int argc, char **argv)
{
	OPT_HELP("Change to parent working directory.  Usage:\n"
			 "  cdup [options]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");
	maxargs(optind - 1);
	ftp_cdup();
}

void opt_help(int argc, char **argv, char *help)
{
	struct option longopts[] = {
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	int c;

	optind = 0;
	while((c = getopt_long(argc, argv, "h", longopts, 0)) != EOF) {
		switch(c) {
		  case 'h':
			fprintf(stderr, help);
		  case '?':
			optind = -1;
			return;
		}
	}
}

void cmd_pwd(int argc, char **argv)
{
	OPT_HELP("Print the current working directory.  Usage:\n"
			 "  pwd [options]\n"
			 "Options:\n"
			 "  -h, --help     show this help\n");

	maxargs(optind - 1);

	ftp_set_tmp_verbosity(vbCommand);
	ftp_cmd("PWD");
}

void cmd_url(int argc, char **argv)
{
	int c;
	struct option longopts[] = {
		{"no-encoding", no_argument, 0, 'e'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	bool no_encoding = false;

	optind = 0;
	while((c = getopt_long(argc, argv, "eh", longopts, 0)) != EOF) {
		switch(c) {
		  case 'h':
			printf(_("Print the current URL.  Usage:\n"
					 "  url [options]\n"
					 "Options:\n"
					 "  -e, --no-encoding    don't encode URL as rfc1738 says\n"
					 "  -h, --help           show this help\n"));
			return;
		  case 'e':
			no_encoding = true;
			break;
		  case '?':
			return;
		}
	}

	maxargs(optind - 1);

	printf("ftp://");

	if(no_encoding) {
		printf("%s@%s", ftp->url->username, ftp->url->hostname);
		if(ftp->url->port != 21)
			printf(":%d", ftp->url->port);
		printf("/%s\n", ftp->curdir);
		return;
	}

	if(!url_isanon(ftp->url)) {
		char *e = encode_url_username(ftp->url->username);
		printf("%s@", e);
		xfree(e);
	}
	printf("%s", ftp->url->hostname);
	if(ftp->url->port != 21)
		printf(":%u", ftp->url->port);
	if(strcmp(ftp->curdir, ftp->homedir) != 0) {
		char *d;
		char *e = ftp->curdir;
#if 0
		if(strncmp(ftp->curdir, ftp->homedir, strlen(ftp->homedir)) == 0)
			e = ftp->curdir + strlen(ftp->homedir) + 1;
#endif
		d = encode_url_directory(e);
		printf("/%s", d);
		xfree(d);
	}
	printf("\n");
}

void cmd_close(int argc, char **argv)
{
	if(argv) {
		OPT_HELP("Close open connection.  Usage:\n"
				 "  close [options]\n"
				 "Options:\n"
				 "  -h, --help     show this help\n");
		maxargs(optind - 1);
	}

	ftp_quit();
	if(list_numitem(gvFtpList) > 1) {
		list_delitem(gvFtpList, gvCurrentFtp);
		gvCurrentFtp = gvFtpList->first;
		ftp_use((Ftp *)gvCurrentFtp->data);
	}
}

void cmd_rhelp(int argc, char **argv)
{
	char *e;

	e = args_cat(argc, argv, 1);
	ftp_help(e);
	xfree(e);
}

void cmd_mkdir(int argc, char **argv)
{
	int i;
	OPT_HELP("Create directory.  Usage:\n"
			 "  mkdir [options]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");
	minargs(optind);

	for(i=optind; i<argc; i++)
		ftp_mkdir(argv[i]);
}

void cmd_rmdir(int argc, char **argv)
{
	int i;
	OPT_HELP("Remove directory.  Usage:\n"
			 "  rmdir [options]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");
	minargs(optind);

	for(i=optind; i<argc; i++)
		ftp_rmdir(argv[i]);
}

void cmd_idle(int argc, char **argv)
{
	OPT_HELP("Get or set idle timeout.  Usage:\n"
			 "  idle [options] [timeout]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n"
			 "Without the timeout option, print the current idle timeout\n");
	maxargs(optind);

	if(argc - 1 == optind)
		ftp_idle(argv[optind]);
	else
		ftp_idle(0);
}

void cmd_nop(int argc, char **argv)
{
	OPT_HELP("Do nothing (send a NOOP command).  Usage:\n"
			 "  nop [options]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");
	maxargs(optind - 1);

	ftp_noop();
}

void cmd_rstatus(int argc, char **argv)
{
	OPT_HELP("Show status of remote host.  Usage:\n"
			 "  rstatus [options]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	maxargs(optind - 1);
	ftp_set_tmp_verbosity(vbCommand);
	ftp_cmd("STAT");
}

void cmd_status(int argc, char **argv)
{
	OPT_HELP("Show status.  Usage:\n"
			 "  status [options]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	maxargs(optind - 1);

	if(ftp_connected())
		printf(_("connected to '%s'\n"), ftp->host->ohostname);
	else
		puts(_("not connected"));
	if(ftp_loggedin())
		printf(_("logged in as '%s'\n"), ftp->url->username);
#if defined(KRB4) || defined(KRB5)
	if(ftp_connected())
		sec_status();
#endif
	if(list_numitem(gvFtpList) > 1 || ftp_connected())
		printf(_("There are totally %u connections open\n"), list_numitem(gvFtpList));
}

void cmd_site(int argc, char **argv)
{
	char *e;
	OPT_HELP("Send site specific command.  Usage:\n"
			 "  site [options] [command]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n"
			 "[command] should be a valid SITE argument\n"
			 "try 'site help' or 'rhelp site' for more information\n");

	minargs(optind);

	e = args_cat(argc, argv, optind);
	ftp_set_tmp_verbosity(vbCommand);
	ftp_cmd("SITE %s", e);
	xfree(e);
}

void cmd_chmod(int argc, char **argv)
{
	int i;
	list *gl;
	listitem *li;

	OPT_HELP("Change permissions on remote file.  Usage:\n"
			 "  chmod [options] <mode> <file>\n"
			 "Options:\n"
			 "  -h, --help    show this help\n"
			 "<mode> is the permission mode, in octal (ex 644)\n");

	minargs(optind + 1);

	gl = rglob_create();
	for(i=optind+1; i<argc; i++) {
		stripslash(argv[i]);
		rglob_glob(gl, argv[i], true, true, NODOTDIRS);
	}

	for(li=gl->first; li; li=li->next) {
		ftp_chmod(((rfile *)li->data)->path, argv[optind]);
		printf("%s: %s\n", ((rfile *)li->data)->path, ftp_getreply(false));
	}
	rglob_destroy(gl);
}

void cmd_mv(int argc, char **argv)
{
	OPT_HELP("Rename or move a file.  Usage:\n"
			 "  mv [options] <src> <dest>\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	minargs(optind + 1);
	maxargs(optind + 1);

	ftp_set_tmp_verbosity(vbError);
	if(ftp_rename(argv[optind], argv[optind + 1]) == 0)
		printf("%s -> %s\n", argv[optind], argv[optind + 1]);
}

void cmd_cache(int argc, char **argv)
{
	int c;
	struct option longopts[] = {
		{"clear", no_argument, 0, 'c'},
		{"list", no_argument, 0, 'l'},
		{"touch", no_argument, 0, 't'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	bool touch = false;

	optind = 0;
	while((c = getopt_long(argc, argv, "clt::h", longopts, 0)) != EOF) {
		switch(c) {
		  case 'c':
			ftp_cache_clear();
			return;
		  case 'l':
			ftp_cache_list_contents();
			return;
		  case 't':
			  touch = true;
			  break;
		  case 'h':
			printf(_("Control the directory cache.  Usage:\n"
					 "  cache [option] [directories]\n"
					 "Options:\n"
					 "  -c, --clear        clear whole directory cache\n"
					 "  -l, --list         list contents of cache\n"
					 "  -t, --touch        remove directories from cache\n"
					 "                     if none given, remove current directory\n"
					 "  -h, --help         show this help\n"));
			return;
		  case '?':
			return;
		}
	}

	if(touch) {
		if(optind < argc) {
			int i;
			for(i = optind; i < argc; i++)
				ftp_cache_flush_mark(argv[i]);
		} else
			ftp_cache_flush_mark(ftp->curdir);
	} else {
		minargs(1);
		maxargs(0);
	}
}

void cmd_quote(int argc, char **argv)
{
	char *e;

	OPT_HELP("Send arbitrary FTP command.  Usage:\n"
			 "  quote [options] <commands>\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	minargs(optind);
	e = args_cat(argc, argv, optind);
	ftp_set_tmp_verbosity(vbDebug);
	ftp_cmd("%s", e);
	xfree(e);
}

void cmd_filetime(int argc, char **argv)
{
	int i;

	OPT_HELP("Show modification time of remote file.  Usage:\n"
			 "  filetime [options] <file>...\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	minargs(optind);

	for(i=optind;i<argc;i++) {
		time_t t = ftp_filetime(argv[i]);
		if(t != (time_t) -1)
			printf("%s: %s", argv[i], ctime(&t));
		else
			printf("%s\n", ftp_getreply(false));
	}
}

void cmd_filesize(int argc, char **argv)
{
	int i;

	OPT_HELP("Show size of a remote file.  Usage:\n"
			 "  filesize [options] <file>...\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	minargs(optind);

	for(i=optind;i<argc;i++) {
		unsigned long s = ftp_filesize(argv[i]);
		if(s != (unsigned long) -1)
			printf(_("%s: %lu bytes\n"), argv[i], s);
	}
}

/* in rc.c */
int parse_rc(const char *file, bool warn);

/* in completion.c */
void init_host_completion(void);

/* in main.c */
void init_ftp(void);

void cmd_source(int argc, char **argv)
{
	int i;

	OPT_HELP("Read (source) a configuration file.  Usage:\n"
			 "  source [options] <file>...\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	minargs(optind);

	for(i=optind; i<argc; i++)
		parse_rc(argv[i], true);
	init_host_completion();
	init_ftp();
}

void cmd_system(int argc, char **argv)
{
	if(argv) {
		OPT_HELP("Show type of remote system.  Usage:\n"
				 "  system [options]\n"
				 "Options:\n"
				 "  -h, --help    show this help\n");

		maxargs(optind - 1);
	}

	if(ftp_get_verbosity() != vbDebug)
		fprintf(stderr, _("remote system: "));
	ftp_set_tmp_verbosity(vbCommand);
	ftp_cmd("SYST");
}

static int switch_search(const Ftp *f, const char *name)
{
	if(f->url->alias) {
		if(strcmp(f->url->alias, name) == 0)
			return 0;
	}
	return strcmp(f->url->hostname, name);
}

listitem *ftplist_search(const char *str)
{
	if(isdigit((int)str[0]) && strchr(str, '.') == 0) {
		listitem *li;
		int i = 1;
		int n = atoi(str);
		if(n <= 0 || n > list_numitem(gvFtpList)) {
			ftp_err(_("invalid connection number: '%d'\n"), n);
			return 0;
		}
		for(li=gvFtpList->first; li; li=li->next, i++) {
			if(i == n)
				return li;
		}
	} else {
		listitem *li = list_search(gvFtpList,
								   (listsearchfunc)switch_search, str);
		if(li)
			return li;
		else
			ftp_err(_("no such connection open: '%s'\n"), str);
	}
	return 0;
}

void cmd_switch(int argc, char **argv)
{
	OPT_HELP("Switch between active connections.  Usage:\n"
			 "  switch [options] [number | name]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n"
			 "The argument can either be the connection number, host name or its alias\n"
			 "Without argument, switch to the next active connection\n");

	maxargs(optind);

	if(argc > optind) {
		listitem *tmp = ftplist_search(argv[optind]);
		if(tmp)
			gvCurrentFtp = tmp;
	} else {
		if(gvCurrentFtp == gvFtpList->last)
			gvCurrentFtp = gvFtpList->first;
		else {
			if(gvCurrentFtp->next)
				gvCurrentFtp = gvCurrentFtp->next;
		}
	}

	ftp_use((Ftp *)gvCurrentFtp->data);
}

void cmd_flush(int argc, char **argv)
{
	OPT_HELP("Flushes all replies.  Usage:\n"
			 "  flush [options]\n"
			 "Options:\n"
			 "  -h, --help    show this help\n");

	maxargs(0);

	ftp_flush_reply();
}
