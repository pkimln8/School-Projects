#!/user/bin/python
import errno
import os
import sys

# the script will be invoked with four arguments:
# argv[1]: the name of the hosting shell's eshoutput.py file
# argv[2]: a string " -p dirname" where dirname is passed to stdriver.py
#		this will be passed on to the shell verbatim.
esh_output_filename = sys.argv[1]
shell_arguments = sys.argv[2]

import imp.atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");

#Ensure the shell process is terminated
def force_shell_termination(shell_process):
	c.close(force=True)

#pulling in the regular expression and other definitions
def_module = imp.load_source('', esh_output_filename)
logfile = None
if hasattr(def_module, 'logfile'):
	logfile = def_module.logfile

#spawn an instance of the shell
c = pexpect.spawn(def_module.shell + shell_arguments, drainpty=True, logfile=logfile)
#c = pexpect.spawn("bash", drainpty=True, logfile=logfile)

atexit.register(force_shell_termination, shell_process=c)

#set timeout for all following 'expect*' calls to 2 seconds
c.timeout = 2
#c.setecho(False)

# ensure that shell prints expected prompt
print def_module.promt
assert c.expect(def_module.promt) == 0, "Shell did not print expected prompt (1)"
##############################################
#
# End of Boilerplate
#
##############################################


c.sendline("second");
assert c.expect("Need an argument \nSuccess\n") == 0, "Shell didnt produce the right result"

c.sendline("second 10000");
assert c.expect("10000sec is 0day 2hour 46min 40sec.") == 0, "Shell didnt produce the right result"

c.sendline("second 50000");
assert c.expect("50000sec is 0day 13hour 53min 20sec.") == 0, "Shell didnt produce the right result"

c.sendline("second 100000");
assert c.expect("100000sec is 1day 3hour 46min 40sec.") == 0, "Shell didnt produce the right result"

c.sendline("second abcde");
assert c.expect("0sec is 0day 0hour 0min 0sec.") == 0, "Shell didnt produce the right result"


shellio.success()
