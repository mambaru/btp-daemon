#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <pwd.h>
#include <grp.h>

namespace common {

	std::string mkdate() {
		time_t ts = time(0);
		struct tm t1;
		localtime_r(&ts, &t1);
		char buf[100];
		int sz = strftime(buf,sizeof(buf), "[%Y-%m-%d %H:%M:%S] ",&t1);
		return std::string(buf,sz);
	}

	inline void daemonize(bool allow_stderr = false,bool allow_stdout = false) {
	  int null = ::open("/dev/null", O_RDWR);
	  if(-1 == null)
	  {
		::perror("/dev/null");
		::exit(EXIT_FAILURE);
	  }

	  switch(::fork())
	  {
	  case 0:
		::setsid();
		::umask(0);
		::close(0);
		::dup2(null, 0);
		if (!allow_stdout) {
			::close(1);
			::dup2(null, 1);
		}
		if (!allow_stderr) {
			::close(2);
			::dup2(null, 2);
		}
		break;

	  case -1:
		::perror("fork()");
		::exit(EXIT_FAILURE);

	  default:
		::exit(EXIT_SUCCESS);
	  }
	}

	void autoup() {
	    for (;;) {
	      pid_t pid = fork();
	      if (pid == 0) break;
	      int status = 0;
	      waitpid(pid, &status, 0);
	      kill(pid, SIGKILL);
	    }
	}

	void change_user(std::string user) {
		if (getuid()!= 0 && geteuid()!=0) return;

		int uid_by_name = 1;
		char *username = strdup(user.c_str());

		struct passwd *pw;
		struct passwd pws;

		// alternative "--user=uid/gid"
		int pos = user.find('/');
		if (pos>=0) {
			uid_by_name = 0;
			pws.pw_uid = atoi(user.substr(0,pos).c_str());
			pws.pw_gid = atoi(user.substr(pos+1).c_str());
			pw = &pws;
		} else {
			pw = getpwnam(user.c_str());
			if (pw == 0) {
				fprintf(stderr, "can't find the user %s to switch to\n", username);
				throw new std::exception();
			}
		}

		if (setgid(pw->pw_gid) < 0 // failed to set gid
			|| (uid_by_name ? (initgroups(username, pw->pw_gid) < 0) : (setgroups(0, NULL) < 0)) // failed to set supplementary groups
			|| setuid(pw->pw_uid) < 0) { // failed to set uid
			fprintf(stderr, "failed to assume identity of user %s\n", username);
			throw new std::exception();
		}
	}
}

double microtime() {
    timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec + tv.tv_usec/1000000.0;
}
std::string microtime_str() {
	char buf[20];
    sprintf(buf,"%.4f",microtime());
    return std::string(buf,strlen(buf));
}

void gdb_break() {
    raise(SIGINT);
}

#define log_write(string) { std::cerr << common::mkdate() << string << "\n"; std::cerr.flush(); }


#include <glob.h>

namespace misc {

std::string rtrim(const std::string &str, char ch) {
	std::string ret;
	if (str[str.length()-1] == ch) {
		ret = str.substr(0,str.length()-1);
	} else ret = str;
	return ret;
}

/*std::string str_printf(const char *format, ...) {
	va_list args;
	//va_start(args, format);
	//va_end(Numbers);
	//return (Sum/Count);
}*/

std::vector<std::string> listFilesByPattern(std::string path) {
	glob_t tglob;
	std::vector<std::string> ret;

	if (glob(path.c_str(), GLOB_MARK,NULL, &tglob)!=0) return ret;
	for (unsigned int i=0;i<tglob.gl_pathc;i++) {
		ret.push_back(tglob.gl_pathv[i]);
	}
	globfree(&tglob);
	return ret;
}
}
/*
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <sched.h>
#include <pthread.h>

namespace misc {

	void set_thread_priority(std::thread &thr, int val) {
		auto h = thr.native_handle();
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_
	}

}

#endif
*/
