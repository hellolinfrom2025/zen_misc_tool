#ifndef __zen_misc_tool_h__
#define __zen_misc_tool_h__

#define ZEN_MISC_TOOL_NAMESPACE_BEGIN namespace zen{ namespace misc {
#define ZEN_MISC_TOOL_NAMESPACE_END   }}

#include <memory>
#include <cassert>
#include <string>
#include <atomic>

#define ZEN_RTTR_DLL
#include <zen_rttr/variant.h>
#include <zen_rttr/library.h>

#ifdef _DEBUG
#pragma comment(lib,"zen_rttr_core_096d.lib")
#else
#pragma comment(lib,"zen_rttr_core_096.lib")
#endif // _DEBUG

ZEN_MISC_TOOL_NAMESPACE_BEGIN

class IStringTool;
class IMiscToolFactory;
using StringToolPtr = std::shared_ptr<IStringTool>;
using MiscToolFactoryPtr = std::shared_ptr<IMiscToolFactory>;

// 1#使用局部对象
inline MiscToolFactoryPtr getMiscToolFactory(const std::string& lib_dir) {
	static std::atomic<bool> init{ false };
	if (!init.load(std::memory_order_acquire) && !lib_dir.empty()) {
#ifdef _DEBUG
		zen_rttr::library lib(lib_dir + "/zen_misc_toold");
#else
		zen_rttr::library lib(lib_dir + "/zen_misc_tool");
#endif // _DEBUG
		if (!lib.is_loaded()) {
			if (!lib.load()) {
				auto err = lib.get_error_string().data();
				assert(0);
			}
			else {
				init.store(true, std::memory_order_release);
			}
		}
		else {
			init.store(true, std::memory_order_release);
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
	virtual std::string version()=0;

	virtual StringToolPtr createStringTool() = 0;
};

class IStringTool {
public:
	IStringTool() = default;
	IStringTool(const IStringTool&) = delete;
	IStringTool& operator=(const IStringTool&) = delete;
	virtual std::string version() = 0;

	virtual bool isUtf8(const std::string& string) = 0;
	virtual std::string utf8ToLocal(const std::string& u8_string) = 0;
	virtual std::string localToUtf8(const std::string& local_string) = 0;
	virtual std::string unicode16ToUtf8(const std::wstring& wstring) = 0;
	virtual std::wstring utf8ToUnicode16(const std::string& u8_string) = 0;
	virtual std::string unicode16ToLocal(const std::wstring& wstring) = 0;
	virtual std::wstring localToUnicode16(const std::string& local_string) = 0;
};

ZEN_MISC_TOOL_NAMESPACE_END
#endif
