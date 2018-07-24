## libpq
asynchronous LISTEN interface of libpq in a dedicated
thread. Using select for the time being. 
In conjuction with build-in kore's postgresql driver - asynchronous interface.
kore_task_channel_write(),
Server Sent Events,
Ajax request(GET) with some parameters.

## note

Not for a "production". Just experiment. One should a little change the source code for using this possibility
in a "production mode" just like this:

[how to reserve one long-live dadicated thread](https://gist.github.com/Globik/80fc3c9b83877a76c46e61ed1ccc78dd)
A dedicated long-live thread must not to be inserted in a pool(in a list) like jorisvink does.
