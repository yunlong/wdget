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

#ifndef _INIFILE_H__
#define _INIFILE_H__

namespace matrix {

class IniFile
{
private:
    string           m_filename;
    map<string, string> m_config;
public:
    IniFile(const string& filename = "wdget.inf");
    virtual ~IniFile();
    bool valid () const;
    bool read (const string& sect, const string& key, string *pStr) const;
    bool read (const string& sect, const string& key, int *pl) const;
    bool read (const string& sect, const string& key, double* val) const;
    bool read (const string& sect, const string& key, bool* val) const;
    string read (const string& sect, const string& key) const;

    bool write (const string& sect, const string& key, const string& value);
    bool write (const string& sect, const string& key, int value);
    bool write (const string& sect, const string& key, double value);
    bool write (const string& sect, const string& key, bool value);

    bool flush();
    bool erase (const string& sect, const string& key );
    bool reload ();
public:
    string trim_blank (const string &str);
    void parse_config (istream &is, map<string, string> &config);
    void save_config (void);
    void load_config (void);
	void show_config(void);
	
};

}

#endif
