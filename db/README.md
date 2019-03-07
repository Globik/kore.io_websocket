```
ldd kore
	linux-gate.so.1 =>  (0xb76ef000)
	libcrypto.so.1.0.0 => /usr/local/ssl/lib/libcrypto.so.1.0.0 (0xb7505000)
	libpq.so.5 => /usr/local/pgsql/lib/libpq.so.5 (0xb74be000)
	libpthread.so.0 => /lib/i386-linux-gnu/libpthread.so.0 (0xb7484000)
	libdl.so.2 => /lib/i386-linux-gnu/libdl.so.2 (0xb747f000)
	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xb72cd000)
	libssl.so.1.0.0 => /usr/local/ssl/lib/libssl.so.1.0.0 (0xb7264000)
	/lib/ld-linux.so.2 (0xb76f0000)


```

# fake_Makefile

How to compile kore.c with -Wl,-rpath workaround about non standard directory
for openssl and postgres 12 (dev). I'm on Lubuntu 14 with fucking old default openssl lib.
Custom directories , openssl: usr local ssl lib (1.0.2n)! and libpq is in usr local pgsql lib
The shit.


```
ldd /usr/local/pgsql/lib/libpq.so.5
	linux-gate.so.1 =>  (0xb76f3000)
	libssl.so.1.0.0 => /usr/local/ssl/lib/libssl.so.1.0.0 (0xb7642000)
	libcrypto.so.1.0.0 => /usr/local/ssl/lib/libcrypto.so.1.0.0 (0xb7459000)
	libpthread.so.0 => /lib/i386-linux-gnu/libpthread.so.0 (0xb741f000)
	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xb726e000)
	libdl.so.2 => /lib/i386-linux-gnu/libdl.so.2 (0xb7268000)
	/lib/ld-linux.so.2 (0xb76f4000)


```

##  Postgres 12 dev

Compiled from source(from gihub cloned) with --with-openssl and --with-libraries and --with-includes for ssl crypto libs
and width LDFLAGS where I wrote -Wl,-rpath for ssl and crypto
