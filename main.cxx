#include <iostream>

#include "src/address.hxx"
#include "src/name.hxx"
#include "yaml-cpp/yaml.h"

boost::asio::ip::address_v4 getIPv4Address();
boost::asio::ip::address_v6 getIPv6Address();

std::string normalizeIPv4Address(std::string_view str)
{
	return boost::asio::ip::make_address_v4(str).to_string();
}

std::string normalizeIPv6Address(std::string_view str)
{
	return boost::asio::ip::make_address_v6(str).to_string();
}

struct Config
{
	std::string username;
	std::string apiKey;
	uint32_t ttlToUpdate = 0;
};

Config readConfigFromFile()
{
	YAML::Node config = YAML::LoadFile("/etc/dyname.yaml");

	return {
		config["username"].as<std::string>(),
		config["apikey"].as<std::string>(),
		config["ttlToUpdate"].as<uint32_t>()
	};
}

int main(int argc, char** argv)
{
	const Config config = readConfigFromFile();

	auto ipv4 = getIPv4Address().to_string();
	auto ipv6 = getIPv6Address().to_string();
	std::cout << "IPv4: " << ipv4 << std::endl;
	std::cout << "IPv6: " << ipv6 << std::endl;

	NameComApi api{false, config.username, config.apiKey};

	auto domains = api.listDomains();

	for (const auto& domain : domains)
	{
		std::cout << "domain: " << domain.domainName << std::endl;

		auto records = api.listRecords(domain.domainName);

		for (auto& record : records)
		{
			std::cout << "    record: " << record.host << " " << record.type << " " << record.answer << " " << std::to_string(record.ttl) << std::endl;

			if (record.ttl == config.ttlToUpdate)
			{
				if (record.type == "A")
				{
					auto currentAddress = normalizeIPv4Address(record.answer);
					std::cout << "    ours:   " << ipv4 << std::endl << "    theirs: " << currentAddress << std::endl;

					if (currentAddress != ipv4)
					{
						std::cout << "    updating..." << std::endl;
						record.answer = ipv4;
						api.updateRecord(record);
					}
				}
				else if (record.type == "AAAA")
				{
					auto currentAddress = normalizeIPv6Address(record.answer);
					std::cout << "    ours:   " << ipv6 << std::endl << "    theirs: " << currentAddress << std::endl;

					if (currentAddress != ipv6)
					{
						std::cout << "    updating..." << std::endl;
						record.answer = ipv6;
						api.updateRecord(record);
					}
				}
				else
				{
					std::cout << "    unknown record type " << record.type << ", ignoring" << std::endl;
				}
			}
		}

		std::cout << std::endl;
	}
}