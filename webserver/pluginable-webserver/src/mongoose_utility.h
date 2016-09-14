/*
 * mongoose_utility.h
 *
 *  Created on: Mar 26, 2013
 *      Author: ezoltbo
 */

#ifndef MONGOOSE_UTILITY_H_
#define MONGOOSE_UTILITY_H_

#include <mongoose.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace cc 
{ 
namespace mongoose 
{

const std::string DOCUMENT_ROOT = "document_root";
const std::string GLOBAL_AUTH_FILE = "global_auth_file";
const std::string PLUGIN_DIR = "plugin_dir";
const std::string CONFIG_FILE = "config_file";
const std::string HELP = "help";
const std::string LISTENING_PORTS = "listening_port";
const std::string NUM_THREADS = "num_threads";
const std::string LDAP = "ldap";

boost::program_options::options_description getCoreOptions();

boost::program_options::options_description getMongooseOptions();

boost::program_options::options_description getAuthOptions();

void parseConfiguration(const boost::program_options::options_description&
  options, int argc, char **argv,
  boost::program_options::variables_map& varMap, bool allowUnkown
  );

char **createCArrayOfOptions(const ::boost::program_options::variables_map&
  varMap, const ::boost::program_options::options_description& options);

void deleteCArrayOfOptions(char **options);

/**
 * This overloaded operator exists for debug reasons. By this function we can print the content of
 * a cookie.
 */
std::ostream& operator<<(std::ostream& out, struct mg_connection *conn);

/**
 * This function returns the username and password sent by the login page.
 */
std::pair<std::string, std::string> getUsernamePassword(struct mg_connection *conn);

/**
 * This function returns the login page html file. The path given as document_root must contain a
 * login.html file of which the content will be returned.
 */
std::string getLoginPage(const std::string& document_root);

/**
 * This function returns the value from Cookie http header which belongs to the "authtoken" key.
 * If no such attribute is found, then empty string is returned.
 */
std::string getAuthToken(struct mg_connection *conn);

/**
 * This function returns the content part of a http header as string.
 */
std::string getContent(mg_connection *conn);

/**
 * This function returns the remote IP address from which the HTTP header has been sent.
 */
std::string getRemoteIp(mg_connection *conn);

std::string ipToString(unsigned ip);

std::string getCurrentDate();

} // mongoose 
} // cc

#endif /* MONGOOSE_UTILITY_H_ */
