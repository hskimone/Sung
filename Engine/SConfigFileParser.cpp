//
// SConfigFileParser.cpp
//

#include "config_file_parser.h"

//----------------------------------------------------------------------------
ConfigFileParser::ConfigFileParser(std::string filename, std::string delimiter, std::string comment, std::string sentry) 
: 
myDelimiter(delimiter),
myComment(comment), 
mySentry(sentry)
{
	// Construct a ConfigFileParser, getting keys and values from given file
	
	std::ifstream in(filename.c_str());
	
	if (!in)
		throw file_not_found(filename); 
	
	in >> (*this);
}
//----------------------------------------------------------------------------
ConfigFileParser::ConfigFileParser() 
: 
myDelimiter(std::string(1, '=')), 
myComment(std::string(1, '#'))
{
}
//----------------------------------------------------------------------------
void ConfigFileParser::remove (const std::string &key) 
{
	myContents.erase(myContents.find(key));
	return;
}
//----------------------------------------------------------------------------
bool ConfigFileParser::keyExists(const std::string& key) const 
{
	mapci p = myContents.find(key);
	return (p != myContents.end());
}
//----------------------------------------------------------------------------
void ConfigFileParser::trim(std::string& s) 
{
	static const char whitespace[] = " \n\t\v\r\f";
	s.erase(0, s.find_first_not_of(whitespace));
	s.erase(s.find_last_not_of(whitespace) + 1U);
}
//----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const ConfigFileParser& cf) 
{
	for (ConfigFileParser::mapci p = cf.myContents.begin(); p != cf.myContents.end(); ++p ) 
	{
		os << p->first << " " << cf.myDelimiter << " ";
		os << p->second << std::endl;
	}
	return os;
}
//----------------------------------------------------------------------------
std::istream& operator>>(std::istream& is, ConfigFileParser& cf) 
{
	typedef std::string::size_type pos;
	const std::string& delim  = cf.myDelimiter;  // separator
	const std::string& comm   = cf.myComment;    // comment
	const std::string& sentry = cf.mySentry;     // end of file sentry
	const pos skip = delim.length();        // length of separator
	
	std::string nextline = "";  // might need to read ahead to see where value ends
	
	while (is || nextline.length() > 0) {
		// Read an entire line at a time
		std::string line;
		if (nextline.length() > 0) {
			line = nextline;  // we read ahead; use it now
			nextline = "";
		} else
			std::getline(is, line);
		
		// Ignore comments
		line = line.substr(0, line.find(comm));
		
		// Check for end of file sentry
		if (sentry != "" && line.find(sentry) != std::string::npos)
			return is;
		
		// Parse the line if it contains a delimiter
		pos delimPos = line.find(delim);
		if (delimPos < std::string::npos) {
			// Extract the key
			std::string key = line.substr(0, delimPos);
			line.replace(0, delimPos+skip, "");
			
			// See if value continues on the next line
			// Stop at blank line, next line with a key, end of stream,
			// or end of file sentry
			bool terminate = false;
			while (!terminate && is) {
				std::getline(is, nextline);
				terminate = true;
				
				std::string nlcopy = nextline;
				ConfigFileParser::trim(nlcopy);
				if (nlcopy == "")
					continue;
				
				nextline = nextline.substr(0, nextline.find(comm));
				if (nextline.find(delim) != std::string::npos)
					continue;
				if (sentry != "" && nextline.find(sentry) != std::string::npos)
					continue;
				
				nlcopy = nextline;
				ConfigFileParser::trim(nlcopy);
				if( nlcopy != "" )
					line += "\n";
				line += nextline;
				terminate = false;
			}
			
			// Store key and value
			ConfigFileParser::trim(key);
			ConfigFileParser::trim(line);
			cf.myContents[key] = line;  // overwrites if key is repeated
		}
	}
	
	return is;
}
//----------------------------------------------------------------------------