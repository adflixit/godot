#include "gdml.h"

#include "core/config/engine.h"
#include "core/string/print_string.h"

const char *GDML::tk_name[TK_MAX] = {
	"end of line",
	"identifier",
	"':'",
	"'='",
	"','",
	"'#'",
	"newline"
};

String GDML::_make_indent(const String &p_indent, int p_size) {
	return p_indent.repeat(p_size);
}

String GDML::_stringify(const Variant &p_var, const String &p_indent, int p_cur_indent, bool p_sort_keys, HashSet<const void *> &p_markers, bool p_full_precision) {
	ERR_FAIL_COND_V_MSG(p_cur_indent > Variant::MAX_RECURSION_DEPTH, "...", "GDML structure is too deep. Bailing.");

	String colon = ":";
	String end_statement = "";

	if (!p_indent.is_empty()) {
		colon += " ";
		end_statement += "\n";
	}

	switch (p_var.get_type()) {
		case Variant::NIL:
			return "null";
		case Variant::BOOL:
			return p_var.operator bool() ? "true" : "false";
		case Variant::INT:
			return itos(p_var);
		case Variant::FLOAT: {
			double num = p_var;
			if (p_full_precision) {
				// Store unreliable digits (17) instead of just reliable
				// digits (14) so that the value can be decoded exactly.
				return String::num(num, 17 - (int)floor(log10(num)));
			} else {
				// Store only reliable digits (14) by default.
				return String::num(num, 14 - (int)floor(log10(num)));
			}
		}
		case Variant::PACKED_INT32_ARRAY:
		case Variant::PACKED_INT64_ARRAY:
		case Variant::PACKED_FLOAT32_ARRAY:
		case Variant::PACKED_FLOAT64_ARRAY:
		case Variant::PACKED_STRING_ARRAY:
		case Variant::ARRAY: {
			Array a = p_var;
			if (a.is_empty()) {
				return "[]";
			}
			String s = "[";
			s += end_statement;

			ERR_FAIL_COND_V_MSG(p_markers.has(a.id()), "\"[...]\"", "Converting circular structure to GDML.");
			p_markers.insert(a.id());

			bool first = true;
			for (const Variant &var : a) {
				if (first) {
					first = false;
				} else {
					s += ",";
					s += end_statement;
				}
				s += _make_indent(p_indent, p_cur_indent + 1) + _stringify(var, p_indent, p_cur_indent + 1, p_sort_keys, p_markers);
			}
			s += end_statement + _make_indent(p_indent, p_cur_indent) + "]";
			p_markers.erase(a.id());
			return s;
		}
		case Variant::DICTIONARY: {
			String s = "{";
			s += end_statement;
			Dictionary d = p_var;

			ERR_FAIL_COND_V_MSG(p_markers.has(d.id()), "\"{...}\"", "Converting circular structure to GDML.");
			p_markers.insert(d.id());

			List<Variant> keys;
			d.get_key_list(&keys);

			if (p_sort_keys) {
				keys.sort_custom<StringLikeVariantOrder>();
			}

			bool first_key = true;
			for (const Variant &E : keys) {
				if (first_key) {
					first_key = false;
				} else {
					s += ",";
					s += end_statement;
				}
				s += _make_indent(p_indent, p_cur_indent + 1) + _stringify(String(E), p_indent, p_cur_indent + 1, p_sort_keys, p_markers);
				s += colon;
				s += _stringify(d[E], p_indent, p_cur_indent + 1, p_sort_keys, p_markers);
			}

			s += end_statement + _make_indent(p_indent, p_cur_indent) + "}";
			p_markers.erase(d.id());
			return s;
		}
		default:
			return "\"" + String(p_var).gdml_escape() + "\"";
	}
}

Error GDML::_get_indent(const char32_t *p_str, int &index, int p_len, int &r_indent, int &line, String &r_err_str) {
	while (p_len > 0) {
		switch (p_str[index]) {
			case '\t': {
				r_indent++;
				index++;
				return OK;
			}
			case ' ': {
				r_err_str = "Inconsistent indentation.";
				return ERR_PARSE_ERROR;
			}
			default: {
				return OK;
			}
		}
	}

	return ERR_PARSE_ERROR;
}

Error GDML::_get_token(const char32_t *p_str, int &index, int p_len, Token &r_token, int &line, String &r_err_str) {
	bool string_name = false;

	while (p_len > 0) {
		switch (p_str[index]) {
			case ':': {
				r_token.type = TK_COLON;
				index++;
				return OK;
			}
			case ',': {
				r_token.type = TK_COMMA;
				index++;
				return OK;
			}
			case '=': {
				r_token.type = TK_EQUAL;
				index++;
				return OK;
			}
			case '\n': {
				r_token.type = TK_NEWLINE;
				index++;
				line++;
				return OK;
			}
			case '#': {
				while (true) {
					index++;
					if (p_str[index] == 0) {
						r_token.type = TK_EOF;
						return OK;
					}
					if (p_str[index] == '\n') {
						break;
					}
				}
				[[fallthrough]];
			}
			case 0: {
				r_token.type = TK_EOF;
				return OK;
			}
			case '&': { // StringName.
				cchar = p_stream->get_char();
				if (cchar != '"') {
					r_err_str = "Expected '\"' after '&'";
					r_token.type = TK_ERROR;
					return ERR_PARSE_ERROR;
				}

				string_name = true;
				[[fallthrough]];
			}
			case '"': {
				String str;
				char32_t prev = 0;
				while (true) {
					char32_t ch = p_stream->get_char();

					if (ch == 0) {
						r_err_str = "Unterminated String";
						r_token.type = TK_ERROR;
						return ERR_PARSE_ERROR;
					} else if (ch == '"') {
						break;
					} else if (ch == '\\') {
						//escaped characters...
						char32_t next = p_stream->get_char();
						if (next == 0) {
							r_err_str = "Unterminated String";
							r_token.type = TK_ERROR;
							return ERR_PARSE_ERROR;
						}
						char32_t res = 0;

						switch (next) {
							case 'b':
								res = 8;
								break;
							case 't':
								res = 9;
								break;
							case 'n':
								res = 10;
								break;
							case 'f':
								res = 12;
								break;
							case 'r':
								res = 13;
								break;
							case 'U':
							case 'u': {
								// Hexadecimal sequence.
								int hex_len = (next == 'U') ? 6 : 4;
								for (int j = 0; j < hex_len; j++) {
									char32_t c = p_stream->get_char();

									if (c == 0) {
										r_err_str = "Unterminated String";
										r_token.type = TK_ERROR;
										return ERR_PARSE_ERROR;
									}
									if (!is_hex_digit(c)) {
										r_err_str = "Malformed hex constant in string";
										r_token.type = TK_ERROR;
										return ERR_PARSE_ERROR;
									}
									char32_t v;
									if (is_digit(c)) {
										v = c - '0';
									} else if (c >= 'a' && c <= 'f') {
										v = c - 'a';
										v += 10;
									} else if (c >= 'A' && c <= 'F') {
										v = c - 'A';
										v += 10;
									} else {
										ERR_PRINT("Bug parsing hex constant.");
										v = 0;
									}

									res <<= 4;
									res |= v;
								}

							} break;
							default: {
								res = next;
							} break;
						}

						// Parse UTF-16 pair.
						if ((res & 0xfffffc00) == 0xd800) {
							if (prev == 0) {
								prev = res;
								continue;
							} else {
								r_err_str = "Invalid UTF-16 sequence in string, unpaired lead surrogate";
								r_token.type = TK_ERROR;
								return ERR_PARSE_ERROR;
							}
						} else if ((res & 0xfffffc00) == 0xdc00) {
							if (prev == 0) {
								r_err_str = "Invalid UTF-16 sequence in string, unpaired trail surrogate";
								r_token.type = TK_ERROR;
								return ERR_PARSE_ERROR;
							} else {
								res = (prev << 10UL) + res - ((0xd800 << 10UL) + 0xdc00 - 0x10000);
								prev = 0;
							}
						}
						if (prev != 0) {
							r_err_str = "Invalid UTF-16 sequence in string, unpaired lead surrogate";
							r_token.type = TK_ERROR;
							return ERR_PARSE_ERROR;
						}
						str += res;
					} else {
						if (prev != 0) {
							r_err_str = "Invalid UTF-16 sequence in string, unpaired lead surrogate";
							r_token.type = TK_ERROR;
							return ERR_PARSE_ERROR;
						}
						if (ch == '\n') {
							line++;
						}
						str += ch;
					}
				}
				if (prev != 0) {
					r_err_str = "Invalid UTF-16 sequence in string, unpaired lead surrogate";
					r_token.type = TK_ERROR;
					return ERR_PARSE_ERROR;
				}

				if (p_stream->is_utf8()) {
					str.parse_utf8(str.ascii(true).get_data());
				}
				if (string_name) {
					r_token.type = TK_STRING_NAME;
					r_token.value = StringName(str);
				} else {
					r_token.type = TK_STRING;
					r_token.value = str;
				}
				return OK;

			} break;
			default: {
				if (cchar <= 32) {
					break;
				}

				if (cchar == '-' || (cchar >= '0' && cchar <= '9')) {
					//a number

					StringBuffer<> num;
#define READING_SIGN 0
#define READING_INT 1
#define READING_DEC 2
#define READING_EXP 3
#define READING_DONE 4
					int reading = READING_INT;

					if (cchar == '-') {
						num += '-';
						cchar = p_stream->get_char();
					}

					char32_t c = cchar;
					bool exp_sign = false;
					bool exp_beg = false;
					bool is_float = false;

					while (true) {
						switch (reading) {
							case READING_INT: {
								if (is_digit(c)) {
									//pass
								} else if (c == '.') {
									reading = READING_DEC;
									is_float = true;
								} else if (c == 'e') {
									reading = READING_EXP;
									is_float = true;
								} else {
									reading = READING_DONE;
								}

							} break;
							case READING_DEC: {
								if (is_digit(c)) {
								} else if (c == 'e') {
									reading = READING_EXP;
								} else {
									reading = READING_DONE;
								}

							} break;
							case READING_EXP: {
								if (is_digit(c)) {
									exp_beg = true;

								} else if ((c == '-' || c == '+') && !exp_sign && !exp_beg) {
									exp_sign = true;

								} else {
									reading = READING_DONE;
								}
							} break;
						}

						if (reading == READING_DONE) {
							break;
						}
						num += c;
						c = p_stream->get_char();
					}

					p_stream->saved = c;

					r_token.type = TK_NUMBER;

					if (is_float) {
						r_token.value = num.as_double();
					} else {
						r_token.value = num.as_int();
					}
					return OK;
				} else if (is_ascii_alphabet_char(cchar) || is_underscore(cchar)) {
					StringBuffer<> id;
					bool first = true;

					while (is_ascii_alphabet_char(cchar) || is_underscore(cchar) || (!first && is_digit(cchar))) {
						id += cchar;
						cchar = p_stream->get_char();
						first = false;
					}

					p_stream->saved = cchar;

					r_token.type = TK_IDENTIFIER;
					r_token.value = id.as_string();
					return OK;
				} else {
					r_err_str = "Unexpected character.";
					r_token.type = TK_ERROR;
					return ERR_PARSE_ERROR;
				}
			}
		}
	}

	return ERR_PARSE_ERROR;
}

Error GDML::_parse_value(Variant &value, Token &token, const char32_t *p_str, int &index, int p_len, int &line, int p_depth, String &r_err_str) {
	int start_index = index;
	bool in_string = false;
	bool in_parentheses = false;

	while (true) {
		Error err = _get_token(p_str, index, p_len, token, line, r_err_str);
		if (err) {
			return err;
		}

		if (token.type == TK_NEWLINE || token.type == TK_EOF) {
			if (in_string) {
				r_err_str = "Unterminated String.";
				return ERR_PARSE_ERROR;
			}
			if (in_parentheses) {
				r_err_str = "Expected closing parenthesis.";
				return ERR_PARSE_ERROR;
			}
			break;
		}

		if (token.type == TK_QUOTATION_MARK) {
			in_string = !in_string;
			continue;
		}

		if (!in_string) {
			if (token.type == TK_PARENTHESIS_OPEN) {
				in_parentheses = true;
				continue;
			}
			if (token.type == TK_PARENTHESIS_CLOSE) {
				in_parentheses = false;
				continue;
			}
			if (token.type == TK_COMMA && !in_parentheses) {
				break;
			}
		}
	}

	VariantParser::StreamString stream;
	stream.s.copy_from(p_str, index - start_index - 1);

	VariantParser::Token token;
	int lines = 0;
	VariantParser::get_token(&stream, token, lines, r_err_str);
	if (err) {
		return err;
	}

	VariantParser::parse_value(token, value, &stream, lines, r_err_str);
	if (err) {
		return err;
	}

	return OK;
}

Error GDML::_parse_array(Array &array, const char32_t *p_str, int &index, int p_len, int &line, int p_depth, String &r_err_str) {
	Token token;
	bool is_inline = false;
	bool need_comma = false;

	while (index < p_len) {
		Error err = _get_token(p_str, index, p_len, token, line, r_err_str);
		if (err) {
			return err;
		}

		if (token.type == TK_NEWLINE) {
			return OK;
		}

		if (need_comma) {
			if (token.type != TK_COMMA) {
				r_err_str = "Expected ','";
				return ERR_PARSE_ERROR;
			} else {
				need_comma = false;
				continue;
			}
		}

		Variant v;
		err = _parse_value(v, token, p_str, index, p_len, line, p_depth, r_err_str);
		if (err) {
			return err;
		}

		array.push_back(v);
		need_comma = true;
	}

	r_err_str = "Expected ']'";
	return ERR_PARSE_ERROR;
}

Error GDML::_parse_dictionary(Dictionary &dict, const char32_t *p_str, int &index, int p_len, int &line, int p_depth, int p_indent, String &r_err_str) {
	if (p_depth > Variant::MAX_RECURSION_DEPTH) {
		r_err_str = "GDML structure is too deep. Bailing.";
		return ERR_OUT_OF_MEMORY;
	}

	bool at_key = true;
	String key;
	Token token;
	bool need_comma = false;
	int indent = 0;

	while (index < p_len) {
		Error err = _get_indent(str, idx, len, indent, r_err_line, r_err_str);
		if (err) {
			return err;
		}

		err = _get_token(p_str, index, p_len, token, line, r_err_str);
		if (err) {
			return err;
		}

		if {token.type == TK_IDENTIFIER) {
			key = token.value;
			err = _get_token(p_str, index, p_len, token, line, r_err_str);
			if (err) {
				return err;
			}

			if (token.type == TK_EQUAL) {
				Variant v;
				err = _parse_value(v, token, p_str, index, p_len, line, p_depth, r_err_str);
				if (err) {
					return err;
				}
				if (token.type == TK_COMMA) {
					r_err_str = "Unexpected ','";
					return ERR_PARSE_ERROR;
				}
				dict[key] = v;
			} else if (token.type == TK_COLON) {
				Token l_token;
				int l_idx = index;
				int l_line = line;

				err = _get_token(p_str, l_idx, p_len, l_token, l_line, r_err_str);
				if (err) {
					return err;
				}

				if (l_token.type == TK_COLON) {
					err = _get_token(p_str, l_idx, p_len, l_token, l_line, r_err_str);
					if (err) {
						return err;
					}
					if (l_token.type != TK_NEWLINE) {
						r_err_str = "Expected newline.";
						return ERR_PARSE_ERROR;
					}
					
					Array a;
					Error err = _parse_records(a, p_str, index, p_len, line, p_depth + 1, r_err_str);
					if (err) {
						return err;
					}
					value = a;
				} else if (l_token.type == TK_NEWLINE) {
					
					Array a;
					Error err = _parse_array(a, false, p_str, index, p_len, line, p_depth + 1, r_err_str);
					if (err) {
						return err;
					}
					value = a;
				} else {
					Array a;
					Error err = _parse_array(a, true, p_str, index, p_len, line, p_depth + 1, r_err_str);
					if (err) {
						return err;
					}
					value = a;
				}
			} else {
				r_err_str = "Expected '=' or ':' after identifier.";
				return ERR_PARSE_ERROR;
			}
		} else {
			r_err_str = "Expected identifier.";
			return ERR_PARSE_ERROR;
		}

		if (indent - p_indent > 1) {
			r_err_str = "Inconsistent indentation.";
			return ERR_PARSE_ERROR;
		}

		if (at_key) {
			Error err = _get_token(p_str, index, p_len, token, line, r_err_str);
			if (err != OK) {
				return err;
			}

			if (token.type == TK_CURLY_BRACKET_CLOSE) {
				return OK;
			}

			if (need_comma) {
				if (token.type != TK_COMMA) {
					r_err_str = "Expected '}' or ','";
					return ERR_PARSE_ERROR;
				} else {
					need_comma = false;
					continue;
				}
			}

			if (token.type != TK_STRING) {
				r_err_str = "Expected key";
				return ERR_PARSE_ERROR;
			}

			key = token.value;
			err = _get_token(p_str, index, p_len, token, line, r_err_str);
			if (err != OK) {
				return err;
			}
			if (token.type != TK_COLON) {
				r_err_str = "Expected ':'";
				return ERR_PARSE_ERROR;
			}
			at_key = false;
		} else {
			Error err = _get_token(p_str, index, p_len, token, line, r_err_str);
			if (err != OK) {
				return err;
			}

			Variant v;
			err = _parse_value(v, token, p_str, index, p_len, line, p_depth, r_err_str);
			if (err) {
				return err;
			}
			object[key] = v;
			need_comma = true;
			at_key = true;
		}
	}

	r_err_str = "Expected '}'";
	return ERR_PARSE_ERROR;
}

void GDML::set_data(const Variant &p_data) {
	data = p_data;
	text.clear();
}

Error GDML::_parse_string(const String &p_gdml, Variant &r_ret, String &r_err_str, int &r_err_line) {
	const char32_t *str = p_gdml.ptr();
	int idx = 0;
	int len = p_gdml.length();
	int indent = 0;
	int new_indent = 0;
	Token token;
	r_err_line = 0;

	err = _get_token(str, idx, len, token, r_err_line, r_err_str);
	if (err) {
		return err;
	}

	err = _parse_value(r_ret, token, str, idx, len, r_err_line, 0, r_err_str);
	err = _parse_dictionary(r_ret, token, str, idx, len, r_err_line, 0, r_err_str);

	// Check if EOF is reached
	// or it's a type of the next token.
	if (err == OK && idx < len) {
		err = _get_token(str, idx, len, token, r_err_line, r_err_str);

		if (err || token.type != TK_EOF) {
			r_err_str = "Expected 'EOF'";
			// Reset return value to empty `Variant`
			r_ret = Variant();
			return ERR_PARSE_ERROR;
		}
	}

	return err;
}

Error GDML::parse(const String &p_gdml_string, bool p_keep_text) {
	Error err = _parse_string(p_gdml_string, data, err_str, err_line);
	if (err == Error::OK) {
		err_line = 0;
	}
	if (p_keep_text) {
		text = p_gdml_string;
	}
	return err;
}

String GDML::get_parsed_text() const {
	return text;
}

String GDML::stringify(const Variant &p_var, const String &p_indent, bool p_sort_keys, bool p_full_precision) {
	Ref<GDML> jason;
	jason.instantiate();
	HashSet<const void *> markers;
	return jason->_stringify(p_var, p_indent, 0, p_sort_keys, markers, p_full_precision);
}

Variant GDML::parse_string(const String &p_gdml_string) {
	Ref<GDML> jason;
	jason.instantiate();
	Error error = jason->parse(p_gdml_string);
	ERR_FAIL_COND_V_MSG(error != Error::OK, Variant(), vformat("Parse GDML failed. Error at line %d: %s", jason->get_error_line(), jason->get_error_message()));
	return jason->get_data();
}

void GDML::_bind_methods() {
	ClassDB::bind_static_method("GDML", D_METHOD("stringify", "data", "indent", "sort_keys", "full_precision"), &GDML::stringify, DEFVAL(""), DEFVAL(true), DEFVAL(false));
	ClassDB::bind_static_method("GDML", D_METHOD("parse_string", "gdml_string"), &GDML::parse_string);
	ClassDB::bind_method(D_METHOD("parse", "gdml_text", "keep_text"), &GDML::parse, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("get_data"), &GDML::get_data);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &GDML::set_data);
	ClassDB::bind_method(D_METHOD("get_parsed_text"), &GDML::get_parsed_text);
	ClassDB::bind_method(D_METHOD("get_error_line"), &GDML::get_error_line);
	ClassDB::bind_method(D_METHOD("get_error_message"), &GDML::get_error_message);

	ClassDB::bind_static_method("GDML", D_METHOD("to_native", "gdml", "allow_classes", "allow_scripts"), &GDML::to_native, DEFVAL(false), DEFVAL(false));
	ClassDB::bind_static_method("GDML", D_METHOD("from_native", "variant", "allow_classes", "allow_scripts"), &GDML::from_native, DEFVAL(false), DEFVAL(false));

	ADD_PROPERTY(PropertyInfo(Variant::NIL, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT), "set_data", "get_data"); // Ensures that it can be serialized as binary.
}

#define GDTYPE "__gdtype"
#define VALUES "values"
#define PASS_ARG p_allow_classes, p_allow_scripts

Variant GDML::from_native(const Variant &p_variant, bool p_allow_classes, bool p_allow_scripts) {
	switch (p_variant.get_type()) {
		case Variant::NIL: {
			Dictionary nil;
			nil[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return nil;
		} break;
		case Variant::BOOL: {
			return p_variant;
		} break;
		case Variant::INT: {
			return p_variant;
		} break;
		case Variant::FLOAT: {
			return p_variant;
		} break;
		case Variant::STRING: {
			return p_variant;
		} break;
		case Variant::VECTOR2: {
			Dictionary d;
			Vector2 v = p_variant;
			Array values;
			values.push_back(v.x);
			values.push_back(v.y);
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::VECTOR2I: {
			Dictionary d;
			Vector2i v = p_variant;
			Array values;
			values.push_back(v.x);
			values.push_back(v.y);
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::RECT2: {
			Dictionary d;
			Rect2 r = p_variant;
			d["position"] = from_native(r.position);
			d["size"] = from_native(r.size);
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::RECT2I: {
			Dictionary d;
			Rect2i r = p_variant;
			d["position"] = from_native(r.position);
			d["size"] = from_native(r.size);
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::VECTOR3: {
			Dictionary d;
			Vector3 v = p_variant;
			Array values;
			values.push_back(v.x);
			values.push_back(v.y);
			values.push_back(v.z);
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::VECTOR3I: {
			Dictionary d;
			Vector3i v = p_variant;
			Array values;
			values.push_back(v.x);
			values.push_back(v.y);
			values.push_back(v.z);
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::TRANSFORM2D: {
			Dictionary d;
			Transform2D t = p_variant;
			d["x"] = from_native(t[0]);
			d["y"] = from_native(t[1]);
			d["origin"] = from_native(t[2]);
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::VECTOR4: {
			Dictionary d;
			Vector4 v = p_variant;
			Array values;
			values.push_back(v.x);
			values.push_back(v.y);
			values.push_back(v.z);
			values.push_back(v.w);
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::VECTOR4I: {
			Dictionary d;
			Vector4i v = p_variant;
			Array values;
			values.push_back(v.x);
			values.push_back(v.y);
			values.push_back(v.z);
			values.push_back(v.w);
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PLANE: {
			Dictionary d;
			Plane p = p_variant;
			d["normal"] = from_native(p.normal);
			d["d"] = p.d;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::QUATERNION: {
			Dictionary d;
			Quaternion q = p_variant;
			Array values;
			values.push_back(q.x);
			values.push_back(q.y);
			values.push_back(q.z);
			values.push_back(q.w);
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::AABB: {
			Dictionary d;
			AABB aabb = p_variant;
			d["position"] = from_native(aabb.position);
			d["size"] = from_native(aabb.size);
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::BASIS: {
			Dictionary d;
			Basis t = p_variant;
			d["x"] = from_native(t.get_column(0));
			d["y"] = from_native(t.get_column(1));
			d["z"] = from_native(t.get_column(2));
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::TRANSFORM3D: {
			Dictionary d;
			Transform3D t = p_variant;
			d["basis"] = from_native(t.basis);
			d["origin"] = from_native(t.origin);
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PROJECTION: {
			Dictionary d;
			Projection t = p_variant;
			d["x"] = from_native(t[0]);
			d["y"] = from_native(t[1]);
			d["z"] = from_native(t[2]);
			d["w"] = from_native(t[3]);
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::COLOR: {
			Dictionary d;
			Color c = p_variant;
			Array values;
			values.push_back(c.r);
			values.push_back(c.g);
			values.push_back(c.b);
			values.push_back(c.a);
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::STRING_NAME: {
			Dictionary d;
			d["name"] = String(p_variant);
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::NODE_PATH: {
			Dictionary d;
			d["path"] = String(p_variant);
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::RID: {
			Dictionary d;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::OBJECT: {
			Object *obj = p_variant.get_validated_object();

			if (p_allow_classes && obj) {
				Dictionary d;
				List<PropertyInfo> property_list;
				obj->get_property_list(&property_list);

				d["type"] = obj->get_class();
				Dictionary p;
				for (const PropertyInfo &P : property_list) {
					if (P.usage & PROPERTY_USAGE_STORAGE) {
						if (P.name == "script" && !p_allow_scripts) {
							continue;
						}
						p[P.name] = from_native(obj->get(P.name), PASS_ARG);
					}
				}
				d["properties"] = p;
				d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
				return d;
			} else {
				Dictionary nil;
				nil[GDTYPE] = Variant::get_type_name(p_variant.get_type());
				return nil;
			}
		} break;
		case Variant::CALLABLE:
		case Variant::SIGNAL: {
			Dictionary nil;
			nil[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return nil;
		} break;
		case Variant::DICTIONARY: {
			Dictionary d = p_variant;
			List<Variant> keys;
			d.get_key_list(&keys);
			bool all_strings = true;
			for (const Variant &K : keys) {
				if (K.get_type() != Variant::STRING) {
					all_strings = false;
					break;
				}
			}

			if (all_strings) {
				Dictionary ret_dict;
				for (const Variant &K : keys) {
					ret_dict[K] = from_native(d[K], PASS_ARG);
				}
				return ret_dict;
			} else {
				Dictionary ret;
				Array pairs;
				for (const Variant &K : keys) {
					Dictionary pair;
					pair["key"] = from_native(K, PASS_ARG);
					pair["value"] = from_native(d[K], PASS_ARG);
					pairs.push_back(pair);
				}
				ret["pairs"] = pairs;
				ret[GDTYPE] = Variant::get_type_name(p_variant.get_type());
				return ret;
			}
		} break;
		case Variant::ARRAY: {
			Array arr = p_variant;
			Array ret;
			for (int i = 0; i < arr.size(); i++) {
				ret.push_back(from_native(arr[i], PASS_ARG));
			}
			return ret;
		} break;
		case Variant::PACKED_BYTE_ARRAY: {
			Dictionary d;
			PackedByteArray arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				values.push_back(arr[i]);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PACKED_INT32_ARRAY: {
			Dictionary d;
			PackedInt32Array arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				values.push_back(arr[i]);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;

		} break;
		case Variant::PACKED_INT64_ARRAY: {
			Dictionary d;
			PackedInt64Array arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				values.push_back(arr[i]);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PACKED_FLOAT32_ARRAY: {
			Dictionary d;
			PackedFloat32Array arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				values.push_back(arr[i]);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PACKED_FLOAT64_ARRAY: {
			Dictionary d;
			PackedFloat64Array arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				values.push_back(arr[i]);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PACKED_STRING_ARRAY: {
			Dictionary d;
			PackedStringArray arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				values.push_back(arr[i]);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PACKED_VECTOR2_ARRAY: {
			Dictionary d;
			PackedVector2Array arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				Vector2 v = arr[i];
				values.push_back(v.x);
				values.push_back(v.y);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PACKED_VECTOR3_ARRAY: {
			Dictionary d;
			PackedVector3Array arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				Vector3 v = arr[i];
				values.push_back(v.x);
				values.push_back(v.y);
				values.push_back(v.z);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PACKED_COLOR_ARRAY: {
			Dictionary d;
			PackedColorArray arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				Color v = arr[i];
				values.push_back(v.r);
				values.push_back(v.g);
				values.push_back(v.b);
				values.push_back(v.a);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		case Variant::PACKED_VECTOR4_ARRAY: {
			Dictionary d;
			PackedVector4Array arr = p_variant;
			Array values;
			for (int i = 0; i < arr.size(); i++) {
				Vector4 v = arr[i];
				values.push_back(v.x);
				values.push_back(v.y);
				values.push_back(v.z);
				values.push_back(v.w);
			}
			d[VALUES] = values;
			d[GDTYPE] = Variant::get_type_name(p_variant.get_type());
			return d;
		} break;
		default: {
			ERR_PRINT(vformat("Unhandled conversion from native Variant type '%s' to GDML.", Variant::get_type_name(p_variant.get_type())));
		} break;
	}

	Dictionary nil;
	nil[GDTYPE] = Variant::get_type_name(p_variant.get_type());
	return nil;
}

Variant GDML::to_native(const Variant &p_gdml, bool p_allow_classes, bool p_allow_scripts) {
	switch (p_gdml.get_type()) {
		case Variant::BOOL: {
			return p_gdml;
		} break;
		case Variant::INT: {
			return p_gdml;
		} break;
		case Variant::FLOAT: {
			return p_gdml;
		} break;
		case Variant::STRING: {
			return p_gdml;
		} break;
		case Variant::STRING_NAME: {
			return p_gdml;
		} break;
		case Variant::CALLABLE: {
			return p_gdml;
		} break;
		case Variant::DICTIONARY: {
			Dictionary d = p_gdml;
			if (d.has(GDTYPE)) {
				// Specific Godot Variant types serialized to GDML.
				String type = d[GDTYPE];
				if (type == Variant::get_type_name(Variant::VECTOR2)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() != 2, Variant());
					Vector2 v;
					v.x = values[0];
					v.y = values[1];
					return v;
				} else if (type == Variant::get_type_name(Variant::VECTOR2I)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() != 2, Variant());
					Vector2i v;
					v.x = values[0];
					v.y = values[1];
					return v;
				} else if (type == Variant::get_type_name(Variant::RECT2)) {
					ERR_FAIL_COND_V(!d.has("position"), Variant());
					ERR_FAIL_COND_V(!d.has("size"), Variant());
					Rect2 r;
					r.position = to_native(d["position"]);
					r.size = to_native(d["size"]);
					return r;
				} else if (type == Variant::get_type_name(Variant::RECT2I)) {
					ERR_FAIL_COND_V(!d.has("position"), Variant());
					ERR_FAIL_COND_V(!d.has("size"), Variant());
					Rect2i r;
					r.position = to_native(d["position"]);
					r.size = to_native(d["size"]);
					return r;
				} else if (type == Variant::get_type_name(Variant::VECTOR3)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() != 3, Variant());
					Vector3 v;
					v.x = values[0];
					v.y = values[1];
					v.z = values[2];
					return v;
				} else if (type == Variant::get_type_name(Variant::VECTOR3I)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() != 3, Variant());
					Vector3i v;
					v.x = values[0];
					v.y = values[1];
					v.z = values[2];
					return v;
				} else if (type == Variant::get_type_name(Variant::TRANSFORM2D)) {
					ERR_FAIL_COND_V(!d.has("x"), Variant());
					ERR_FAIL_COND_V(!d.has("y"), Variant());
					ERR_FAIL_COND_V(!d.has("origin"), Variant());
					Transform2D t;
					t[0] = to_native(d["x"]);
					t[1] = to_native(d["y"]);
					t[2] = to_native(d["origin"]);
					return t;
				} else if (type == Variant::get_type_name(Variant::VECTOR4)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() != 4, Variant());
					Vector4 v;
					v.x = values[0];
					v.y = values[1];
					v.z = values[2];
					v.w = values[3];
					return v;
				} else if (type == Variant::get_type_name(Variant::VECTOR4I)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() != 4, Variant());
					Vector4i v;
					v.x = values[0];
					v.y = values[1];
					v.z = values[2];
					v.w = values[3];
					return v;
				} else if (type == Variant::get_type_name(Variant::PLANE)) {
					ERR_FAIL_COND_V(!d.has("normal"), Variant());
					ERR_FAIL_COND_V(!d.has("d"), Variant());
					Plane p;
					p.normal = to_native(d["normal"]);
					p.d = d["d"];
					return p;
				} else if (type == Variant::get_type_name(Variant::QUATERNION)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() != 4, Variant());
					Quaternion v;
					v.x = values[0];
					v.y = values[1];
					v.z = values[2];
					v.w = values[3];
					return v;
				} else if (type == Variant::get_type_name(Variant::AABB)) {
					ERR_FAIL_COND_V(!d.has("position"), Variant());
					ERR_FAIL_COND_V(!d.has("size"), Variant());
					AABB r;
					r.position = to_native(d["position"]);
					r.size = to_native(d["size"]);
					return r;
				} else if (type == Variant::get_type_name(Variant::BASIS)) {
					ERR_FAIL_COND_V(!d.has("x"), Variant());
					ERR_FAIL_COND_V(!d.has("y"), Variant());
					ERR_FAIL_COND_V(!d.has("z"), Variant());
					Basis b;
					b.set_column(0, to_native(d["x"]));
					b.set_column(1, to_native(d["y"]));
					b.set_column(2, to_native(d["z"]));
					return b;
				} else if (type == Variant::get_type_name(Variant::TRANSFORM3D)) {
					ERR_FAIL_COND_V(!d.has("basis"), Variant());
					ERR_FAIL_COND_V(!d.has("origin"), Variant());
					Transform3D t;
					t.basis = to_native(d["basis"]);
					t.origin = to_native(d["origin"]);
					return t;
				} else if (type == Variant::get_type_name(Variant::PROJECTION)) {
					ERR_FAIL_COND_V(!d.has("x"), Variant());
					ERR_FAIL_COND_V(!d.has("y"), Variant());
					ERR_FAIL_COND_V(!d.has("z"), Variant());
					ERR_FAIL_COND_V(!d.has("w"), Variant());
					Projection p;
					p[0] = to_native(d["x"]);
					p[1] = to_native(d["y"]);
					p[2] = to_native(d["z"]);
					p[3] = to_native(d["w"]);
					return p;
				} else if (type == Variant::get_type_name(Variant::COLOR)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() != 4, Variant());
					Color c;
					c.r = values[0];
					c.g = values[1];
					c.b = values[2];
					c.a = values[3];
					return c;
				} else if (type == Variant::get_type_name(Variant::NODE_PATH)) {
					ERR_FAIL_COND_V(!d.has("path"), Variant());
					NodePath np = d["path"];
					return np;
				} else if (type == Variant::get_type_name(Variant::STRING_NAME)) {
					ERR_FAIL_COND_V(!d.has("name"), Variant());
					StringName s = d["name"];
					return s;
				} else if (type == Variant::get_type_name(Variant::OBJECT)) {
					ERR_FAIL_COND_V(!d.has("type"), Variant());
					ERR_FAIL_COND_V(!d.has("properties"), Variant());

					ERR_FAIL_COND_V(!p_allow_classes, Variant());

					String obj_type = d["type"];
					bool is_script = obj_type == "Script" || ClassDB::is_parent_class(obj_type, "Script");
					ERR_FAIL_COND_V(!p_allow_scripts && is_script, Variant());
					Object *obj = ClassDB::instantiate(obj_type);
					ERR_FAIL_NULL_V(obj, Variant());

					Dictionary p = d["properties"];

					List<Variant> keys;
					p.get_key_list(&keys);

					for (const Variant &K : keys) {
						String property = K;
						Variant value = to_native(p[K], PASS_ARG);
						obj->set(property, value);
					}

					Variant v(obj);

					return v;
				} else if (type == Variant::get_type_name(Variant::DICTIONARY)) {
					ERR_FAIL_COND_V(!d.has("pairs"), Variant());
					Array pairs = d["pairs"];
					Dictionary r;
					for (int i = 0; i < pairs.size(); i++) {
						Dictionary p = pairs[i];
						ERR_CONTINUE(!p.has("key"));
						ERR_CONTINUE(!p.has("value"));
						r[to_native(p["key"], PASS_ARG)] = to_native(p["value"]);
					}
					return r;
				} else if (type == Variant::get_type_name(Variant::ARRAY)) {
					ERR_PRINT(vformat("Unexpected Array with '%s' key. Arrays are supported natively.", GDTYPE));
				} else if (type == Variant::get_type_name(Variant::PACKED_BYTE_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					PackedByteArray pbarr;
					pbarr.resize(values.size());
					for (int i = 0; i < pbarr.size(); i++) {
						pbarr.write[i] = values[i];
					}
					return pbarr;
				} else if (type == Variant::get_type_name(Variant::PACKED_INT32_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					PackedInt32Array arr;
					arr.resize(values.size());
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = values[i];
					}
					return arr;
				} else if (type == Variant::get_type_name(Variant::PACKED_INT64_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					PackedInt64Array arr;
					arr.resize(values.size());
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = values[i];
					}
					return arr;
				} else if (type == Variant::get_type_name(Variant::PACKED_FLOAT32_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					PackedFloat32Array arr;
					arr.resize(values.size());
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = values[i];
					}
					return arr;
				} else if (type == Variant::get_type_name(Variant::PACKED_FLOAT64_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					PackedFloat64Array arr;
					arr.resize(values.size());
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = values[i];
					}
					return arr;
				} else if (type == Variant::get_type_name(Variant::PACKED_STRING_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					PackedStringArray arr;
					arr.resize(values.size());
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = values[i];
					}
					return arr;
				} else if (type == Variant::get_type_name(Variant::PACKED_VECTOR2_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() % 2 != 0, Variant());
					PackedVector2Array arr;
					arr.resize(values.size() / 2);
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = Vector2(values[i * 2 + 0], values[i * 2 + 1]);
					}
					return arr;
				} else if (type == Variant::get_type_name(Variant::PACKED_VECTOR3_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() % 3 != 0, Variant());
					PackedVector3Array arr;
					arr.resize(values.size() / 3);
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = Vector3(values[i * 3 + 0], values[i * 3 + 1], values[i * 3 + 2]);
					}
					return arr;
				} else if (type == Variant::get_type_name(Variant::PACKED_COLOR_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() % 4 != 0, Variant());
					PackedColorArray arr;
					arr.resize(values.size() / 4);
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = Color(values[i * 4 + 0], values[i * 4 + 1], values[i * 4 + 2], values[i * 4 + 3]);
					}
					return arr;
				} else if (type == Variant::get_type_name(Variant::PACKED_VECTOR4_ARRAY)) {
					ERR_FAIL_COND_V(!d.has(VALUES), Variant());
					Array values = d[VALUES];
					ERR_FAIL_COND_V(values.size() % 4 != 0, Variant());
					PackedVector4Array arr;
					arr.resize(values.size() / 4);
					for (int i = 0; i < arr.size(); i++) {
						arr.write[i] = Vector4(values[i * 4 + 0], values[i * 4 + 1], values[i * 4 + 2], values[i * 4 + 3]);
					}
					return arr;
				} else {
					return Variant();
				}
			} else {
				// Regular dictionary with string keys.
				List<Variant> keys;
				d.get_key_list(&keys);
				Dictionary r;
				for (const Variant &K : keys) {
					r[K] = to_native(d[K], PASS_ARG);
				}
				return r;
			}
		} break;
		case Variant::ARRAY: {
			Array arr = p_gdml;
			Array ret;
			ret.resize(arr.size());
			for (int i = 0; i < arr.size(); i++) {
				ret[i] = to_native(arr[i], PASS_ARG);
			}
			return ret;
		} break;
		default: {
			ERR_PRINT(vformat("Unhandled conversion from GDML type '%s' to native Variant type.", Variant::get_type_name(p_gdml.get_type())));
			return Variant();
		}
	}

	return Variant();
}

#undef GDTYPE
#undef VALUES
#undef PASS_ARG

////////////

Ref<Resource> ResourceFormatLoaderGDML::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	if (!FileAccess::exists(p_path)) {
		*r_error = ERR_FILE_NOT_FOUND;
		return Ref<Resource>();
	}

	Ref<GDML> gdml;
	gdml.instantiate();

	Error err = gdml->parse(FileAccess::get_file_as_string(p_path), Engine::get_singleton()->is_editor_hint());
	if (err != OK) {
		String err_text = "Error parsing GDML file at '" + p_path + "', on line " + itos(gdml->get_error_line()) + ": " + gdml->get_error_message();

		if (Engine::get_singleton()->is_editor_hint()) {
			// If running on editor, still allow opening the GDML so the code editor can edit it.
			WARN_PRINT(err_text);
		} else {
			if (r_error) {
				*r_error = err;
			}
			ERR_PRINT(err_text);
			return Ref<Resource>();
		}
	}

	if (r_error) {
		*r_error = OK;
	}

	return gdml;
}

void ResourceFormatLoaderGDML::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("gdml");
}

bool ResourceFormatLoaderGDML::handles_type(const String &p_type) const {
	return (p_type == "GDML");
}

String ResourceFormatLoaderGDML::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "gdml") {
		return "GDML";
	}
	return "";
}

Error ResourceFormatSaverGDML::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	Ref<GDML> gdml = p_resource;
	ERR_FAIL_COND_V(gdml.is_null(), ERR_INVALID_PARAMETER);

	String source = gdml->get_parsed_text().is_empty() ? GDML::stringify(gdml->get_data(), "\t", false, true) : gdml->get_parsed_text();

	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);

	ERR_FAIL_COND_V_MSG(err, err, vformat("Cannot save gdml '%s'.", p_path));

	file->store_string(source);
	if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF) {
		return ERR_CANT_CREATE;
	}

	return OK;
}

void ResourceFormatSaverGDML::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const {
	Ref<GDML> gdml = p_resource;
	if (gdml.is_valid()) {
		p_extensions->push_back("gdml");
	}
}

bool ResourceFormatSaverGDML::recognize(const Ref<Resource> &p_resource) const {
	return p_resource->get_class_name() == "GDML"; //only gdml, not inherited
}
