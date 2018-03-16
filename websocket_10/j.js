var express=require('express');
var path=require('path');
var app=express();
app.set('port',3000);
//app.use('static',express.static(path.join(__dirname,'./html')));
app.use(express.static('html'));
var server=app.listen(app.get('port'),function(){
var port=server.address().port;
console.log("soll on port: ",port);
});
/*
wget https://github.com/openssl/openssl/releases/tag/OpenSSL_1_0_2n.tar.gz

wget http://www.openssl.org/source/openssl-1.0.2n.tar.gz
??
export LIBRPATH=/usr/local/ssl-1.0.2n/lib 
./config --prefix=/usr/local/ssl-1.0.2n --openssldir=/usr/local/ssl-1.0.2n -Wl,--enable-new-dtags,-rpath,'$(LIBRPATH)'  zlib-dynamic shared

./config --prefix=/usr/local/ssl-1.0.2n --openssldir=/usr/local/ssl-1.0.2n -Wl,--enable-new-dtags,-rpath,'$(LIBRPATH)'  zlib-dynamic shared 
-Wl,--version-script=/home/globik/openssl-1.0.2n/openssl.ld -Wl,-Bsymbolic-functions

sudo ln -s /usr/local/ssl-1.0.2n/lib/libssl.so.1.0.0 /usr/local/lib/libssl.so.1.0.0


       ./config  reconf --prefix=/usr/local/ssl-1.0.2n --openssldir=/usr/local/ssl-1.0.2n -Wl,--enable-new-dtags,-rpath,'$(LIBRPATH)' sctp srtp zlib-dynamic

  ./config shared  zlib-dynamic -Wl,-rpath=/usr/local/ssl/lib --openssldir=/usr/local/ssl

readelf -d /usr/local/ssl-1.0.2n/lib/libssl.so | grep -i rpath
readelf -d libssl.so | grep -i rpath
ldd /usr/local/ssl/bin/openssl
readelf -d /lib/i386-linux-gnu/libcrypto.so.1.0.0 | grep -i rpath

make depend
globik@globik-laptop:~/libsrtp-2.0.0$ export CFLAGS="-I/usr/local/ssl/include/ -L/usr/local/ssl/lib -Wl,-rpath=/usr/local/ssl/lib -lssl -lcrypto"
globik@globik-laptop:~/libsrtp-2.0.0$ export CXXFLAGS="-I/usr/local/ssl/include/ -L/usr/local/ssl/lib -Wl,-rpath=/usr/local/ssl/lib -lssl -lcrypto"



objdump -f /lib/i386-linux-gnu/libssl.so.1.0.0 -x | grep -i SRTP
/usr/lib/i386-linux-gnu/libssl.so
objdump -f /lib/i386-linux-gnu/libssl.so -x | grep -i SRTP
objdump -f /lib/i386-linux-gnu/libssl.so.1.0.0  -x | grep -i rtp_default
	libssl.so.1.0.0 => /lib/i386-linux-gnu/libssl.so.1.0.0 (0xb6fa0000)
	libcrypto.so.1.0.0 => /lib/i386-linux-gnu/libcrypto.so.1.0.0 (0xb6df3000)


objdump -f /usr/local/ssl-1.0.2n/lib/libssl.a  -x | grep -i ctypto_policy_set_rtp_default

objdump -f /usr/local/ssl-1.0.2n/lib/libssl.a  -x | grep -i CRYPTO_POLICY
objdump -f /usr/local/ssl-1.0.2n/lib/libcrypto.a  -x | grep -i set_rtp_default
*/


 --enable-generic-aesicm compile in changes for ISMAcryp
  --enable-openssl        compile in OpenSSL crypto engine
  --enable-openssl-kdf    Use OpenSSL KDF algorithm
  --enable-stdout  
--with-openssl-dir      Location of OpenSSL installation
export PKG_CONFIG_PATH=/usr/local/ssl-1.0.2n/lib/pkgconfig


  CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" ./configure --enable-generic-aesicm --enable-shared --enable-openssl --enable-stdout  --with-openssl-dir=/usr/local/ssl

LDFLAGS="-L/usr/local/ssl-1.0.2n/lib -Wl,-rpath /usr/local/ssl-1.0.2n/lib -lcrypto"
make shared_library

ar cr libsrtp2.a srtp/srtp.o srtp/ekt.o crypto/cipher/cipher.o crypto/cipher/null_cipher.o crypto/cipher/aes_icm_ossl.o crypto/cipher/aes_gcm_ossl.o crypto/hash/null_auth.o crypto/hash/auth.o crypto/hash/hmac_ossl.o crypto/math/datatypes.o crypto/math/stat.o crypto/kernel/crypto_kernel.o crypto/kernel/alloc.o crypto/kernel/key.o crypto/kernel/err.o crypto/replay/rdb.o crypto/replay/rdbx.o crypto/replay/ut_sim.o
ranlib libsrtp2.a


gcc -shared -o libsrtp2.so.1 -shared -Wl,-soname,libsrtp2.so.1 \
        srtp/srtp.o srtp/ekt.o crypto/cipher/cipher.o crypto/cipher/null_cipher.o crypto/cipher/aes_icm_ossl.o crypto/cipher/aes_gcm_ossl.o crypto/hash/null_auth.o \
 crypto/hash/auth.o crypto/hash/hmac_ossl.o crypto/math/datatypes.o crypto/math/stat.o crypto/kernel/crypto_kernel.o \
 crypto/kernel/alloc.o crypto/kernel/key.o crypto/kernel/err.o crypto/replay/rdb.o \
 crypto/replay/rdbx.o crypto/replay/ut_sim.o -lz -ldl -L.  -L/usr/local/ssl/lib -Wl,-rpath=/usr/local/ssl/lib -lssl -lcrypto 
if [ -n "1" ]; then \
		ln -sfn libsrtp2.so.1 libsrtp2.so; \
	fi

readelf -d libsrtp2.so | grep -i rpath

objdump -f /usr/local/ssl/lib/libssl.so -x | grep -i X509_chain_up_ref

objdump -f /usr/local/ssl/lib/libcrypto.so -x | grep -i X509_chain_up_ref
objdump -f /usr/local/ssl-1.0.2n/lib/libssl.a  -x | grep -i ctypto_policy_set_rtp_default
















