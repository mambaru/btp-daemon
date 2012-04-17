#include "params.h"
#include <netinet/in.h>

std::ostream& operator<<(std::ostream& os, const po::variable_value& v) {
	const std::type_info &t = v.value().type();
	if(t == typeid(int)) {
		os << v.as<int>();
	} else if(t == typeid(unsigned int)) {
		os << v.as<unsigned int>();
	} else if(t == typeid(std::string)) {
		os << v.as<std::string>();
	} else if(t == typeid(in_port_t)) {
		os << v.as<in_port_t>();
	} else if(t == typeid(char*)) {
		os << v.as<char*>();
	} else if(t == typeid(bool)) {
		os << (v.as<bool>()?"true":"false");
	} else if(t == typeid(time_t)) {
		os << v.as<time_t>();
	} else os << "<ERROR: params.cpp, cannot convert parameter from po::variable_value>";//boost::throw_exception(boost::bad_any_cast());
	return os;
}

std::ostream& operator<<(std::ostream& os, const po::variables_map& vm) {
    os << "Parameters:" << std::endl;
//    po::variables_map::iterator it;
    for (auto it = vm.begin();it!=vm.end();it++) {
    	char buf[256];
    	std::string s = it->first;
    	sprintf(buf,"%35s",s.c_str());
    	os << buf << " : ";
    	os <<  it->second;
    	os << std::endl;
    }
    os << std::endl;
    return os;
}

void add_mysql_settings(po::options_description &desc,std::string prefix, mysql_connection_settings &mysql, std::string default_database_name) {
	desc.add_options()
		((prefix+"host").c_str(), po::value< std::string >(&mysql.address)->default_value("dbdevel3") )
		((prefix+"user").c_str(), po::value< std::string >(&mysql.login)->default_value("monamour2") )
		((prefix+"pass").c_str(), po::value< std::string >(&mysql.password)->default_value("monamourchik") )
		((prefix+"db").c_str(), po::value< std::string >(&mysql.database)->default_value(default_database_name) )
		((prefix+"utf8").c_str(), po::value< bool >(&mysql.use_utf8)->default_value(true) )
		((prefix+"connect_timeout").c_str(), po::value< time_t >(&mysql.connect_timeout)->default_value(10) )
		((prefix+"request_timeout").c_str(), po::value< time_t >(&mysql.request_timeout)->default_value(600) )
	;
}
