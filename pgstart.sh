#!/bin/sh
#export PATH=/usr/lib/postgresql/9.6/bin:$PATH
#export PGDATA=$HOME/data
export PATH=/usr/lib/postgresql/10/bin:$PATH
export PGDATA=$HOME/data10
#/usr/lib/postgresql/9.6/bin/pg_ctl start -D $HOME/data
pg_ctl start
exit 0

# creating cluster directory right after sudo apt-get install postgresql 
# pg_ctl -D /home/globik/data10 initdb
# pg_ctl status [check if server running 
# psql -l [list all databases
# psql postgres [connect to databes 'postgres'
# select*from users; [dummy command
# \q [quit
# pg_ctl stop
# .bash_aliases
#alias dab="ls"
#alias pgstart="sudo chmod a+w /var/run/postgresql && /usr/lib/postgresql/9.6/bin/pg_ctl start -D $HOME/data"
#alias pgstop="/usr/lib/postgresql/9.6/bin/pg_ctl stop -D $HOME/data"
