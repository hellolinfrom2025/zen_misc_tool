#include "zen_misc_tool_impl.h"

#include <memory>
#include <iostream>
#include <boost/locale.hpp>

using namespace zen::misc;

////////////////////////////////////////////////
struct StringTool::Impl {
	Impl() {
		// 构造并缓存系统 locale，避免每次转换时重复构造 generator（高开销操作）
		boost::locale::generator gen;
		systemLocale_ = gen(boost::locale::util::get_system_locale());
	}

	~Impl() {

	}

	std::string utf8ToLocal(const std::string& u8_string) {
		try {
			return boost::locale::conv::from_utf(u8_string, systemLocale_);
		} catch (const boost::locale::conv::conversion_error& e) {
			std::cerr << "Boost locale conversion error: " << e.what() << std::endl;
		} catch (const std::runtime_error& e) {
			std::cerr << "Runtime error: " << e.what() << std::endl;
		} catch (const std::exception& e) {
			std::cerr << "Standard exception: " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Unknown exception occurred" << std::endl;
		}
		return std::string(); // 返回空字符串表示转换失败
	}
	std::string localToUtf8(const std::string& local_string) {
		try {
			return boost::locale::conv::to_utf<char>(local_string, systemLocale_);
		} catch (const boost::locale::conv::conversion_error& e) {
			std::cerr << "Boost locale conversion error: " << e.what() << std::endl;
		} catch (const std::runtime_error& e) {
			std::cerr << "Runtime error: " << e.what() << std::endl;
		} catch (const std::exception& e) {
			std::cerr << "Standard exception: " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Unknown exception occurred" << std::endl;
		}
		return std::string(); // 返回空字符串表示转换失败
	}
	std::string unicode16ToUtf8(const std::wstring& wstring) {
		try {
			return boost::locale::conv::utf_to_utf<char>(wstring);
		} catch (const boost::locale::conv::conversion_error& e) {
			std::cerr << "Boost locale conversion error: " << e.what() << std::endl;
		} catch (const std::exception& e) {
			std::cerr << "Standard exception: " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Unknown exception occurred" << std::endl;
		}
		return std::string(); // 返回空字符串表示转换失败
	}
	std::wstring utf8ToUnicode16(const std::string& u8_string) {
		try {
			return boost::locale::conv::utf_to_utf<wchar_t>(u8_string);
		} catch (const boost::locale::conv::conversion_error& e) {
			std::cerr << "Boost locale conversion error: " << e.what() << std::endl;
		} catch (const std::exception& e) {
			std::cerr << "Standard exception: " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Unknown exception occurred" << std::endl;
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

private:
	std::locale systemLocale_; // 缓存系统 locale，避免每次调用重复构造 generator
};

////////////////////////////////////////////////

StringTool::StringTool()
	: pimpl(std::make_unique<Impl>()) {}

StringTool::~StringTool() {}

std::string StringTool::version() {
	return std::string("1.0");
}

bool StringTool::isUtf8(const std::string& string) {
	// 空串是合法的 UTF-8（空序列）
	if (string.empty()) {
		return true;
	}

	bool bIsUTF8 = true;
	const unsigned char* pStart = reinterpret_cast< const unsigned char* >( string.data() );
	const unsigned char* pEnd = pStart + string.length();

	while (pStart < pEnd) {
		if (*pStart < 0x80)         // 值小于0x80的为ASCII字符 
		{
			pStart++;
		}
		else if (*pStart < ( 0xC0 ))  // 值介于0x80与0xC0之间的为无效UTF-8字符
		{
			bIsUTF8 = false;
			break;
		}
		else if (*pStart < ( 0xE0 ))  // 此范围内为2字节UTF-8字符
		{
			if (pStart >= pEnd - 1) {
				// 截断的多字节序列：非合法 UTF-8
				bIsUTF8 = false;
				break;
			}

			if (( pStart[1] & ( 0xC0 ) ) != 0x80) {
				bIsUTF8 = false;
				break;
			}

			pStart += 2;
		}
		else if (*pStart < ( 0xF0 )) // 此范围内为3字节UTF-8字符
		{
			if (pStart >= pEnd - 2) {
				// 截断的多字节序列：非合法 UTF-8
				bIsUTF8 = false;
				break;
			}

			if (( pStart[1] & ( 0xC0 ) ) != 0x80 || ( pStart[2] & ( 0xC0 ) ) != 0x80) {
				bIsUTF8 = false;
				break;
			}

			pStart += 3;
		}
		else if (*pStart < ( 0xF8 )) // 此范围内为4字节UTF-8字符
		{
			if (pStart >= pEnd - 3) {
				// 截断的多字节序列：非合法 UTF-8
				bIsUTF8 = false;
				break;
			}

			if (( pStart[1] & ( 0xC0 ) ) != 0x80 ||
				( pStart[2] & ( 0xC0 ) ) != 0x80 ||
				( pStart[3] & ( 0xC0 ) ) != 0x80) {
				bIsUTF8 = false;
				break;
			}

			pStart += 4;
		}
		else {
			bIsUTF8 = false;
			break;
		}
	}
	return bIsUTF8;
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
