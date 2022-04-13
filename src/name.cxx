#include "name.hxx"

#define NOMINMAX
#include <cpr/cpr.h>
#include <iostream>
#undef NOMINMAX

// Name.com says they aggressively remove values from these responses to save bandwidth if the values are default
// strings default to ""
// bools default to false
// numerics default to 0

int64_t getInt64(const boost::json::object& o, boost::string_view key)
{
	auto iter{ o.find(key) };

	if (iter == o.end())
	{
		return 0;
	}
	else
	{
		return (*iter).value().as_int64();
	}
}

int32_t getInt32(const boost::json::object& o, boost::string_view key)
{
	int64_t result = getInt64(o, key);

	if (result > std::numeric_limits<int32_t>::max()
		|| result < std::numeric_limits<int32_t>::min())
	{
		throw std::runtime_error(std::to_string(result) + " cannot fit inside an int32_t");
	}

	return static_cast<int32_t>(result);
}

uint32_t getUInt32(const boost::json::object& o, boost::string_view key)
{
	int64_t result = getInt64(o, key);

	if (result > std::numeric_limits<uint32_t>::max()
		|| result < std::numeric_limits<uint32_t>::min())
	{
		throw std::runtime_error(std::to_string(result) + " cannot fit inside an uint32_t");
	}

	return static_cast<uint32_t>(result);
}

double getDouble(const boost::json::object& o, boost::string_view key)
{
	auto iter{ o.find(key) };

	if (iter == o.end())
	{
		return 0.0;
	}
	else
	{
		return (*iter).value().as_double();
	}
}

bool getBool(const boost::json::object& o, boost::string_view key)
{
	auto iter{ o.find(key) };

	if (iter == o.end())
	{
		return false;
	}
	else
	{
		return (*iter).value().as_bool();
	}
}

std::string getString(const boost::json::object& o, boost::string_view key)
{
	auto iter{ o.find(key) };

	if (iter == o.end())
	{
		return "";
	}
	else
	{
		return std::string((*iter).value().as_string());
	}
}

Domain::Domain(const boost::json::object& o)
	: domainName(getString(o, "domainName"))
	, privacyEnabled(getBool(o, "privacyEnabled"))
	, locked(getBool(o, "locked"))
	, autorenewEnabled(getBool(o, "autorenewEnabled"))
	, expireDate(getString(o, "expireDate"))
	, createDate(getString(o, "createDate"))
	, renewalPrice(getDouble(o, "renewalPrice"))
{
	auto nameserversIter = o.find("nameservers");

	if (nameserversIter != o.end())
	{
		const boost::json::array& nameservers = (*nameserversIter).value().as_array();

		this->nameservers.reserve(nameservers.size());

		for (const boost::json::value& nameserver : nameservers)
		{
			this->nameservers.push_back(std::string(nameserver.as_string()));
		}
	}
}

Record::Record(const boost::json::object& o)
	: id(getInt32(o, "id"))
	, domainName(getString(o, "domainName"))
	, host(getString(o, "host"))
	, fqdn(getString(o, "fqdn"))
	, type(getString(o, "type"))
	, answer(getString(o, "answer"))
	, ttl(getUInt32(o, "ttl"))
	, priority(getUInt32(o, "priority"))
{
}

boost::json::object Record::toJson() const
{
	return {
		{"id", id},
		{"domainName", domainName},
		{"host", host},
		{"fqdn", fqdn},
		{"type", type},
		{"answer", answer},
		{"ttl", ttl},
		{"priority", priority}
	};
}

std::vector<Domain> NameComApi::listDomains() const
{
	std::vector<Domain> result;

	int32_t lastPage = 0;
	int32_t nextPage = 0;
	int32_t curPageIndex = -1;

	while (curPageIndex != lastPage)
	{
		curPageIndex = nextPage;
		auto curPage = listDomains(1000, nextPage);
		lastPage = curPage.lastPage;
		nextPage = curPage.nextPage;

		result.reserve(result.size() + curPage.data.size());
		for (auto& domain : curPage.data)
		{
			result.emplace_back(std::move(domain));
		}
	}

	return result;
}

PagedResponse<Domain> NameComApi::listDomains(int32_t perPage, int32_t page) const
{
	cpr::Response r = cpr::Get(cpr::Url{ m_apiDomain + "/v4/domains" },
		cpr::Parameters{ {"perPage", std::to_string(perPage)}, {"page", std::to_string(page)} },
		cpr::Authentication{ m_username, m_apiKey });

	if (r.status_code == 0)
		throw std::runtime_error(r.error.message);
	else if (r.status_code >= 400)
		throw std::runtime_error("HTTP error " + std::to_string(r.status_code));

	const boost::json::object responseData{ boost::json::parse(r.text).as_object() };

	PagedResponse<Domain> result{};

	result.nextPage = getInt32(responseData, "nextPage");
	result.lastPage = getInt32(responseData, "lastPage");

	auto domainsIter = responseData.find("domains");

	if (domainsIter != responseData.end())
	{
		const boost::json::array& domains = responseData.at("domains").as_array();

		result.data.reserve(domains.size());

		for (const boost::json::value& domainValue : domains)
		{
			result.data.emplace_back(Domain{ domainValue.as_object() });
		}
	}

	return result;
}

std::vector<Record> NameComApi::listRecords(std::string_view domainName) const
{
	std::vector<Record> result;

	int32_t lastPage = 0;
	int32_t nextPage = 0;
	int32_t curPageIndex = -1;

	while (curPageIndex != lastPage)
	{
		curPageIndex = nextPage;
		auto curPage = listRecords(domainName, 1000, nextPage);
		lastPage = curPage.lastPage;
		nextPage = curPage.nextPage;

		result.reserve(result.size() + curPage.data.size());
		for (auto& record : curPage.data)
		{
			result.emplace_back(std::move(record));
		}
	}

	return result;
}


PagedResponse<Record> NameComApi::listRecords(std::string_view domainName, int32_t perPage, int32_t page) const
{
	cpr::Response r = cpr::Get(cpr::Url{ m_apiDomain + "/v4/domains/" + std::string(domainName) + "/records"},
		cpr::Parameters{ {"perPage", std::to_string(perPage)}, {"page", std::to_string(page)} },
		cpr::Authentication{ m_username, m_apiKey });

	if (r.status_code == 0)
		throw std::runtime_error(r.error.message);
	else if (r.status_code >= 400)
		throw std::runtime_error("HTTP error " + std::to_string(r.status_code));

	const boost::json::object responseData{ boost::json::parse(r.text).as_object() };

	PagedResponse<Record> result{};

	result.nextPage = getInt32(responseData, "nextPage");
	result.lastPage = getInt32(responseData, "lastPage");

	auto recordsIter = responseData.find("records");

	if (recordsIter != responseData.end())
	{
		const boost::json::array& records = (*recordsIter).value().as_array();

		for (const boost::json::value& recordValue : records)
		{
			result.data.emplace_back(Record{ recordValue.as_object() });
		}
	}

	return result;
}

void NameComApi::updateRecord(const Record& record) const
{
	cpr::Response r = cpr::Put(cpr::Url{ m_apiDomain + "/v4/domains/" + record.domainName + "/records/" + std::to_string(record.id) },
		cpr::Body{ boost::json::serialize(record.toJson()) },
		cpr::Authentication{ m_username, m_apiKey });

	if (r.status_code == 0)
		throw std::runtime_error(r.error.message);
	else if (r.status_code >= 400)
		throw std::runtime_error("HTTP error " + std::to_string(r.status_code));
}