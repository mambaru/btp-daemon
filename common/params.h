/*
 * params.h
 *
 *  Created on: Jun 27, 2011
 *      Author: shepik
 */

#ifndef PARAMS_H_
#define PARAMS_H_

#include <boost/program_options.hpp>
#include <string>
#include <map>
namespace po = ::boost::program_options;


struct mysql_connection_settings {
	std::string address;
	std::string login;
	std::string password;
	std::string database;
	bool use_utf8;
	time_t connect_timeout;
	time_t request_timeout;
};


std::ostream& operator<<(std::ostream& os, const po::variable_value& v);

/**
 * вывод бустовой variables_map в поток
 */
std::ostream& operator<<(std::ostream& os, const po::variables_map& vm);


/**
 * добавление настроек mysql к boost program options
 */
void add_mysql_settings(po::options_description &desc,std::string prefix, mysql_connection_settings &mysql, std::string default_database_name);

#endif /* PARAMS_H_ */
