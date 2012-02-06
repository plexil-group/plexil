#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE 0

#include <string.h> // because Borland's STL is braindead, we have to include <string.h> _before_ <string> in order to get memcpy

#include "common.hpp"

#include "writer_string.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <sstream>

#include <string>
#include <algorithm>

#ifdef __MINGW32__
#	include <io.h> // for unlink in C++0x mode
#endif

#if defined(__CELLOS_LV2__)
#	include <unistd.h> // for unlink
#endif

TEST(document_create_empty)
{
	pugi::xml_document doc;
	CHECK_NODE(doc, STR(""));
}

TEST(document_create)
{
	pugi::xml_document doc;
	doc.append_child().set_name(STR("node"));
	CHECK_NODE(doc, STR("<node />"));
}

#ifndef PUGIXML_NO_STL
TEST(document_load_stream)
{
	pugi::xml_document doc;

	std::istringstream iss("<node/>");
	CHECK(doc.load(iss));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_stream_offset)
{
	pugi::xml_document doc;

	std::istringstream iss("<foobar> <node/>");

	std::string s;
	iss >> s;

	CHECK(doc.load(iss));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_stream_text)
{
	pugi::xml_document doc;

	std::ifstream iss("tests/data/multiline.xml");
	CHECK(doc.load(iss));
	CHECK_NODE(doc, STR("<node1 /><node2 /><node3 />"));
}

TEST(document_load_stream_error)
{
	pugi::xml_document doc;

	std::ifstream fs("filedoesnotexist");
	CHECK(doc.load(fs).status == status_io_error);
	
	std::istringstream iss("<node/>");
	test_runner::_memory_fail_threshold = 1;
	CHECK(doc.load(iss).status == status_out_of_memory);
}

TEST(document_load_stream_empty)
{
	std::istringstream iss;

	pugi::xml_document doc;
	doc.load(iss); // parse result depends on STL implementation
	CHECK(!doc.first_child());
}

TEST(document_load_stream_wide)
{
	pugi::xml_document doc;

	std::basic_istringstream<wchar_t> iss(L"<node/>");
	CHECK(doc.load(iss));
	CHECK_NODE(doc, STR("<node />"));
}

#ifndef PUGIXML_NO_EXCEPTIONS
TEST(document_load_stream_exceptions)
{
	pugi::xml_document doc;

	// Windows has newline translation for text-mode files, so reading from this stream reaches eof and sets fail|eof bits.
	// This test does not cause stream to throw an exception on Linux - I have no idea how to get read() to fail except
	// newline translation.
	std::ifstream iss("tests/data/multiline.xml");
	iss.exceptions(std::ios::eofbit | std::ios::badbit | std::ios::failbit);

	try
	{
		doc.load(iss);

		CHECK(iss.good()); // if the exception was not thrown, stream reading should succeed without errors
	}
	catch (const std::ios_base::failure&)
	{
		CHECK(!doc.first_child());
	}
}
#endif

TEST(document_load_stream_error_previous)
{
	pugi::xml_document doc;
	CHECK(doc.load(STR("<node/>")));
	CHECK(doc.first_child());

	std::ifstream fs1("filedoesnotexist");
	CHECK(doc.load(fs1).status == status_io_error);
	CHECK(!doc.first_child());
}

TEST(document_load_stream_wide_error_previous)
{
	pugi::xml_document doc;
	CHECK(doc.load(STR("<node/>")));
	CHECK(doc.first_child());

	std::basic_ifstream<wchar_t> fs1("filedoesnotexist");
	CHECK(doc.load(fs1).status == status_io_error);
	CHECK(!doc.first_child());
}

template <typename T> class char_array_buffer: public std::basic_streambuf<T>
{
public:
    char_array_buffer(T* begin, T* end)
    {
        this->setg(begin, begin, end);
    }

    typename std::basic_streambuf<T>::int_type underflow()
    {
        return this->gptr() == this->egptr() ? std::basic_streambuf<T>::traits_type::eof() : std::basic_streambuf<T>::traits_type::to_int_type(*this->gptr());
    }
};

TEST(document_load_stream_nonseekable)
{
    char contents[] = "<node />";
    char_array_buffer<char> buffer(contents, contents + sizeof(contents) / sizeof(contents[0]));
    std::istream in(&buffer);

    pugi::xml_document doc;
    CHECK(doc.load(in));
    CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_stream_wide_nonseekable)
{
    wchar_t contents[] = L"<node />";
    char_array_buffer<wchar_t> buffer(contents, contents + sizeof(contents) / sizeof(contents[0]));
    std::basic_istream<wchar_t> in(&buffer);

    pugi::xml_document doc;
    CHECK(doc.load(in));
    CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_stream_nonseekable_large)
{
	std::basic_string<pugi::char_t> str;
	str += STR("<node>");
	for (int i = 0; i < 10000; ++i) str += STR("<node />");
	str += STR("</node>");

    char_array_buffer<pugi::char_t> buffer(&str[0], &str[0] + str.length());
    std::basic_istream<pugi::char_t> in(&buffer);

    pugi::xml_document doc;
    CHECK(doc.load(in));
    CHECK_NODE(doc, str.c_str());
}
#endif

TEST(document_load_string)
{
	pugi::xml_document doc;

	CHECK(doc.load(STR("<node/>")));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_file)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("tests/data/small.xml"));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_file_empty)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("tests/data/empty.xml"));
	CHECK(!doc.first_child());
}

TEST(document_load_file_large)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("tests/data/large.xml"));

	std::basic_string<pugi::char_t> str;
	str += STR("<node>");
	for (int i = 0; i < 10000; ++i) str += STR("<node />");
	str += STR("</node>");

	CHECK_NODE(doc, str.c_str());
}

TEST(document_load_file_error)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("filedoesnotexist").status == status_file_not_found);

#ifdef _WIN32
	CHECK(doc.load_file("con").status == status_io_error);
#endif

	test_runner::_memory_fail_threshold = 1;
	CHECK(doc.load_file("tests/data/small.xml").status == status_out_of_memory);
}

TEST(document_load_file_error_previous)
{
	pugi::xml_document doc;
	CHECK(doc.load(STR("<node/>")));
	CHECK(doc.first_child());

	CHECK(doc.load_file("filedoesnotexist").status == status_file_not_found);
	CHECK(!doc.first_child());
}

TEST(document_load_file_wide_ascii)
{
	pugi::xml_document doc;

	CHECK(doc.load_file(L"tests/data/small.xml"));
	CHECK_NODE(doc, STR("<node />"));
}

TEST_XML(document_save, "<node/>")
{
	xml_writer_string writer;

	doc.save(writer, STR(""), pugi::format_no_declaration | pugi::format_raw, get_native_encoding());

	CHECK(writer.as_string() == STR("<node />"));
}

#ifndef PUGIXML_NO_STL
TEST_XML(document_save_stream, "<node/>")
{
	std::ostringstream oss;

	doc.save(oss, STR(""), pugi::format_no_declaration | pugi::format_raw);

	CHECK(oss.str() == "<node />");
}

TEST_XML(document_save_stream_wide, "<node/>")
{
	std::basic_ostringstream<wchar_t> oss;

	doc.save(oss, STR(""), pugi::format_no_declaration | pugi::format_raw);

	CHECK(oss.str() == L"<node />");
}
#endif

TEST_XML(document_save_bom, "<n/>")
{
	unsigned int flags = format_no_declaration | format_raw | format_write_bom;

	// specific encodings
	CHECK(test_save_narrow(doc, flags, encoding_utf8, "\xef\xbb\xbf<n />", 8));
	CHECK(test_save_narrow(doc, flags, encoding_utf16_be, "\xfe\xff\x00<\x00n\x00 \x00/\x00>", 12));
	CHECK(test_save_narrow(doc, flags, encoding_utf16_le, "\xff\xfe<\x00n\x00 \x00/\x00>\x00", 12));
	CHECK(test_save_narrow(doc, flags, encoding_utf32_be, "\x00\x00\xfe\xff\x00\x00\x00<\x00\x00\x00n\x00\x00\x00 \x00\x00\x00/\x00\x00\x00>", 24));
	CHECK(test_save_narrow(doc, flags, encoding_utf32_le, "\xff\xfe\x00\x00<\x00\x00\x00n\x00\x00\x00 \x00\x00\x00/\x00\x00\x00>\x00\x00\x00", 24));

	// encodings synonyms
	CHECK(save_narrow(doc, flags, encoding_utf16) == save_narrow(doc, flags, (is_little_endian() ? encoding_utf16_le : encoding_utf16_be)));
	CHECK(save_narrow(doc, flags, encoding_utf32) == save_narrow(doc, flags, (is_little_endian() ? encoding_utf32_le : encoding_utf32_be)));

	size_t wcharsize = sizeof(wchar_t);
	CHECK(save_narrow(doc, flags, encoding_wchar) == save_narrow(doc, flags, (wcharsize == 2 ? encoding_utf16 : encoding_utf32)));
}

TEST_XML(document_save_declaration, "<node/>")
{
	xml_writer_string writer;

	doc.save(writer, STR(""), pugi::format_default, get_native_encoding());

	CHECK(writer.as_string() == STR("<?xml version=\"1.0\"?>\n<node />\n"));
}

TEST(document_save_declaration_empty)
{
	xml_document doc;

	xml_writer_string writer;

	doc.save(writer, STR(""), pugi::format_default, get_native_encoding());

	CHECK(writer.as_string() == STR("<?xml version=\"1.0\"?>\n"));
}

TEST_XML(document_save_declaration_present_first, "<node/>")
{
	doc.insert_child_before(node_declaration, doc.first_child()).append_attribute(STR("encoding")) = STR("utf8");

	xml_writer_string writer;

	doc.save(writer, STR(""), pugi::format_default, get_native_encoding());

	CHECK(writer.as_string() == STR("<?xml encoding=\"utf8\"?>\n<node />\n"));
}

TEST_XML(document_save_declaration_present_second, "<node/>")
{
	doc.insert_child_before(node_declaration, doc.first_child()).append_attribute(STR("encoding")) = STR("utf8");
	doc.insert_child_before(node_comment, doc.first_child()).set_value(STR("text"));

	xml_writer_string writer;

	doc.save(writer, STR(""), pugi::format_default, get_native_encoding());

	CHECK(writer.as_string() == STR("<!--text-->\n<?xml encoding=\"utf8\"?>\n<node />\n"));
}

TEST_XML(document_save_declaration_present_last, "<node/>")
{
	doc.append_child(node_declaration).append_attribute(STR("encoding")) = STR("utf8");

	xml_writer_string writer;

	doc.save(writer, STR(""), pugi::format_default, get_native_encoding());

	// node writer only looks for declaration before the first element child
	CHECK(writer.as_string() == STR("<?xml version=\"1.0\"?>\n<node />\n<?xml encoding=\"utf8\"?>\n"));
}

struct temp_file
{
	char path[512];
	int fd;
	
	temp_file(): fd(0)
	{
	#ifdef __unix
		strcpy(path, "/tmp/pugiXXXXXX");

		fd = mkstemp(path);
		CHECK(fd != -1);
	#elif defined(__CELLOS_LV2__)
		path[0] = 0; // no temporary file support
	#else
		tmpnam(path);
	#endif
	}

	~temp_file()
	{
		CHECK(unlink(path) == 0);

	#ifdef __unix
		CHECK(close(fd) == 0);
	#endif
	}
};

TEST_XML(document_save_file, "<node/>")
{
	temp_file f;

	CHECK(doc.save_file(f.path));

	CHECK(doc.load_file(f.path, pugi::parse_default | pugi::parse_declaration));
	CHECK_NODE(doc, STR("<?xml version=\"1.0\"?><node />"));
}

TEST_XML(document_save_file_wide, "<node/>")
{
	temp_file f;

	// widen the path
	wchar_t wpath[32];
	std::copy(f.path, f.path + strlen(f.path) + 1, wpath + 0);

	CHECK(doc.save_file(wpath));

	CHECK(doc.load_file(f.path, pugi::parse_default | pugi::parse_declaration));
	CHECK_NODE(doc, STR("<?xml version=\"1.0\"?><node />"));
}

TEST_XML(document_save_file_error, "<node/>")
{
	CHECK(!doc.save_file("tests/data/unknown/output.xml"));
}

TEST(document_load_buffer)
{
	const pugi::char_t text[] = STR("<?xml?><node/>");

	pugi::xml_document doc;

	CHECK(doc.load_buffer(text, sizeof(text)));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_buffer_inplace)
{
	pugi::char_t text[] = STR("<?xml?><node/>");

	pugi::xml_document doc;

	CHECK(doc.load_buffer_inplace(text, sizeof(text)));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_buffer_inplace_own)
{
	allocation_function alloc = get_memory_allocation_function();

	size_t size = strlen("<?xml?><node/>") * sizeof(pugi::char_t);

	pugi::char_t* text = static_cast<pugi::char_t*>(alloc(size));
	CHECK(text);

	memcpy(text, STR("<?xml?><node/>"), size);

	pugi::xml_document doc;

	CHECK(doc.load_buffer_inplace_own(text, size));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_parse_result_bool)
{
	xml_parse_result result;

	result.status = status_ok;
	CHECK(result);
	CHECK(!!result);
	CHECK(result == true);

	for (int i = 1; i < 20; ++i)
	{
		result.status = static_cast<xml_parse_status>(i);
		CHECK(!result);
		CHECK(result == false);
	}
}

TEST(document_parse_result_description)
{
	xml_parse_result result;

	for (int i = 0; i < 20; ++i)
	{
		result.status = static_cast<xml_parse_status>(i);

		CHECK(result.description() != 0);
		CHECK(result.description()[0] != 0);
	}
}

TEST(document_load_fail)
{
	xml_document doc;
	CHECK(!doc.load(STR("<foo><bar/>")));
	CHECK(doc.child(STR("foo")).child(STR("bar")));
}

inline void check_utftest_document(const xml_document& doc)
{
	// ascii text
	CHECK_STRING(doc.last_child().first_child().name(), STR("English"));

	// check that we have parsed some non-ascii text
	CHECK(static_cast<unsigned int>(doc.last_child().last_child().name()[0]) >= 0x80);

	// check magic string
	const pugi::char_t* v = doc.last_child().child(STR("Heavy")).previous_sibling().child_value();

#ifdef PUGIXML_WCHAR_MODE
	CHECK(v[0] == 0x4e16 && v[1] == 0x754c && v[2] == 0x6709 && v[3] == 0x5f88 && v[4] == 0x591a && v[5] == 0x8bed && v[6] == 0x8a00);

	// last character is a surrogate pair
	unsigned int v7 = v[7];
	size_t wcharsize = sizeof(wchar_t);

	CHECK(wcharsize == 2 ? (v[7] == 0xd852 && v[8] == 0xdf62) : (v7 == 0x24b62));
#else
	// unicode string
	CHECK_STRING(v, "\xe4\xb8\x96\xe7\x95\x8c\xe6\x9c\x89\xe5\xbe\x88\xe5\xa4\x9a\xe8\xaf\xad\xe8\xa8\x80\xf0\xa4\xad\xa2");
#endif
}

TEST(document_load_file_convert_auto)
{
	const char* files[] =
	{
		"tests/data/utftest_utf16_be.xml",
		"tests/data/utftest_utf16_be_bom.xml",
		"tests/data/utftest_utf16_be_nodecl.xml",
		"tests/data/utftest_utf16_le.xml",
		"tests/data/utftest_utf16_le_bom.xml",
		"tests/data/utftest_utf16_le_nodecl.xml",
		"tests/data/utftest_utf32_be.xml",
		"tests/data/utftest_utf32_be_bom.xml",
		"tests/data/utftest_utf32_be_nodecl.xml",
		"tests/data/utftest_utf32_le.xml",
		"tests/data/utftest_utf32_le_bom.xml",
		"tests/data/utftest_utf32_le_nodecl.xml",
		"tests/data/utftest_utf8.xml",
		"tests/data/utftest_utf8_bom.xml",
		"tests/data/utftest_utf8_nodecl.xml"
	};

	xml_encoding encodings[] =
	{
		encoding_utf16_be, encoding_utf16_be, encoding_utf16_be,
		encoding_utf16_le, encoding_utf16_le, encoding_utf16_le,
		encoding_utf32_be, encoding_utf32_be, encoding_utf32_be,
		encoding_utf32_le, encoding_utf32_le, encoding_utf32_le,
		encoding_utf8, encoding_utf8, encoding_utf8
	};

	for (unsigned int i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		xml_document doc;
		xml_parse_result res = doc.load_file(files[i]);

		CHECK(res);
		CHECK(res.encoding == encodings[i]);
		check_utftest_document(doc);
	}
}

TEST(document_load_file_convert_specific)
{
	const char* files[] =
	{
		"tests/data/utftest_utf16_be.xml",
		"tests/data/utftest_utf16_be_bom.xml",
		"tests/data/utftest_utf16_be_nodecl.xml",
		"tests/data/utftest_utf16_le.xml",
		"tests/data/utftest_utf16_le_bom.xml",
		"tests/data/utftest_utf16_le_nodecl.xml",
		"tests/data/utftest_utf32_be.xml",
		"tests/data/utftest_utf32_be_bom.xml",
		"tests/data/utftest_utf32_be_nodecl.xml",
		"tests/data/utftest_utf32_le.xml",
		"tests/data/utftest_utf32_le_bom.xml",
		"tests/data/utftest_utf32_le_nodecl.xml",
		"tests/data/utftest_utf8.xml",
		"tests/data/utftest_utf8_bom.xml",
		"tests/data/utftest_utf8_nodecl.xml"
	};

	xml_encoding encodings[] =
	{
		encoding_utf16_be, encoding_utf16_be, encoding_utf16_be,
		encoding_utf16_le, encoding_utf16_le, encoding_utf16_le,
		encoding_utf32_be, encoding_utf32_be, encoding_utf32_be,
		encoding_utf32_le, encoding_utf32_le, encoding_utf32_le,
		encoding_utf8, encoding_utf8, encoding_utf8
	};

	for (unsigned int i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		for (unsigned int j = 0; j < sizeof(files) / sizeof(files[0]); ++j)
		{
			xml_encoding encoding = encodings[j];

			xml_document doc;
			xml_parse_result res = doc.load_file(files[i], parse_default, encoding);

			if (encoding == encodings[i])
			{
				CHECK(res);
				CHECK(res.encoding == encoding);
				check_utftest_document(doc);
			}
			else
			{
				// should not get past first tag
				CHECK(!doc.first_child());
			}
		}
	}
}

TEST(document_load_file_convert_native_endianness)
{
	const char* files[2][6] =
	{
		{
			"tests/data/utftest_utf16_be.xml",
			"tests/data/utftest_utf16_be_bom.xml",
			"tests/data/utftest_utf16_be_nodecl.xml",
			"tests/data/utftest_utf32_be.xml",
			"tests/data/utftest_utf32_be_bom.xml",
			"tests/data/utftest_utf32_be_nodecl.xml",
		},
		{
			"tests/data/utftest_utf16_le.xml",
			"tests/data/utftest_utf16_le_bom.xml",
			"tests/data/utftest_utf16_le_nodecl.xml",
			"tests/data/utftest_utf32_le.xml",
			"tests/data/utftest_utf32_le_bom.xml",
			"tests/data/utftest_utf32_le_nodecl.xml",
		}
	};

	xml_encoding encodings[] =
	{
		encoding_utf16, encoding_utf16, encoding_utf16,
		encoding_utf32, encoding_utf32, encoding_utf32
	};

	for (unsigned int i = 0; i < sizeof(files[0]) / sizeof(files[0][0]); ++i)
	{
		const char* right_file = files[is_little_endian()][i];
		const char* wrong_file = files[!is_little_endian()][i];

		for (unsigned int j = 0; j < sizeof(encodings) / sizeof(encodings[0]); ++j)
		{
			xml_encoding encoding = encodings[j];

			// check file with right endianness
			{
				xml_document doc;
				xml_parse_result res = doc.load_file(right_file, parse_default, encoding);

				if (encoding == encodings[i])
				{
					CHECK(res);
					check_utftest_document(doc);
				}
				else
				{
					// should not get past first tag
					CHECK(!doc.first_child());
				}
			}

			// check file with wrong endianness
			{
				xml_document doc;
				doc.load_file(wrong_file, parse_default, encoding);
				CHECK(!doc.first_child());
			}
		}
	}
}

static bool load_file_in_memory(const char* path, char*& data, size_t& size)
{
	FILE* file = fopen(path, "rb");
	if (!file) return false;

	fseek(file, 0, SEEK_END);
	size = static_cast<size_t>(ftell(file));
	fseek(file, 0, SEEK_SET);

	data = new char[size];

	CHECK(fread(data, 1, size, file) == size);
	fclose(file);

	return true;
}

TEST(document_contents_preserve)
{
	struct file_t
	{
		const char* path;
		xml_encoding encoding;

		char* data;
		size_t size;
	};

	file_t files[] =
	{
		{"tests/data/utftest_utf16_be_clean.xml", encoding_utf16_be, 0, 0},
		{"tests/data/utftest_utf16_le_clean.xml", encoding_utf16_le, 0, 0},
		{"tests/data/utftest_utf32_be_clean.xml", encoding_utf32_be, 0, 0},
		{"tests/data/utftest_utf32_le_clean.xml", encoding_utf32_le, 0, 0},
		{"tests/data/utftest_utf8_clean.xml", encoding_utf8, 0, 0}
	};

	// load files in memory
	for (unsigned int i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		CHECK(load_file_in_memory(files[i].path, files[i].data, files[i].size));
	}

	// convert each file to each format and compare bitwise
	for (unsigned int src = 0; src < sizeof(files) / sizeof(files[0]); ++src)
	{
		for (unsigned int dst = 0; dst < sizeof(files) / sizeof(files[0]); ++dst)
		{
			// parse into document (preserve comments, declaration and whitespace pcdata)
			xml_document doc;
			CHECK(doc.load_buffer(files[src].data, files[src].size, parse_default | parse_ws_pcdata | parse_declaration | parse_comments));

			// compare saved document with the original (raw formatting, without extra declaration, write bom if it was in original file)
			CHECK(test_save_narrow(doc, format_raw | format_no_declaration | format_write_bom, files[dst].encoding, files[dst].data, files[dst].size));
		}
	}

	// cleanup
	for (unsigned int j = 0; j < sizeof(files) / sizeof(files[0]); ++j)
	{
		delete[] files[j].data;
	}
}

static bool test_parse_fail(const void* buffer, size_t size, xml_encoding encoding = encoding_utf8)
{
	// copy buffer to heap (to enable out-of-bounds checks)
	void* temp = malloc(size);
	memcpy(temp, buffer, size);

	// check that this parses without buffer overflows (yielding an error)
	xml_document doc;
	bool result = doc.load_buffer_inplace(temp, size, parse_default, encoding);

	free(temp);

	return !result;
}

TEST(document_convert_invalid_utf8)
{
	// invalid 1-byte input
	CHECK(test_parse_fail("<\xb0", 2));

	// invalid 2-byte input
	CHECK(test_parse_fail("<\xc0", 2));
	CHECK(test_parse_fail("<\xd0", 2));

	// invalid 3-byte input
	CHECK(test_parse_fail("<\xe2\x80", 3));
	CHECK(test_parse_fail("<\xe2", 2));

	// invalid 4-byte input
	CHECK(test_parse_fail("<\xf2\x97\x98", 4));
	CHECK(test_parse_fail("<\xf2\x97", 3));
	CHECK(test_parse_fail("<\xf2", 2));

	// invalid 5-byte input
	CHECK(test_parse_fail("<\xf8", 2));
}

TEST(document_convert_invalid_utf16)
{
	// check non-terminated degenerate handling
	CHECK(test_parse_fail("\x00<\xda\x1d", 4, encoding_utf16_be));
	CHECK(test_parse_fail("<\x00\x1d\xda", 4, encoding_utf16_le));

	// check incorrect leading code
	CHECK(test_parse_fail("\x00<\xde\x24", 4, encoding_utf16_be));
	CHECK(test_parse_fail("<\x00\x24\xde", 4, encoding_utf16_le));
}

TEST(document_load_buffer_empty)
{
	xml_encoding encodings[] =
	{
		encoding_auto,
		encoding_utf8,
		encoding_utf16_le,
		encoding_utf16_be,
		encoding_utf16,
		encoding_utf32_le,
		encoding_utf32_be,
		encoding_utf32,
		encoding_wchar
	};

	char buffer[1];

	for (unsigned int i = 0; i < sizeof(encodings) / sizeof(encodings[0]); ++i)
	{
		xml_encoding encoding = encodings[i];

		xml_document doc;
		CHECK(doc.load_buffer(buffer, 0, parse_default, encoding) && !doc.first_child());
		CHECK(doc.load_buffer(0, 0, parse_default, encoding) && !doc.first_child());

		CHECK(doc.load_buffer_inplace(buffer, 0, parse_default, encoding) && !doc.first_child());
		CHECK(doc.load_buffer_inplace(0, 0, parse_default, encoding) && !doc.first_child());

		void* own_buffer = pugi::get_memory_allocation_function()(1);

		CHECK(doc.load_buffer_inplace_own(own_buffer, 0, parse_default, encoding) && !doc.first_child());
		CHECK(doc.load_buffer_inplace_own(0, 0, parse_default, encoding) && !doc.first_child());
	}
}

TEST(document_progressive_truncation)
{
	char* original_data;
	size_t original_size;

	CHECK(load_file_in_memory("tests/data/truncation.xml", original_data, original_size));

	char* buffer = new char[original_size];

	for (size_t i = 1; i < original_size; ++i)
	{
		char* truncated_data = buffer + original_size - i;

		memcpy(truncated_data, original_data, i);

		xml_document doc;
		bool result = doc.load_buffer_inplace(truncated_data, i);

		// some truncate locations are parseable - those that come after declaration, declaration + doctype, declaration + doctype + comment and eof
		CHECK(((i - 21) < 3 || (i - 66) < 3 || (i - 95) < 3 || i >= 3325) ? result : !result);
	}

	delete[] buffer;
	delete[] original_data;
}

TEST(document_load_buffer_short)
{
	char* data = new char[4];
	memcpy(data, "abcd", 4);

	xml_document doc;

	CHECK(doc.load_buffer(data, 4));
	CHECK(doc.load_buffer(data + 1, 3));
	CHECK(doc.load_buffer(data + 2, 2));
	CHECK(doc.load_buffer(data + 3, 1));
	CHECK(doc.load_buffer(data + 4, 0));
	CHECK(doc.load_buffer(0, 0));

	delete[] data;
}

TEST(document_load_buffer_inplace_short)
{
	char* data = new char[4];
	memcpy(data, "abcd", 4);

	xml_document doc;

	CHECK(doc.load_buffer_inplace(data, 4));
	CHECK(doc.load_buffer_inplace(data + 1, 3));
	CHECK(doc.load_buffer_inplace(data + 2, 2));
	CHECK(doc.load_buffer_inplace(data + 3, 1));
	CHECK(doc.load_buffer_inplace(data + 4, 0));
	CHECK(doc.load_buffer_inplace(0, 0));

	delete[] data;
}

#ifndef PUGIXML_NO_EXCEPTIONS
TEST(document_load_exceptions)
{
    bool thrown = false;

    try
    {
        pugi::xml_document doc;
        if (!doc.load(STR("<node attribute='value"))) throw std::bad_alloc();

        CHECK_FORCE_FAIL("Expected parsing failure");
    }
    catch (const std::bad_alloc&)
    {
        thrown = true;
    }

    CHECK(thrown);
}
#endif

TEST_XML_FLAGS(document_element, "<?xml version='1.0'?><node><child/></node><!---->", parse_default | parse_declaration | parse_comments)
{
    CHECK(doc.document_element() == doc.child(STR("node")));
}

TEST_XML_FLAGS(document_element_absent, "<!---->", parse_comments)
{
    CHECK(doc.document_element() == xml_node());
}

TEST_XML(document_reset, "<node><child/></node>")
{
    CHECK(doc.first_child());

    doc.reset();
    CHECK(!doc.first_child());
    CHECK_NODE(doc, STR(""));

    doc.reset();
    CHECK(!doc.first_child());
    CHECK_NODE(doc, STR(""));

    CHECK(doc.load(STR("<node/>")));
    CHECK(doc.first_child());
    CHECK_NODE(doc, STR("<node />"));

    doc.reset();
    CHECK(!doc.first_child());
    CHECK_NODE(doc, STR(""));
}

TEST(document_reset_empty)
{
    xml_document doc;

    doc.reset();
    CHECK(!doc.first_child());
    CHECK_NODE(doc, STR(""));
}

TEST_XML(document_reset_copy, "<node><child/></node>")
{
    xml_document doc2;

    CHECK_NODE(doc2, STR(""));

    doc2.reset(doc);

    CHECK_NODE(doc2, STR("<node><child /></node>"));
    CHECK(doc.first_child() != doc2.first_child());

    doc.reset(doc2);

    CHECK_NODE(doc, STR("<node><child /></node>"));
    CHECK(doc.first_child() != doc2.first_child());

    CHECK(doc.first_child().offset_debug() == -1);
}

TEST_XML(document_reset_copy_self, "<node><child/></node>")
{
    CHECK_NODE(doc, STR("<node><child /></node>"));

    doc.reset(doc);

    CHECK(!doc.first_child());
    CHECK_NODE(doc, STR(""));
}
