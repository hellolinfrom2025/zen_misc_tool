#include "zen_msic_tool_factory.h"

#define ZEN_RTTR_DLL
#include <zen_rttr/registration>

#include "zen_misc_tool_impl.h"

using namespace zen::misc;

namespace zen
{
	namespace misc
	{
		MiscToolFactoryPtr _getMiscToolFactory();
	}
}

//注册默认类型
RTTR_REGISTRATION
{
	using namespace zen_rttr;
registration::method("zen::misc::getMiscToolFactory", &zen::misc::_getMiscToolFactory);
}

std::string MiscToolFactory::version() {
	return std::string("1.0");
}

//---------------
StringToolPtr MiscToolFactory::createStringTool() {
	return std::make_shared<StringTool>();
}

//创建局部对象-----------
MiscToolFactoryPtr zen::misc::_getMiscToolFactory() {
	return std::make_shared<MiscToolFactory>();
}
