#include "zen_misc_tool_impl.h"

#include <memory>
#include <boost/locale.hpp>

using namespace zen::misc;

////////////////////////////////////////////////
struct StringTool::Impl {
	Impl() {
		// 构造并缓存系统 locale，避免每次转换时重复构造 generator（高开销操作）
		boost::locale::generator gen;
		systemLocale_ = gen(boost::locale::util::get_system_locale());
		try {
			gbkLocale_ = gen("zh_CN.GBK");
		} catch (...) {
			try {
				gbkLocale_ = gen("Chinese_China.936");
			} catch (...) {
				gbkLocale_ = systemLocale_;
			}
		}
	}

	~Impl() {

	}

	std::string utf8ToLocal(const std::string& u8_string) {
		try {
			return boost::locale::conv::from_utf(u8_string, systemLocale_);
		} catch (...) {
		}
		return std::string(); // 返回空字符串表示转换失败
	}
	std::string localToUtf8(const std::string& local_string) {
		try {
			return boost::locale::conv::to_utf<char>(local_string, systemLocale_);
		} catch (...) {
		}
		return std::string(); // 返回空字符串表示转换失败
	}
	std::string unicode16ToUtf8(const std::wstring& wstring) {
		try {
			return boost::locale::conv::utf_to_utf<char>(wstring);
		} catch (...) {
		}
		return std::string(); // 返回空字符串表示转换失败
	}
	std::wstring utf8ToUnicode16(const std::string& u8_string) {
		try {
			return boost::locale::conv::utf_to_utf<wchar_t>(u8_string);
		} catch (...) {
		}
		return std::wstring();
	}
	std::string unicode16ToLocal(const std::wstring& wstring) {
		auto u8str = unicode16ToUtf8(wstring);
		if (u8str.empty()) {
			return std::string();
		}
		return utf8ToLocal(u8str);
	}
	std::wstring localToUnicode16(const std::string& local_string) {
		auto u8str = localToUtf8(local_string);
		if (u8str.empty()) {
			return std::wstring();
		}
		return utf8ToUnicode16(u8str);
	}

	bool isUtf8WithBom(const std::string& str, bool* hasBom) {
		if (hasBom) *hasBom = false;
		if (str.empty()) return true;
		size_t i = 0;
		if (str.size() >= 3 &&
		    static_cast<unsigned char>(str[0]) == 0xEF &&
		    static_cast<unsigned char>(str[1]) == 0xBB &&
		    static_cast<unsigned char>(str[2]) == 0xBF) {
			if (hasBom) *hasBom = true;
			i = 3;
		}
		return isUtf8Bytes(str.data() + i, str.size() - i);
	}

	bool isUtf8Bytes(const char* data, size_t size) {
		const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
		const unsigned char* end = p + size;
		while (p < end) {
			unsigned char c = *p;
			int nBytes = 0;
			unsigned int codePoint = 0;
			if (c <= 0x7F) {
				p++;
				continue;
			} else if ((c & 0xE0) == 0xC0) {
				nBytes = 2;
				codePoint = c & 0x1F;
			} else if ((c & 0xF0) == 0xE0) {
				nBytes = 3;
				codePoint = c & 0x0F;
			} else if ((c & 0xF8) == 0xF0) {
				nBytes = 4;
				codePoint = c & 0x07;
			} else {
				return false;
			}
			if (p + nBytes > end) return false;
			for (int j = 1; j < nBytes; j++) {
				if ((p[j] & 0xC0) != 0x80) return false;
				codePoint = (codePoint << 6) | (p[j] & 0x3F);
			}
			if (nBytes == 2 && codePoint < 0x0080) return false;
			if (nBytes == 3 && codePoint < 0x0800) return false;
			if (nBytes == 4 && codePoint < 0x10000) return false;
			if (codePoint >= 0xD800 && codePoint <= 0xDFFF) return false;
			if (codePoint > 0x10FFFF) return false;
			p += nBytes;
		}
		return true;
	}

	std::string utf8ToGbk(const std::string& u8_string) {
		try {
			return boost::locale::conv::from_utf(u8_string, gbkLocale_);
		} catch (...) {
		}
		return std::string();
	}

	std::string gbkToUtf8(const std::string& gbk_string) {
		try {
			return boost::locale::conv::to_utf<char>(gbk_string, gbkLocale_);
		} catch (...) {
		}
		return std::string();
	}

	std::string unicode16ToGbk(const std::wstring& wstring) {
		auto u8str = unicode16ToUtf8(wstring);
		if (u8str.empty()) {
			return std::string();
		}
		return utf8ToGbk(u8str);
	}

	std::wstring gbkToUnicode16(const std::string& gbk_string) {
		auto u8str = gbkToUtf8(gbk_string);
		if (u8str.empty()) {
			return std::wstring();
		}
		return utf8ToUnicode16(u8str);
	}

private:
	std::locale systemLocale_; // 缓存系统 locale，避免每次调用重复构造 generator
	std::locale gbkLocale_;    // 缓存 GBK locale
};

////////////////////////////////////////////////

StringTool::StringTool()
	: pimpl(std::make_unique<Impl>()) {}

StringTool::~StringTool() {}

std::string StringTool::version() {
	return std::string(ZEN_MISC_TOOL_VERSION);
}

std::string StringTool::utf8ToLocal(const std::string& u8_string) {
	return pimpl->utf8ToLocal(u8_string);
}

std::string StringTool::localToUtf8(const std::string& local_string) {
	return pimpl->localToUtf8(local_string);
}

std::string StringTool::unicode16ToUtf8(const std::wstring& wstring) {
	return pimpl->unicode16ToUtf8(wstring);
}

std::wstring StringTool::utf8ToUnicode16(const std::string& u8_string) {
	return pimpl->utf8ToUnicode16(u8_string);
}

std::string StringTool::unicode16ToLocal(const std::wstring& wstring) {
	return pimpl->unicode16ToLocal(wstring);
}

std::wstring StringTool::localToUnicode16(const std::string& local_string) {
	return pimpl->localToUnicode16(local_string);
}

bool StringTool::isUtf8WithBom(const std::string& str, bool* hasBom) {
	return pimpl->isUtf8WithBom(str, hasBom);
}

std::string StringTool::utf8ToGbk(const std::string& u8_string) {
	return pimpl->utf8ToGbk(u8_string);
}

std::string StringTool::gbkToUtf8(const std::string& gbk_string) {
	return pimpl->gbkToUtf8(gbk_string);
}

std::string StringTool::unicode16ToGbk(const std::wstring& wstring) {
	return pimpl->unicode16ToGbk(wstring);
}

std::wstring StringTool::gbkToUnicode16(const std::string& gbk_string) {
	return pimpl->gbkToUnicode16(gbk_string);
}

////////////////////////////////////////////////
struct CodecTool::Impl {
	Impl() {}
	~Impl() {}

	std::string base64Encode(const std::string& input) {
		static const std::string base64Chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string ret;
		const auto* bytes = reinterpret_cast<const unsigned char*>(input.data());
		size_t len = input.size();
		size_t pos = 0;

		while (pos < len) {
			unsigned char b0 = bytes[pos++];
			unsigned char b1 = (pos < len) ? bytes[pos++] : 0;
			unsigned char b2 = (pos < len) ? bytes[pos++] : 0;
			ret += base64Chars[(b0 >> 2) & 0x3F];
			ret += base64Chars[((b0 & 0x03) << 4) | ((b1 >> 4) & 0x0F)];
			ret += base64Chars[((b1 & 0x0F) << 2) | ((b2 >> 6) & 0x03)];
			ret += base64Chars[b2 & 0x3F];
		}

		size_t mod = input.size() % 3;
		if (mod == 1) {
			ret[ret.size() - 2] = '=';
			ret[ret.size() - 1] = '=';
		} else if (mod == 2) {
			ret[ret.size() - 1] = '=';
		}
		return ret;
	}

	std::string base64Decode(const std::string& input) {
		auto isBase64 = [](unsigned char c) {
			return (isalnum(c) || c == '+' || c == '/');
		};

		static const std::string base64Chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		std::string ret;
		int i = 0;
		unsigned char char4[4], char3[3];
		size_t pos = 0;
		size_t len = input.size();

		while (pos < len && input[pos] != '=' && isBase64(static_cast<unsigned char>(input[pos]))) {
			char4[i++] = static_cast<unsigned char>(input[pos++]);
			if (i == 4) {
				for (int j = 0; j < 4; j++)
					char4[j] = static_cast<unsigned char>(base64Chars.find(char4[j]));
				char3[0] = (char4[0] << 2) | (char4[1] >> 4);
				char3[1] = (char4[1] << 4) | (char4[2] >> 2);
				char3[2] = (char4[2] << 6) | char4[3];
				for (int j = 0; j < 3; j++)
					ret += char3[j];
				i = 0;
			}
		}

		if (i > 0) {
			for (int j = i; j < 4; j++) char4[j] = 0;
			for (int j = 0; j < 4; j++)
				char4[j] = static_cast<unsigned char>(base64Chars.find(char4[j]));
			char3[0] = (char4[0] << 2) | (char4[1] >> 4);
			char3[1] = (char4[1] << 4) | (char4[2] >> 2);
			char3[2] = (char4[2] << 6) | char4[3];
			for (int j = 0; j < i - 1; j++)
				ret += char3[j];
		}
		return ret;
	}
};

////////////////////////////////////////////////

CodecTool::CodecTool()
	: pimpl(std::make_unique<Impl>()) {}

CodecTool::~CodecTool() {}

std::string CodecTool::version() {
	return std::string(ZEN_MISC_TOOL_VERSION);
}

std::string CodecTool::base64Encode(const std::string& input) {
	return pimpl->base64Encode(input);
}

std::string CodecTool::base64Decode(const std::string& input) {
	return pimpl->base64Decode(input);
}
