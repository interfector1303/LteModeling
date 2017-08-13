#pragma once

#include <string>
#include <fstream>
#include <iostream>

class CsvFile;

inline static CsvFile& endrow(CsvFile& file);
inline static CsvFile& flush(CsvFile& file);

class CsvFile
{
public:
	CsvFile(const std::string& p_fileName, const std::string& p_separator = ";")
		: m_separator(p_separator)
	{
		m_stream.exceptions(std::ios::failbit | std::ios::badbit);
		m_stream.open(p_fileName);
	}

	~CsvFile()
	{
		flush();
		m_stream.close();
	}

	void flush()
	{
		m_stream.flush();
	}

	void endrow()
	{
		m_stream << std::endl;
	}

	CsvFile& operator << (CsvFile& (*p_func)(CsvFile&))
	{
		return p_func(*this);
	}

	CsvFile& operator << (const char* p_val)
	{
		m_stream << '"' << p_val << '"' << m_separator;
		return *this;
	}

	CsvFile& operator << (const std::string& p_val)
	{
		m_stream << '"' << p_val << '"' << m_separator;
		return *this;
	}

	template <typename T>
	CsvFile& operator << (const T& p_val)
	{
		m_stream << p_val << m_separator;
		return *this;
	}


private:
	std::ofstream m_stream;
	std::string m_separator;
};

inline static CsvFile& endrow(CsvFile& p_file)
{
	p_file.endrow();
	return p_file;
}

inline static CsvFile& flush(CsvFile& p_file)
{
	p_file.flush();
	return p_file;
}
