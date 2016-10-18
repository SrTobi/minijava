#include "cli.hpp"

#include <cerrno>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include <boost/program_options.hpp>

#include "global.hpp"


namespace minijava
{

	namespace /* anonymous */
	{

		void echo_file(const std::string& filename, std::ostream& dst) {
			auto buffer = std::vector<char>(std::size_t{1} << 2);
			std::ifstream src{filename};
			do {
				src.read(buffer.data(), buffer.size());
				dst.write(buffer.data(), src.gcount());
			} while (src && dst);
			dst.flush();
			if (src.bad() || !src.eof() || !dst) {
				const auto ec = std::error_code{errno, std::system_category()};
				throw std::system_error{ec, "Cannot read file: " + filename};
			}
		}

	}  // namespace /* anonymous */


	void real_main(const std::vector<const char *>& args,
				   std::ostream& thestdout,
				   std::ostream& thestderr) {
		namespace po = boost::program_options;
		auto optdesc = po::options_description{"Options"};
		optdesc.add_options()
			("help", "show help text and exit")
			("version", "show version text and exit")
			("echo", po::value<std::string>()->value_name("FILE"),
			 "output FILE to standard output and exit");
		auto argdesc = po::positional_options_description{};
		auto varmap = po::variables_map{};
		po::store(po::command_line_parser(args.size(), args.data())
				  .options(optdesc).positional(argdesc).run(), varmap);
		po::notify(varmap);
		if (varmap.count("help")) {
			thestdout << optdesc;
			return;
		}
		if (varmap.count("version")) {
			thestdout << MINIJAVA_PROJECT_NAME << " " << MINIJAVA_PROJECT_VERSION << "\n"
					  << "Copyright (C) 2016 T. Kahlert, P.J. Serrer, M. Baumann and M. Klammler\n"
					  << "This is free software; see the source for copying conditions.  There is NO\n"
					  << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
			return;
		}
		if (varmap.count("echo")) {
			echo_file(varmap["echo"].as<std::string>(), thestdout);
			return;
		}
	}

}  // namespace minijava
