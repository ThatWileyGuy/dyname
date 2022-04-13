#include <cpr/cpr.h>
#include <iostream>

#include "address.hxx"

boost::asio::ip::address_v4 getIPv4Address()
{
	cpr::Response r = cpr::Get(cpr::Url{ "https://v4.ident.me" });

	if (r.status_code == 0)
		throw std::runtime_error(r.error.message);
	else if (r.status_code >= 400)
		throw std::runtime_error("HTTP error " + std::to_string(r.status_code));

	return boost::asio::ip::make_address_v4(r.text);
}

boost::asio::ip::address_v6 getIPv6Address()
{
	cpr::Response r = cpr::Get(cpr::Url{ "https://v6.ident.me" });

	if (r.status_code == 0)
		throw std::runtime_error(r.error.message);
	else if (r.status_code >= 400)
		throw std::runtime_error("HTTP error " + std::to_string(r.status_code));

	return boost::asio::ip::make_address_v6(r.text);
}