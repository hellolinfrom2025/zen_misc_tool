#ifndef __zen_misc_tool_impl_h__
#define __zen_misc_tool_impl_h__

#include <memory>
#include <string>

#include "zen_misc_tool.h"

ZEN_MISC_TOOL_NAMESPACE_BEGIN

class StringTool :public IStringTool {
public:
	StringTool();
	~StringTool();
	StringTool(const StringTool&) = delete;
	StringTool& operator=(const StringTool&) = delete;
	virtual std::string version() override;

	virtual std::string utf8ToLocal(const std::string& u8_string) override;
	virtual std::string localToUtf8(const std::string& local_string) override;
	virtual std::string unicode16ToUtf8(const std::wstring& wstring)override;
	virtual std::wstring utf8ToUnicode16(const std::string& u8_string) override;
	virtual std::string unicode16ToLocal(const std::wstring& wstring) override;
	virtual std::wstring localToUnicode16(const std::string& local_string)override;

	virtual bool isUtf8WithBom(const std::string& str, bool* hasBom = nullptr) override;
	virtual std::string utf8ToGbk(const std::string& u8_string) override;
	virtual std::string gbkToUtf8(const std::string& gbk_string) override;
	virtual std::string unicode16ToGbk(const std::wstring& wstring) override;
	virtual std::wstring gbkToUnicode16(const std::string& gbk_string) override;

private:
	struct Impl;
	std::unique_ptr<Impl> pimpl;
};

class CodecTool :public ICodecTool {
public:
	CodecTool();
	~CodecTool();
	CodecTool(const CodecTool&) = delete;
	CodecTool& operator=(const CodecTool&) = delete;
	virtual std::string version() override;

	virtual std::string base64Encode(const std::string& input) override;
	virtual std::string base64Decode(const std::string& input) override;

private:
	struct Impl;
	std::unique_ptr<Impl> pimpl;
};

ZEN_MISC_TOOL_NAMESPACE_END
#endif // __zen_misc_tool_impl_h__

