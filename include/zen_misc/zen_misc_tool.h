#ifndef __zen_misc_tool_h__
#define __zen_misc_tool_h__

#define ZEN_MISC_TOOL_NAMESPACE_BEGIN namespace zen{ namespace misc {
#define ZEN_MISC_TOOL_NAMESPACE_END   }}

#include <memory>
#include <cassert>
#include <string>
#include <mutex>

#define ZEN_RTTR_DLL
#include <zen_rttr/variant.h>
#include <zen_rttr/library.h>

#define ZEN_MISC_TOOL_VERSION "1.0"

#ifdef _DEBUG
#pragma comment(lib,"zen_rttr_core_096d.lib")
#else
#pragma comment(lib,"zen_rttr_core_096.lib")
#endif // _DEBUG

ZEN_MISC_TOOL_NAMESPACE_BEGIN

class IStringTool;
class ICodecTool;
class IMiscToolFactory;
using StringToolPtr = std::shared_ptr<IStringTool>;
using CodecToolPtr = std::shared_ptr<ICodecTool>;
using MiscToolFactoryPtr = std::shared_ptr<IMiscToolFactory>;

// 1#使用局部对象
inline MiscToolFactoryPtr getMiscToolFactory(const std::string& lib_dir) {
	static std::mutex load_mutex;
	static std::unique_ptr<zen_rttr::library> loaded_lib;

	if (!lib_dir.empty()) {
		std::lock_guard<std::mutex> lock(load_mutex);
		if (!loaded_lib) {
#ifdef _DEBUG
			std::unique_ptr<zen_rttr::library> lib(new zen_rttr::library(lib_dir + "/zen_misc_toold"));
#else
			std::unique_ptr<zen_rttr::library> lib(new zen_rttr::library(lib_dir + "/zen_misc_tool"));
#endif // _DEBUG
			if (!lib->is_loaded()) {
				if (!lib->load()) {
					assert(0);
					return nullptr;
				}
			}
			loaded_lib.swap(lib);
		}
	}
	zen_rttr::method m = zen_rttr::type::get_global_method("zen::misc::getMiscToolFactory");
	if (m.is_valid()) {
		zen_rttr::variant result = m.invoke({});
		if (result.is_valid()) {
			auto ptr = result.get_value<MiscToolFactoryPtr>();
			if (ptr) {
				return ptr;
			}
		}
	}
	return nullptr;
};

class IMiscToolFactory
{
public:
	virtual ~IMiscToolFactory() = default;
	virtual std::string version()=0;

	virtual StringToolPtr createStringTool() = 0;
	virtual CodecToolPtr createCodecTool() = 0;
};

class ICodecTool {
public:
	ICodecTool() = default;
	virtual ~ICodecTool() = default;
	ICodecTool(const ICodecTool&) = delete;
	ICodecTool& operator=(const ICodecTool&) = delete;
	virtual std::string version() = 0;

	virtual std::string base64Encode(const std::string& input) = 0;
	virtual std::string base64Decode(const std::string& input) = 0;
};

class IStringTool {
public:
	IStringTool() = default;
	virtual ~IStringTool() = default;
	IStringTool(const IStringTool&) = delete;
	IStringTool& operator=(const IStringTool&) = delete;
	virtual std::string version() = 0;

	virtual std::string utf8ToLocal(const std::string& u8_string) = 0;
	virtual std::string localToUtf8(const std::string& local_string) = 0;
	virtual std::string unicode16ToUtf8(const std::wstring& wstring) = 0;
	virtual std::wstring utf8ToUnicode16(const std::string& u8_string) = 0;
	virtual std::string unicode16ToLocal(const std::wstring& wstring) = 0;
	virtual std::wstring localToUnicode16(const std::string& local_string) = 0;

	virtual bool isUtf8WithBom(const std::string& str, bool* hasBom = nullptr) = 0;
	virtual std::string utf8ToGbk(const std::string& u8_string) = 0;
	virtual std::string gbkToUtf8(const std::string& gbk_string) = 0;
	virtual std::string unicode16ToGbk(const std::wstring& wstring) = 0;
	virtual std::wstring gbkToUnicode16(const std::string& gbk_string) = 0;
};

ZEN_MISC_TOOL_NAMESPACE_END
#endif
