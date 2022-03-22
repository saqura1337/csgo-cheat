#include "../SDK/Utils/JSON.hpp"
#include <sstream>
#include "curl/curl.h"

std::string GetHwUID()
{
	HW_PROFILE_INFO hwProfileInfo;
	std::string szHwProfileGuid = "";
	if (GetCurrentHwProfile(&hwProfileInfo) != NULL)
	{
		szHwProfileGuid = hwProfileInfo.szHwProfileGuid;
	}

	return szHwProfileGuid;
}
std::string GetCpuid()
{
	int CPUInfo[4] = { -1 };
	char CPUBrandString[0x40];
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];

	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	for (size_t i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}
	std::string cpuid = CPUBrandString;
	return cpuid;
}
std::string GetCompUserName(bool User)
{
	std::string CompUserName = "";
	char szCompName[MAX_COMPUTERNAME_LENGTH + 1];
	char szUserName[MAX_COMPUTERNAME_LENGTH + 1];

	DWORD dwCompSize = sizeof(szCompName);
	DWORD dwUserSize = sizeof(szUserName);

	if (GetComputerNameA(szCompName, &dwCompSize))
	{
		CompUserName = szCompName;
		if (User && GetUserNameA(szUserName, &dwUserSize))
		{
			CompUserName = szUserName;
		}
	}
	return CompUserName;
}
DWORD GetVolumeID()
{
	DWORD VolumeSerialNumber;
	BOOL GetVolumeInformationFlag = GetVolumeInformationA("c:\\", 0, 0, &VolumeSerialNumber, 0, 0, 0, 0);
	if (GetVolumeInformationFlag) return VolumeSerialNumber;
	return 0;
}
std::string StringToHex(const std::string input)
{
	const char* lut = "0123456789ABCDEFGHJUYTYMKJ";
	size_t len = input.length();
	std::string output = "";

	output.reserve(2 * len);

	for (size_t i = 0; i < len; i++)
	{
		const unsigned char c = input[i];
		output.push_back(lut[c >> 4]);
		output.push_back(lut[c & 15]);
	}

	return output;
}
std::string GetSerialKey()
{
	std::string SerialKey = "61A345B5496B2";
	std::string CompName = GetCompUserName(false);
	std::string UserName = GetCompUserName(true);

	SerialKey.append(StringToHex(GetHwUID()));
	SerialKey.append("-");
	SerialKey.append(StringToHex(std::to_string(GetVolumeID())));
	SerialKey.append("-");
	SerialKey.append(StringToHex(CompName));
	SerialKey.append("-");
	SerialKey.append(StringToHex(UserName));

	return SerialKey;
}
std::string GetHashText(const void* data, const size_t data_size)
{
	HCRYPTPROV hProv = NULL;

	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) return "";

	BOOL hash_ok = FALSE;
	HCRYPTPROV hHash = NULL;

	hash_ok = CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);
	if (!hash_ok)
	{
		CryptReleaseContext(hProv, 0);
		return "";
	}

	if (!CryptHashData(hHash, static_cast<const BYTE*>(data), data_size, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return "";
	}

	DWORD cbHashSize = 0, dwCount = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&cbHashSize, &dwCount, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return "";
	}

	std::vector<BYTE> buffer(cbHashSize);

	if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(&buffer[0]), &cbHashSize, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return "";
	}

	std::ostringstream oss;

	for (std::vector<BYTE>::const_iterator iter = buffer.begin(); iter != buffer.end(); ++iter)
	{
		oss.fill('0');
		oss.width(2);
		oss << std::hex << static_cast<const int>(*iter);
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	return oss.str();
}
std::string GetHashSerialKey()
{
	std::string SerialKey = GetSerialKey();
	const void* pData = SerialKey.c_str();
	size_t Size = SerialKey.size();
	std::string Hash = GetHashText(pData, Size);

	for (auto& c : Hash)
	{
		if (c >= 'a' && c <= 'f') c = '4';
		else if (c == 'b') c = '5';
		else if (c == 'c')c = '6';
		else if (c == 'd') c = '7';
		else if (c == 'e') c = '8';
		else if (c == 'f') c = '9';
		c = toupper(c);
	}

	return Hash;
}
std::string GetHashCpuid()
{
	std::string Cpuid = GetCpuid();
	const void* pData = Cpuid.c_str();
	size_t Size = Cpuid.size();
	std::string Hash = GetHashText(pData, Size);

	for (auto& c : Hash)
	{
		if (c >= 'a' && c <= 'f') c = '4';
		else if (c == 'b') c = '5';
		else if (c == 'c')c = '6';
		else if (c == 'd') c = '7';
		else if (c == 'e') c = '8';
		else if (c == 'f') c = '9';
		c = toupper(c);
	}

	return Hash;
}
std::string GetSerial()
{
	std::string Serial = "";
	std::string HashSerialKey = GetHashSerialKey();

	std::string Serial1 = HashSerialKey.substr(0, 4);
	std::string Serial2 = HashSerialKey.substr(4, 4);
	std::string Serial3 = HashSerialKey.substr(8, 4);
	std::string Serial4 = HashSerialKey.substr(12, 4);

	std::string HashCpuid = GetHashCpuid();
	std::string Serial5 = HashSerialKey.substr(HashCpuid.size() / 2 - 2, 4);

	Serial += Serial1;
	Serial += '-';
	Serial += Serial2;
	Serial += '-';
	Serial += Serial3;
	Serial += '-';
	Serial += Serial4;
	Serial += '-';
	Serial += Serial5;

	return Serial;
}

/////////////////

nlohmann::json data;

auto get_user_data() -> bool {

	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();

	if (curl) {
		std::string response;
		//std::string post_fields = "zeeron.su//userapi.php?h=" + MD5(GetSerial());
		//curl_easy_setopt(curl, CURLOPT_URL, post_fields);
		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);
		if (res == CURLE_OK)
		{
        	data = nlohmann::json::parse(response);

			std::string username = data["username"].get<std::string>().c_str();
			std::string user_id = data["user_id"].get<std::string>().c_str();
			std::string end_date = data["end_date"].get<std::string>().c_str();
		}
	}

	return false;
}