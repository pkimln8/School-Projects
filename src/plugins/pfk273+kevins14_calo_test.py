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


c.sendline("calo");
assert c.expect("Need three arguments \nSuccess\n") == 0, "Shell didnt produce the right result"

c.sendline("calo 200 150 50");
assert c.expect("1850cal is total calories for 200grams of carbs, 200grams of prtein and 50grams of fat.") == 0, "Shell didnt produce the right result"

c.sendline("calo 300 100 80");
assert c.expect("2320cal is total calories for 300grams of carbs, 100grams of prtein and 80grams of fat.") == 0, "Shell didnt produce the right result"

c.sendline("calo 325 56 73");
assert c.expect("2181cal is total calories for 325grams of carbs, 56grams of prtein and 73grams of fat.") == 0, "Shell didnt produce the right result"


shellio.success()
