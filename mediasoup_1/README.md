```
[parent]: running on http://127.0.0.1:8888
ignoring "tls_dhparam" on line 6
ignoring "certfile" on line 16
ignoring "certkey" on line 17
[parent]: kore is starting up
[parent]: tasks built-in enabled
[wrk 0]: worker 0 started (cpu#0)
[wrk 0]: worker configure
[wrk 0]: A task created
[wrk 0]: A message came: mama
uv_callback_t &to_cpp init: 0
uv_callback_t &from_cpp init: 0
uv_callback_t &cb_result init: 0

Out in: D[id:345678] Settings::PrintConfiguration() | <configuration> :: 61

Out in: D[id:345678] Settings::PrintConfiguration() |   logLevel            : "debug" :: 77

Out in: D[id:345678] Settings::PrintConfiguration() |   logTags             : "info" :: 76

Out in: D[id:345678] Settings::PrintConfiguration() |   rtcIPv4             : "127.0.0.1" :: 81

Out in: D[id:345678] Settings::PrintConfiguration() |   rtcIPv6             : (unavailable) :: 83

Out in: D[id:345678] Settings::PrintConfiguration() |   rtcAnnouncedIPv4    : (unset) :: 77

Out in: D[id:345678] Settings::PrintConfiguration() |   rtcAnnouncedIPv6    : (unset) :: 77

Out in: D[id:345678] Settings::PrintConfiguration() |   rtcMinPort          : 40000 :: 75

Out in: D[id:345678] Settings::PrintConfiguration() |   rtcMaxPort          : 49999 :: 75

Out in: D[id:345678] Settings::PrintConfiguration() | </configuration> :: 62

Out in: D[id:345678] DepLibUV::PrintVersion() | loaded libuv version: "1.19.3-dev" :: 74

Out in: D[id:345678] DepOpenSSL::ClassInit() | loaded openssl version: "OpenSSL 1.0.2n  7 Dec 2017" :: 91

Out in: D[id:345678] DepLibSRTP::ClassInit() | loaded libsrtp version: "libsrtp 2.0.0" :: 78
^C[parent]: server shutting down
[parent]: waiting for workers to drain and shutdown
uv_callback_t &from_cpp fire: 0
[wrk 0]: ON_FROM_CPP data came: exit
EXIT!!!
libuv loop ended.
The loop should be ending now!
look ma, ~Loop() destructor.
[wrk 0]: Destroy m_destroy().
[wrk 0]: A message came: mama
[wrk 0]: Bye. *******

[wrk 0]: SUCCESS: And exit with success status.
[parent]: worker 0 (17827)-> status 0
[parent]: goodbye

```