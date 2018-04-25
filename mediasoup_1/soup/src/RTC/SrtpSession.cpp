#define MS_CLASS "RTC::SrtpSession"
// #define MS_LOG_DEV

#include "RTC/SrtpSession.hpp"
#include "DepLibSRTP.hpp"
#include "Logger.hpp"
#include "MediaSoupError.hpp"
#include <cstring> // std::memset(), std::memcpy()

namespace RTC
{
	/* Static. */

	static constexpr size_t EncryptBufferSize{ 65536 };
	static uint8_t EncryptBuffer[EncryptBufferSize];

	/* Class methods. */

	void SrtpSession::ClassInit()
	{
		// Set libsrtp event handler.

		srtp_err_status_t err;

		err = srtp_install_event_handler(static_cast<srtp_event_handler_func_t*>(OnSrtpEvent));
		if (DepLibSRTP::IsError(err))
			MS_THROW_ERROR("srtp_install_event_handler() failed: %s", DepLibSRTP::GetErrorString(err));
	}

	void SrtpSession::OnSrtpEvent(srtp_event_data_t* data)
	{
		MS_TRACE();

		switch (data->event)
		{
			case event_ssrc_collision:
				MS_WARN_TAG(srtp, "SSRC collision occurred");
				break;
			case event_key_soft_limit:
				MS_WARN_TAG(srtp, "stream reached the soft key usage limit and will expire soon");
				break;
			case event_key_hard_limit:
				MS_WARN_TAG(srtp, "stream reached the hard key usage limit and has expired");
				break;
			case event_packet_index_limit:
				MS_WARN_TAG(srtp, "stream reached the hard packet limit (2^48 packets)");
				break;
		}
	}

	/* Instance methods. */

	SrtpSession::SrtpSession(Type type, Profile profile, uint8_t* key, size_t keyLen)
	{
		MS_TRACE();

		srtp_err_status_t err;
		srtp_policy_t policy{};

		// Set all policy fields to 0.
		std::memset(&policy, 0, sizeof(srtp_policy_t));

		switch (profile)
		{
			case Profile::AES_CM_128_HMAC_SHA1_80:
				srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtp);
				srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);
				break;
			case Profile::AES_CM_128_HMAC_SHA1_32:
				srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy.rtp);
				srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp); // NOTE: Must be 80 for RTCP!.
				break;
			default:
				MS_ABORT("unknown SRTP suite");
		}

		MS_ASSERT(
		    (int)keyLen == policy.rtp.cipher_key_len,
		    "given keyLen does not match policy.rtp.cipher_keyLen");

		switch (type)
		{
			case Type::INBOUND:
				policy.ssrc.type = ssrc_any_inbound;
				break;
			case Type::OUTBOUND:
				policy.ssrc.type = ssrc_any_outbound;
				break;
		}

		policy.ssrc.value = 0;
		policy.key        = key;
		// Required for sending RTP retransmission without RTX.
		policy.allow_repeat_tx = 1;
		policy.window_size     = 2048;
		policy.next            = nullptr;

		// Set the SRTP session.
		err = srtp_create(&this->session, &policy);
		if (DepLibSRTP::IsError(err))
			MS_THROW_ERROR("srtp_create() failed: %s", DepLibSRTP::GetErrorString(err));
	}

	SrtpSession::~SrtpSession()
	{
		MS_TRACE();

		if (this->session != nullptr)
		{
			srtp_err_status_t err;

			err = srtp_dealloc(this->session);
			if (DepLibSRTP::IsError(err))
				MS_ABORT("srtp_dealloc() failed: %s", DepLibSRTP::GetErrorString(err));
		}
	}

	void SrtpSession::Destroy()
	{
		MS_TRACE();

		delete this;
	}

	bool SrtpSession::EncryptRtp(const uint8_t** data, size_t* len)
	{
		MS_TRACE();

		// Ensure that the resulting SRTP packet fits into the encrypt buffer.
		if (*len + SRTP_MAX_TRAILER_LEN > EncryptBufferSize)
		{
			MS_WARN_TAG(srtp, "cannot encrypt RTP packet, size too big (%zu bytes)", *len);

			return false;
		}

		std::memcpy(EncryptBuffer, *data, *len);

		srtp_err_status_t err;

		err = srtp_protect(this->session, (void*)EncryptBuffer, reinterpret_cast<int*>(len));
		if (DepLibSRTP::IsError(err))
		{
			MS_WARN_TAG(srtp, "srtp_protect() failed: %s", DepLibSRTP::GetErrorString(err));

			return false;
		}

		// Update the given data pointer.
		*data = (const uint8_t*)EncryptBuffer;

		return true;
	}

	bool SrtpSession::DecryptSrtp(const uint8_t* data, size_t* len)
	{
		MS_TRACE();

		srtp_err_status_t err;

		err = srtp_unprotect(this->session, (void*)data, reinterpret_cast<int*>(len));
		if (DepLibSRTP::IsError(err))
		{
			MS_DEBUG_TAG(srtp, "srtp_unprotect() failed: %s", DepLibSRTP::GetErrorString(err));

			return false;
		}

		return true;
	}

	bool SrtpSession::EncryptRtcp(const uint8_t** data, size_t* len)
	{
		MS_TRACE();

		// Ensure that the resulting SRTCP packet fits into the encrypt buffer.
		if (*len + SRTP_MAX_TRAILER_LEN > EncryptBufferSize)
		{
			MS_WARN_TAG(srtp, "cannot encrypt RTCP packet, size too big (%zu bytes)", *len);

			return false;
		}

		std::memcpy(EncryptBuffer, *data, *len);

		srtp_err_status_t err;

		err = srtp_protect_rtcp(this->session, (void*)EncryptBuffer, reinterpret_cast<int*>(len));
		if (DepLibSRTP::IsError(err))
		{
			MS_WARN_TAG(srtp, "srtp_protect_rtcp() failed: %s", DepLibSRTP::GetErrorString(err));

			return false;
		}

		// Update the given data pointer.
		*data = (const uint8_t*)EncryptBuffer;

		return true;
	}

	bool SrtpSession::DecryptSrtcp(const uint8_t* data, size_t* len)
	{
		MS_TRACE();

		srtp_err_status_t err;

		err = srtp_unprotect_rtcp(this->session, (void*)data, reinterpret_cast<int*>(len));
		if (DepLibSRTP::IsError(err))
		{
			MS_DEBUG_TAG(srtp, "srtp_unprotect_rtcp() failed: %s", DepLibSRTP::GetErrorString(err));

			return false;
		}

		return true;
	}
} // namespace RTC
//c wrapper
void rtc_srtp_session_class_init(){
RTC::SrtpSession::ClassInit();}
