#include "md.h"

#include "log4cxx/logmanager.h"
#include "log4cxx/xml/domconfigurator.h"

int main(int argc, char **argv) {
    log4cxx::LogManager::resetConfiguration();
    log4cxx::xml::DOMConfigurator::configure(LOG4CXX_PATH);

	md(argc,argv);
	return 0;
}
