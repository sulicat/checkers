/* $Id$

   This file is a part of ponder, a English/American checkers game.

   Copyright (c) 2006, 2007, 2008, 2009 Quux Information.
                     Gong Jie <neo@quux.me>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
/** @file signal.cpp
 *  @brief Signal handling, examine and change signal action.
 */

extern "C"
{
	#include <execinfo.h>
	#include <fcntl.h>
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <unistd.h>
}
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include "signal.hpp"

namespace checkers
{
	sighandler_t signal(int signum, sighandler_t handler)
	{
		struct sigaction action;
		struct sigaction old_action;

		action.sa_handler = handler;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		if (SIGALRM != signum)
		{
			action.sa_flags |= SA_RESTART;
		}

		if (::sigaction(signum, &action, &old_action) < 0) {
			/// @throw std::runtime_error when sigaction() failed.
			throw std::runtime_error(
				std::string("sigaction() failed: ") +
				std::strerror(errno));
		}

		/// @return the previous value of the signal handler.
		return (old_action.sa_handler);
	}

	sigaction_t signal(int signum, sigaction_t handler)
	{
		struct sigaction action;
		struct sigaction old_action;

		action.sa_sigaction = handler;
		sigemptyset(&action.sa_mask);
		action.sa_flags = SA_SIGINFO;
		if (SIGALRM != signum)
		{
			action.sa_flags |= SA_RESTART;
		}

		if (::sigaction(signum, &action, &old_action) < 0) {
			/// @throw std::runtime_error when sigaction() failed.
			throw std::runtime_error(
				std::string("sigaction() failed: ") +
				std::strerror(errno));
		}

		/// @return the previous value of the signal handler.
		return (old_action.sa_sigaction);
	}

	// ================================================================

	/// Print dump information to stdout.
	static void crash_dump(const char* buf)
	{
		for (; *buf != '\0'; ++buf)
		{
			/** Since printf() is not reenterable.  Use system call
			 *  write() to print messages in signal handler.
			 */ 
			(void)::write(STDERR_FILENO, buf, 1);
		}
	}

	inline static void crash_dump(int n)
	{
		char b[std::numeric_limits<int>::digits10 + 3];
		char *s = b + sizeof(b) - 1;
		bool sign = (n < 0);

		if (sign)
		{
			n = -n;
		}
		*s = 0;
		do
		{
			*--s = '0' + (n % 10);
			n /= 10;
		} while (n);
		if (sign)
		{
			*--s = '-';
		}
		crash_dump(s);
	}

	void crash_handler(int signum, siginfo_t* siginfo, void* context)
	{
		/** Set stdin, stdout and stderr to block I/O.  Flush all open
		 *  output streams.  Synchronize stdin, stdout and stderr's
		 *  in-core state.
		 */
		(void)fcntl(STDIN_FILENO,  F_SETFL, 0);
		(void)fcntl(STDOUT_FILENO, F_SETFL, 0);
		(void)fcntl(STDERR_FILENO, F_SETFL, 0);

		(void)fflush(NULL);

		(void)fsync(STDIN_FILENO);
		(void)fsync(STDOUT_FILENO);
		(void)fsync(STDERR_FILENO);

		crash_dump("\n"
			"  =================================================="
				"====================\n"
			"  * Received Signal ");
		switch (signum)
		{
		case SIGABRT:
			crash_dump("SIGABRT\n");
			goto common_si_code;
		case SIGALRM:
			crash_dump("SIGALRM\n");
			goto common_si_code;
		case SIGBUS:
			crash_dump("SIGBUS\n");
			switch (siginfo->si_code)
			{
			case BUS_ADRALN:
				crash_dump("  * invalid address alignment\n");
				break;
			case BUS_ADRERR:
				crash_dump("  * non-existent physical"
					" address\n");
				break;
			case BUS_OBJERR:
				crash_dump("  * object specific hardware"
					" error\n");
				break;
			default:
				goto common_si_code;
			}
			break;
		case SIGCHLD:
			crash_dump("SIGCHLD\n");
			switch (siginfo->si_code)
			{
			case CLD_EXITED   :
				crash_dump("  * child has exited\n");
				break;
			case CLD_KILLED   :
				crash_dump("  * child was killed\n");
				break;
			case CLD_DUMPED   :
				crash_dump("  * child terminated abnormally\n");
				break;
			case CLD_TRAPPED  :
				crash_dump("  * traced child has trapped\n");
				break;
			case CLD_STOPPED  :
				crash_dump("  * child has stopped\n");
				break;
			case CLD_CONTINUED:
				crash_dump("  * stopped child has continued"
					" (since Linux 2.6.9)\n");
				break;
			default:
				goto common_si_code;
			}
			break;
		case SIGFPE:
			crash_dump("SIGFPE\n");
			switch (siginfo->si_code)
			{
			case FPE_INTDIV:
				crash_dump("  * integer divide by zero\n");
				break;
			case FPE_INTOVF:
				crash_dump("  * integer overflow\n");
				break;
			case FPE_FLTDIV:
				crash_dump("  * floating point divide by"
					" zero\n");
				break;
			case FPE_FLTOVF:
				crash_dump("  * floating point overflow\n");
				break;
			case FPE_FLTUND:
				crash_dump("  * floating point underflow\n");
				break;
			case FPE_FLTRES:
				crash_dump("  * floating point inexact"
					" result\n");
				break;
			case FPE_FLTINV:
				crash_dump("  * floating point invalid"
					" operation\n");
				break;
			case FPE_FLTSUB:
				crash_dump("  * subscript out of range\n");
				break;
			default:
				goto common_si_code;
			}
			break;
		case SIGHUP:
			crash_dump("SIGHUP\n");
			goto common_si_code;
		case SIGILL:
			crash_dump("SIGILL\n");
			switch (siginfo->si_code)
			{
			case ILL_ILLOPC:
				crash_dump("  * illegal opcode\n");
				break;
			case ILL_ILLOPN:
				crash_dump("  * illegal operand\n");
				break;
			case ILL_ILLADR:
				crash_dump("  * illegal addressing mode\n");
				break;
			case ILL_ILLTRP:
				crash_dump("  * illegal trap\n");
				break;
			case ILL_PRVOPC:
				crash_dump("  * privileged opcode\n");
				break;
			case ILL_PRVREG:
				crash_dump("  * privileged register\n");
				break;
			case ILL_COPROC:
				crash_dump("  * coprocessor error\n");
				break;
			case ILL_BADSTK:
				crash_dump("  * internal stack error\n");
				break;
			default:
				goto common_si_code;
			}
			break;
		case SIGINT:
			crash_dump("SIGINT\n");
			goto common_si_code;
		case SIGPIPE:
			crash_dump("SIGPIPE\n");
			goto common_si_code;
#ifdef SIGPOLL
		case SIGPOLL:
			crash_dump("SIGPOLL\n");
			switch (siginfo->si_code)
			{
#  ifdef POLL_IN
			case POLL_IN :
				crash_dump("  * data input available\n");
				break;
#  endif // POLL_IN
#  ifdef POLL_OUT
			case POLL_OUT:
				crash_dump("  * output buffers available\n");
				break;
#  endif // POLL_OUT
#  ifdef POLL_MSG
			case POLL_MSG:
				crash_dump("  * input message available\n");
				break;
#  endif // POLL_MSG
#  ifdef POLL_ERR
			case POLL_ERR:
				crash_dump("  * I/O error\n");
				break;
#  endif // POLL_ERR
#  ifdef POLL_PRI
			case POLL_PRI:
				crash_dump("  * high priority input"
					" available\n");
				break;
#  endif // POLL_PRI
#  ifdef POLL_HUP
			case POLL_HUP:
				crash_dump("  * device disconnected\n");
				break;
#  endif // POLL_HUP
			default:
				goto common_si_code;
			}
			break;
#endif // SIGPOLL
		case SIGQUIT:
			crash_dump("SIGQUIT\n");
			goto common_si_code;
		case SIGSEGV:
			crash_dump("SIGSEGV\n");
			switch (siginfo->si_code)
			{
			case SEGV_MAPERR:
				crash_dump("  * address not mapped to"
					" object\n");
				break;
			case SEGV_ACCERR:
				crash_dump("  * invalid permissions for mapped"
					" object\n");
				break;
			default:
				goto common_si_code;
			}
			break;
		case SIGTRAP:
			crash_dump("SIGTRAP\n");
			switch (siginfo->si_code)
			{
#ifdef TRAP_BRKPT
			case TRAP_BRKPT:
				crash_dump("  * process breakpoint\n");
				break;
#endif // TRAP_BRKPT
#ifdef TRAP_TRACE
			case TRAP_TRACE:
				crash_dump("  * process trace trap\n");
				break;
#endif // TRAP_TRACE
			default:
				goto common_si_code;
			}
			break;
		case SIGTERM:
			crash_dump("SIGTERM\n");
			goto common_si_code;
		case SIGUSR1:
			crash_dump("SIGUSR1\n");
			goto common_si_code;
		case SIGUSR2:
			crash_dump("SIGUSR2\n");
			goto common_si_code;
		default:
			crash_dump(signum);
			crash_dump("\n"
				"  * This should not happen.\n");
		common_si_code:
			switch (siginfo->si_code)
			{
			case SI_USER   :
				crash_dump("  * kill(), sigsend(), or"
					" raise()\n");
				break;
#ifdef SI_KERNEL
			case SI_KERNEL :
				crash_dump("  * The kernel\n");
				break;
#endif // SI_KERNEL
			case SI_QUEUE  :
				crash_dump("  * sigqueue()\n");
				break;
			case SI_TIMER  :
				crash_dump("  * POSIX timer expired\n");
				break;
			case SI_MESGQ  :
				crash_dump("  * POSIX message queue state"
					" changed (since Linux 2.6.6)\n");
				break;
			case SI_ASYNCIO:
				crash_dump("  * AIO completed\n");
				break;
#ifdef SI_SIGIO
			case SI_SIGIO  :
				crash_dump("  * queued SIGIO\n");
				break;
#endif // SI_SIGIO
#ifdef SI_TKILL
			case SI_TKILL  :
				crash_dump("  * tkill() or tgkill() (since"
					" Linux 2.4.19)\n");
				break;
#endif // SI_TKILL
			default:
				crash_dump("  * Unknown or not-specified"
					" cause\n");
				break;
			}
			break;
		}

		/** Print out signal received, backtrace information and then
		 *  make a core dump.
		 */
		crash_dump("  * Creating caller backtrace...\n");
		void* callers[255];
		size_t stacks = backtrace(callers,
				sizeof(callers) / sizeof(callers[0]));
		char** call_names = backtrace_symbols(callers, stacks);
		for (size_t i = 0; i < stacks; ++i)
		{
			crash_dump("  * ");
			crash_dump(call_names[i]);
			crash_dump("\n");
		}
		free(call_names);

		crash_dump("  ==============================================="
				"=======================\n");

		(void)context;

		/// @note Core file will dump to /tmp directory.
		(void)chdir("/tmp");

		const struct rlimit limit =
		{
			RLIM_INFINITY,
			RLIM_INFINITY
		};

		(void)setrlimit(RLIMIT_CORE, &limit);

		struct sigaction action;
		action.sa_handler = SIG_DFL;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		(void)sigaction(SIGABRT, &action, NULL);

		/// @warning This function will not return but abort().
		abort();
		// This should never be executed.
		exit(1);
	}
}

// End of file
