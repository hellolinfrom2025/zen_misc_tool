#ifndef __zen_misc_tool_factory_h__
#define __zen_misc_tool_factory_h__

#include "zen_misc_tool.h"

ZEN_MISC_TOOL_NAMESPACE_BEGIN

class MiscToolFactory : public IMiscToolFactory {
public:
	MiscToolFactory() = default;
	virtual ~MiscToolFactory() = default;
	virtual std::string version() override;

	virtual StringToolPtr createStringTool()override;
	virtual CodecToolPtr createCodecTool() override;
};

ZEN_MISC_TOOL_NAMESPACE_END
#endif