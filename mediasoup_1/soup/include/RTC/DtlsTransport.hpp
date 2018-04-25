#ifndef MS_RTC_DTLS_TRANSPORT_HPP
#define MS_RTC_DTLS_TRANSPORT_HPP
#ifdef __cplusplus
#include "common.hpp"
#include "RTC/SrtpSession.hpp"
#include "handles/Timer.hpp"
#include <json/json.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <map>
#include <string>
#include <vector>

namespace RTC
{
	class DtlsTransport : public Timer::Listener
	{
	public:
		enum class DtlsState
		{
			NEW = 1,
			CONNECTING,
			CONNECTED,
			FAILED,
			CLOSED
		};

	public:
		enum class Role
		{
			NONE = 0,
			AUTO = 1,
			CLIENT,
			SERVER
		};

	public:
		enum class FingerprintAlgorithm
		{
			NONE = 0,
			SHA1 = 1,
			SHA224,
			SHA256,
			SHA384,
			SHA512
		};

	public:
		struct Fingerprint
		{
			FingerprintAlgorithm algorithm;
			std::string value;
		};

	private:
		struct SrtpProfileMapEntry
		{
			RTC::SrtpSession::Profile profile;
			const char* name;
		};

	public:
		class Listener
		{
		public:
			// DTLS is in the process of negotiating a secure connection. Incoming
			// media can flow through.
			// NOTE: The caller MUST NOT call any method during this callback.
			virtual void OnDtlsConnecting(const RTC::DtlsTransport* dtlsTransport) = 0;
			// DTLS has completed negotiation of a secure connection (including DTLS-SRTP
			// and remote fingerprint verification). Outgoing media can now flow through.
			// NOTE: The caller MUST NOT call any method during this callback.
			virtual void OnDtlsConnected(
			    const RTC::DtlsTransport* dtlsTransport,
			    RTC::SrtpSession::Profile srtpProfile,
			    uint8_t* srtpLocalKey,
			    size_t srtpLocalKeyLen,
			    uint8_t* srtpRemoteKey,
			    size_t srtpRemoteKeyLen,
			    std::string& remoteCert) = 0;
			// The DTLS connection has been closed as the result of an error (such as a
			// DTLS alert or a failure to validate the remote fingerprint).
			// NOTE: The caller MUST NOT call Destroy() during this callback.
			virtual void OnDtlsFailed(const RTC::DtlsTransport* dtlsTransport) = 0;
			// The DTLS connection has been closed due to receipt of a close_notify alert.
			// NOTE: The caller MUST NOT call Destroy() during this callback.
			virtual void OnDtlsClosed(const RTC::DtlsTransport* dtlsTransport) = 0;
			// Need to send DTLS data to the peer.
			// NOTE: The caller MUST NOT call Destroy() during this callback.
			virtual void OnOutgoingDtlsData(
			    const RTC::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) = 0;
			// DTLS application data received.
			// NOTE: The caller MUST NOT call Destroy() during this callback.
			virtual void OnDtlsApplicationData(
			    const RTC::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) = 0;
		};

	public:
		static void ClassInit();
		static void ClassDestroy();
		static Json::Value& GetLocalFingerprints();
		static Role StringToRole(const std::string& role);
		static FingerprintAlgorithm GetFingerprintAlgorithm(const std::string& fingerprint);
		static bool IsDtls(const uint8_t* data, size_t len);

	private:
		static void GenerateCertificateAndPrivateKey();
		static void ReadCertificateAndPrivateKeyFromFiles();
		static void CreateSslCtx();
		static void GenerateFingerprints();

	private:
		static X509* certificate;
		static EVP_PKEY* privateKey;
		static SSL_CTX* sslCtx;
		static uint8_t sslReadBuffer[];
		static std::map<std::string, Role> string2Role;
		static std::map<std::string, FingerprintAlgorithm> string2FingerprintAlgorithm;
		static Json::Value localFingerprints;
		static std::vector<SrtpProfileMapEntry> srtpProfiles;

	public:
		explicit DtlsTransport(Listener* listener);

	private:
		~DtlsTransport() override;

	public:
		void Destroy();
		void Dump() const;
		void Run(Role localRole);
		void SetRemoteFingerprint(Fingerprint fingerprint);
		void ProcessDtlsData(const uint8_t* data, size_t len);
		DtlsState GetState() const;
		Role GetLocalRole() const;
		void SendApplicationData(const uint8_t* data, size_t len);

	private:
		bool IsRunning() const;
		void Reset();
		bool CheckStatus(int returnCode);
		void SendPendingOutgoingDtlsData();
		bool SetTimeout();
		void ProcessHandshake();
		bool CheckRemoteFingerprint();
		void ExtractSrtpKeys(RTC::SrtpSession::Profile srtpProfile);
		RTC::SrtpSession::Profile GetNegotiatedSrtpProfile();

		/* Callbacks fired by OpenSSL events. */
	public:
		void OnSslInfo(int where, int ret);

		/* Pure virtual methods inherited from Timer::Listener. */
	public:
		void OnTimer(Timer* timer) override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Allocated by this.
		SSL* ssl{ nullptr };
		BIO* sslBioFromNetwork{ nullptr }; // The BIO from which ssl reads.
		BIO* sslBioToNetwork{ nullptr };   // The BIO in which ssl writes.
		Timer* timer{ nullptr };
		// Others.
		DtlsState state{ DtlsState::NEW };
		Role localRole{ Role::NONE };
		Fingerprint remoteFingerprint{ FingerprintAlgorithm::NONE, "" };
		bool handshakeDone{ false };
		bool handshakeDoneNow{ false };
		std::string remoteCert;
	};

	/* Inline static methods. */

	inline Json::Value& DtlsTransport::GetLocalFingerprints()
	{
		return DtlsTransport::localFingerprints;
	}

	inline DtlsTransport::Role DtlsTransport::StringToRole(const std::string& role)
	{
		auto it = DtlsTransport::string2Role.find(role);

		if (it != DtlsTransport::string2Role.end())
			return it->second;
		else
			return DtlsTransport::Role::NONE;
	}

	inline DtlsTransport::FingerprintAlgorithm DtlsTransport::GetFingerprintAlgorithm(
	    const std::string& fingerprint)
	{
		auto it = DtlsTransport::string2FingerprintAlgorithm.find(fingerprint);

		if (it != DtlsTransport::string2FingerprintAlgorithm.end())
			return it->second;
		else
			return DtlsTransport::FingerprintAlgorithm::NONE;
	}

	inline bool DtlsTransport::IsDtls(const uint8_t* data, size_t len)
	{
		return (
		    // Minimum DTLS record length is 13 bytes.
		    (len >= 13) &&
		    // DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
		    (data[0] > 19 && data[0] < 64));
	}

	/* Inline instance methods. */

	inline DtlsTransport::DtlsState DtlsTransport::GetState() const
	{
		return this->state;
	}

	inline DtlsTransport::Role DtlsTransport::GetLocalRole() const
	{
		return this->localRole;
	}

	inline bool DtlsTransport::IsRunning() const
	{
		switch (this->state)
		{
			case DtlsState::NEW:
				return false;
			case DtlsState::CONNECTING:
			case DtlsState::CONNECTED:
				return true;
			case DtlsState::FAILED:
			case DtlsState::CLOSED:
				return false;
		}

		// Make GCC 4.9 happy.
		return false;
	}
} // namespace RTC

#endif

#ifdef __cplusplus
extern "C"
{
#endif
void rtc_dtls_transport_class_init(void);
void rtc_dtls_transport_class_destroy(void);
#ifdef __cplusplus
}
#endif
#endif
