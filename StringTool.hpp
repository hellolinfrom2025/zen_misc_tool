#pragma once
#ifndef StringTool_h__
#define StringTool_h__
#include <string>
#include <fstream>
#include <vector>
#include <climits>
#include <cstring>
#include <limits>

namespace  StringToolDetail
{
	// =====================================================================
// 工具函数：Base64 编解码（纯标准库实现）
// =====================================================================

	static const std::string s_kBase64Chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	static std::string _Base64Encode(const std::string& input)
	{
		std::string ret;
		const auto* bytes = reinterpret_cast< const unsigned char* >( input.data() );
		size_t len = input.size();
		size_t pos = 0;

		while( pos < len ) {
			unsigned char b0 = bytes[pos++];
			unsigned char b1 = ( pos < len ) ? bytes[pos++] : 0;
			unsigned char b2 = ( pos < len ) ? bytes[pos++] : 0;
			int count = 1 + ( b1 != 0 || pos - 1 < len ? 1 : 0 );
			// 重新正确计算实际字节数
			// 简化：直接按剩余量判断
			ret += s_kBase64Chars[( b0 >> 2 ) & 0x3F];
			ret += s_kBase64Chars[( ( b0 & 0x03 ) << 4 ) | ( ( b1 >> 4 ) & 0x0F )];
			ret += s_kBase64Chars[( ( b1 & 0x0F ) << 2 ) | ( ( b2 >> 6 ) & 0x03 )];
			ret += s_kBase64Chars[b2 & 0x3F];
		}

		// 补 padding
		size_t mod = input.size() % 3;
		if( mod == 1 ) { ret[ret.size() - 2] = '='; ret[ret.size() - 1] = '='; } else if( mod == 2 ) { ret[ret.size() - 1] = '='; }

		return ret;
	}

	static std::string _Base64Decode(const std::string& input)
	{
		auto isBase64 = [](unsigned char c) {
			return ( isalnum(c) || c == '+' || c == '/' );
		};

		std::string ret;
		int i = 0;
		unsigned char char4[4], char3[3];
		size_t pos = 0;
		size_t len = input.size();

		while( pos < len && input[pos] != '=' && isBase64(input[pos]) ) {
			char4[i++] = static_cast< unsigned char >( input[pos++] );
			if( i == 4 ) {
				for( int j = 0; j < 4; j++ )
					char4[j] = static_cast< unsigned char >( s_kBase64Chars.find(char4[j]) );
				char3[0] = ( char4[0] << 2 ) | ( char4[1] >> 4 );
				char3[1] = ( char4[1] << 4 ) | ( char4[2] >> 2 );
				char3[2] = ( char4[2] << 6 ) | char4[3];
				for( int j = 0; j < 3; j++ )
					ret += char3[j];
				i = 0;
			}
		}

		if( i > 0 ) {
			for( int j = i; j < 4; j++ ) char4[j] = 0;
			for( int j = 0; j < 4; j++ )
				char4[j] = static_cast< unsigned char >( s_kBase64Chars.find(char4[j]) );
			char3[0] = ( char4[0] << 2 ) | ( char4[1] >> 4 );
			char3[1] = ( char4[1] << 4 ) | ( char4[2] >> 2 );
			char3[2] = ( char4[2] << 6 ) | char4[3];
			for( int j = 0; j < i - 1; j++ )
				ret += char3[j];
		}
		return ret;
	}

	// =====================================================================
	// 核心：编码标记前缀（只要不与普通内容冲突即可）
	// =====================================================================
	static const std::string s_kUTF8B64Prefix = "##U8B64##";
}

inline int StringToolSizeToInt(size_t nSize)
{
	return nSize <= static_cast< size_t >( INT_MAX ) ? static_cast< int >( nSize ) : -1;
}

inline std::wstring StringToolMultiByteToWString(unsigned int nCodePage,
	const char* pszText,
	size_t                              nTextLen,
	unsigned long                       dwFlags = 0)
{
	int nSrcLen = StringToolSizeToInt(nTextLen);
	if( pszText == nullptr || nSrcLen <= 0 )
		return std::wstring();

	int nWideLen = MultiByteToWideChar(nCodePage, dwFlags, pszText, nSrcLen, NULL, 0);
	if( nWideLen <= 0 )
		return std::wstring();

	std::wstring strResult(nWideLen, L'\0');
	if( MultiByteToWideChar(nCodePage, dwFlags, pszText, nSrcLen, &strResult[0], nWideLen) != nWideLen )
		return std::wstring();

	return strResult;
}

inline std::string StringToolWStringToMultiByte(unsigned int  nCodePage,
	const wchar_t* pszText,
	size_t                                            nTextLen,
	unsigned long                                     dwFlags = 0)
{
	int nSrcLen = StringToolSizeToInt(nTextLen);
	if( pszText == nullptr || nSrcLen <= 0 )
		return std::string();

	int nMultiByteLen = WideCharToMultiByte(nCodePage, dwFlags, pszText, nSrcLen, NULL, 0, NULL, NULL);
	if( nMultiByteLen <= 0 )
		return std::string();

	std::string strResult(nMultiByteLen, '\0');
	if( WideCharToMultiByte(nCodePage, dwFlags, pszText, nSrcLen, &strResult[0], nMultiByteLen, NULL, NULL) != nMultiByteLen )
		return std::string();

	return strResult;
}

inline CString StringToolWStringToCString(const std::wstring& str)
{
#ifdef UNICODE
	if( str.empty() )
		return CString();

	int nLength = StringToolSizeToInt(str.size());
	if( nLength <= 0 )
		return CString();

	CString strResult;
	wchar_t* pBuffer = strResult.GetBufferSetLength(nLength);
	memcpy(pBuffer, str.data(), str.size() * sizeof(wchar_t));
	strResult.ReleaseBuffer(nLength);
	return strResult;
#else
	return CString(StringToolWStringToMultiByte(CP_ACP, str.data(), str.size()).c_str());
#endif
}

inline std::string UnicodeToUTF8(const std::wstring& wstr)
{
	return StringToolWStringToMultiByte(CP_UTF8, wstr.data(), wstr.size());
}

inline std::wstring UTF8ToUnicode(const std::string& str)
{
	return StringToolMultiByteToWString(CP_UTF8, str.data(), str.size(), MB_ERR_INVALID_CHARS);
}

inline std::string UnicodeToGBK(const std::wstring& wstr)
{
	return StringToolWStringToMultiByte(936, wstr.data(), wstr.size());
}

inline std::wstring GBKToUnicode(const std::string& str)
{
	return StringToolMultiByteToWString(936, str.data(), str.size());
}

inline std::string UTF8ToGBK(const std::string& str)
{
	std::wstring wstr = UTF8ToUnicode(str);
	return UnicodeToGBK(wstr);
}

inline std::string GBKToUTF8(const std::string& str)
{
	std::wstring wstr = GBKToUnicode(str);
	return UnicodeToUTF8(wstr);
}

inline bool IsUTF8Bytes(const unsigned char* pData, size_t nSize, bool* pbHasBOM = nullptr)
{
	if( pbHasBOM )
		*pbHasBOM = false;

	if( pData == nullptr )
		return nSize == 0;

	if( nSize == 0 )
		return true;

	size_t i = 0;
	if( nSize >= 3 &&
		pData[0] == 0xEF &&
		pData[1] == 0xBB &&
		pData[2] == 0xBF ) {
		if( pbHasBOM )
			*pbHasBOM = true;
		i = 3;
	}

	while( i < nSize ) {
		unsigned char c = pData[i];
		int           nBytes = 0;
		unsigned int  codePoint = 0;

		if( c <= 0x7F ) {
			i++;
			continue;
		} else if( ( c & 0xE0 ) == 0xC0 ) {
			nBytes = 2;
			codePoint = c & 0x1F;
		} else if( ( c & 0xF0 ) == 0xE0 ) {
			nBytes = 3;
			codePoint = c & 0x0F;
		} else if( ( c & 0xF8 ) == 0xF0 ) {
			nBytes = 4;
			codePoint = c & 0x07;
		} else {
			return false;
		}

		if( i + nBytes > nSize )
			return false;

		for( int j = 1; j < nBytes; j++ ) {
			if( ( pData[i + j] & 0xC0 ) != 0x80 )
				return false;
			codePoint = ( codePoint << 6 ) | ( pData[i + j] & 0x3F );
		}

		if( nBytes == 2 && codePoint < 0x0080 )  return false;
		if( nBytes == 3 && codePoint < 0x0800 )  return false;
		if( nBytes == 4 && codePoint < 0x10000 ) return false;

		if( codePoint >= 0xD800 && codePoint <= 0xDFFF ) return false;
		if( codePoint > 0x10FFFF )                        return false;

		i += nBytes;
	}

	return true;
}

inline bool IsUTF8(const std::string& str, bool* pbHasBOM = nullptr)
{
	return IsUTF8Bytes(
		reinterpret_cast< const unsigned char* >( str.data() ),
		str.size(),
		pbHasBOM);
}

inline bool StringToolReadBinaryFileToBuffer(const CString& strFilePath, std::vector<unsigned char>& buffer)
{
	buffer.clear();

	std::ifstream file(strFilePath.GetString(), std::ios::binary | std::ios::ate);
	if( !file.is_open() )
		return false;

	std::streamoff fileSize = static_cast< std::streamoff >( file.tellg() );
	auto maxsize1 = ( std::numeric_limits<std::streamsize>::max )( );
	auto maxsize2 = ( std::numeric_limits<std::size_t>::max )( );
	if( fileSize < 0 || fileSize >= maxsize1 || fileSize >= maxsize2 )
		return false;

	file.seekg(0, std::ios::beg);
	if( fileSize == 0 )
		return true;

	std::streamsize nFileSize = static_cast< std::streamsize >( fileSize );
	buffer.resize(static_cast< size_t >( nFileSize ));
	file.read(reinterpret_cast< char* >( &buffer[0] ), nFileSize);
	return file.gcount() == nFileSize;
}

//------------------------------------------------
inline bool IsFileUTF8(const CString& strFilePath, bool* pbHasBOM = nullptr)
{
	if( pbHasBOM )
		*pbHasBOM = false;

	std::vector<unsigned char> buffer;
	if( !StringToolReadBinaryFileToBuffer(strFilePath, buffer) )
		return false;

	if( buffer.empty() )
		return true;

	return IsUTF8Bytes(&buffer[0], buffer.size(), pbHasBOM);
}

// GB2312 -> Wide -> UTF-8
inline std::string GB2312FileToUTF8(const CString& strFilePath)
{
	// Read file content in binary mode.
	std::vector<unsigned char> buffer;
	if( !StringToolReadBinaryFileToBuffer(strFilePath, buffer) )
		return std::string();

	if( buffer.empty() )
		return std::string();

	const char* pszGB2312 = reinterpret_cast< const char* >( buffer.data() );
	return GBKToUTF8(std::string(pszGB2312, buffer.size()));
}

// Save GB2312 file content as UTF-8.
inline bool GB2312FileToUTF8File(const CString& strSrcPath,
	const CString& strDstPath,
	bool           bWriteBOM = false)
{
	std::string strUTF8 = GB2312FileToUTF8(strSrcPath);
	if( strUTF8.empty() )
		return false;

	std::ofstream outFile(strDstPath.GetString(), std::ios::binary);
	if( !outFile.is_open() )
		return false;

	// Optional BOM.
	if( bWriteBOM ) {
		const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		outFile.write(reinterpret_cast< const char* >( bom ), sizeof(bom));
	}

	outFile.write(strUTF8.c_str(), strUTF8.size());
	outFile.close();

	return true;
}

inline std::string CStringToANSI(CString cstr)
{
#ifdef UNICODE
	return StringToolWStringToMultiByte(CP_ACP, cstr.GetString(), cstr.GetLength());
#else
	return std::string(cstr.GetString());
#endif
}

inline std::string WStringToANSI(const std::wstring& wstr)
{
	return StringToolWStringToMultiByte(CP_ACP, wstr.data(), wstr.size());
}

inline CString ANSIToCString(const std::string& str)
{
#ifdef UNICODE
	return StringToolWStringToCString(StringToolMultiByteToWString(CP_ACP, str.data(), str.size()));
#else
	return CString(str.c_str());
#endif
}

inline std::wstring ANSIToWString(const std::string& str)
{
	return StringToolMultiByteToWString(CP_ACP, str.data(), str.size());
}

inline std::string CStringToUTF8(CString cstr)
{
#ifdef UNICODE
	// In UNICODE builds CString already stores wchar_t.
	return StringToolWStringToMultiByte(CP_UTF8, cstr.GetString(), cstr.GetLength());
#else
	// In MBCS builds convert ANSI -> Wide -> UTF-8.

	return UnicodeToUTF8(StringToolMultiByteToWString(CP_ACP, cstr.GetString(), cstr.GetLength()));
#endif
}

inline CString UTF8ToCString(const std::string& strUtf8)
{
#ifdef UNICODE
	// In UNICODE builds convert UTF-8 -> Wide.

	return StringToolWStringToCString(UTF8ToUnicode(strUtf8));
#else
	// In MBCS builds convert UTF-8 -> Wide -> ANSI.

	std::wstring wstr = UTF8ToUnicode(strUtf8);
	return CString(StringToolWStringToMultiByte(CP_ACP, wstr.data(), wstr.size()).c_str());
#endif
}

inline CString UTF8OrANSIToCString(const std::string& str)
{
	if( str.empty() )
		return CString();

	std::wstring wstr = UTF8ToUnicode(str);
	if( !wstr.empty() )
		return StringToolWStringToCString(wstr);

	return ANSIToCString(str);
}

// =====================================================================
// 检测 CString 是否能被 GBK 无损编码
// =====================================================================
inline bool CanEncodeAsGBK(const CString& str)
{
	// 先转 ANSI(GBK)，再转回 Unicode，比较是否等价
	CStringA ansi(str);  // MFC 在 Unicode 项目中会自动走 WideCharToMultiByte(CP_ACP,...)
	CString back(ansi);  // 再走 MultiByteToWideChar(CP_ACP,...)
	return back == str;
}

// =====================================================================
// 读取接口：替换你原来的 UTF8OrANSIToCString(...)
// =====================================================================
inline CString DecodeValueFromGBKConf(const std::string& raw)
{
	if( raw.compare(0, StringToolDetail::s_kUTF8B64Prefix.size(), StringToolDetail::s_kUTF8B64Prefix) == 0 ) {
		// 识别到特殊编码：Base64 → UTF-8 → CString
		std::string base64Part = raw.substr(StringToolDetail::s_kUTF8B64Prefix.size());
		std::string utf8 = StringToolDetail::_Base64Decode(base64Part);
		return UTF8ToCString(utf8);  // 你现有的 UTF-8→CString 工具
	}

	// 普通内容，走原有逻辑
	return UTF8OrANSIToCString(raw);
}

// =====================================================================
// 写入接口：替换你原来的 CStringToANSI(...)
// =====================================================================
inline std::string EncodeValueForGBKConf(const CString& str)
{
	if( CanEncodeAsGBK(str) ) {
		// 普通 GBK 可表示的内容（中文/英文等），按原逻辑写入
		return CStringToANSI(str);
	}

	// 含越南语等非 GBK 字符：UTF-8 → Base64 → 加前缀
	std::string utf8 = CStringToUTF8(str);
	return StringToolDetail::s_kUTF8B64Prefix + StringToolDetail::_Base64Encode(utf8);
}

#endif // StringTool_h__
