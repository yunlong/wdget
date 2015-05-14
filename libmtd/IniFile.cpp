/*
 *  dget - download get for linux by yunlong.lee
 *  @author           YunLong.Lee    <yunlong.lee@163.com>
 *  @version          0.5v
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 */

#define USE_INIFILE_H

#include "matrix.h"


namespace matrix {


#define MAX_CONFIG_LINE_LENGTH   1024

const char * const MIDDLESTRING = "_____***_______";

IniFile :: IniFile(const string& filename) : m_filename (filename)
{
    load_config ();
}

IniFile :: ~IniFile( void )
{
    flush ();
}

bool IniFile :: valid() const
{
    return !m_filename.empty();
}

string IniFile::read(const string& sect, const string& key) const
{
    if (!valid () || key.empty()) return false;

	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    map <string, string>::const_iterator i = m_config.find (mapkey);
    map <string, string>::const_iterator end = m_config.end ();

    if (i != end && i->second.length ()) 
	{
        return i->second;
    }

    return  string ("");
}

bool IniFile::read(const string& sect, const string& key, string *pStr) const
{
    if (!valid () || !pStr || key.empty()) return false;

	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    map <string, string>::const_iterator i = m_config.find (mapkey);
    map <string, string>::const_iterator end = m_config.end ();

    if (i != end && i->second.length ()) 
	{
        *pStr = i->second;
        return true;
    }

    *pStr = string ("");
    return false;
}

bool IniFile::read(const string& sect, const string& key, int *pl) const
{
    if (!valid () || !pl || key.empty()) return false;

	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    map <string, string>::const_iterator i = m_config.find (mapkey);
    map <string, string>::const_iterator end = m_config.end ();

    if (i != end && i->second.length ()) 
	{
        *pl = strtol (i->second.c_str (), (char**) NULL, 10);
        return true;
    }

    *pl = 0;
    return false;
}


bool IniFile::read(const string& sect, const string& key, double* val) const
{
    if (!valid () || !val || key.empty()) return false;

	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

	
    map <string, string>::const_iterator i = m_config.find (mapkey);
    map <string, string>::const_iterator end = m_config.end ();

    if (i != end && i->second.length ()) 
	{
        *val = strtod (i->second.c_str (), (char**) NULL);
        return true;
    }

    *val = 0;
    return false;
}


bool IniFile::read(const string& sect, const string& key, bool* val) const
{
    if (!valid () || !val || key.empty()) return false;
    
	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    map <string, string>::const_iterator i = m_config.find (mapkey);
    map <string, string>::const_iterator end = m_config.end ();

    if (i != end && i->second.length ()) 
	{
        if (i->second == "true" || i->second == "TRUE" || 
									i->second == "True" || i->second == "1") 
		{
            *val = true;
            return true;
        } 
		else if (i->second == "false" || i->second == "FALSE"  || 
							i->second == "False" || i->second == "0") 
		{
            *val = false;
            return true;
        }
    }

    *val = false;
    return false;
}

bool IniFile::write(const string& sect, const string& key, const string& value)
{
    if (!valid () || key.empty()) return false;
	
	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    m_config [mapkey] = value;

    return true;
}

bool IniFile::write(const string& sect, const string& key, int value)
{
    if (!valid () || key.empty()) return false;

	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    char buf [256];
    snprintf (buf, 255, "%d", value);
    m_config [mapkey] = string (buf);

    return true;
}

bool IniFile::write(const string& sect, const string& key, double value)
{
    if (!valid () || key.empty()) return false;    

	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    char buf [256];
    snprintf (buf, 255, "%lf", value);
    m_config [mapkey] = string (buf);

    return true;
}

bool IniFile::write(const string& sect, const string& key, bool value)
{
    if (!valid () || key.empty()) return false;
	
	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    if (value)
        m_config [mapkey] = string ("true");
    else
        m_config [mapkey] = string ("false");

    return true;
}

bool IniFile::flush()
{
    if (!valid ()) return false;

    if (m_config.size()) 
	{
		save_config();
        load_config ();
        return true;
    }
    return false;
}

bool IniFile::erase(const string& sect, const string& key)
{
    if (!valid ()) return false;
	
	string mapkey = sect;
	mapkey += MIDDLESTRING;
	mapkey += key;

    map<string, string>::iterator j = m_config.find(mapkey);
    bool ok = false;

    if (j != m_config.end ()) 
	{
        m_config.erase (j);
        ok = true;
    }
    return ok;

}

bool IniFile::reload()
{
    if (!valid ()) return false;

    load_config(); 
    return true;
}

string IniFile::trim_blank(const string &str)
{
    string::size_type begin, len;

    begin = str.find_first_not_of (" \t\n\v");

    if (begin == string::npos)
        return string ();

    len = str.find_last_not_of (" \t\n\v") - begin + 1;

    return str.substr (begin, len);
}

void IniFile::parse_config(istream &is, map <string, string> &config)
{
    char *conf_line = new char [MAX_CONFIG_LINE_LENGTH];
	string strsect;

    while (!is.eof()) 
	{
        is.getline(conf_line, MAX_CONFIG_LINE_LENGTH);
        if (!is.eof()) 
		{
			
            string normalized_line = trim_blank(conf_line);

		//	cout << normalized_line << endl;

			int first = normalized_line.find('[');
			int last = normalized_line.rfind(']');
			if( first != string::npos && last != string::npos && first != last + 1)
			{
				strsect = normalized_line.substr(first + 1,last - first - 1);
				continue ;
			}

			if(strsect.empty())
				continue ;

			if( ( first = normalized_line.find('=') )== string::npos)
				continue ;

			string s1 = normalized_line.substr(0, first);
			string s2 = normalized_line.substr(first + 1, string::npos);
			first= s1.find_first_not_of(" \t");
			last = s1.find_last_not_of(" \t");

			if(first == string::npos || last == string::npos)
				continue ;

			string strkey = s1.substr(first, last - first + 1);
			first = s2.find_first_not_of(" \t");
			if(((last = s2.find("\t#", first )) != -1) ||
            		((last = s2.find(" #", first )) != -1) ||
            		((last = s2.find("\t//", first )) != -1)||
            		((last = s2.find(" //", first )) != -1))
    		{
            	s2 = s2.substr(0, last - first);
    		}

    		last = s2.find_last_not_of(" \t");
		    if(first == string::npos || last == string::npos)
    			continue ;

		    string value = s2.substr(first, last - first + 1);

			string mapkey = strsect + MIDDLESTRING;

			mapkey += strkey;
		    config[mapkey] = value;
	//		cout << mapkey << '\t' << value << endl;
    		continue ;

        }
    }

    delete [] conf_line;

}

void IniFile::save_config(void)
{
    if (!valid ()) return;

	bool firstTry = true;
   	string sysconf = m_filename;
    ofstream os(sysconf.c_str ());

    map<string, string>::iterator i;
	map<string, bool> sectmap;
	string sect, key, val;
    for (i = m_config.begin (); i != m_config.end (); ++i) 
	{
		string mapkey = i->first;
		//cout << mapkey << endl;
	   	string::size_type loc = mapkey.find( MIDDLESTRING, 0 );
	   	if( loc != string::npos ) 
	   	{
	        sect = mapkey.substr(0, loc);
			key	 = mapkey.substr( loc + strlen(MIDDLESTRING) );
			val  = i->second;

			//cout << "[" << sect << "]" << '\t' << key << "=" << val << endl;
			if(sectmap.find(sect) != sectmap.end() )
			{
			//	cout << key << "=" << val << endl;
				os << key << "=" << val << endl;
			}
			else
			{	
				sectmap[sect] = true;
			//	cout << endl << "[" << sect << "]" << endl;
				if(firstTry == false)
				{
					os << endl <<  "[" << sect << "]" << endl;
					os << key << "=" << val << endl;
				}
				else
				{
					os << "[" << sect << "]" << endl;
					os << key << "=" << val << endl;
					firstTry = false;
				}
			}
	   	} 
    }
	os.close();
}

void IniFile::load_config()
{
    string sysconf = m_filename;
   	m_config.clear();

    if (sysconf.length ()) 
	{
        ifstream is (sysconf.c_str ());
        if (is) 
		{
            parse_config (is, m_config);
        }
		is.close();

    }

}

void IniFile::show_config()
{
    if (!valid ()) return;

	bool firstTry = true;
    map<string, string>::iterator i;
	map<string, bool> sectmap;
	string sect, key, val;
    for (i = m_config.begin (); i != m_config.end (); ++i) 
	{
		string mapkey = i->first;
	   	string::size_type loc = mapkey.find( MIDDLESTRING, 0 );
	   	if( loc != string::npos ) 
	   	{
	        sect = mapkey.substr(0, loc);
			key	 = mapkey.substr( loc + strlen(MIDDLESTRING) );
			val  = i->second;

			if(sectmap.find(sect) != sectmap.end() )
			{
				cout << key << "=" << val << endl;
			}
			else
			{
				sectmap[sect] = true;
				if(firstTry == false)
				{
					cout << endl << "[" << sect << "]" << endl;
					cout << key << "=" << val << endl;
				}
				else
				{
					cout << "[" << sect << "]" << endl;
					cout << key << "=" << val << endl;
					firstTry = false;
				}
			}
	   	} 
    }
}

}

