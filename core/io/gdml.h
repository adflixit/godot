#pragma once

#include "core/io/resource.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/variant/variant.h"

class GDML : public Resource {
	GDCLASS(GDML, Resource);

	enum TokenType {
		TK_IDENTIFIER,
		TK_CONSTRUCT,
		TK_STRING,
		TK_STRING_NAME,
		TK_NUMBER,
		TK_COLON,
		TK_COMMA,
		TK_EQUAL,
		TK_NEWLINE,
		TK_EOF,
		TK_MAX
	};

	enum Expecting {
		EXPECT_OBJECT,
		EXPECT_OBJECT_KEY,
		EXPECT_COLON,
		EXPECT_OBJECT_VALUE,
	};

	struct Token {
		TokenType type;
		Variant value;
	};

	String text;
	Variant data;
	String err_str;
	int err_line = 0;

	static const char *tk_name[];

	static String _make_indent(const String &p_indent, int p_size);
	static String _stringify(const Variant &p_var, const String &p_indent, int p_cur_indent, bool p_sort_keys, HashSet<const void *> &p_markers, bool p_full_precision = false);
	static Error _get_indent(const char32_t *p_str, int &index, int p_len, int &r_indent, int &line, String &r_err_str);
	static Error _get_token(const char32_t *p_str, int &index, int p_len, Token &r_token, int &line, String &r_err_str);
	static Error _parse_value(Variant &value, Token &token, const char32_t *p_str, int &index, int p_len, int &line, int p_depth, String &r_err_str);
	static Error _parse_array(Array &array, bool p_inline, const char32_t *p_str, int &index, int p_len, int &line, int p_depth, String &r_err_str);
	static Error _parse_records(Array &array, const char32_t *p_str, int &index, int p_len, int &line, int p_depth, String &r_err_str);
	static Error _parse_dictionary(Dictionary &object, const char32_t *p_str, int &index, int p_len, int &line, int p_depth, String &r_err_str);
	static Error _parse_string(const String &p_gdml, Variant &r_ret, String &r_err_str, int &r_err_line);

protected:
	static void _bind_methods();

public:
	Error parse(const String &p_gdml_string, bool p_keep_text = false);
	String get_parsed_text() const;

	static String stringify(const Variant &p_var, const String &p_indent = "", bool p_sort_keys = true, bool p_full_precision = false);
	static Variant parse_string(const String &p_gdml_string);

	inline Variant get_data() const { return data; }
	void set_data(const Variant &p_data);
	inline int get_error_line() const { return err_line; }
	inline String get_error_message() const { return err_str; }

	static Variant from_native(const Variant &p_variant, bool p_allow_classes = false, bool p_allow_scripts = false);
	static Variant to_native(const Variant &p_gdml, bool p_allow_classes = false, bool p_allow_scripts = false);
};

class ResourceFormatLoaderGDML : public ResourceFormatLoader {
public:
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual bool handles_type(const String &p_type) const override;
	virtual String get_resource_type(const String &p_path) const override;
};

class ResourceFormatSaverGDML : public ResourceFormatSaver {
public:
	virtual Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags = 0) override;
	virtual void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const override;
	virtual bool recognize(const Ref<Resource> &p_resource) const override;
};
