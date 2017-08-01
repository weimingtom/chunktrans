#include <iostream>
#include <fstream>
#include <sstream>
#include <strstream>
#include <iomanip>
#include <cctype>

const bool DEBUG = false;
static int position = 0;

void readbuf(std::ifstream &fin, char *bytes, int len) 
{
	fin.read(bytes, len);
	position += len;
	if (DEBUG) 
	{
		std::cout << ">>position=" << position << "<<" << std::endl;
	}
}

void tab(std::ostringstream &ostring, int n)
{
	using namespace std;
	for (int i = 0; i < n; i++)
	{
		ostring << "\t";
	}
}

void readbyte(std::ostringstream &ostring, std::ifstream &fin, const char *name, int level)
{
	using namespace std;
	char byte;
	readbuf(fin, &byte, 1);
	
	tab(ostring, level);
	ostring << "\"" << name << "\"" << ":"
		<< (unsigned int)byte
		<< "," << endl;
}

void readmultibytes(std::ostringstream &ostring, std::ifstream &fin, const char *name, int length, int level)
{
	using namespace std;
	char *bytes = new char[length];
	readbuf(fin, bytes, length);

	tab(ostring, level);
	ostring << "\"" << name << "\"" << ":" << "\"";
	for (int i = 0; i < length; i++)
	{
		if (isspace(bytes[i]) || !isalnum(bytes[i]))
		{
			ostring << "\\x" << setiosflags(ios::uppercase) << hex << setw(2) << setfill('0')
				<< (unsigned int)bytes[i]
				<< dec;
		}
		else
		{
			ostring << bytes[i];
		}
	}
	ostring << "\"" << "," << endl;
	delete[] bytes;
}

void readstring(std::ostringstream &ostring, std::ifstream &fin, const char *name, int level)
{
	using namespace std;
	union str_header {
		char bytes[4];
		int length;
	} header;
	readbuf(fin, header.bytes, 4);
	int length = header.length;
	char *bytes = new char[length];
	readbuf(fin, bytes, length);
	
	tab(ostring, level);
	ostring << "\"" << name << "\"" << ":" << "\"";
	for (int i = 0; i < length; i++)
	{
		if (isspace(bytes[i]) || iscntrl(bytes[i]))
		{
			ostring << "\\x" << setiosflags(ios::uppercase) << hex << setw(2) << setfill('0')
				<< (unsigned int)bytes[i]
				<< dec;
		}
		else if (bytes[i] == '"')
		{
			ostring << "\\\"";
		}
		else
		{
			ostring << bytes[i];
		}
	}
	ostring << "\"" << "," << endl;
	delete[] bytes;
}

void readint32(std::ostringstream &ostring, std::ifstream &fin, const char *name, int level)
{
	using namespace std;
	union str_header {
		char bytes[4];
		int length;
	} header;
	readbuf(fin, header.bytes, sizeof(header.bytes));
	
	tab(ostring, level);
	ostring << "\"" << name << "\"" << ":"
		<< header.length
		<< "," << endl;
}

void readnumber(std::ostringstream &ostring, std::ifstream &fin, const char *name, int level)
{
	using namespace std;
	union str_header {
		char bytes[8];
		double length;
	} header;
	readbuf(fin, header.bytes, sizeof(header.bytes));

	tab(ostring, level);
	ostring << "\"" << name << "\"" << ":"
		<< header.length
		<< "," << endl;
}

void readcode(std::ostringstream &ostring, std::ifstream &fin, const char *name, int level)
{
	using namespace std;
	union str_header {
		char bytes[4];
		int length;
	} header;
	readbuf(fin, header.bytes, 4);
	int length = header.length;
	unsigned int *bytes = new unsigned int[length];
	readbuf(fin, (char *)bytes, sizeof(unsigned int) * length);
	
	tab(ostring, level);
	ostring << "\"" << name << "\"" << ":" << endl;
	
	tab(ostring, level);
	ostring << "[" << endl;
	for (int i = 0; i < length; i++)
	{
		tab(ostring, level + 1);
		ostring << "0x" << setiosflags(ios::uppercase)
			<< hex << setw(8) << setfill('0')
			<< bytes[i]
			<< dec
			<< ","
			<< endl;
	}
	tab(ostring, level);
	ostring << "]" << "," << endl;
	delete[] bytes;
}

void readheader(std::ostringstream &ostring, std::ifstream &fin, int level)
{
	using namespace std;
	readmultibytes(ostring, fin, "header_signature", 4, level);
	readbyte(ostring, fin, "version", level);
	readbyte(ostring, fin, "format", level);
	readbyte(ostring, fin, "endianness", level);
	readbyte(ostring, fin, "size_of_int", level);
	readbyte(ostring, fin, "size_of_size_t", level);
	readbyte(ostring, fin, "size_of_Instruction", level);
	readbyte(ostring, fin, "size_of_number", level);
	readbyte(ostring, fin, "integral", level);
}

void readdebug(std::ostringstream &ostring, std::ifstream &fin, int level)
{
	using namespace std;
	union str_header {
		char bytes[4];
		int length;
	} header;
	readbuf(fin, header.bytes, 4);
	
	tab(ostring, level);
	ostring << "\"" << "sizelineinfo" << "\"" << ":"
		<< header.length
		<< "," << endl;

	int length = header.length; 
	unsigned int *bytes = new unsigned int[length];
	readbuf(fin, (char *)bytes, sizeof(unsigned int) * length);

	tab(ostring, level);
	ostring << "\"" << "lineinfo" << "\"" << ":" << endl;
	
	tab(ostring, level);
	ostring << "[" << endl;
	
	//lineinfo
	for (int i = 0; i < header.length; ++i)
	{
		tab(ostring, level + 1);
		ostring << "0x" << setiosflags(ios::uppercase)
			<< hex << setw(8) << setfill('0')
			<< bytes[i]
			<< dec
			<< ","
			<< endl;
	}
	
	tab(ostring, level);
	ostring << "]" << "," << endl;

	//sizelocvars
	readbuf(fin, header.bytes, 4);

	tab(ostring, level);
	ostring << "\"" << "sizelocvars" << "\"" << ":"
		<< header.length
		<< "," << endl;

	tab(ostring, level);
	ostring << "\"" << "locvars" << "\"" << ":" << endl;
	
	tab(ostring, level);
	ostring << "[" << endl;
	
	for (int j = 0; j < header.length; j++)
	{
		//readstring(fin, "varname");
		union str_header {
			char bytes[4];
			int length;
		} header;
		readbuf(fin, header.bytes, 4);
		int length = header.length;
		char *bytes = new char[length];
		readbuf(fin, bytes, length);

		tab(ostring, level + 1);
		ostring << "{"; 
		ostring << "\"" << "varname" << "\"" << ":" << "\"";
		for (int i = 0; i < length; i++)
		{
			if (isspace(bytes[i]) || iscntrl(bytes[i]))
			{
				ostring << "\\x" << setiosflags(ios::uppercase) << hex << setw(2) << setfill('0') 
					<< (unsigned int)bytes[i]
					<< dec;
			}
			else if (bytes[i] == '"')
			{
				ostring << "\\\"";
			}
			else
			{
				ostring << bytes[i];
			}
		}
		ostring << "\"" << ", ";
		delete[] bytes;
		readbuf(fin, header.bytes, 4);
		ostring << "\"" << "startpc" << "\"" << ":" << header.length << ", "; 
		readbuf(fin, header.bytes, 4);
		ostring << "\"" << "endpc" << "\"" << ":" << header.length << ", "; 
		ostring << "}," << endl;
		//readint32(fin, "startpc");
		//readint32(fin, "endpc");
	}
	
	tab(ostring, level);
	ostring << "]" << "," << endl;
	
	//upvalues
	readbuf(fin, header.bytes, 4);

	tab(ostring, level);
	ostring << "\"" << "sizeupvalues" << "\"" << ":"
		<< header.length
		<< "," << endl;
	
	tab(ostring, level);
	ostring << "\"" << "upvalues" << "\"" << ":" << endl;
	
	tab(ostring, level);
	ostring << "[" << endl;
	
	for (int k = 0; k < header.length; k++)
	{
		union str_header {
			char bytes[4];
			int length;
		} header;
		readbuf(fin, header.bytes, 4);
		int length = header.length;
		char *bytes = new char[length];
		readbuf(fin, bytes, length);
		
		tab(ostring, level + 1);
		ostring << "\"";
		
		for (int i = 0; i < length; i++)
		{
			if (isspace(bytes[i]) || iscntrl(bytes[i]))
			{
				ostring << "\\x" << setiosflags(ios::uppercase) << hex << setw(2) << setfill('0')
					<< (unsigned int)bytes[i]
					<< dec;
			}
			else if (bytes[i] == '"')
			{
				ostring << "\\\"";
			}
			else
			{
				ostring << bytes[i];
			}
		}
		ostring << "\"" << "," << endl;
		delete[] bytes;
	}
	
	tab(ostring, level);
	ostring << "]" << "," << endl;
}

void readconstant(std::ostringstream &ostring, std::ifstream &fin, int level);

void readfunction(std::ostringstream &ostring, std::ifstream &fin, int level)
{
	readstring(ostring, fin, "source_name", level);
	readint32(ostring, fin, "line_defined", level);
	readint32(ostring, fin, "last_line_defined", level);
	readbyte(ostring, fin, "nups", level);
	readbyte(ostring, fin, "numparams", level);
	readbyte(ostring, fin, "is_vararg", level);
	readbyte(ostring, fin, "maxstacksize", level);

	readcode(ostring, fin, "code", level);
	readconstant(ostring, fin, level);
	readdebug(ostring, fin, level);
}

void readconstant(std::ostringstream &ostring, std::ifstream &fin, int level)
{
	using namespace std;
	union str_header {
		char bytes[4];
		int length;
	} header;
	readbuf(fin, header.bytes, 4);
	
	tab(ostring, level);
	ostring << "\"" << "sizek" << "\"" << ":"
		<< header.length
		<< "," << endl;

	tab(ostring, level);
	ostring << "\"" << "constant" << "\"" << ":" << endl;

	tab(ostring, level);
	ostring << "[" << endl;

	for (int i = 0; i < header.length; i++)
	{
		char type;
		readbuf(fin, &type, 1);
		//ostring << "type=>" << (int)type << endl;
		switch (type)
		{
		case 0: //LUA_TNIL:
			{
				tab(ostring, level + 1);
				ostring << "{" << "\"" << "type" << "\"" << ":" << (int)type << ", "
					<< "\"" << "value" << "\"" << ":" << 0 << "," << "}" 
					<< "," << endl;
			}
			break;
		
		case 1: //LUA_TBOOLEAN:
			{
				char byte;
				readbuf(fin, &byte, 1);
				
				tab(ostring, level + 1);
				ostring << "{" << "\"" << "type" << "\"" << ":" << (int)type << ", "
					<< "\"" << "value" << "\"" << ":" << (unsigned int)byte << "," << "}" 
					<< "," << endl;
			}
			break;
		
		case 3: //LUA_TNUMBER:
			{
				union str_header {
					char bytes[8];
					double value;
				} header;
				readbuf(fin, header.bytes, sizeof(header.bytes));
				
				tab(ostring, level + 1);
				ostring << "{" << "\"" << "type" << "\"" << ":" << (int)type << ", "
					<< "\"" << "value" << "\"" << ":" << (double)header.value << ",}" 
					<< "," << endl;
			}
			break;

		case 4: //LUA_TSTRING:
			{
				union str_header {
					char bytes[4];
					int length;
				} header;
				readbuf(fin, header.bytes, 4);
				int length = header.length;
				char *bytes = new char[length];
				readbuf(fin, bytes, length);
				
				tab(ostring, level + 1);
				ostring << "{" << "\"" << "type" << "\"" << ":" << (int)type << ", ";
				ostring << "\"" << "value" << "\"" << ":" << "\"";
				for (int i = 0; i < length; i++)
				{
					if (isspace(bytes[i]) || iscntrl(bytes[i]))
					{
						ostring << "\\x" << setiosflags(ios::uppercase) << hex << setw(2) << setfill('0') 
							<< (unsigned int)bytes[i]
							<< dec;
					}
					else if (bytes[i] == '"')
					{
						ostring << "\\\"";
					}
					else
					{
						ostring << bytes[i];
					}
				}
				ostring << "\"" << "," 
					<< "}, " << endl;
				delete[] bytes;
			}
			break;
		
		default:
			abort();
		}
	}

	tab(ostring, level);
	ostring << "]," << endl;

	readbuf(fin, header.bytes, sizeof(header.bytes));
	
	tab(ostring, level);
	ostring << "\"" << "sizep" << "\"" << ":"
		<< header.length
		<< "," << endl;
	
	tab(ostring, level);
	ostring << "\"" << "function" << "\"" << ":" << endl;
	
	tab(ostring, level);
	ostring << "[" << endl;
	for(int j = 0; j < header.length; ++j)
	{
		tab(ostring, level + 1);
		ostring << "{" << endl;
		
		readfunction(ostring, fin, level + 2);
		
		tab(ostring, level + 1);
		ostring << "}" << "," << endl;
	}
	tab(ostring, level);
	ostring << "]" << "," << endl;
}

int readfile(std::ostringstream &ostring, const char* filename, int level)
{
	using namespace std;
	ifstream fin(filename, ios_base::in | ios_base::binary);
	if (!fin.good())
		return -1;
	tab(ostring, level); 
	ostring << '{' << endl;
	{
		readheader(ostring, fin, level + 1);
		readfunction(ostring, fin, level + 1);
	}
	tab(ostring, level);
	ostring << '}' << endl;
	return 0;
}

int main(int argc, char** argv)
{
	using namespace std;
	ostringstream ostring;	
	ostring << "<html>" << endl
		<< "<body>" << endl
		<< "<script type=\"application/javascript\">" << endl;
	ostring << "var luac_out = " << endl;
	int result = readfile(ostring, "luac.out", 0);
	ostring << "document.write(\"hello, world!\");" << endl;
	ostring << "</script>" << endl
		<< "</body>" << endl
		<< "</html>" << endl;
	if (false)
	{
		cout << ostring.str();
	}
	else
	{
		ofstream outf("out.htm");
		outf << ostring.str();
	}

	return 0;
}
