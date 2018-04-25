#define MS_CLASS "RTC::RtpCodecMime"
// #define MS_LOG_DEV

#include "Logger.hpp"
#include "MediaSoupError.hpp"
#include "Utils.hpp"
#include "RTC/RtpDictionaries.hpp"

namespace RTC
{
	/* Class variables. */

	// clang-format off
	std::unordered_map<std::string, RtpCodecMime::Type> RtpCodecMime::string2Type =
	{
		{ "audio", RtpCodecMime::Type::AUDIO },
		{ "video", RtpCodecMime::Type::VIDEO }
	};
	std::map<RtpCodecMime::Type, std::string> RtpCodecMime::type2String =
	{
		{ RtpCodecMime::Type::AUDIO, "audio" },
		{ RtpCodecMime::Type::VIDEO, "video" }
	};
	std::unordered_map<std::string, RtpCodecMime::Subtype> RtpCodecMime::string2Subtype =
	{
		// Audio codecs:
		{ "opus",            RtpCodecMime::Subtype::OPUS            },
		{ "pcma",            RtpCodecMime::Subtype::PCMA            },
		{ "pcmu",            RtpCodecMime::Subtype::PCMU            },
		{ "isac",            RtpCodecMime::Subtype::ISAC            },
		{ "g722",            RtpCodecMime::Subtype::G722            },
		{ "ilbc",            RtpCodecMime::Subtype::ILBC            },
		{ "silk",            RtpCodecMime::Subtype::SILK            },
		// Video codecs:
		{ "vp8",             RtpCodecMime::Subtype::VP8             },
		{ "vp9",             RtpCodecMime::Subtype::VP9             },
		{ "h264",            RtpCodecMime::Subtype::H264            },
		{ "x-h264uc",        RtpCodecMime::Subtype::X_H264UC        },
		{ "h265",            RtpCodecMime::Subtype::H265            },
		// Complementary codecs:
		{ "cn",              RtpCodecMime::Subtype::CN              },
		{ "telephone-event", RtpCodecMime::Subtype::TELEPHONE_EVENT },
		// Feature codecs:
		{ "rtx",             RtpCodecMime::Subtype::RTX             },
		{ "ulpfec",          RtpCodecMime::Subtype::ULPFEC          },
		{ "flexfec",         RtpCodecMime::Subtype::FLEXFEC         },
		{ "x-ulpfecuc",      RtpCodecMime::Subtype::X_ULPFECUC      },
		{ "red",             RtpCodecMime::Subtype::RED             }
	};
	std::map<RtpCodecMime::Subtype, std::string> RtpCodecMime::subtype2String =
	{
		// Audio codecs:
		{ RtpCodecMime::Subtype::OPUS,            "opus"            },
		{ RtpCodecMime::Subtype::PCMA,            "PCMA"            },
		{ RtpCodecMime::Subtype::PCMU,            "PCMU"            },
		{ RtpCodecMime::Subtype::ISAC,            "ISAC"            },
		{ RtpCodecMime::Subtype::G722,            "G722"            },
		{ RtpCodecMime::Subtype::ILBC,            "iLBC"            },
		{ RtpCodecMime::Subtype::SILK,            "SILK"            },
		// Video codecs:
		{ RtpCodecMime::Subtype::VP8,             "VP8"             },
		{ RtpCodecMime::Subtype::VP9,             "VP9"             },
		{ RtpCodecMime::Subtype::H264,            "H264"            },
		{ RtpCodecMime::Subtype::X_H264UC,        "X-H264UC"        },
		{ RtpCodecMime::Subtype::H265,            "H265"            },
		// Complementary codecs:
		{ RtpCodecMime::Subtype::CN,              "CN"              },
		{ RtpCodecMime::Subtype::TELEPHONE_EVENT, "telephone-event" },
		// Feature codecs:
		{ RtpCodecMime::Subtype::RTX,             "rtx"             },
		{ RtpCodecMime::Subtype::ULPFEC,          "ulpfec"          },
		{ RtpCodecMime::Subtype::FLEXFEC,         "flexfec"         },
		{ RtpCodecMime::Subtype::X_ULPFECUC,      "x-ulpfecuc"      },
		{ RtpCodecMime::Subtype::RED,             "red"             }
	};
	// clang-format on

	/* Instance methods. */

	void RtpCodecMime::SetName(const std::string& name)
	{
		MS_TRACE();

		auto slashPos = name.find('/');

		if (slashPos == std::string::npos || slashPos == 0 || slashPos == name.length() - 1)
			MS_THROW_ERROR("wrong codec MIME");

		std::string type    = name.substr(0, slashPos);
		std::string subtype = name.substr(slashPos + 1);

		// Force lowcase names.
		Utils::String::ToLowerCase(type);
		Utils::String::ToLowerCase(subtype);

		// Set MIME type.
		{
			auto it = RtpCodecMime::string2Type.find(type);

			if (it != RtpCodecMime::string2Type.end())
				this->type = it->second;
			else
				MS_THROW_ERROR("unknown codec MIME type '%s'", type.c_str());
		}

		// Set MIME subtype.
		{
			auto it = RtpCodecMime::string2Subtype.find(subtype);

			if (it != RtpCodecMime::string2Subtype.end())
				this->subtype = it->second;
			else
				MS_THROW_ERROR("unknown codec MIME subtype '%s'", subtype.c_str());
		}

		// Set name.
		this->name = type2String[this->type] + "/" + subtype2String[this->subtype];
	}
} // namespace RTC
