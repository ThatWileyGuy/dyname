#include <cstdint>
#include <string>
#include <vector>

#define NOMINMAX
#include <boost/json.hpp>
#undef NOMINMAX

struct Domain
{
	Domain(const boost::json::object&);

	std::string domainName;
	std::vector<std::string> nameservers;
	//Contacts contacts;
	bool privacyEnabled;
	bool locked;
	bool autorenewEnabled;
	const std::string expireDate;
	const std::string createDate;
	const double renewalPrice;
};

struct Record
{
	Record(const boost::json::object&);
	boost::json::object toJson() const;

	const int32_t id = 0;
	const std::string domainName;
	std::string host;
	const std::string fqdn;
	std::string type;
	std::string answer;
	uint32_t ttl = 0;
	uint32_t priority = 0;
};

template<typename T>
struct PagedResponse
{
	std::vector<T> data;
	int32_t nextPage = 0;
	int32_t lastPage = 0;
};

class NameComApi
{
private:
	std::string m_apiDomain;
	std::string m_username;
	std::string m_apiKey;

public:
	NameComApi(bool test, std::string_view username, std::string_view apiKey)
		: m_apiDomain(test ? "https://api.dev.name.com" : "https://api.name.com")
		, m_username(username)
		, m_apiKey(apiKey)
	{
	}

	std::vector<Domain> listDomains() const;
	PagedResponse<Domain> listDomains(int32_t perPage, int32_t page) const;
	std::vector<Record> listRecords(std::string_view domainName) const;
	PagedResponse<Record> listRecords(std::string_view domainName, int32_t perPage, int32_t page) const;
	void updateRecord(const Record& record) const;
};
