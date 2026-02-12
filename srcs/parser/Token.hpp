#ifndef TOKEN_HPP
#define TOKEN_HPP

class Token {
protected:
	const unsigned char* const	_isDelimiter;

	unsigned char*	_start;
	int				_len;
	int				_lineN;

private:
	// Explicit disables
	Token(const Token& other);
	Token& operator=(const Token& other);

public:
	enum e_delimiters{
		WORD,
		SPACE,
		ENDLINE,
		QUOTE,
		SLASH,
		OPENBLOCK,
		CLOSEBLOCK,
		COMMENT
	};

	enum {
		IS,
		IS_NOT
	};
	// Constructors and destructors
	Token(const unsigned char* table);
	~Token();

	// Operators overload

	// Getters and setters

	// Methods
	void printToken();
	inline bool is(char condition, unsigned char c);
	static const unsigned char* configDelimiters();
	unsigned char* next(unsigned char *str);

};

#endif

